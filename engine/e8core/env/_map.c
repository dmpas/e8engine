#include "_map.h"
#include "macro.h"
#include <malloc.h>
#include "_strings.h"
#include "e8core/translator/lexer.h"
#include <stddef.h>
#include "_command.h"

#define _MAP(name) e8_map *name = (e8_map *)obj

static e8_vtable vmt_map;
static e8_vtable vmt_map_iterator;
static bool init = false;


#define _ITERATOR(name) e8_map_iterator *name = (e8_map_iterator *)obj;

static
E8_DECLARE_SUB(__e8_map_iterator_has_next)
{
    _ITERATOR(it);

    e8_var_bool(result, it->current < it->map->size);

    E8_RETURN_OK;
}

static
E8_DECLARE_SUB(__e8_map_iterator_next)
{
    _ITERATOR(it);

    if (it->current < it->map->size) {
        e8_var_object(result, it->map->data[it->current++]);
        E8_RETURN_OK;
    }

    E8_THROW(E8_RT_INDEX_OUT_OF_BOUNDS);
}

static void
__e8_map_iterator_destroy(e8_object obj)
{
    _ITERATOR(it);
    e8_gc_free_object(it->map);

    free(it);
}


static
E8_METHOD_LIST_START(iterator_methods)
    E8_METHOD_LIST_ADD("__ЕстьСледующий",   "__HasNext",    __e8_map_iterator_has_next)
    E8_METHOD_LIST_ADD("__Следующий",       "__Next",       __e8_map_iterator_next)
E8_METHOD_LIST_END

static E8_DECLARE_GET_METHOD(__e8_map_iterator_get_method)
{
    e8_simple_get_method(name, iterator_methods, fn);
    E8_RETURN_OK;
}


E8_DECLARE_SUB(__e8_map_iterator_new)
{
    _MAP(m);

    e8_map_iterator *it = AALLOC(e8_map_iterator, 1);
    e8_gc_register_object(it, &vmt_map_iterator);

    it->current = 0;
    it->map = m;
    e8_gc_use_object(m);

    e8_var_object(result, it);

    E8_RETURN_OK;
}


static int
__e8_map_index(const e8_map *m, const e8_var *key)
{
    int i;
    for (i = 0; i < m->size; ++i) {

        e8_var eq;
        e8_var_eq(&m->data[i]->s_key, key, &eq);

        if (eq.bv)
            return i;

    }
    return -1;
}

static
E8_DECLARE_SUB(__e8_map_insert)
{
    _MAP(m);
    e8_runtime_error *__err;

    if (m->fixed)
        E8_THROW(E8_RT_VALUE_CANNOT_BE_WRITTEN); /* TODO: Понятное исключение */

    e8_var_object(result, m); /* Return This; */

    while (argc) {

        E8_ARGUMENT(v_key);
        E8_ARGUMENT(v_value);

        argc -= 2;

        E8_VAR(v_skey); /* Уточнённый ключ поиска */

        if (m->structure) {

            e8_string *s_key;

            e8_var_cast_string(&v_key, &s_key);

            e8_uchar *lname = e8_utf_strdup(s_key->s);
            e8_utf_lower_case(lname);
            s_key = e8_string_init(lname);
            e8_utf_free(lname);

            /* TODO: Проверить на правильный идентиификатор */

            e8_var_string(&v_skey, s_key->s);

            e8_string_destroy(s_key);

        } else
            e8_var_assign(&v_skey, &v_key);

        int i;

        i = __e8_map_index(m, &v_skey);

        if (i == -1) {

            i = m->size;
            if (( __err = e8_collection_add_size(m, 1) ))
                return __err;

            e8_keyvalue *it = e8_keyvalue_new(m->fixed);

            e8_gc_use_object(it);

            e8_var_assign(&it->key, &v_key);
            e8_var_assign(&it->s_key, &v_skey);
            e8_var_assign(&it->value, &v_value);

            m->data[i] = it;

        } else {
            e8_var_assign(&m->data[i]->value, &v_value);
        }

        e8_var_destroy(&v_skey);
        e8_var_destroy(&v_key);
        e8_var_destroy(&v_value);
    }

    E8_RETURN_OK;
}

static
E8_DECLARE_SUB(__e8_map_count)
{
    _MAP(m);

    e8_var_long(result, m->size);

    E8_RETURN_OK;
}

static
E8_DECLARE_SUB(__e8_map_clear)
{
    _MAP(m);

    e8_var_object(result, m); /* Return This; */

    while (m->size--)
        e8_gc_free_object(m->data[m->size]);
    m->size = 0;

    /* TODO: realloc (?) */

    E8_RETURN_OK;
}

static
E8_DECLARE_SUB(__e8_map_get)
{
    _MAP(m);

    E8_ARGUMENT(key);

    int i = __e8_map_index(m, &key);
    if (i == -1)
        e8_var_undefined(result);
    else
        e8_var_assign(result, &m->data[i]->value);

    e8_var_destroy(&key);

    E8_RETURN_OK;
}

static
E8_DECLARE_SUB(__e8_map_delete)
{
    _MAP(m);

    E8_ARGUMENT(key);

    int i = __e8_map_index(m, &key);
    if (i != -1) {
        e8_gc_free_object(m->data[i]);
        --m->size;
        while (i < m->size)
            m->data[i] = m->data[i + 1];
    }

    e8_var_destroy(&key);

    E8_RETURN_OK;
}

static void
__e8_map_destroy(e8_object obj)
{
    _MAP(m);

    int i = m->size;
    while (i--) {
        e8_gc_free_object(m->data[i]);
    }

    free(m->data);
    free(m);
}

static
E8_DECLARE_SUB(__e8_map_property)
{
    _MAP(m);

    if (!m->structure)
        E8_THROW(E8_RT_METHOD_NOT_FOUND);

    E8_ARGUMENT(v_name);

    e8_string *s_name;
    e8_var_cast_string(&v_name, &s_name);
    e8_var_destroy(&v_name);

    e8_uchar *lname = e8_utf_strdup(s_name->s);
    e8_utf_lower_case(lname);
    e8_var_string(&v_name, lname);

    e8_string_destroy(s_name);
    e8_utf_free(lname);

    int i;
    i = __e8_map_index(m, &v_name);
    if (i == -1) {
        e8_var_bool(result, false);
    } else {
        e8_var_bool(result, true);
        if (argc > 1) {

            if (argv->can_set)
                argv->set(argv, &m->data[i]->value);
            else
                E8_THROW(E8_RT_VALUE_CANNOT_BE_WRITTEN);
        }
    }

    e8_var_destroy(&v_name);

    E8_RETURN_OK;
}

static
E8_METHOD_LIST_START(methods)
    E8_METHOD_LIST_ADD("количество",        "count",        __e8_map_count)
    E8_METHOD_LIST_ADD("вставить",          "insert",       __e8_map_insert)
    E8_METHOD_LIST_ADD("получить",          "get",          __e8_map_get)
    E8_METHOD_LIST_ADD("удалить",           "delete",       __e8_map_delete)
    E8_METHOD_LIST_ADD("очистить",          "clear",        __e8_map_clear)
    E8_METHOD_LIST_ADD("свойство",          "property",     __e8_map_property)
    E8_METHOD_LIST_ADD("#присоединить",     "#append",      __e8_map_insert)
    E8_METHOD_LIST_ADD("__итератор",        "__iterator",   __e8_map_iterator_new)
E8_METHOD_LIST_END


static E8_DECLARE_GET_METHOD(__e8_map_get_method)
{
    e8_simple_get_method(name, methods, fn);

    if (*fn)
        E8_RETURN_OK;

    _MAP(m);
    e8_var key;
    e8_uchar *lname = e8_utf_strdup(name);
    e8_utf_lower_case(lname);

    e8_var_string(&key, lname);

    e8_utf_free(lname);

    int i = __e8_map_index(m, &key);

    if (i == -1)
        E8_RETURN_OK;

    e8_var *V = &m->data[i]->value;
    if (V->type != varObject)
        E8_RETURN_OK;

    e8_vtable *vmt = e8_get_vtable(V->obj);
    if (vmt != &action_command_vmt)
        E8_RETURN_OK;

    e8_action_command *c = (e8_action_command *)V->obj;

    return c->vmt->get_method(c->obj, c->method, fn, user_data);
}

static E8_DECLARE_GET_BY_INDEX(__e8_map_by_index)
{
    _MAP(m);

    E8_VAR(v_s_index);
    if (m->structure) {

        e8_string *s_index;

        e8_var_cast_string(index, &s_index);

        e8_uchar *lname = e8_utf_strdup(s_index->s);
        e8_utf_lower_case(lname);
        e8_string_destroy(s_index);

        e8_var_string(&v_s_index, lname);
        e8_utf_free(lname);


    } else
        e8_var_assign(&v_s_index, index);

    e8_runtime_error *__err;

    int i = __e8_map_index(m, &v_s_index);
    if (i == -1) {

        if (m->structure) {
            e8_string *s_index;
            e8_var_cast_string(index, &s_index);

            e8_uchar *u_index = e8_utf_strdup(s_index->s);
            e8_string_destroy(s_index);

            E8_THROW_DETAIL(E8_RT_PROPERTY_NOT_FOUND, u_index);
        }
        /* Соответствие */

        i = m->size;

        if (( __err = e8_collection_add_size(m, 1) ))
            return __err;

        e8_keyvalue *it = e8_keyvalue_new(m->fixed);

        e8_gc_use_object(it);

        e8_var_assign(&it->key, &v_s_index);
        e8_var_assign(&it->s_key, &v_s_index);
        e8_var_undefined(&it->value);

        m->data[i] = it;
    }

    e8_create_variant_property(property, &m->data[i]->value);
    property->can_set = !m->fixed;

    e8_var_destroy(&v_s_index);

    E8_RETURN_OK;
}
static E8_DECLARE_GET_PROPERTY(__e8_map_get_property)
{
    E8_VAR(v_index);
    e8_var_string(&v_index, name);

    e8_runtime_error *__err = __e8_map_by_index(obj, &v_index, property);

    e8_var_destroy(&v_index);
    return __err;
}


static void
__init_vmt()
{
    e8_vtable_template(&vmt_map);

    vmt_map.dtor = __e8_map_destroy;
    vmt_map.get_method = __e8_map_get_method;
    vmt_map.by_index = __e8_map_by_index;
    vmt_map.get_property = __e8_map_get_property;

    e8_prepare_call_elements(methods, "utf-8");

    e8_vtable_template(&vmt_map_iterator);
    vmt_map_iterator.dtor = __e8_map_iterator_destroy;
    vmt_map_iterator.get_method = __e8_map_iterator_get_method;

    e8_prepare_call_elements(iterator_methods, "utf-8");

    init = true;
}

static e8_map *
__e8_map_new(bool fixed, bool structure)
{
    if (!init)
        __init_vmt();

    e8_map *res = AALLOC(e8_map, 1);
    E8_COLLECTION_INIT(*res);
    res->fixed = fixed;
    res->structure = structure;

    e8_gc_register_object(res, &vmt_map);

    return res;
}

E8_DECLARE_SUB(e8_map_new)
{
    e8_map *res = __e8_map_new(false, false);
    e8_var_object(result, res);
    E8_RETURN_OK;
}

E8_DECLARE_SUB(e8_fixed_map_new)
{
    e8_map *res = __e8_map_new(true, false);
    e8_var_object(result, res);

    if (argc) {
        E8_ARGUMENT(v_src);

        e8_map *src = (e8_map *)v_src.obj;

        e8_collection_add_size(res, src->size);
        int i;
        for (i = 0; i < src->size; ++i) {

            e8_keyvalue *it = e8_keyvalue_new(true);

            e8_gc_use_object(it);

            e8_var_assign(&it->key, &src->data[i]->key);
            e8_var_assign(&it->s_key, &src->data[i]->s_key);
            e8_var_assign(&it->value, &src->data[i]->value);

            res->data[i] = it;
        }

        e8_var_destroy(&v_src);
    }

    E8_RETURN_OK;
}

static const e8_uchar *
__e8_utf_strstr(const e8_uchar *s, const char c)
{
    const e8_uchar *uc = s;
    while (*uc) {

        if (*uc == (e8_uchar)c)
            return uc + 1;

        ++uc;
    }
    return uc;
}

E8_DECLARE_SUB(e8_structure_new)
{
    e8_map *res = __e8_map_new(false, true);
    e8_var_object(result, res);

    if (argc) {
        E8_ARG_STRING(s_list);
        int p = 1; /* второй параметр */

        const e8_uchar *uc = s_list->s;
        while (*uc) {

            const e8_uchar *next = __e8_utf_strstr(uc, ',');
            ptrdiff_t size = next - uc;

            e8_uchar *el = e8_utf_malloc(size);
            e8_uchar *mel = el;
            e8_utf_strncpy(el, uc, size);
            e8_uchar *ls = &el[size - 1];
            while (ls > el && (*ls == ',' || lexer_is_ws(*ls))) {
                *ls = 0;
                --ls;
            }
            while (*el && lexer_is_ws(*el)) ++el;

            E8_VAR(value);
            if (p < argc) {
                argv->get(argv, &value);
                ++argv;
                ++p;
            }

            e8_var v_name;
            e8_var_string(&v_name, el);
            e8_simple_call(__e8_map_insert, res, NULL, /*argc=*/2, v_name, value);

            e8_var_destroy(&v_name);
            e8_utf_free(mel);
            e8_var_destroy(&value);

            uc = next;
        }

        e8_string_destroy(s_list);
    }

    E8_RETURN_OK;
}

E8_DECLARE_SUB(e8_fixed_structure_new)
{
    if (argc) {

        if (argc == 1) {
            E8_ARGUMENT(v_first);

            e8_map *res = __e8_map_new(true, true);

            /* Ожидаем здесь структуру. НАДО: Проверить тип объекта */
            e8_map *src = (e8_map *)v_first.obj;

            E8_COLLECTION_INIT_SIZE(*res, src->size);

            int i;
            for (i = 0; i < res->size; ++i) {
                e8_keyvalue *new_value = e8_keyvalue_new(true);
                res->data[i] = new_value;

                e8_var_assign(&new_value->key, &src->data[i]->key);
                e8_var_assign(&new_value->s_key, &src->data[i]->s_key);
                e8_var_assign(&new_value->value, &src->data[i]->value);
            }

        } else {
            /* НАДО: Сделать вариант создания структуры по списку полей и значений */
            e8_structure_new(obj, user_data, argc, argv, result);
            ((e8_map*)result->obj)->fixed = true;
        }

    } else {
        e8_map *res = __e8_map_new(true, true);
        e8_var_object(result, res);
    }

    E8_RETURN_OK;
}

