#include "connection.h"
#include <malloc.h>
#include "query.h"

e8_vtable vmt_sqlite_connection;
static bool __init = false;

#define _CONN(name) e8_sqlite_connection *name = (e8_sqlite_connection *)obj

static E8_DECLARE_DESTRUCTOR(e8_sqlite_connection_destroy)
{
    _CONN(c);
    if (c->db)
        sqlite3_close(c->db);
    c->db = NULL;
    free(c);
}

static const e8_uchar NAME[] = {'S', 'q', 'l', 'i', 't', 'e',
        'C', 'o', 'n', 'n', 'e', 'c', 't', 'i', 'o', 'n', 0};
static E8_DECLARE_TO_STRING(e8_sqlite_connection_to_string)
{
    return NAME;
}


static E8_DECLARE_SUB(e8_sqlite_connection_close)
{
    _CONN(c);

    if (c->db)
        sqlite3_close(c->db);

    c->db = NULL;

    if (result)
        e8_var_object(result, c);

    E8_RETURN_OK;
}

static E8_DECLARE_SUB(e8_sqlite_connection_open)
{
    _CONN(c);

    e8_sqlite_connection_close(obj, user_data, 0, 0, 0);

    if (result)
        e8_var_object(result, c);

    E8_ARG_UTF_STRING(filename);
    int r = sqlite3_open(filename, &c->db);

    if (r == SQLITE_OK)
        E8_RETURN_OK;

    const char *err = sqlite3_errmsg(c->db);
    c->db = NULL;
    E8_THROW_DETAIL(E8_RT_EXCEPTION_RAISED, strdup(err));
}

static E8_DECLARE_SUB(e8_sqlite_connection_create_query)
{
    _CONN(c);

    e8_sqlite_query *q = malloc(sizeof(*q));
    q->conn = c;
    q->stmt = NULL;
    q->text = NULL;

    int i;
    for (i = 0; i < MAX_PARAMS; ++i)
        e8_var_undefined(&q->params[i]);

    if (argc) {
        E8_ARG_STRING(s_text);
        q->text = e8_utf_strdup(s_text->s);
        e8_string_destroy(s_text);
    }

    e8_gc_register_object(q, &vmt_sqlite_query);
    e8_var_object(result, q);

    e8_gc_use_object(c);

    E8_RETURN_OK;
}

static E8_METHOD_LIST_START(e8_sqlite_connection_methods)
    E8_METHOD_LIST_ADD("Открыть", "Open", e8_sqlite_connection_open)
    E8_METHOD_LIST_ADD("Закрыть", "Close", e8_sqlite_connection_close)
    E8_METHOD_LIST_ADD("СоздатьЗапрос", "CreateQuery", e8_sqlite_connection_create_query)
E8_METHOD_LIST_END

static E8_DECLARE_GET_METHOD(e8_sqlite_connection_get_method)
{
    e8_simple_get_method(name, e8_sqlite_connection_methods, fn);
    E8_RETURN_OK;
}

static void
__init_vmt()
{
    e8_prepare_call_elements(e8_sqlite_connection_methods, "utf-8");

    e8_vtable_template(&vmt_sqlite_connection);

    vmt_sqlite_connection.to_string = e8_sqlite_connection_to_string;
    vmt_sqlite_connection.dtor = e8_sqlite_connection_destroy;
    vmt_sqlite_connection.get_method = e8_sqlite_connection_get_method;

    __init_query_vmt();
    __init = true;
}


E8_DECLARE_SUB(e8_sqlite_connection_constructor)
{

    if (!__init)
        __init_vmt();

    e8_sqlite_connection *c = malloc(sizeof(*c));
    c->db = NULL;

    e8_gc_register_object(c, &vmt_sqlite_connection);
    e8_var_object(result, c);

    if (argc)
        return e8_sqlite_connection_open(c, NULL, argc, argv, NULL);

    E8_RETURN_OK;
}

E8_DECLARE_TYPE(type_sqlite_connection)
    E8_SET_TO_STRING    (e8_sqlite_connection_to_string)
    E8_SET_CONSTRUCTOR  (e8_sqlite_connection_constructor)
E8_END_TYPE

