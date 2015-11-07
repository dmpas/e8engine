#include "sharedlibs.h"
#include "macro.h"
#include <malloc.h>
#include "env_p.h"
#include "_array.h"
#include "utils.h"

#ifdef __WINNT
#include <windows.h>
#endif // __WINNT

#ifdef __linux
#include <dlfcn.h>
#endif // __linux

#include "e8core/plugin/plugin.h"

typedef struct __shared_struct {
    #ifdef __WINNT
    HMODULE     handle;
    #endif // __WINNT

    #ifdef __linux
    void       *handle;
    #endif // __linux

    e8_env      E;
} e8_shared_struct;

static bool init = false;
e8_vtable vmt_shared;

static void
__e8_shared_destroy(e8_object obj)
{

    e8_shared_struct *d = (e8_shared_struct *)obj;

    #ifdef __WINNT
    FreeLibrary(d->handle);
    #endif // __WINNT

    #ifdef __linux
    dlclose(d->handle);
    #endif // __linux

    free(d);
}

static void *
__e8_shared_sym(const e8_shared_struct *d, const char *sym)
{
    #ifdef __WINNT
    return (void*)GetProcAddress(d->handle, sym);
    #endif // __WINNT

    #ifdef __linux
    return dlsym(d->handle, sym);
    #endif // __linux

    return 0;
}

static void
__init_vmt()
{
    e8_vtable_template(&vmt_shared);

    vmt_shared.dtor = __e8_shared_destroy;

    init = true;
}


static int
__e8_register_global_type(/* e8_env * */void *env, const char *utf_name, e8_type_info *type)
{
    e8_environment d = ((e8_env*)env)->data;

    e8_env_add_type_utf(d, utf_name, type->constructor);

    return 0;
}
static int
__e8_register_global_function(/* e8_env * */void *env, const char *utf_name, e8_function_signature fn)
{
    e8_environment d = ((e8_env*)env)->data;

    e8_env_add_function_utf(d, utf_name, fn);

    return 0;
}

static int
__e8_register_global_property(/* e8_env * */void *env, const char* utf_name, e8_property *property)
{

    e8_env_data *d = ((e8_env*)env)->data;

    e8_uchar *name = 0;
    e8_utf_from_8bit(utf_name, &name, "utf-8");

    e8_scope_add(d->global_scope, name, property, false);

    e8_utf_free(name);

    return 0;
}

static e8_env_vtable env_table = {
        __e8_register_global_function, __e8_register_global_property,
        __e8_register_global_type, e8_get_constructor
};


e8_runtime_error *
e8_env_unit_from_shared_library(e8_environment env, const char *path, e8_unit *unit)
{

    if (!init)
        __init_vmt();


    e8_shared_struct *d = AALLOC(e8_shared_struct, 1);
    memset(d, 0, sizeof(*d));

    #ifdef __WINNT
    d->handle = LoadLibrary(path);
    #endif // __WIN

    #ifdef __linux
    d->handle = dlopen(path, RTLD_NOW | RTLD_DEEPBIND);

    if (!d->handle)
        E8_THROW_DETAIL(E8_RT_FILE_READ_ERROR, __e8_strdup(dlerror()));
    #endif // __linux

    e8_plugin_entry entry = __e8_shared_sym(d, "e8_entry");
    if (!entry) {
        free(d);
        char buf[256];
        sprintf(buf, "no entry: %s", path);
        E8_THROW_DETAIL(E8_RT_FILE_READ_ERROR, __e8_strdup(buf));
    }

    d->E.vmt = &env_table;
    d->E.data = env;

    entry(&d->E);

    e8_gc_register_object(d, &vmt_shared);

    e8_var v_env, v_units, v_unit;

    /* Формируем вариант самостоятельно, чтобы исключиться из подсчёта ссылок */
    v_env.type = varObject;
    v_env.obj = env;

    e8_var_object(&v_unit, d);
    e8_var_undefined(&v_units);

    e8_get_property_value(&v_env, "Units", &v_units);

    e8_array_add_nvalues(&v_units, 1, v_unit);

    e8_var_destroy(&v_units);
    e8_var_destroy(&v_unit);

    if (unit)
        *unit = d;

    E8_RETURN_OK;
}


