#include "simpleenum.h"
#include "macro.h"
#include <malloc.h>

typedef struct __enum_element {
    e8_uchar           *rus;
    e8_uchar           *eng;
    e8_var              value;
    int                 index;
} enum_element;

typedef struct __enum_data {
    int             size;
    enum_element   *data;
} enum_data;

#define _DATA(name) enum_data *name = (enum_data *)obj

static void
__e8_simple_enum_destroy(e8_object obj)
{
    _DATA(d);
    if (d->data) {

        enum_element *e = d->data;
        while (d->size--) {

            e8_utf_free(e->rus);
            e8_utf_free(e->eng);

            e8_var_destroy(&e->value);

            ++e;
        }

        free(d->data);
    }

    free(d);
}

static
E8_DECLARE_GET_PROPERTY(__e8_simple_enum_get_property)
{
    _DATA(d);

    e8_uchar *lname = e8_utf_strdup(name);
    e8_utf_lower_case(lname);

    int i;
    for (i = 0; i < d->size; ++i) {
        if ((e8_utf_strcmp(d->data[i].rus, lname) == 0) || (e8_utf_strcmp(d->data[i].eng, lname) == 0)) {

            e8_create_variant_property(property, &d->data[i].value);

            e8_utf_free(lname);
            E8_RETURN_OK;
        }
    }

    e8_utf_free(lname);
    E8_THROW_DETAIL(E8_RT_PROPERTY_NOT_FOUND, e8_utf_strdup(name));
}

void e8_simple_enum_init_vtable(e8_vtable *vmt)
{
    e8_vtable_template(vmt);
    vmt->dtor = __e8_simple_enum_destroy;
    vmt->get_property = __e8_simple_enum_get_property;
}

void e8_simple_enum_attach_list(
                        e8_var                             *_enum,
                        const e8_simple_enum_element       *t
                    )
{
    const e8_simple_enum_element *e;
    enum_data *d = (enum_data *)_enum->obj;

    e = t;
    while (e->index != -1) {
        ++d->size;
        ++e;
    }

    d->data = AALLOC(enum_element, d->size);
    enum_element *eel = d->data;

    e8_vtable *vmt = e8_get_vtable(_enum->obj);

    e = t;
    while (e->index != -1) {

        eel->rus = e8_utf_strcdup(e->rus, "utf-8");
        eel->eng = e8_utf_strcdup(e->eng, "utf-8");

        e8_utf_lower_case(eel->rus);
        e8_utf_lower_case(eel->eng);

        eel->index = e->index;
        /* e8_var_undefined(&eel->value); */
        e8_var_enum(&eel->value, vmt, eel->index);

        ++e;
        ++eel;
    }

}


void e8_simple_enum_set_value(e8_var *_enum, int element, const e8_var *value)
{
    enum_data *d = (enum_data *)_enum->obj;

    int i;
    for (i = 0; i < d->size; ++i) {
        if (d->data[i].index == element) {
            e8_var_assign(&d->data[i].value, value);
            break;
        }
    }

}

void e8_simple_enum_new(e8_var *result, e8_vtable *vmt)
{
    enum_data *d = AALLOC(enum_data, 1);
    d->data = 0;
    d->size = 0;
    e8_gc_register_object(d, vmt);
    e8_var_object(result, d);
}

int e8_simple_enum_find_value(const e8_vtable *vmt, const e8_var *value)
{
    if (value->type != varEnum)
        return -1;
    if (value->_enum.md != vmt)
        return -1;

    return value->_enum.index;
}
