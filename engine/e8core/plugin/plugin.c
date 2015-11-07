#include "plugin.h"
#include <stdarg.h>
#include <malloc.h>

#define AALLOC(type, size) malloc( sizeof(type)*((size)) )

void e8_prepare_call_elements(e8_call_element *table, const char *encoding)
{
    while (table->fn) {
        e8_uchar *t;

        t = table->u_eng;
        e8_utf_from_8bit(table->a_eng, &t, encoding);
        e8_utf_lower_case(t);

        t = table->u_rus;
        e8_utf_from_8bit(table->a_rus, &t, encoding);
        e8_utf_lower_case(t);

        ++table;
    }
}

void e8_simple_get_method(const e8_uchar *name, const e8_call_element *table, e8_function_signature *fn)
{
    const e8_uchar *lname = name;

    *fn = 0;
    while (table->fn) {

        if ( e8_utf_stricmp(lname, table->u_eng) == 0 ) {
            *fn = table->fn;
            break;
        }

        if ( e8_utf_stricmp(lname, table->u_rus) == 0 ) {
            *fn = table->fn;
            break;
        }

        ++table;
    }
}

void e8_prepare_property_elements(e8_property_element *table, const char *encoding)
{
    while (table->index) {
        e8_uchar *t;

        t = table->u_eng;
        e8_utf_from_8bit(table->a_eng, &t, encoding);
        e8_utf_lower_case(t);

        t = table->u_rus;
        e8_utf_from_8bit(table->a_rus, &t, encoding);
        e8_utf_lower_case(t);

        ++table;
    }
}

int e8_find_simple_property(const e8_uchar *name, const e8_property_element *table)
{
    e8_uchar *lname = e8_utf_strdup(name);
    e8_utf_lower_case(lname);

    int found = 0;
    while (table->index) {

        if ( e8_utf_stricmp(lname, table->u_eng) == 0 ) {
            found = table->index;
            break;
        }

        if ( e8_utf_stricmp(lname, table->u_rus) == 0 ) {
            found = table->index;
            break;
        }

        ++table;
    }
    e8_utf_free(lname);
    return found;
}

e8_property *e8_arg_vlist(int argc, va_list list)
{
    e8_property *r = malloc(sizeof(e8_property)*argc);
    e8_property *p = r;


    while (argc--)
        e8_create_variant_property(p++, va_arg(list, e8_var*));

    return r;
}

e8_property *e8_arg_list(int argc, ...)
{
    va_list l;
    va_start(l, argc);

    e8_property *r = e8_arg_vlist(argc, l);

    va_end(l);

    return r;
}

e8_runtime_error *e8_simple_call(
                            const e8_function_signature         fn,
                            void                               *obj,
                            e8_var                             *result,
                            int                                 argc,
                            ...                                 /* e8_var[] */
                        )
{
    if (argc) {

        va_list l;
        va_start(l, argc);

        e8_runtime_error *__err = e8_simple_vcall(fn, obj, result, argc, l);

        va_end(l);

        return __err;
    }

    return e8_simple_vcall(fn, obj, result, argc, /*argv=*/0);
}

e8_runtime_error *e8_simple_pcall(
                            const e8_function_signature         fn,
                            void                               *obj,
                            e8_var                             *result,
                            int                                 argc,
                            e8_var                             *argv
                        )
{
    e8_property *r = 0;

    if (argc) {

        r = AALLOC(e8_property, argc);
        e8_property *p = r;

        int i;
        for (i = 0; i < argc; ++i) {
            e8_create_variant_property(p++, &argv[i]);
        }

    }

    e8_var tmpres;

    e8_runtime_error *__err = fn(obj, 0, argc, r, result ? result : &tmpres);

    if (!result)
        e8_var_destroy(&tmpres);

    if (r)
        free(r);

    return __err;
}

e8_runtime_error *e8_simple_vcall(
                            const e8_function_signature         fn,
                            void                               *obj,
                            e8_var                             *result,
                            int                                 argc,
                            va_list                             argv
                        )
{
    e8_property *r = 0;
    e8_var *vr = 0;

    if (argc) {

        r = AALLOC(e8_property, argc);
        e8_property *p = r;
        vr = AALLOC(e8_var, argc);
        e8_var *vri = vr;

        int i = argc;
        while (i--) {
            *vri = va_arg(argv, e8_var);
            e8_create_variant_property(p++, vri);
            ++vri;
        }

    }

    e8_var tmpres;

    e8_runtime_error *__err = fn(obj, 0, argc, r, result ? result : &tmpres);

    if (!result)
        e8_var_destroy(&tmpres);

    if (r)
        free(r);
    if (vr)
        free(vr);

    return __err;

}


e8_runtime_error *e8_simple_call_utf(
                            const char                         *fn_name,
                            void                               *obj,
                            e8_var                             *result,
                            int                                 argc,
                            e8_var                             *argv
                        )
{
    e8_property *r = 0;
    e8_var *vr = 0;

    if (argc) {

        r = AALLOC(e8_property, argc);
        e8_property *p = r;
        vr = AALLOC(e8_var, argc);

        int i = argc;
        while (i--)
            e8_create_variant_property(p++, argv++);

    }

    e8_vtable *vmt = e8_get_vtable(obj);
    e8_function_signature fn;
    e8_uchar *u_name = e8_utf_strcdup(fn_name, "utf-8");

    e8_runtime_error *__err;
    void *user_data;

    __err = vmt->get_method(obj, u_name, &fn, &user_data);
    e8_utf_free(u_name);

    if (__err)
        return __err;

    e8_var tmpres;

    __err = fn(obj, 0, argc, r, result ? result : &tmpres);

    if (!result)
        e8_var_destroy(&tmpres);

    if (r)
        free(r);
    if (vr)
        free(vr);

    return __err;

}

e8_var *e8_get_property_value(const e8_var *object, const char *utf_name, e8_var *result)
{
    e8_vtable *vmt = e8_get_vtable(object->obj);
    e8_property p;

    e8_uchar *uname = e8_utf_strcdup(utf_name, "utf-8");

    vmt->get_property(object->obj, uname, &p);

    e8_utf_free(uname);

    p.get(&p, result);
    return result;
}
