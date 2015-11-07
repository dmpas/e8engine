#include "_valuetable.h"

#include "macro.h"
#include <malloc.h>
#include "_array.h"
#include "_comparevalues.h"
#include "utils.h"

#define _TABLE(name) e8_valuetable *name = (e8_valuetable *)obj
#define _T_ITR(name) table_iterator *name = (table_iterator *)obj

static e8_vtable vmt_valuetable;
static e8_vtable vmt_valuetable_iterator;

static e8_vtable vmt_valuetable_column;
static e8_vtable vmt_valuetable_columns;
static e8_vtable vmt_valuetable_columns_iterator;

static e8_vtable vmt_valuetable_index;
static e8_vtable vmt_valuetable_indexes;
static e8_vtable vmt_valuetable_indexes_iterator;

static e8_vtable vmt_valuetable_row;
static e8_vtable vmt_valuetable_row_iterator;

static bool init = false;

typedef struct __row_iterator {
    E8_ITERATOR(e8_valuetable_rows)
} table_iterator;

typedef struct __column_iterator {
    E8_ITERATOR(e8_valuetable_columns)
} columns_iterator;

typedef struct __values_iterator {
    E8_ITERATOR(e8_valuetable_row)
} row_iterator;

static
E8_DECLARE_DESTRUCTOR(__table_iterator_destroy)
{
    _T_ITR(it);
    e8_gc_free_object(it->ref);
    free(it);
}

static
E8_DECLARE_SUB(__table_iterator_has_next)
{
    _T_ITR(it);
    e8_var_bool(result, e8_iterator_has_next(it));
    E8_RETURN_OK;
}

static
E8_DECLARE_SUB(__table_iterator_next)
{
    _T_ITR(it);

    e8_var_object(result, it->ref->data[it->current]);
    e8_iterator_next(it);

    E8_RETURN_OK;
}
static
E8_METHOD_LIST_START(table_iterator_methods)
    E8_METHOD_LIST_ADD_HAS_NEXT(__table_iterator_has_next)
    E8_METHOD_LIST_ADD_NEXT(__table_iterator_next)
E8_METHOD_LIST_END

static
E8_DECLARE_GET_METHOD(__table_iterator_get_method)
{
    e8_simple_get_method(name, table_iterator_methods, fn);
    E8_RETURN_OK;
}

static
E8_DECLARE_SUB(__table_iterator)
{
    _TABLE(t);
    table_iterator *ri = malloc(sizeof(*ri));

    ri->ref = &t->rows;
    ri->current = 0;

    e8_gc_use_object(ri->ref);

    e8_gc_register_object(ri, &vmt_valuetable_iterator);
    e8_var_object(result, ri);

    E8_RETURN_OK;
}

static
E8_DECLARE_TO_STRING(__table_to_string)
{
    static const e8_uchar NAME[] = {'V', 'a', 'l', 'u', 'e', 'T', 'a', 'b', 'l', 'e', 0};
    return NAME;
}

#define _ROW(name) e8_valuetable_row *name = (e8_valuetable_row *)obj

static void
__row_append_column(e8_valuetable_row *r, e8_valuetable_column *c)
{
    e8_valuetable_row_value *v = malloc(sizeof(*v));

    e8_gc_use_object(c);

    v->column = c;
    e8_var_undefined(&v->value);

    int i = r->size;
    e8_collection_add_size(r, 1);
    r->data[i] = v;
}

e8_valuetable_row *
e8_valuetable_add(e8_valuetable *t)
{
    e8_valuetable_row *r = malloc(sizeof(*r));
    E8_COLLECTION_INIT(*r);

    r->owner = t; /* weak */

    e8_gc_register_object(r, &vmt_valuetable_row);

    int i;
    for (i = 0; i < t->columns.size; ++i)
        __row_append_column(r, t->columns.data[i]);

    return r;
}

static
E8_DECLARE_DESTRUCTOR(__table_row_destroy)
{
    _ROW(r);

    int i;
    for (i = 0; i < r->size; ++i) {

        e8_var_destroy(&r->data[i]->value);
        e8_gc_free_object(r->data[i]->column);

        free(r->data[i]);
    }

    free(r);
}

static
E8_DECLARE_TO_STRING(__table_row_to_string)
{
    static const e8_uchar NAME[] = {'V', 'a', 'l', 'u', 'e', 'T', 'a', 'b', 'l', 'e', 'R', 'o', 'w', 0};
    return NAME;
}

static
E8_DECLARE_GET_PROPERTY(__table_row_get_property)
{
    _ROW(r);

    e8_uchar *l_name = e8_utf_strdup(name);
    e8_utf_lower_case(l_name);

    int i;
    bool found = false;
    for (i = 0; i < r->size; ++i) {
        if (e8_utf_strcmp(r->data[i]->column->s_name.str->s, l_name) == 0) {
            e8_create_variant_property(property, &r->data[i]->value);
            found = true;
            break;
        }
    }

    e8_utf_free(l_name);

    if (!found)
        E8_THROW_DETAIL(E8_RT_PROPERTY_NOT_FOUND, e8_utf_strdup(name));


    E8_RETURN_OK;
}

static e8_valuetable_row_value*
__find_row_value(const e8_valuetable_row *r, const e8_valuetable_column *c)
{
    int i;
    for (i = 0; i < r->size; ++i)
        if (r->data[i]->column == c)
            return r->data[i];
    return 0;
}

static
E8_DECLARE_GET_BY_INDEX(__table_row_by_index)
{
    _ROW(r);

    e8_runtime_error *__err;

    if (index->type == varNumeric) {
        long l_index = e8_numeric_as_long(&index->num);

        if (l_index < 0 || l_index >= r->owner->columns.size)
            E8_THROW(E8_RT_INDEX_OUT_OF_BOUNDS);

        e8_valuetable_row_value *v = __find_row_value(r, r->owner->columns.data[l_index]);
        e8_create_variant_property(property, &v->value);

    } else {

        e8_string *s_index;
        e8_var_cast_string(index, &s_index);

        __err = __table_row_get_property(obj, s_index->s, property);

        e8_string_destroy(s_index);

        return __err;
    }

    E8_RETURN_OK;
}

#define _V_ITR(name) row_iterator *name = (row_iterator *)obj

static
E8_DECLARE_SUB(__table_row_iterator_has_next)
{
    _V_ITR(it);
    e8_var_bool(result, it->current < it->ref->size);
    E8_RETURN_OK;
}

static
E8_DECLARE_SUB(__table_row_iterator_next)
{
    _V_ITR(it);

    e8_var_assign(result, &it->ref->data[it->current]->value);

    ++it->current;

    E8_RETURN_OK;
}


static
E8_METHOD_LIST_START(table_row_iterator_methods)
    E8_METHOD_LIST_ADD_HAS_NEXT(__table_row_iterator_has_next)
    E8_METHOD_LIST_ADD_NEXT(__table_row_iterator_next)
E8_METHOD_LIST_END

static
E8_DECLARE_GET_METHOD(table_row_iterator_get_method)
{
    e8_simple_get_method(name, table_row_iterator_methods, fn);
    E8_RETURN_OK;
}

static
E8_DECLARE_SUB(__table_row_iterator)
{
    _ROW(r);

    row_iterator *ri = malloc(sizeof(*ri));
    ri->ref = r;
    ri->current = 0;

    e8_gc_use_object(r);
    e8_gc_register_object(ri, &vmt_valuetable_row_iterator);

    e8_var_object(result, ri);

    E8_RETURN_OK;

}

static
E8_METHOD_LIST_START(table_row_methods)
    E8_METHOD_LIST_ADD_ITERATOR(__table_row_iterator)
E8_METHOD_LIST_END

static
E8_DECLARE_GET_METHOD(__table_row_get_method)
{
    e8_simple_get_method(name, table_row_methods, fn);
    E8_RETURN_OK;
}

static
e8_valuetable_column *
__find_table_column_by_name(const e8_valuetable_columns *C, const e8_uchar *name)
{
    int i;
    e8_uchar *lname = e8_utf_strdup(name);
    e8_utf_lower_case(lname);

    for (i = 0; i < C->size; ++i) {
        if (e8_utf_strcmp(C->data[i]->s_name.str->s, lname) == 0) {
            e8_utf_free(lname);
            return C->data[i];
        }
    }

    e8_utf_free(lname);
    return 0;
}



static
E8_DECLARE_SUB(__table_add)
{
    _TABLE(t);

    e8_runtime_error *__err;

    int i = t->rows.size;
    if (( __err = e8_collection_add_size(&t->rows, 1) ))
        return __err;

    e8_valuetable_row *r = e8_valuetable_add(t);
    e8_gc_use_object(r);

    t->rows.data[i] = r;

    e8_var_object(result, r);

    E8_RETURN_OK;
}

static
E8_DECLARE_SUB(__table_insert)
{
    _TABLE(t);

    E8_ARG_LONG(l_index);

    if (l_index < 0 || l_index > t->rows.size)
        E8_THROW(E8_RT_INDEX_OUT_OF_BOUNDS);

    e8_collection_insert(t, l_index);
    e8_valuetable_row *r = e8_valuetable_add(t);

    e8_gc_use_object(r);

    t->rows.data[l_index] = r;

    E8_RETURN_OK;
}

static
E8_DECLARE_SUB(__table_fill_values)
{
    _TABLE(t);

    E8_ARGUMENT(value);

    bool all_columns = true;
    E8_VAR(v_columns);

    if (argc > 1) {
        E8_ARG_STRING(s_columns_src);
        e8_string *s_columns = e8_string_trim(s_columns_src, true, true);
        e8_string_destroy(s_columns_src);

        e8_delimit_string(s_columns->s, &v_columns, ",");

        e8_string_destroy(s_columns);

        int sz = e8_array_size(&v_columns);
        if (sz != 0) {
            all_columns = false;

            int i;
            for (i = 0; i < sz; ++i) {
                e8_var el;

                e8_array_get(&v_columns, i, &el);
                e8_valuetable_column *c = __find_table_column_by_name(&t->columns, el.str->s);

                e8_var_destroy(&el);

                if (c == 0)
                    E8_THROW_DETAIL(E8_RT_INDEX_OUT_OF_BOUNDS, e8_usprintf(0, "Wrong column name!"));

                e8_var_object(&el, c);
                e8_array_set(&v_columns, i, &el);

            }
        }
    } /* if argc > 1*/

    int sz = 0;

    if (!all_columns)
        sz = e8_array_size(&v_columns);
    e8_array *A = v_columns.obj;

    int i;
    for (i = 0; i < t->rows.size; ++i) {

        e8_valuetable_row *r = t->rows.data[i];

        int j;
        for (j = 0; j < r->size; ++j) {

            e8_valuetable_row_value *v = r->data[j];

            if (!all_columns) {
                int k;
                bool found = false;
                for (k = 0; k < sz; ++k)
                    if (A->data[k].obj == v->column) {
                        found = true;
                        break;
                    }

                if (!found)
                    continue;
            }

            e8_var_assign(&v->value, &value);

        } // for j=(row)

    } // for i=(rows)

    e8_var_destroy(&value);
    e8_var_destroy(&v_columns);

    e8_var_object(result, t);
    E8_RETURN_OK;
}

static e8_runtime_error *
__table_detect_column(const e8_var *v_column, const e8_valuetable_columns *C, e8_valuetable_column **c)
{
    if (v_column->type == varObject)
        *c = v_column->obj;

    else if (v_column->type == varNumeric) {

        long l_index = e8_numeric_as_long(&v_column->num);
        if (l_index < 0 || l_index >= C->size)
            E8_THROW(E8_RT_INDEX_OUT_OF_BOUNDS);

        *c = C->data[l_index];

    } else if (v_column->type == varString) {
        *c = __find_table_column_by_name(C, v_column->str->s);

        if (*c == 0)
            E8_THROW(E8_RT_EXCEPTION_RAISED);

    } else
        E8_THROW(E8_RT_EXCEPTION_RAISED);

    E8_RETURN_OK;
}

static
E8_DECLARE_SUB(__table_unload_column)
{
    _TABLE(t);

    E8_ARGUMENT(v_column);

    e8_valuetable_column *c = 0;

    e8_runtime_error *__err;

    if (( __err = __table_detect_column(&v_column, &t->columns, &c) ))
        return __err;

    e8_array_new(0, 0, 0, 0, result);

    e8_array *A = result->obj;
    e8_collection_resize(A, t->rows.size);

    int i;
    for (i = 0; i < A->size; ++i) {
        e8_var_undefined(&A->data[i]);

        e8_valuetable_row *r = t->rows.data[i];
        int j;
        for (j = 0; j < r->size; ++j) {
            if (r->data[j]->column == c) {
                e8_var_assign(&A->data[i], &r->data[j]->value);
                break;
            }
        }
    }

    e8_var_destroy(&v_column);

    E8_RETURN_OK;
}

static
E8_DECLARE_SUB(__table_load_column)
{
    _TABLE(t);

    E8_ARGUMENT(v_data);
    E8_ARGUMENT(v_column);

    e8_valuetable_column *c = 0;

    e8_runtime_error *__err;

    if (( __err = __table_detect_column(&v_column, &t->columns, &c) ))
        return __err;

    e8_array *A = v_data.obj;

    int i;
    for (i = 0; i < A->size; ++i) {

        e8_valuetable_row *r = t->rows.data[i];
        int j;
        for (j = 0; j < r->size; ++j) {
            if (r->data[j]->column == c) {
                e8_var_assign(&r->data[j]->value, &A->data[i]);
                break;
            }
        }
    }

    e8_var_destroy(&v_column);
    e8_var_destroy(&v_data);

    E8_RETURN_OK;
}

static
E8_DECLARE_SUB(__table_total)
{
    _TABLE(t);

    E8_ARGUMENT(v_column);

    e8_runtime_error *__err;
    e8_valuetable_column *c = 0;

    if (( __err = __table_detect_column(&v_column, &t->columns, &c) ))
        return __err;

    bool was = false;
    e8_var_long(result, 0);
    int i;

    for (i = 0; i < t->rows.size; ++i) {

        e8_valuetable_row *r = t->rows.data[i];

        int j;
        for (j = 0; j < r->size; ++j)
            if (r->data[j]->column == c) {

                if (r->data[j]->value.type == varNumeric) {
                    e8_var_add(result, &r->data[j]->value);
                    was = true;
                }

                break;
            }
    }

    if (!was)
        e8_var_undefined(result);

    E8_RETURN_OK;
}

struct __column_sort_def {
    const e8_valuetable_column         *c;
    const e8_compare_values            *cv;
          int                           k;
};

static int
__compare_rows(const struct __column_sort_def *def, int nsize,
                const e8_valuetable_row *a, const e8_valuetable_row *b)
{
    int i;
    for (i = 0; i < nsize; ++i) {
        const e8_valuetable_column *c = def[i].c;

        int j, k;
        for (j = 0; j < a->size; ++j) {
            if (a->data[j]->column == c) {

                for (k = 0; k < b->size; ++k) {
                    if (b->data[k]->column == c) {

                        int r = e8_compare_values_compare(def[i].cv, &a->data[j]->value, &b->data[k]->value);

                        if (r != 0)
                            return r*def[i].k;

                        break;
                    }
                } // for k
                break;
            }
        } // for j
    } // for i

    return 0;
}

static
E8_DECLARE_SUB(__table_sort)
{

    _TABLE(t);

    E8_ARG_STRING(s_columns);

    e8_compare_values _cv;
    _cv.invert = false;
    _cv.built_in = false;

    e8_compare_values *cv = &_cv;

    if (argc > 1) {

        E8_ARGUMENT(v_cv);

        cv = v_cv.obj;
        e8_gc_use_object(cv);

        e8_var_destroy(&v_cv);
    }

    E8_VAR(va_columns);
    e8_delimit_string(s_columns->s, &va_columns, ",");

    e8_array *A = (e8_array *)va_columns.obj;

    struct __column_sort_def *def = malloc(A->size * sizeof(*def));

    int N = A->size;
    int i;
    for (i = 0; i < N; ++i) {

        e8_string *s_element;
        e8_var_cast_string(&A->data[i], &s_element);

        static e8_uchar __spaces[] = {' ', 0};
        static const e8_string s_spaces = {__spaces, 1, 0};

        def[i].cv = cv;
        def[i].k = 1;

        e8_string *s_trimmed = e8_string_trim(s_element, true, true);
        e8_string_destroy(s_element);

        int index = e8_string_find(s_trimmed, &s_spaces, 0);

        e8_uchar *u_element = e8_utf_strdup(s_trimmed->s);

        e8_string_destroy(s_trimmed);

        if (index != -1) {

            u_element[index] = 0;
            e8_uchar *ad = &u_element[index + 1];
            ad = e8_utf_trim(ad, true, true);
            e8_utf_lower_case(ad);

            static const e8_uchar DESC[] = {'d', 'e', 's', 'c', 0};
            static const e8_uchar UBIV[] = {0x0443, 0x0431, 0x044B, 0x0432, 0};

            #ifdef DEBUG
            /* Unused */
            static const e8_uchar VOZR[] = {0x0432, 0x043E, 0x0437, 0x0440, 0};
            static const e8_uchar ASC[]  = {'a', 's', 'c', 0};
            #endif

            if ((e8_utf_strcmp(ad, DESC) == 0) || e8_utf_strcmp(ad, UBIV) == 0)
                def[i].k = -1;
        }

        def[i].c = __find_table_column_by_name(&t->columns, u_element);

    }
    e8_var_destroy(&va_columns);

    int j;

    /* TODO: Толковая сортировка */
    for (i = 0; i < t->rows.size - 1; ++i) {
        for (j = i + 1; j < t->rows.size; ++j) {

            int r = __compare_rows(def, N, t->rows.data[i], t->rows.data[j]);

            if (r > 0) {
                e8_valuetable_row *tmp = t->rows.data[i];
                t->rows.data[i] = t->rows.data[j];
                t->rows.data[j] = tmp;
            }
        } // for j
    } // for i

    free(def);

    if (cv != &_cv)
        e8_gc_free_object(cv);

    e8_var_object(result, t);

    E8_RETURN_OK;
}

static
E8_METHOD_LIST_START(table_methods)
    E8_METHOD_LIST_ADD("Добавить", "Add", __table_add)
    E8_METHOD_LIST_ADD("Вставить", "Insert", __table_insert)
    E8_METHOD_LIST_ADD("ЗаполнитьЗначения", "FillValues", __table_fill_values)
    E8_METHOD_LIST_ADD("ВыгрузитьКолонку", "UnloadColumn", __table_unload_column)
    E8_METHOD_LIST_ADD("ЗагрузитьКолонку", "LoadColumn", __table_load_column)
    E8_METHOD_LIST_ADD("Итог", "Total", __table_total)
    E8_METHOD_LIST_ADD("Сортировать", "Sort", __table_sort)
    E8_METHOD_LIST_ADD_ITERATOR(__table_iterator)
E8_METHOD_LIST_END

static
E8_DECLARE_GET_METHOD(__table_get_method)
{
    e8_simple_get_method(name, table_methods, fn);
    E8_RETURN_OK;
}

static
E8_DECLARE_PROPERTY_GET(__table_columns__get)
{
    e8_property *p = (e8_property *)property;
    e8_valuetable *t = (e8_valuetable *)p->data;
    e8_var_object(value, &t->columns);

    E8_RETURN_OK;
}

static
E8_DECLARE_PROPERTY_GET(__table_indexes__get)
{
    e8_property *p = (e8_property *)property;
    e8_valuetable *t = (e8_valuetable *)p->data;
    e8_var_object(value, &t->indexes);

    E8_RETURN_OK;
}

static
E8_DECLARE_GET_PROPERTY(__table_get_property)
{
    e8_uchar *lname = e8_utf_strdup(name);
    e8_utf_lower_case(lname);
    char *uname = 0;
    e8_utf_to_8bit(lname, &uname, "utf-8");
    e8_utf_free(lname);

    property->can_set = false;
    property->data = obj;
    property->can_get = true;

    if ((strcmp(uname, "колонки") == 0) || (strcmp(uname, "columns") == 0)) {
        property->get = __table_columns__get;
    } else

    if ((strcmp(uname, "индексы") == 0) || (strcmp(uname, "indexes") == 0)) {
        property->get = __table_indexes__get;
    } else

        E8_THROW_DETAIL(E8_RT_PROPERTY_NOT_FOUND, e8_utf_strdup(name));

    free(uname);
    E8_RETURN_OK;
}

static
E8_DECLARE_PROPERTY_GET(__table__get)
{
    e8_var_object(value, ((e8_property*)property)->data);
    E8_RETURN_OK;
}

static
E8_DECLARE_GET_BY_INDEX(__table_by_index)
{
    _TABLE(t);

    if (index->type == varNumeric) {

        long l_index = e8_numeric_as_long(&index->num);

        if (l_index < 0 || l_index > t->rows.size)
            E8_THROW(E8_RT_INDEX_OUT_OF_BOUNDS);

        property->can_set = false;
        property->can_get = true;
        property->get = __table__get;
        property->data = t->rows.data[l_index];

        E8_RETURN_OK;
    }

    e8_string *s_name;
    e8_var_cast_string(index, &s_name);

    e8_runtime_error *__err = __table_get_property(obj, s_name->s, property);

    e8_string_destroy(s_name);

    return __err;

}

#define _COLUMNS(name) e8_valuetable_columns *name = (e8_valuetable_columns *)obj
#define _C_ITR(name) columns_iterator *name = (columns_iterator *)obj

static void
__construct_column(e8_valuetable_column *c, int argc, e8_property *argv)
{
    e8_var_undefined(&c->name);
    e8_var_undefined(&c->s_name);
    e8_var_undefined(&c->title);
    e8_var_undefined(&c->value_type);
    e8_var_undefined(&c->width);

    if (argc > 0) {
        E8_ARGUMENT(v_name);
        e8_var_assign(&c->name, &v_name);

        e8_string *s_name;
        e8_var_cast_string(&c->name, &s_name);
        e8_uchar *lname = e8_utf_strdup(s_name->s);
        e8_utf_lower_case(lname);
        e8_var_string(&c->s_name, lname);
        e8_utf_free(lname);
        e8_string_destroy(s_name);
    }

    if (argc > 1) {
        E8_ARGUMENT(v_title);
        e8_var_assign(&c->title, &v_title);
    }

    if (argc > 2) {
        E8_ARGUMENT(v_value_type);
        e8_var_assign(&c->value_type, &v_value_type);
    }

    if (argc > 3) {
        E8_ARGUMENT(v_width);
        e8_var_assign(&c->width, &v_width);
    }
}

#define _COLUMN(name) e8_valuetable_column *name = (e8_valuetable_column *)obj
#define _VAR_GET(var) {\
    e8_valuetable_column *c = (e8_valuetable_column*)((e8_property *)property)->data;\
    e8_var_assign(value, &c-> var); \
    E8_RETURN_OK; \
}
#define _VAR_SET(var) {\
    e8_valuetable_column *c = (e8_valuetable_column*)((e8_property *)property)->data;\
    e8_var_assign(&c-> var, value); \
    E8_RETURN_OK; \
}

static
E8_DECLARE_PROPERTY_GET(__table_column__name_get) _VAR_GET(name)

/* TODO: Проверка повторения имён колонок */
static
E8_DECLARE_PROPERTY_SET(__table_column__name_set)
{
    e8_valuetable_column *c = (e8_valuetable_column*)((e8_property *)property)->data;
    e8_var_assign(&c->name, value);

    e8_string *s_name;
    e8_var_cast_string(&c->name, &s_name);
    e8_uchar *lname = e8_utf_strdup(s_name->s);
    e8_utf_lower_case(lname);
    e8_var_string(&c->s_name, lname);
    e8_string_destroy(s_name);
    e8_utf_free(lname);

    E8_RETURN_OK;
}


static
E8_DECLARE_PROPERTY_GET(__table_column__title_get) _VAR_GET(title)

static
E8_DECLARE_PROPERTY_SET(__table_column__title_set) _VAR_SET(title)


static
E8_DECLARE_PROPERTY_GET(__table_column__width_get) _VAR_GET(width)

static
E8_DECLARE_PROPERTY_SET(__table_column__width_set) _VAR_SET(width)


static
E8_DECLARE_PROPERTY_GET(__table_column__value_type_get) _VAR_GET(value_type)

static
E8_DECLARE_GET_PROPERTY(__table_column_get_property)
{
    e8_uchar *lname = e8_utf_strdup(name);
    e8_utf_lower_case(lname);
    char *uname = 0;
    e8_utf_to_8bit(lname, &uname, "utf-8");
    e8_utf_free(lname);

    property->can_set = true;
    property->can_get = true;
    property->data = obj;

    if ((strcmp(uname, "имя") == 0) || (strcmp(uname, "name") == 0)) {
        property->get = __table_column__name_get;
        property->set = __table_column__name_set;
    } else
    if ((strcmp(uname, "заголовок") == 0) || (strcmp(uname, "title") == 0)) {
        property->get = __table_column__title_get;
        property->set = __table_column__title_set;
    } else
    if ((strcmp(uname, "ширина") == 0) || (strcmp(uname, "width") == 0)) {
        property->get = __table_column__width_get;
        property->set = __table_column__width_set;
    } else
    if ((strcmp(uname, "типзначения") == 0) || (strcmp(uname, "valuetype") == 0)) {
        property->get = __table_column__value_type_get;
        property->can_set = false;
    } else

        E8_THROW_DETAIL(E8_RT_PROPERTY_NOT_FOUND, e8_utf_strdup(name));

    free(uname);
    E8_RETURN_OK;
}

static
E8_DECLARE_SUB(__table_columns_add)
{
    _COLUMNS(C);

    e8_runtime_error *__err;

    int i = C->size;
    if ( (__err = e8_collection_add_size(C, 1)) )
        return __err;

    e8_valuetable_column *c = malloc(sizeof(*c));

    __construct_column(c, argc, argv);

    e8_gc_register_object(c, &vmt_valuetable_column);
    e8_gc_use_object(c);

    C->data[i] = c;

    for (i = 0; i < C->owner->rows.size; ++i) {
        __row_append_column(C->owner->rows.data[i], c);
    }

    e8_var_object(result, C);

    E8_RETURN_OK;
}

static
E8_DECLARE_SUB(__table_columns_get)
{
    _COLUMNS(C);

    E8_ARG_LONG(l_index);
    if (l_index < 0 || l_index > C->size)
        E8_THROW(E8_RT_INDEX_OUT_OF_BOUNDS);

    e8_valuetable_column *c = C->data[l_index];

    e8_var_object(result, c);

    E8_RETURN_OK;
}

static
E8_DECLARE_SUB(__table_columns_insert)
{
    _COLUMNS(C);

    e8_runtime_error *__err;

    E8_ARG_LONG(l_index); --argc;

    if (( __err = e8_collection_insert(C, l_index) ))
        return __err;

    e8_valuetable_column *c = malloc(sizeof(*c));
    __construct_column(c, argc, argv);

    e8_gc_register_object(c, &vmt_valuetable_column);
    e8_gc_use_object(c);

    C->data[l_index] = c;

    e8_var_object(result, C);


    E8_RETURN_OK;
}

static
E8_DECLARE_SUB(__table_columns_index)
{
    _COLUMNS(C);

    E8_ARGUMENT(v_column);

    int l_index = -1;

    e8_valuetable_column *c = v_column.obj;
    int i;
    for (i = 0; i < C->size; ++i)
        if (C->data[i] == c) {
            l_index = i;
            break;
        }

    e8_var_destroy(&v_column);

    e8_var_long(result, l_index);

    E8_RETURN_OK;
}

static
E8_DECLARE_SUB(__table_columns_find)
{
    _COLUMNS(C);

    E8_ARG_STRING(s_name);
    e8_uchar *lname = e8_utf_strdup(s_name->s);
    e8_utf_lower_case(lname);
    e8_string_destroy(s_name);

    e8_var_undefined(result);

    e8_valuetable_column *c = __find_table_column_by_name(C, lname);

    e8_utf_free(lname);

    if (c) {
        e8_var_object(result, c);
        E8_RETURN_OK;
    }

    E8_THROW_DETAIL(E8_RT_INDEX_OUT_OF_BOUNDS, e8_usprintf(0, "Wrong column name!"));
}

static
E8_DECLARE_PROPERTY_GET(__columns__get_column)
{
    e8_var_object(value, ((e8_property *)property)->data);
    E8_RETURN_OK;
}


static
E8_DECLARE_GET_PROPERTY(__table_columns_get_property)
{
    _COLUMNS(C);

    e8_uchar *lname = e8_utf_strdup(name);
    e8_utf_lower_case(lname);

    property->can_set = false;
    property->can_get = true;
    property->get = __columns__get_column;

    int i;
    bool found = false;
    for (i = 0; i < C->size; ++i) {
        if (e8_utf_strcmp(C->data[i]->s_name.str->s, lname) == 0) {
            property->data = C->data[i];
            found = true;
            break;
        }
    }

    e8_utf_free(lname);

    if (!found)
        E8_THROW_DETAIL(E8_RT_PROPERTY_NOT_FOUND, e8_utf_strdup(name));

    E8_RETURN_OK;
}

static
E8_DECLARE_DESTRUCTOR(__table_columns_iterator_destroy)
{
    _C_ITR(it);
    e8_gc_free_object(it->ref);
    free(it);
}

static
E8_DECLARE_SUB(__table_columns_iterator_has_next)
{
    _C_ITR(it);
    e8_var_bool(result, e8_iterator_has_next(it));
    E8_RETURN_OK;
}

static
E8_DECLARE_SUB(__table_columns_iterator_next)
{
    _C_ITR(it);

    e8_var_object(result, it->ref->data[it->current]);
    e8_iterator_move_next(it);

    E8_RETURN_OK;
}

static
E8_METHOD_LIST_START(table_columns_iterator_methods)
    E8_METHOD_LIST_ADD_HAS_NEXT(__table_columns_iterator_has_next)
    E8_METHOD_LIST_ADD_NEXT(__table_columns_iterator_next)
E8_METHOD_LIST_END

static
E8_DECLARE_GET_METHOD(__table_columns_iterator_get_method)
{
    e8_simple_get_method(name, table_columns_iterator_methods, fn);
    E8_RETURN_OK;
}

static
E8_DECLARE_SUB(__table_columns_iterator)
{
    _COLUMNS(C);

    columns_iterator *it;

    E8_ITR_INIT(it, C);

    e8_gc_use_object(it->ref);

    e8_gc_register_object(it, &vmt_valuetable_columns_iterator);
    e8_var_object(result, it);

    E8_RETURN_OK;
}

static
E8_DECLARE_TO_STRING(__table_columns_to_string)
{
    static const e8_uchar NAME[] = {'V', 'a', 'l', 'u', 'e', 'T', 'a', 'b', 'l', 'e',
        'C', 'o', 'l', 'u', 'm', 'n', 's', 0};
    return NAME;
}

static
E8_METHOD_LIST_START(table_columns_methods)
    E8_METHOD_LIST_ADD("Добавить", "Add", __table_columns_add)
    E8_METHOD_LIST_ADD("Получить", "Get", __table_columns_get)
    E8_METHOD_LIST_ADD("Индекс", "Index", __table_columns_index)
    E8_METHOD_LIST_ADD("Найти", "Find", __table_columns_find)
    E8_METHOD_LIST_ADD("Вставить", "Insert", __table_columns_insert)
    E8_METHOD_LIST_ADD_ITERATOR(__table_columns_iterator)
E8_METHOD_LIST_END

static
E8_DECLARE_GET_METHOD(__table_columns_get_method)
{
    e8_simple_get_method(name, table_columns_methods, fn);
    E8_RETURN_OK;
}

static void
__init_columns()
{
    e8_vtable_template(&vmt_valuetable_column);
    e8_vtable_template(&vmt_valuetable_columns);
    e8_vtable_template(&vmt_valuetable_columns_iterator);

    e8_prepare_call_elements(table_columns_methods, "utf-8");
    e8_prepare_call_elements(table_columns_iterator_methods, "utf-8");

    vmt_valuetable_columns.get_method = __table_columns_get_method;
    vmt_valuetable_columns.to_string = __table_columns_to_string;
    vmt_valuetable_columns.get_property = __table_columns_get_property;

    vmt_valuetable_columns_iterator.get_method = __table_columns_iterator_get_method;
    vmt_valuetable_columns_iterator.dtor = __table_columns_iterator_destroy;

    vmt_valuetable_column.get_property = __table_column_get_property;

}

static void
__init_indexes()
{
    e8_vtable_template(&vmt_valuetable_index);
    e8_vtable_template(&vmt_valuetable_indexes);
    e8_vtable_template(&vmt_valuetable_indexes_iterator);
}

static void
__init_core()
{
    e8_vtable_template(&vmt_valuetable);
    e8_vtable_template(&vmt_valuetable_iterator);
    e8_vtable_template(&vmt_valuetable_row);
    e8_vtable_template(&vmt_valuetable_row_iterator);

    e8_prepare_call_elements(table_iterator_methods, "utf-8");
    e8_prepare_call_elements(table_methods, "utf-8");
    e8_prepare_call_elements(table_row_methods, "utf-8");
    e8_prepare_call_elements(table_row_iterator_methods, "utf-8");

    vmt_valuetable.get_method = __table_get_method;
    vmt_valuetable.get_property = __table_get_property;
    vmt_valuetable.to_string = __table_to_string;
    vmt_valuetable.by_index = __table_by_index;

    vmt_valuetable_row.dtor = __table_row_destroy;
    vmt_valuetable_row.get_property = __table_row_get_property;
    vmt_valuetable_row.to_string = __table_row_to_string;
    vmt_valuetable_row.by_index = __table_row_by_index;
    vmt_valuetable_row.get_method = __table_row_get_method;

    vmt_valuetable_iterator.get_method = __table_iterator_get_method;
    vmt_valuetable_iterator.dtor = __table_iterator_destroy;

    vmt_valuetable_row_iterator.get_method = table_row_iterator_get_method;

}

static void
__init_vmt()
{
    __init_core();
    __init_columns();
    __init_indexes();

    init = true;
}

static e8_valuetable *
__e8_valuetable_new()
{
    e8_valuetable *res = malloc(sizeof(*res));

    e8_gc_register_object(res, &vmt_valuetable);
    e8_gc_register_object(&res->columns, &vmt_valuetable_columns);
    e8_gc_register_object(&res->indexes, &vmt_valuetable_indexes);

    e8_gc_use_object(&res->columns);
    e8_gc_use_object(&res->indexes);

    E8_COLLECTION_INIT(res->rows);
    E8_COLLECTION_INIT(res->columns);
    E8_COLLECTION_INIT(res->indexes);

    res->columns.owner = res;

    return res;
}


E8_DECLARE_SUB(e8_valuetable_new)
{

    if (!init)
        __init_vmt();

    e8_valuetable *res = __e8_valuetable_new();
    e8_var_object(result, res);

    E8_RETURN_OK;
}

#undef _VAR_GET
#undef _VAR_SET
#undef _TABLE
#undef _T_ITR
#undef _C_ITR
#undef _COLUMNS
#undef _COLUMN
