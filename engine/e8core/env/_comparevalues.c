#include "_comparevalues.h"
#include <malloc.h>

e8_vtable vmt_comparevalues;

#define _CV(name) e8_compare_values *name = (e8_compare_values *)obj;

int e8_compare_values_compare(const e8_compare_values *cv, const e8_var *value1, const e8_var *value2)
{
    E8_VAR(eq);

    int k = cv->invert ? -1 : 1;

    if (e8_var_eq(value1, value2, &eq) == 0)
        if (eq.bv)
            return 0;

    if (value1->type == value2->type) {

        if (value1->type == varObject) {
            if (value1->obj < value2->obj)
                return -k;
            else
                return k;
        } else if (value1->type == varBool) {
            if (value1->bv < value2->bv)
                return -k;
            else
                return k;
        } else {
            e8_var_le(value1, value2, &eq);
            if (eq.bv)
                return -k;
            else
                return k;
        }

    } else {

        if (value1->type < value2->type)
            return -k;
        else
            return k;
    }

}

static
E8_DECLARE_SUB(__comparevalues_compare)
{
    _CV(cv);

    E8_ARGUMENT(value1);
    E8_ARGUMENT(value2);

    e8_var_long(result, e8_compare_values_compare(cv, &value1, &value2));

    e8_var_destroy(&value1);
    e8_var_destroy(&value2);

    E8_RETURN_OK;
}

static
E8_METHOD_LIST_START(comparevalues_methods)
    E8_METHOD_LIST_ADD("Сравнить", "Compare", __comparevalues_compare)
E8_METHOD_LIST_END

static
E8_DECLARE_GET_METHOD(__comparevalues_get_method)
{
    e8_simple_get_method(name, comparevalues_methods, fn);
    E8_RETURN_OK;
}

#define _COMP(name) e8_compare_values *name = (e8_compare_values *)obj

static
E8_DECLARE_DESTRUCTOR(__comparevalues_destroy)
{
    _COMP(cv);
    if (!cv->built_in)
        free(cv);
}

static bool __init = false;

static void
__init_vmt()
{
    e8_prepare_call_elements(comparevalues_methods, "utf-8");

    e8_vtable_template(&vmt_comparevalues);
    vmt_comparevalues.dtor = __comparevalues_destroy;
    vmt_comparevalues.get_method = __comparevalues_get_method;

    __init = true;
}

E8_DECLARE_SUB(e8_comparevalues_new)
{
    if (!__init)
        __init_vmt();

    e8_compare_values *res = malloc(sizeof(*res));
    res->built_in = false;

    e8_gc_register_object(res, &vmt_comparevalues);

    e8_var_object(result, res);

    E8_RETURN_OK;
}
