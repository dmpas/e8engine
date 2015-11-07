#include "variant.h"
#include <malloc.h>

void e8_obj_to_string(const e8_object ref, e8_var *str)
{
    e8_object *obj = ref;
    e8_vtable *vmt = e8_get_vtable(obj);
    if (vmt)
        if (vmt->to_string) {
            e8_var_string(str, vmt->to_string(obj));
            return;
        }
    e8_var_string_utf(str, "(object)");
}

static e8_runtime_error* property_set(void *property, const e8_var *value)
{
    e8_property *p = (e8_property*)property;
    e8_var_assign((e8_var*)p->data, value);
    return 0;
}
static e8_runtime_error* property_get(void *property, e8_var *value)
{
    e8_property *p = (e8_property*)property;
    e8_var_assign(value, (e8_var*)p->data);
    return 0;
}


void e8_create_variant_property(e8_property *property, e8_var *value)
{
    property->can_get = true;
    property->can_set = true;
    property->data = value;
    property->get = &property_get;
    property->set = &property_set;
}

void e8_destroy_variant_property(e8_property *property)
{
    e8_var_destroy((e8_var*)property->data);
}

/* --- */
static e8_runtime_error*
__get_property_t(e8_object obj, const e8_uchar *name, e8_property *property)
{
    E8_THROW_DETAIL(E8_RT_PROPERTY_NOT_FOUND, e8_utf_strdup(name));
}
static e8_runtime_error*
__get_method_t(e8_object obj, const e8_uchar *name, e8_function_signature *method, void **user_data)
{
    E8_THROW_DETAIL(E8_RT_METHOD_NOT_FOUND, e8_utf_strdup(name));
}
static const e8_uchar*
__to_string_t(const e8_object obj)
{
    return 0;
}

static e8_runtime_error*
__get_by_index_t(e8_object obj, const e8_var *index, e8_property *property)
{
    e8_string *s_index;
    e8_var_cast_string(index, &s_index);

    e8_vtable *vmt = e8_get_vtable(obj);

    if (!vmt)
        E8_THROW(E8_RT_NOT_AN_OBJECT);

    e8_runtime_error *__err = vmt->get_property(obj, s_index->s, property);
    e8_string_destroy(s_index);

    return __err;
}
static void __destroy_t(e8_object ref)
{
    free(ref);
}

/* --- */

void e8_vtable_template(e8_vtable *vmt)
{
    if (!vmt)
        return;

    vmt->to_string = __to_string_t;
    vmt->type = 0;
    vmt->get_method = __get_method_t;
    vmt->get_property = __get_property_t;
    vmt->by_index = __get_by_index_t;
    vmt->dtor = __destroy_t;
}
