#include "_valuelist.h"
#include "macro.h"
#include <malloc.h>
#include "_array.h"

#define _LIST(name) e8_valuelist *name = (e8_valuelist *)obj

static e8_vtable vmt_valuelist;
static e8_vtable vmt_valuelist_iterator;
static bool init = false;


#define _ITERATOR(name) e8_valuelist_iterator *name = (e8_valuelist_iterator *)obj;


staticE8_DECLARE_SUB(e8_valuelist_iterator_has_next)
{
    _ITERATOR(it);

    e8_var_bool(result, it->current < it->list->size);

    E8_RETURN_OK;
}

staticE8_DECLARE_SUB(e8_valuelist_iterator_next)
{
    _ITERATOR(it);

    if (it->current < it->list->size) {
        e8_var_object(result, it->list->data[it->current++]);
        E8_RETURN_OK;
    }

    E8_THROW(E8_RT_INDEX_OUT_OF_BOUNDS);
}

static void
__e8_valuelist_iterator_destroy(e8_object obj)
{
    _ITERATOR(it);
    e8_gc_free_object(it->list);

    free(it);
}

static
E8_METHOD_LIST_START(iterator_methods)
    E8_METHOD_LIST_ADD("__ЕстьСледующий",   "__HasNext",    e8_valuelist_iterator_has_next)
    E8_METHOD_LIST_ADD("__Следующий",       "__Next",       e8_valuelist_iterator_next)
E8_METHOD_LIST_END

staticE8_DECLARE_GET_METHOD(__e8_valuelist_iterator_get_method)
{
    e8_simple_get_method(name, iterator_methods, fn);
    E8_RETURN_OK;
}


staticE8_DECLARE_SUB(e8_valuelist_iterator_new)
{
    _LIST(m);

    e8_valuelist_iterator *it = AALLOC(e8_valuelist_iterator, 1);
    e8_gc_register_object(it, &vmt_valuelist_iterator);

    it->current = 0;
    it->list = m;
    e8_gc_use_object(m);

    e8_var_object(result, it);

    E8_RETURN_OK;
}


static e8_valuelist *
__e8_valuelist_new()
{
    e8_valuelist *res = AALLOC(e8_valuelist, 1);
    E8_COLLECTION_INIT(*res);
    res->max_id = 0;

    e8_gc_register_object(res, &vmt_valuelist);

    return res;
}


int __e8_valuelist_index(const e8_valuelist *m, const e8_valuelist_item *item)
{
    int i;
    for (i = 0; i < m->size; ++i) {

        if (m->data[i] == item)
            return i;

    }
    return -1;
}

static e8_runtime_error *
__e8_valuelist_add_item(e8_valuelist *m, long l_index, int argc, e8_property *argv)
{

    e8_runtime_error *__err;

    if (( __err = e8_collection_add_size(m, 1) ))
        return __err;

    int i = m->size;
    while (--i > l_index)
        m->data[i] = m->data[i - 1];

    E8_ARGUMENT(v_value);
    --argc;

    e8_valuelist_item *it = e8_valuelist_item_new_with_value(&v_value, 0, 0, 0);
    e8_var_destroy(&v_value);

    if (argc) {
        E8_ARGUMENT(v_presentation);
        --argc;
        e8_var_assign(&it->presentation, &v_presentation);
        e8_var_destroy(&v_presentation);
    }

    if (argc) {
        E8_ARGUMENT(v_mark);
        --argc;
        e8_var_assign(&it->mark, &v_mark);
        e8_var_destroy(&v_mark);
    }

    if (argc) {
        E8_ARGUMENT(v_picture);
        --argc;
        e8_var_assign(&it->picture, &v_picture);
        e8_var_destroy(&v_picture);
    }

    it->id = ++m->max_id;
    m->data[l_index] = it;

    e8_gc_use_object(it);

    E8_RETURN_OK;
}

staticE8_DECLARE_SUB(e8_valuelist_insert)
{
    _LIST(m);

    E8_ARGUMENT(v_index);
    --argc;

    int l_index = e8_numeric_as_long(&v_index.num);
    e8_var_destroy(&v_index);

    e8_var_object(result, m);
    return __e8_valuelist_add_item(m, l_index, argc, argv);
}

staticE8_DECLARE_SUB(e8_valuelist_add)
{
    _LIST(m);

    e8_var_object(result, m);
    return __e8_valuelist_add_item(m, m->size, argc, argv);
}


staticE8_DECLARE_SUB(e8_valuelist_count)
{
    _LIST(m);

    e8_var_long(result, m->size);

    E8_RETURN_OK;
}

staticE8_DECLARE_SUB(e8_valuelist_clear)
{
    _LIST(m);

    e8_var_object(result, m); /* Return This; */

    while (m->size--)
        e8_gc_free_object(m->data[m->size]);
    m->size = 0;

    /* TODO: realloc (?) */

    E8_RETURN_OK;
}

staticE8_DECLARE_SUB(e8_valuelist_get)
{
    _LIST(m);

    E8_ARGUMENT(v_index);
    long l_index = e8_numeric_as_long(&v_index.num);

    if (l_index > 0 && l_index < m->size)
        e8_var_object(result, m->data[l_index]);
    else
        E8_THROW(E8_RT_INDEX_OUT_OF_BOUNDS);

    E8_RETURN_OK;
}

staticE8_DECLARE_SUB(e8_valuelist_move)
{
    _LIST(m);

    E8_ARGUMENT(v_el);
    E8_ARGUMENT(v_offset);

    long l_offset = e8_numeric_as_long(&v_offset.num);
    long l_index;

    if (v_el.type == varObject)
        l_index = __e8_valuelist_index(m, v_el.obj);
    else
        l_index = e8_numeric_as_long(&v_el.num);


    if (l_offset) {

        if (l_index >= 0 && l_index < m->size) {

            e8_valuelist_item *it = m->data[l_index];

            while (l_offset) {

                e8_list_index_t new_index;

                if (l_offset > 0) {
                    new_index = (l_index + 1) % m->size;
                    --l_offset;
                } else {
                    new_index = (m->size + l_index - 1) % m->size;
                    ++l_offset;
                }
                m->data[l_index] = m->data[new_index];
                l_index = new_index;
            }

            m->data[l_index] = it;

        } else
            E8_THROW(E8_RT_INDEX_OUT_OF_BOUNDS);
    }

    e8_var_destroy(&v_el);
    e8_var_destroy(&v_offset);

    E8_RETURN_OK;
}

staticE8_DECLARE_SUB(e8_valuelist_find_by_value)
{
    _LIST(m);

    E8_ARGUMENT(v_value);

    e8_list_index_t i, found = -1;
    for (i = 0; i < m->size; ++i) {

        e8_var bv;

        e8_var_eq(&v_value, &m->data[i]->value, &bv);

        if (bv.bv) {
            found = i;
            break;
        }
    }

    e8_var_destroy(&v_value);

    if (found == -1)
        e8_var_undefined(result);
    else
        e8_var_object(result, m->data[found]);

    E8_RETURN_OK;
}

staticE8_DECLARE_SUB(e8_valuelist_find_by_id)
{
    _LIST(m);

    E8_ARGUMENT(v_id);
    long l_id = e8_numeric_as_long(&v_id.num);

    e8_list_index_t i, found = -1;
    for (i = 0; i < m->size; ++i) {

        if (m->data[i]->id == l_id) {
            found = i;
            break;
        }
    }

    e8_var_destroy(&v_id);

    if (found == -1)
        e8_var_undefined(result);
    else
        e8_var_object(result, m->data[found]);

    E8_RETURN_OK;
}

staticE8_DECLARE_SUB(e8_valuelist_index)
{
    _LIST(m);

    E8_ARGUMENT(v_el);

    if (v_el.type == varObject) {
        e8_var_long(result, __e8_valuelist_index(m, v_el.obj));
    } else
        E8_THROW(E8_RT_NOT_AN_OBJECT);

    e8_var_destroy(&v_el);

    E8_RETURN_OK;
}

staticE8_DECLARE_SUB(e8_valuelist_delete)
{
    _LIST(m);

    E8_ARGUMENT(v_index);

    long l_index;

    if (v_index.type == varNumeric) {

        /* .Удалить(Число) */
        l_index = e8_numeric_as_long(&v_index.num);

    } else if (v_index.type == varObject) {

        /* .Удалить(ЭлементСпискаЗначений) */
        l_index = __e8_valuelist_index(m, v_index.obj);

    } else
        E8_THROW(E8_RT_CANNOT_CAST_TO_NUMERIC); /* TODO: Throw */

    if (l_index >= 0 && l_index < m->size) {

        e8_gc_free_object(m->data[l_index]);

        --m->size;

        while (l_index < m->size) {
            m->data[l_index] = m->data[l_index + 1];
            ++l_index;
        }

    } else
        E8_THROW(E8_RT_INDEX_OUT_OF_BOUNDS);

    e8_var_destroy(&v_index);

    e8_var_object(result, m);

    E8_RETURN_OK;
}

staticE8_DECLARE_SUB(e8_valuelist_unload_values)
{
    _LIST(m);

    e8_array *a = __e8_new_array(false, m->size);

    e8_list_index_t i;
    for (i = 0; i < m->size; ++i)
        e8_var_assign(&a->data[i], &m->data[i]->value);

    e8_var_object(result, a);

    E8_RETURN_OK;
}

staticE8_DECLARE_SUB(e8_valuelist_load_values)
{
    _LIST(m);

    E8_ARGUMENT(v_array);

    e8_valuelist_clear(obj, user_data, 0, 0, result);

    e8_array *a = (e8_array *)v_array.obj;

    m->size = a->size;
    e8_list_index_t i;

    for (i = 0; i < m->size; ++i) {

        e8_valuelist_item *it = e8_valuelist_item_new();
        m->data[i] = it;

        e8_var_assign(&it->value, &a->data[i]);

        e8_string *s;
        e8_var_cast_string(&it->value, &s);

        e8_var_string(&it->presentation, s->s);
        e8_string_destroy(s);
    }

    e8_var_destroy(&v_array);

    E8_RETURN_OK;
}

static void
__e8_valuelist_destroy(e8_object obj)
{
    _LIST(m);

    int i = m->size;
    while (i--) {
        e8_gc_free_object(m->data[i]);
    }

    free(m->data);
    free(m);
}

staticE8_DECLARE_SUB(e8_valuelist_copy)
{
    _LIST(m);

    e8_runtime_error *__err;

    e8_valuelist *r = __e8_valuelist_new();

    if (( __err = E8_COLLECTION_INIT_SIZE(*r, m->size) ))
        return __err;

    e8_list_index_t i;
    for (i = 0; i < m->size; ++i) {

        e8_valuelist_item *it = e8_valuelist_item_new();

        e8_var_assign(&it->value,        &m->data[i]->value);
        e8_var_assign(&it->presentation, &m->data[i]->presentation);
        e8_var_assign(&it->mark,         &m->data[i]->mark);
        e8_var_assign(&it->picture,      &m->data[i]->picture);

        e8_gc_use_object(it);

        it->id = ++r->max_id;
        r->data[i] = it;
    }

    e8_var_object(result, r);

    E8_RETURN_OK;
}

staticE8_DECLARE_SUB(e8_valuelist_fill_checks)
{
    _LIST(m);

    E8_ARGUMENT(v_mark);

    bool b_mark;

    if (e8_var_cast_bool(&v_mark, &b_mark))
        E8_THROW(E8_RT_CANNOT_CAST_TO_BOOL);
    else {

        e8_list_index_t i;

        for (i = 0; i < m->size; ++i)
            e8_var_bool(&m->data[i]->mark, b_mark);
    }

    e8_var_object(result, m);

    E8_RETURN_OK;
}

static
E8_METHOD_LIST_START(methods)
    E8_METHOD_LIST_ADD("количество",        "count",            e8_valuelist_count)
    E8_METHOD_LIST_ADD("добавить",          "add",              e8_valuelist_add)
    E8_METHOD_LIST_ADD("вставить",          "insert",           e8_valuelist_insert)
    E8_METHOD_LIST_ADD("получить",          "get",              e8_valuelist_get)
    E8_METHOD_LIST_ADD("удалить",           "delete",           e8_valuelist_delete)
    E8_METHOD_LIST_ADD("очистить",          "clear",            e8_valuelist_clear)
    E8_METHOD_LIST_ADD("скопировать",       "copy",             e8_valuelist_copy)
    E8_METHOD_LIST_ADD("индекс",            "index",            e8_valuelist_index)
    E8_METHOD_LIST_ADD("сдвинуть",          "move",             e8_valuelist_move)
    E8_METHOD_LIST_ADD("загрузитьзначения", "loadvalues",       e8_valuelist_load_values)
    E8_METHOD_LIST_ADD("выгрузитьзначения", "unloadvalues",     e8_valuelist_unload_values)
    E8_METHOD_LIST_ADD("заполнитьпометки",  "fillchecks",       e8_valuelist_fill_checks)
    E8_METHOD_LIST_ADD("найтипозначению",   "findbyvalue",      e8_valuelist_find_by_value)
    E8_METHOD_LIST_ADD("найтипоидентификатору", "findbyid",     e8_valuelist_find_by_id)
    E8_METHOD_LIST_ADD("__итератор",        "__iterator",       e8_valuelist_iterator_new)
E8_METHOD_LIST_END

staticE8_DECLARE_GET_METHOD(__e8_valuelist_get_method)
{
    e8_simple_get_method(name, methods, fn);
    E8_RETURN_OK;
}


static void
__init_vmt()
{
    e8_vtable_template(&vmt_valuelist);

    vmt_valuelist.dtor = __e8_valuelist_destroy;
    vmt_valuelist.get_method = __e8_valuelist_get_method;

    e8_prepare_call_elements(methods, "utf-8");

    e8_vtable_template(&vmt_valuelist_iterator);
    vmt_valuelist_iterator.dtor = __e8_valuelist_iterator_destroy;
    vmt_valuelist_iterator.get_method = __e8_valuelist_iterator_get_method;

    e8_prepare_call_elements(iterator_methods, "utf-8");

    init = true;
}


E8_DECLARE_SUB(e8_valuelist_new)
{
    if (!init)
        __init_vmt();

    e8_valuelist *res = __e8_valuelist_new();
    e8_var_object(result, res);

    E8_RETURN_OK;
}
