#include "_valuelistitem.h"
#include "macro.h"
#include <malloc.h>

#define _ITEM(name) e8_valuelist_item *name = (e8_valuelist_item *)obj

static bool init = false;
e8_vtable vmt_valuelist_item;

static e8_uchar u_a_value[20];
static e8_uchar u_r_value[20];
static e8_uchar u_a_presentation[20];
static e8_uchar u_r_presentation[20];
static e8_uchar u_a_mark[20];
static e8_uchar u_r_mark[20];
static e8_uchar u_a_picture[20];
static e8_uchar u_r_picture[20];


static void
__e8_valuelist_item_destroy(e8_object obj)
{
    _ITEM(it);

    e8_var_destroy(&it->value);
    e8_var_destroy(&it->presentation);
    e8_var_destroy(&it->mark);
    e8_var_destroy(&it->picture);

    free(it);
}

static
E8_DECLARE_SUB(e8_valuelist_item_get_id)
{
    _ITEM(it);

    e8_var_long(result, it->id);

    E8_RETURN_OK;
}


static
E8_METHOD_LIST_START(valuelist_item_methods)
    E8_METHOD_LIST_ADD("ПолучитьИдентификатор", "GetID", e8_valuelist_item_get_id)
E8_METHOD_LIST_END

staticE8_DECLARE_GET_METHOD(__e8_valuelist_item_get_method)
{
    e8_simple_get_method(name, valuelist_item_methods, fn);
    E8_RETURN_OK;
}

static e8_runtime_error *
__e8_valuelist_item_get_property(e8_object obj, const e8_uchar *name, e8_property *property)
{
    _ITEM(it);

    e8_uchar *lname = e8_utf_strdup(name);
    e8_utf_lower_case(lname);

    bool found = true;

    #define CHECK_IS_PROPERTY(name) \
    if ( e8_utf_strcmp(lname, u_r_##name) == 0  || e8_utf_strcmp(lname, u_a_##name) == 0 ) { \
        e8_create_variant_property(property, &it-> name); \
    }


    CHECK_IS_PROPERTY(value) else
    CHECK_IS_PROPERTY(presentation) else
    CHECK_IS_PROPERTY(mark) else
    CHECK_IS_PROPERTY(picture) else
        found = false;

    #undef CHECK_IS_PROPERTY

    e8_utf_free(lname);

    if (!found)
        E8_THROW_DETAIL(E8_RT_PROPERTY_NOT_FOUND, e8_utf_strdup(name));

    E8_RETURN_OK;
}



static void
__init_vmt()
{

    e8_uchar *t;

    #define CAST_U(eng, rus) \
        t = u_a_##eng; \
        e8_utf_from_8bit(#eng, &t, "utf-8"); \
        t = u_r_##eng; \
        e8_utf_from_8bit(#rus, &t, "utf-8")

    CAST_U(value,           значение);
    CAST_U(presentation,    представление);
    CAST_U(mark,            пометка);
    CAST_U(picture,         картинка);

    e8_prepare_call_elements(valuelist_item_methods, "utf-8");

    e8_vtable_template(&vmt_valuelist_item);
    vmt_valuelist_item.dtor         = __e8_valuelist_item_destroy;
    vmt_valuelist_item.get_method   = __e8_valuelist_item_get_method;
    vmt_valuelist_item.get_property = __e8_valuelist_item_get_property;

    init = true;
}


e8_valuelist_item *e8_valuelist_item_new()
{

    if (!init)
        __init_vmt();

    e8_valuelist_item *r = AALLOC(e8_valuelist_item, 1);

    e8_var_undefined(&r->value);
    e8_var_undefined(&r->presentation);
    e8_var_bool(&r->mark, false);
    e8_var_undefined(&r->picture);

    e8_gc_register_object(r, &vmt_valuelist_item);

    return r;
}

e8_valuelist_item *e8_valuelist_item_new_with_value(
                                const e8_var *value,
                                const e8_var *presentation,
                                const e8_var *mark,
                                const e8_var *picture
                    )
{
    #define COPY_IF(a, b) \
        { \
            if (a) \
                e8_var_assign(&b, a);\
        }

    e8_valuelist_item *r = e8_valuelist_item_new();

    COPY_IF(value,          r->value);
    COPY_IF(presentation,   r->presentation);
    COPY_IF(mark,           r->mark);
    COPY_IF(picture,        r->picture);

    #undef COPY_IF

    return r;
}

#undef _ITEM
