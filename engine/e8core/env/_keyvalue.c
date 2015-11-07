#include "_keyvalue.h"
#include "macro.h"
#include <malloc.h>

e8_vtable vmt_keyvalue;
static bool init = false;
static e8_uchar u_r_key[20];
static e8_uchar u_a_key[20];
static e8_uchar u_r_value[20];
static e8_uchar u_a_value[20];

#define _KEYVALUE(name) e8_keyvalue *name = (e8_keyvalue *)obj


static e8_runtime_error *
__e8_keyvalue_get_property(e8_object obj, const e8_uchar *name, e8_property *property)
{
    _KEYVALUE(kv);

    const e8_uchar *lname = name;

    if ( e8_utf_stricmp(lname, u_r_key) == 0  || e8_utf_stricmp(lname, u_a_key) == 0) {
        e8_create_variant_property(property, &kv->key);

        if (kv->fixed)
            property->can_set = false;

        E8_RETURN_OK;
    }

    if ( e8_utf_stricmp(lname, u_r_value) == 0  || e8_utf_stricmp(lname, u_a_value) == 0 ) {
        e8_create_variant_property(property, &kv->value);

        if (kv->fixed)
            property->can_set = false;

        E8_RETURN_OK;
    }

    E8_THROW_DETAIL(E8_RT_PROPERTY_NOT_FOUND, e8_utf_strdup(name));
}

static void
__e8_keyvalue_destroy(e8_object obj)
{
    _KEYVALUE(kv);

    e8_var_destroy(&kv->key);
    e8_var_destroy(&kv->value);
    e8_var_destroy(&kv->s_key);

    free(kv);
}

static e8_runtime_error*
__e8_keyvalue_get_by_index(e8_object obj, const e8_var *index, e8_property *property)
{
    e8_string *s;
    e8_var_cast_string(index, &s);

    e8_runtime_error *__err = __e8_keyvalue_get_property(obj, s->s, property);

    e8_string_destroy(s);
    return __err;
}


static void
__init_vmt()
{
    e8_uchar *t;

    t = u_a_key;
    e8_utf_from_8bit("key", &t, "utf-8");

    t = u_a_value;
    e8_utf_from_8bit("value", &t, "utf-8");

    t = u_r_key;
    e8_utf_from_8bit("ключ", &t, "utf-8");

    t = u_r_value;
    e8_utf_from_8bit("значение", &t, "utf-8");

    e8_vtable_template(&vmt_keyvalue);
    vmt_keyvalue.by_index = __e8_keyvalue_get_by_index;
    vmt_keyvalue.get_property = __e8_keyvalue_get_property;
    vmt_keyvalue.dtor = __e8_keyvalue_destroy;

    init = true;
}

e8_keyvalue *e8_keyvalue_new(bool fixed)
{
    if (!init)
        __init_vmt();

    e8_keyvalue *res = AALLOC(e8_keyvalue, 1);
    e8_var_undefined(&res->key);
    e8_var_undefined(&res->value);
    e8_var_undefined(&res->s_key);
    res->fixed = fixed;

    e8_gc_register_object(res, &vmt_keyvalue);

    return res;
}

#undef _KEYVALUE
