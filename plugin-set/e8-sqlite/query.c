#include "query.h"
#include <stdio.h>

#define _QUERY(name) e8_sqlite_query *name = (e8_sqlite_query *)obj

e8_vtable vmt_sqlite_query;

static E8_DECLARE_SUB(e8_sqlite_query_set_text)
{
    _QUERY(q);

    if (q->text)
        e8_utf_free(q->text);

    E8_ARG_STRING(s_text);
    q->text = e8_utf_strdup(s_text->s);
    e8_string_destroy(s_text);

    e8_var_object(result, q);

    E8_RETURN_OK;
}

static E8_DECLARE_SUB(e8_sqlite_query_get_text)
{
    _QUERY(q);

    if (q->text) {
        e8_var_string(result, q->text);
    } else
        e8_var_string_utf(result, "");

    E8_RETURN_OK;
}

static E8_DECLARE_SUB(e8_sqlite_query_set_parameter)
{
    _QUERY(q);

    e8_var_object(result, q);

    E8_ARG_LONG(index);
    E8_ARGUMENT(v_value);

    if (index > 0 && index <= MAX_PARAMS) {
        e8_var_assign(&q->params[index - 1], &v_value);
        e8_var_destroy(&v_value);
        E8_RETURN_OK;
    } else
        E8_THROW_DETAIL(E8_RT_INDEX_OUT_OF_BOUNDS, strdup("Param index out of bounds!"));
}

static E8_DECLARE_SUB(e8_sqlite_query_execute)
{
    _QUERY(q);
    e8_var_object(result, q);

    /*
    if (!q->stmt)
        E8_THROW_DETAIL(E8_RT_EXCEPTION_RAISED, "Query closed!");
    */
    char *text = NULL;
    e8_utf_to_8bit(q->text, &text, "utf-8");
    int r;
    r = sqlite3_prepare(q->conn->db, text, -1, &q->stmt, NULL);
    if (r != SQLITE_OK) {
        const char *s = sqlite3_errmsg(q->conn->db);
        E8_THROW_DETAIL(E8_RT_EXCEPTION_RAISED, strdup(s));
    }

    int i;
    i = 0;
    while (q->params[i].type != varUndefined) {
        e8_var *v = &q->params[i];
        ++i;
        switch (v->type) {
            case varNumeric : {

                switch (v->num.type) {

                case numFloat:
                    sqlite3_bind_double(q->stmt, i, v->num.d_value);
                    break;

                case numInt:
                default:
                    sqlite3_bind_int(q->stmt, i, v->num.l_value);
                    break;
                } /* switch (.num.type) */

                break;
            }
            case varString : {

                char *utf = 0;
                e8_utf_to_8bit(v->str->s, &utf, "utf-8");
                sqlite3_bind_text(q->stmt, i, utf, -1, free);

                break;
            }
            default:
                break;
        } /* switch (.type) */

    }

    r = sqlite3_step(q->stmt);
    if (r != SQLITE_ROW && r != SQLITE_DONE) {
        const char *s = sqlite3_errmsg(q->conn->db);
        E8_THROW_DETAIL(E8_RT_EXCEPTION_RAISED, strdup(s));
    }
    q->done = r == SQLITE_DONE;
    q->prestep = 1;

    E8_RETURN_OK;
}

static E8_DECLARE_SUB(e8_sqlite_query_next)
{
    _QUERY(q);
    if (q->done) {
        e8_var_bool(result, false);
        E8_RETURN_OK;
    }
    if (q->prestep) {
        --q->prestep;
        e8_var_bool(result, true);
        E8_RETURN_OK;
    }
    int r = sqlite3_step(q->stmt);

    if (r == SQLITE_ROW) {
        e8_var_bool(result, true);
        E8_RETURN_OK;
    }

    if (r == SQLITE_DONE) {
        e8_var_bool(result, false);
        E8_RETURN_OK;
    }

    const char *err = sqlite3_errmsg(q->conn->db);
    E8_THROW_DETAIL(E8_RT_EXCEPTION_RAISED, strdup(err));
}

static E8_DECLARE_SUB(e8_sqlite_query_get_value)
{
    _QUERY(q);

    E8_ARG_LONG(index);

    sqlite3_value *v = sqlite3_column_value(q->stmt, index);

    switch (sqlite3_value_type(v)) {
        case SQLITE_INTEGER: {
            e8_var_long(result, sqlite3_value_int(v));
            break;
        }
        case SQLITE_TEXT: {
            e8_var_string_utf(result, (const char *)sqlite3_value_text(v));
            break;
        }
        case SQLITE_FLOAT: {
            e8_var_double(result, sqlite3_value_double(v));
            break;
        }
        case SQLITE_NULL: {
                e8_var_null(result);
                break;
        }
        default: {
            E8_THROW_DETAIL(E8_RT_EXCEPTION_RAISED, "Wrong sqlite value!");
        }
    }

    E8_RETURN_OK;
}

static E8_DECLARE_DESTRUCTOR(e8_sqlite_query_destroy)
{
    _QUERY(q);
    if (q->stmt) {
        sqlite3_finalize(q->stmt);
        q->stmt = NULL;
    }
    e8_gc_free_object(q->conn);
    q->conn = NULL;
    free(q);
}

static const e8_uchar NAME[] = {'S', 'q', 'l', 'i', 't', 'e', 'Q', 'u', 'e', 'r', 'y', 0};
static E8_DECLARE_TO_STRING(e8_sqlite_query_to_string)
{
    return NAME;
}

static E8_METHOD_LIST_START(e8_sqlite_query_methods)
    E8_METHOD_LIST_ADD("Выполнить", "Execute", e8_sqlite_query_execute)
    E8_METHOD_LIST_ADD("Следующий", "Next", e8_sqlite_query_next)
    E8_METHOD_LIST_ADD("УстановитьТекстЗапроса", "SetQueryText", e8_sqlite_query_set_text)
    E8_METHOD_LIST_ADD("ПолучитьТекстЗапроса", "GetQueryText", e8_sqlite_query_get_text)
    E8_METHOD_LIST_ADD("ПолучитьЗначение", "GetValue", e8_sqlite_query_get_value)
    E8_METHOD_LIST_ADD("УстановитьПараметр", "SetParam", e8_sqlite_query_set_parameter)
E8_METHOD_LIST_END

static E8_DECLARE_GET_METHOD(e8_sqlite_query_get_method)
{
    e8_simple_get_method(name, e8_sqlite_query_methods, fn);
    E8_RETURN_OK;
}

void __init_query_vmt()
{
    e8_prepare_call_elements(e8_sqlite_query_methods, "utf-8");

    e8_vtable_template(&vmt_sqlite_query);
    vmt_sqlite_query.dtor = e8_sqlite_query_destroy;
    vmt_sqlite_query.to_string = e8_sqlite_query_to_string;
    vmt_sqlite_query.get_method = e8_sqlite_query_get_method;
}
