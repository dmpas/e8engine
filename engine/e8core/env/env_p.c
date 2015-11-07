#include "env_p.h"
#include "e8core/plugin/plugin.h"
#include "_paths.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "unit.h"
#include "utils.h"

#define FILE_EXISTS(name) (access((name), R_OK) != -1)

/* Environment as Scope */

e8_vtable vmt_env;


e8_runtime_error*
__e8_env_get_property(e8_object obj, const e8_uchar *name, e8_property *property)
{
    e8_env_data *d = (e8_env_data *)obj;

    const e8_uchar *lname = name;

    int i;
    for (i = 0; i < d->properties.size; ++i) {
        if (e8_utf_stricmp(lname, d->properties.data[i].name) == 0) {
            *property = d->properties.data[i].property;
            E8_RETURN_OK;
        }
    }

    e8_vtable *vmt_global_scope = e8_get_vtable(d->global_scope);

    e8_runtime_error *__err;

    __err = vmt_global_scope->get_property(d->global_scope, name, property);

    return __err;
}

static E8_DECLARE_GET_METHOD(__e8_env_get_method)
{
    e8_env_data *d = (e8_env_data *)obj;

    const e8_uchar *lname = name;

    int i;
    for (i = 0; i < d->fn.size; ++i) {
        if (e8_utf_stricmp(lname, d->fn.data[i].lname) == 0) {
            *fn = d->fn.data[i].fn;
            return 0;
        }
    }

    E8_THROW_DETAIL(E8_RT_METHOD_NOT_FOUND, e8_utf_strdup(name));
}

void __e8_init_vmt_env()
{
    e8_vtable_template(&vmt_env);
    vmt_env.get_method = __e8_env_get_method;
    vmt_env.get_property = __e8_env_get_property;
}

/*! Определяет тип файла по расширению */
int
__e8_detect_filetype(const char *path)
{
    int l = strlen(path);
    while (l && path[l] != '.') --l;

    path += l;

    if (l > 4) {

        if (strcmp(path, ".e8s") == 0)
            return E8_FILE_SOURCE;

        if (strcmp(path, ".dll") == 0)
            return E8_FILE_COMPILED;

        if (strcmp(path, ".so") == 0)
            return E8_FILE_COMPILED;

        if (strcmp(path, ".e8c") == 0)
            return E8_FILE_BYTECODE;
    }

    return E8_FILE_AUTO;
}

static char*
__e8_env_try_path(const char *path, const char *prefix, const char *name, const char *ext)
{
    if (path) {

        char buf[MAX_PATH];

        if (path && strlen(path)) {
            if (ext) {
                sprintf(buf, "%s/%s%s.%s", path, prefix, name, ext);
            } else {
                sprintf(buf, "%s/%s%s", path, prefix, name);
            }
        } else {
            if (ext) {
                sprintf(buf, "%s%s.%s", prefix, name, ext);
            } else {
                sprintf(buf, "%s%s", prefix, name);
            }
        }

        if (FILE_EXISTS(buf)) {
            /* printf("found in %s\n", buf); */
            return __e8_strdup(buf);
        }
    }
    return 0;
}

static char *
__e8_env_search_unit_in_path_list(const char *name, e8_env_data *e, const char **paths, int N)
{
    //e8_env_data *e = (e8_env_data *)d->env;
    const char **p;

    int i = __e8_detect_filetype(name);

    if (i != E8_FILE_AUTO) {
        /* Однозначно задан тип файла */
        char *r;
        i = N;
        p = paths;
        while (i--) {
            if (*p) {

                if (( r = __e8_env_try_path(*p, "", name, 0) ))
                    return r;
                /* Заодно пробуем найти с префиксом lib (libe8std.so) */
                if (( r = __e8_env_try_path(*p, "lib", name, 0) ))
                    return r;

            }
            ++p;
        }
        return 0;
    }

    /* требуется поиск */
    #define ORDER_COUNT 3
    char *ext_order[ORDER_COUNT];

    switch (e->search_priority) {

    case E8_UNIT_PRIORITY_SOURCE: {
            ext_order[0] = "e8s";
            ext_order[1] = "e8c";
            ext_order[2] = SHARED_EXT;
            break;
        }

    case E8_UNIT_PRIORITY_COMPILED: {
            ext_order[0] = SHARED_EXT;
            ext_order[1] = "e8c";
            ext_order[2] = "e8s";
            break;
        }

    case E8_UNIT_PRIORITY_BYTECODE: {
            ext_order[0] = "e8c";
            ext_order[1] = SHARED_EXT;
            ext_order[2] = "e8s";
            break;
        }

    case E8_UNIT_PRIORITY_DEBUG: {
            ext_order[0] = "e8c";
            ext_order[1] = "e8s";
            ext_order[2] = SHARED_EXT;
            break;
        }
    }

    int j;
    for (j = 0; j < ORDER_COUNT; ++j) {

        char *r;
        i = N;
        p = paths;
        while (i--) {
            if (*p) {
                if (( r = __e8_env_try_path(*p, "", name, ext_order[j]) ))
                    return r;
                if (( r = __e8_env_try_path(*p, "lib", name, ext_order[j]) ))
                    return r;
            }
            ++p;
        }

    }

    return 0;
    #undef ORDER_COUNT
}

char *__e8_env_find_unit(const char *name, e8_unit_data *d, e8_env_data *env)
{
    e8_env_data *e = d ? ((e8_env_data *)d->env) : (env);

    #define PATHS_SIZE 4
    const char *paths[PATHS_SIZE] = {"", d ? d->root_path : "", e->root_path, e->bin_path};
    char *r;
    r = __e8_env_search_unit_in_path_list(name, e, paths, PATHS_SIZE);
    #undef PATHS_SIZE

    if (r)
        return r;

    if (e->ext_paths.size) {
        r = __e8_env_search_unit_in_path_list(name, e, (const char **)e->ext_paths.data, e->ext_paths.size);
        if (r)
            return r;
    }

    return 0;
}
