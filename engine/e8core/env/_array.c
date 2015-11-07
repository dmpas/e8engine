#include "_array.h"
#include "e8core/plugin/plugin.h"
#include "macro.h"
#include <malloc.h>
#include <stdarg.h>

static e8_vtable vmt_array;
static e8_vtable vmt_array_iterator;
static int __init = false;


e8_array *
__e8_new_array(bool fixed, int size)
{

    e8_array *r = AALLOC(e8_array, 1);
    if (!r)
        return 0;

    e8_runtime_error *__err;
    __err = E8_COLLECTION_INIT_SIZE(*r, size);
    if (__err) {
        e8_free_exception(__err);
        return 0;
    }

    int i;
    for (i = 0; i < r->size; ++i)
        e8_var_undefined(&r->data[i]);

    r->fixed = fixed;

    e8_gc_register_object(r, &vmt_array);

    return r;
}

#define _ARRAY(name) e8_array *name = (e8_array *)obj

static void
__e8_array_destroy(e8_object obj)
{
    _ARRAY(a);

    while (a->size--)
        e8_var_destroy(&a->data[a->size]);

    if (a->data)
        free(a->data);

    free(a);
}



E8_DECLARE_SUB(__e8_array_count)
{
    _ARRAY(a);
    e8_var_long(result, a->size);
    E8_RETURN_OK;
}

E8_DECLARE_SUB(__e8_array_add)
{
    _ARRAY(a);

    e8_runtime_error *__err;

    int i = a->size;
    if (( __err = e8_collection_add_size(a, argc) ))
        return __err;

    while (argc--) {
        E8_ARGUMENT(value);

        e8_var_undefined(&a->data[i]);
        e8_var_assign(&a->data[i], &value);
        e8_var_destroy(&value);

        ++i;
    }

    if (result)
        e8_var_object(result, a);

    E8_RETURN_OK;
}

static inline long
e8_array_lower_bound(const e8_array *a)
{
    (void)a;
    return 0;
}
static inline long
e8_array_upper_bound(const e8_array *a)
{
    return a->size - 1;
}


#define _ITERATOR(name) e8_array_iterator *name = (e8_array_iterator *)obj;

E8_DECLARE_SUB(__e8_array_iterator_has_next)
{
    _ITERATOR(it);

    e8_var_bool(result, it->current < it->array->size);

    E8_RETURN_OK;
}

E8_DECLARE_SUB(__e8_array_iterator_next)
{
    _ITERATOR(it);

    if (it->current < it->array->size) {
        e8_var_assign(result, &it->array->data[it->current++]);
        E8_RETURN_OK;
    }

    E8_THROW(E8_RT_INDEX_OUT_OF_BOUNDS);
}

static void
__e8_array_iterator_destroy(e8_object obj)
{
    _ITERATOR(it);
    e8_gc_free_object(it->array);
    free(it);
}

static
E8_METHOD_LIST_START(iterator_methods)
    E8_METHOD_LIST_ADD("__ЕстьСледующий",   "__HasNext",    __e8_array_iterator_has_next)
    E8_METHOD_LIST_ADD("__Следующий",       "__Next",       __e8_array_iterator_next)
E8_METHOD_LIST_END

static E8_DECLARE_GET_METHOD(__e8_array_iterator_get_method)
{
    e8_simple_get_method(name, iterator_methods, fn);
    E8_RETURN_OK;
}


E8_DECLARE_SUB(__e8_array_iterator_new)
{
    _ARRAY(a);

    e8_array_iterator *it = AALLOC(e8_array_iterator, 1);
    e8_gc_register_object(it, &vmt_array_iterator);

    it->current = 0;
    it->array = a;
    e8_gc_use_object(a);

    e8_var_object(result, it);

    E8_RETURN_OK;
}

E8_DECLARE_SUB(__e8_array_ubound)
{
    _ARRAY(a);

    e8_var_long(result, e8_array_upper_bound(a));

    E8_RETURN_OK;
}

E8_DECLARE_SUB(__e8_array_lbound)
{
    _ARRAY(a);

    e8_var_long(result, e8_array_lower_bound(a));

    E8_RETURN_OK;
}

E8_DECLARE_SUB(__e8_array_find)
{
    _ARRAY(a);

    E8_ARGUMENT(value);

    e8_var_undefined(result);

    int i;
    for (i = 0; i < a->size; ++i) {
        e8_var eq;
        e8_var_eq(&a->data[i], &value, &eq);

        if (eq.bv) {
            e8_var_long(result, i);
            break;
        }
    }

    e8_var_destroy(&value);

    E8_RETURN_OK;
}

E8_DECLARE_SUB(__e8_array_insert)
{
    _ARRAY(a);

    E8_ARGUMENT(v_index);
    E8_ARGUMENT(v_value);

    int l_index = e8_numeric_as_long(&v_index.num);
    e8_var_destroy(&v_index);

    e8_runtime_error *__err;

    if (( __err = e8_collection_add_size(a, 1) ))
        return __err;

    int i = a->size;
    while (--i > l_index)
        a->data[i] = a->data[i - 1];

    e8_var_undefined(&a->data[i]);
    e8_var_assign(&a->data[i], &v_value);

    e8_var_destroy(&v_value);

    e8_var_object(result, a); /* Return This; */
    E8_RETURN_OK;
}

E8_DECLARE_SUB(__e8_array_clear)
{
    _ARRAY(a);

    int i;
    for (i = 0; i < a->size; ++i)
        e8_var_destroy(&a->data[i]);

    e8_collection_free(a);

    e8_var_object(result, a); /* Return This; */
    E8_RETURN_OK;
}

E8_DECLARE_SUB(__e8_array_get)
{
    _ARRAY(a);

    E8_ARGUMENT(v_index);
    long l_index = e8_numeric_as_long(&v_index.num);
    e8_var_destroy(&v_index);

    if ( l_index < e8_array_lower_bound(a) && l_index > e8_array_upper_bound(a))
        E8_THROW(E8_RT_INDEX_OUT_OF_BOUNDS);

    e8_var_assign(result, &a->data[l_index]);

    E8_RETURN_OK;
}

E8_DECLARE_SUB(__e8_array_delete)
{
    _ARRAY(a);
    E8_ARGUMENT(v_index);
    long l_index = e8_numeric_as_long(&v_index.num);
    e8_var_destroy(&v_index);

    if ( l_index < e8_array_lower_bound(a) && l_index > e8_array_upper_bound(a))
        E8_THROW(E8_RT_INDEX_OUT_OF_BOUNDS);

    e8_var_destroy(&a->data[l_index]);
    --a->size;
    int i = l_index;
    while (i < a->size)
        a->data[i] = a->data[i + 1];

    e8_var_object(result, a); /* Return This; */
    E8_RETURN_OK;
}

E8_DECLARE_SUB(__e8_array_set)
{
    _ARRAY(a);

    E8_ARGUMENT(v_index);
    long l_index = e8_numeric_as_long(&v_index.num);
    e8_var_destroy(&v_index);

    E8_ARGUMENT(v_value);

    if ( l_index < e8_array_lower_bound(a) && l_index > e8_array_upper_bound(a))
        E8_THROW(E8_RT_INDEX_OUT_OF_BOUNDS);

    e8_var_assign(&a->data[l_index], &v_value);
    e8_var_destroy(&v_value);

    e8_var_object(result, a); /* Return This; */

    E8_RETURN_OK;
}

static
E8_METHOD_LIST_START(methods)
/* 0 */
    E8_METHOD_LIST_ADD("#присоединить",         "#append",      __e8_array_add)
    E8_METHOD_LIST_ADD("вставить",              "insert",       __e8_array_insert)
    E8_METHOD_LIST_ADD("удалить",               "delete",       __e8_array_delete)
    E8_METHOD_LIST_ADD("установить",            "set",          __e8_array_set)
    E8_METHOD_LIST_ADD("очистить",              "clear",        __e8_array_clear)
/* 5 */
#define ARRAY_READONLY_START 5
    E8_METHOD_LIST_ADD("__итератор",            "__iterator",   __e8_array_iterator_new)
    E8_METHOD_LIST_ADD("количество",            "count",        __e8_array_count)
    E8_METHOD_LIST_ADD("добавить",              "add",          __e8_array_add)
    E8_METHOD_LIST_ADD("вграница",              "ubound",       __e8_array_ubound)
    E8_METHOD_LIST_ADD("нграница",              "lbound",       __e8_array_lbound)
    E8_METHOD_LIST_ADD("найти",                 "find",         __e8_array_find)
    E8_METHOD_LIST_ADD("получить",              "get",          __e8_array_get)
E8_METHOD_LIST_END

static E8_DECLARE_GET_METHOD(__e8_array_get_method)
{
    _ARRAY(a);

    if (a->fixed)
        e8_simple_get_method(name, &methods[ARRAY_READONLY_START], fn);
    else
        e8_simple_get_method(name, methods, fn);

    E8_RETURN_OK;
}

#undef ARRAY_READONLY_START

static e8_runtime_error*
__e8_array_get_by_index(e8_object obj, const e8_var *index, e8_property *property)
{
    _ARRAY(a);
    /* TODO: Проверить тип */
    long l_i = e8_numeric_as_long(&index->num);

    if (l_i < e8_array_lower_bound(a) || l_i > e8_array_upper_bound(a))
        E8_THROW(E8_RT_INDEX_OUT_OF_BOUNDS);

    e8_create_variant_property(property, &a->data[l_i]);
    property->can_set = !a->fixed;

    E8_RETURN_OK;
}



static void
__e8_array_init_vmt()
{
    e8_vtable_template(&vmt_array);

    vmt_array.dtor = __e8_array_destroy;
    vmt_array.get_method = __e8_array_get_method;
    vmt_array.by_index = __e8_array_get_by_index;

    e8_vtable_template(&vmt_array_iterator);
    vmt_array_iterator.dtor = __e8_array_iterator_destroy;
    vmt_array_iterator.get_method = __e8_array_iterator_get_method;

    e8_prepare_call_elements(methods, "utf-8");
    e8_prepare_call_elements(iterator_methods, "utf-8");

    __init = true;
}


E8_DECLARE_SUB(e8_array_new)
{

    if (!__init)
        __e8_array_init_vmt();

    int initial_size = 0;
    if (argc) {
        E8_ARGUMENT(v_isize);

        /* TODO: проверить тип Numeric */
        initial_size = e8_numeric_as_long(&v_isize.num);

        e8_var_destroy(&v_isize);
    }

    e8_array *r = __e8_new_array(false, initial_size);
    if (!r)
        E8_THROW(E8_RT_ALLOCATION_ERROR);

    e8_var_object(result, r);

    E8_RETURN_OK;
}

E8_DECLARE_SUB(e8_fixed_array_new)
{

    if (!__init)
        __e8_array_init_vmt();

    if (!argc)
        E8_THROW(E8_RT_CANNOT_CAST_TO_NUMERIC);

    E8_ARGUMENT(v_array);

    if (v_array.type != varObject)
        E8_THROW_DETAIL(E8_RT_EXCEPTION_RAISED, "Argument 1 is not an object!");

    e8_array *a = (e8_array *)v_array.obj;

    if (e8_get_vtable(a) != &vmt_array)
        E8_THROW_DETAIL(E8_RT_EXCEPTION_RAISED, "Argument 1 is not an array!");


    e8_array *r = __e8_new_array(true, a->size);
    if (!r)
        E8_THROW(E8_RT_ALLOCATION_ERROR);

    e8_array_index_t i;
    for (i = 0; i < a->size; ++i)
        e8_var_assign(&r->data[i], &a->data[i]);

    e8_var_destroy(&v_array);

    e8_var_object(result, r);

    E8_RETURN_OK;
}

/* Раздел для плагинов */

#define NO_RETURN 0

void e8_array_add_nvalues(e8_var *array, int n, ...)
{
    va_list l;
    va_start(l, n);

    while (n--) {
        e8_var v = va_arg(l, e8_var);
        e8_simple_call(__e8_array_add, array->obj, NO_RETURN, /*argc=*/1, v);
    }

    va_end(l);
}

void e8_array_insert_nvalues(e8_var *array, int index, int n, ...)
{
    va_list l;
    va_start(l, n);

    while (n--) {
        e8_var v = va_arg(l, e8_var), v_i;
        e8_var_long(&v_i, index);
        e8_simple_call(__e8_array_add, array->obj, NO_RETURN, /*argc=*/1, v);
        ++index;
    }

    va_end(l);
}

int e8_array_index_of(const e8_var *array, const e8_var *value)
{
    e8_var r;
    e8_simple_call(__e8_array_find, array->obj, &r, /*argc=*/1, value);
    return e8_numeric_as_long(&r.num);
}

int e8_array_size(const e8_var *array)
{
    e8_var r;
    e8_simple_call(__e8_array_count, array->obj, &r, /*argc=*/0);
    return e8_numeric_as_long(&r.num);
}

void e8_array_clear(e8_var *array)
{
    e8_simple_call(__e8_array_clear, array->obj, NO_RETURN, /*argc=*/0);
}

e8_var *e8_array_get(const e8_var *array, int index, e8_var *result)
{
    e8_var v_index;
    e8_var_long(&v_index, index);
    e8_simple_call(__e8_array_get, array->obj, result, /*argc=*/1, v_index);
    return result;
}

void e8_array_set(e8_var *array, int index, const e8_var *value)
{
    e8_var v_index;
    e8_var_long(&v_index, index);
    e8_simple_call(__e8_array_set, array->obj, 0, /*argc=*/2, v_index, *value);
}

#undef NO_RETURN

#undef _ARRAY
#undef _ITERATOR
