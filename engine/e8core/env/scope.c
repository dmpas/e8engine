#include "scope.h"
#include "e8core/variant/variant.h"
#include <malloc.h>
#include "scope_p.h"
#include "e8core/plugin/plugin.h"

#include "macro.h"

e8_vtable vmt_scope;
static bool init = false;
static e8_uchar a_ThisObject[] = {'t', 'h', 'i', 's', 'o', 'b', 'j', 'e', 'c', 't', 0};
static char ur_ThisObject[] = "этотобъект";
static e8_uchar r_ThisObject[11];

static bool
__is_this_object(const e8_uchar *name)
{
    if (e8_utf_stricmp(name, a_ThisObject) == 0)
        return true;

    if (e8_utf_stricmp(name, r_ThisObject) == 0)
        return true;

    return false;
}

static
E8_DECLARE_PROPERTY_GET(__get_this_property)
{
    e8_var_object(value, ((e8_property *)property)->data);
    E8_RETURN_OK;
}

static void
__this_property(e8_property *p, e8_object this_object)
{
    p->get = __get_this_property;
    p->set = NULL;
    p->can_get = true;
    p->can_set = true;
    p->data = this_object;
}

static void __e8_scope_destroy(e8_object obj)
{
    e8_scope_data *s = (e8_scope)obj;

    while (s->properties.size--) {
        e8_named_property *P = &s->properties.data[s->properties.size];
        e8_utf_free(P->name);

        if (P->local) {
            e8_destroy_variant_property(&P->property);
            free(P->property.data);
        }
    }

    e8_collection_free(&s->params);
    e8_collection_free(&s->properties);

    if (s->this_object)
        e8_gc_free_object(s->this_object);

    e8_scope_data *__parent = s->parent;
    free(s);
    s = NULL;

    if (__parent)
        e8_gc_free_object(__parent);
}

static bool
__e8_scope_get_property_d(e8_scope_data *d, const e8_uchar *name, e8_property *property, bool add)
{

    if (__is_this_object(name) && d->this_object) {
        __this_property(property, d->this_object);
        return true;
    }

    bool found = false;

    int i;
    for (i = 0; i < d->properties.size; ++i) {
        if (e8_utf_stricmp(name, d->properties.data[i].name) == 0) {
            *property = d->properties.data[i].property;
            found = true;
            break;
        }
    }

    if (!found && d->this_object && d->this_vmt) {
        e8_runtime_error *__err = d->this_vmt->get_property(d->this_object, name, property);
        if (__err)
            e8_free_exception(__err);
        else
            found = true;
    }

    if (!found && d->parent) {
        found = __e8_scope_get_property_d(d->parent, name, property, false);
    }

    if (!found && add) {

        e8_property p;
        e8_var *v = AALLOC(e8_var, 1);
        e8_var_undefined(v);

        e8_create_variant_property(&p, v);
        *property = p;

        e8_collection_add_size(&d->properties.data, 1);

        e8_named_property *np = &d->properties.data[d->properties.size - 1];
        np->name = e8_utf_strdup(name);
        np->property = p;
        np->local = true;

        found = true;
    }
    return found;
}

static e8_runtime_error *
__e8_scope_get_property(e8_object obj, const e8_uchar *name, e8_property *property)
{
    e8_scope_data *d = (e8_scope_data *)obj;

    const e8_uchar *lname = name;

    __e8_scope_get_property_d(d, lname, property, true);

    E8_RETURN_OK;
}


struct __this_call {
    e8_function_signature       fn;
    e8_object                   obj;
    void                       *user_data;
};

static
E8_DECLARE_SUB(__e8_scope_this_call)
{
    struct __this_call *d = (struct __this_call *)user_data;
    e8_runtime_error *__err = d->fn(d->obj, d->user_data, argc, argv, result);
    free(d);
    return __err;
}


static bool
__e8_scope_get_method_d(e8_scope_data *d, const e8_uchar *name, e8_function_signature *fn, void **user_data)
{
    if (d->this_object && d->this_vmt) {

        e8_runtime_error *__err = d->this_vmt->get_method(d->this_object, name, fn, user_data);

        if (__err)
            e8_free_exception(__err);
        else {
            if (*fn) {

                struct __this_call *call_data = malloc(sizeof(*call_data));

                call_data->fn = *fn;
                call_data->obj = d->this_object;
                call_data->user_data = *user_data;

                *user_data = call_data;
                *fn = __e8_scope_this_call;

                return true;
            }
        }

    }

    if (d->parent)
        return __e8_scope_get_method_d(d->parent, name, fn, user_data);

    return false;
}


static
E8_DECLARE_GET_METHOD(__e8_scope_get_method)
{
    *fn = NULL;

    e8_scope_data *d = (e8_scope_data *)obj;
    const e8_uchar *lname = name;

    __e8_scope_get_method_d(d, lname, fn, user_data);

    E8_RETURN_OK;
}

static void
__e8_init_scope()
{
    e8_vtable_template(&vmt_scope);
    vmt_scope.dtor = __e8_scope_destroy;
    vmt_scope.get_property = __e8_scope_get_property;
    vmt_scope.get_method = __e8_scope_get_method;

    e8_uchar *t = r_ThisObject;
    e8_utf_from_8bit(ur_ThisObject, &t, "utf-8");

    init = true;
}

e8_runtime_error *e8_scope_new(e8_scope *scope, const e8_scope parent)
{
    if (!init)
        __e8_init_scope();

    e8_scope_data *s = AALLOC(e8_scope_data, 1);
    s->this_object = NULL;
    s->this_vmt = NULL;

    E8_COLLECTION_INIT(s->params);

    s->parent = parent;
    if (parent)
        e8_gc_use_object(parent);

    E8_COLLECTION_INIT(s->properties);

    *scope = s;
    e8_gc_register_object(*scope, &vmt_scope);

    E8_RETURN_OK;
}

e8_runtime_error *e8_scope_add(e8_scope scope, const e8_uchar *name, const e8_property *property, bool local)
{
    e8_scope_data *d = (e8_scope_data *)scope;

    e8_runtime_error *__err;
    if (( __err = e8_collection_add_size(&d->properties.data, 1) ))
        return __err;

    e8_named_property *np = &d->properties.data[d->properties.size - 1];
    np->property = *property;
    np->name = e8_utf_strdup(name);
    np->local = local;

    E8_RETURN_OK;
}
e8_runtime_error *e8_scope_destroy(e8_scope scope)
{
    e8_gc_free_object(scope);
    E8_RETURN_OK;
}

e8_runtime_error *
e8_scope_set_this_object(e8_scope scope, e8_object This)
{
    e8_scope_data *s = (e8_scope_data *)scope;
    s->this_object = This;
    s->this_vmt = e8_get_vtable(s->this_object);

    e8_gc_use_object(This);

    E8_RETURN_OK;
}
