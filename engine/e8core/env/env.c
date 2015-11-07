#include "env.h"
#include "unit.h"
#include "stack.h"
#include <malloc.h>
#include "e8core/variant/variant.h"
#include "macro.h"

#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>

#include "env_p.h"
#include "_paths.h"

#include "sharedlibs.h"
#include "_array.h"
#include "utils.h"



e8_runtime_error *
e8_env_execute(e8_unit unit, bool isolated)
{
    e8_unit_data *d = (e8_unit_data *)unit;

    e8_runtime_error *__err = e8_unit_execute(d);

    if (!isolated) {
        if (d->terminate_flag)
            ((e8_env_data*)d->env)->terminate_flag = true;

        if (d->exit_flag)
            ((e8_env_data*)d->env)->exit_flag = true;
    }

    return __err;
}

static int
__e8_search_type_by_name(e8_env_data *e, const e8_uchar *name)
{
    int i = e->types.size;
    while (i) {
        --i;
        if (e8_utf_stricmp(e->types.data[i].lname, name) == 0)
            return i;
    }

    return -1;
}

e8_function_signature e8_get_constructor(void *env, const char *utf_type)
{
    e8_env_data *e = (e8_env_data *)((e8_env *)env)->data;

    e8_uchar *lname = 0;
    e8_utf_from_8bit(utf_type, &lname, "utf-8");
    e8_utf_lower_case(lname);

    int i = __e8_search_type_by_name(e, lname);

    e8_utf_free(lname);

    if (i != -1)
        return e->types.data[i].fn;

    return 0;
}


E8_DECLARE_SUB(e8_new)
{
    e8_unit_data *d = (e8_unit_data *)obj;

    E8_ARGUMENT(v_type);
    e8_string *s_type;

    e8_var_cast_string(&v_type, &s_type);

    e8_env_data *e = (e8_env_data *)d->env;

    int i = __e8_search_type_by_name(e, s_type->s);

    e8_runtime_error *__err = 0;

    if (i == -1)
        __err = e8_throw_data(E8_RT_TYPE_NOT_FOUND, (void*)e8_utf_strdup(s_type->s));

    e8_string_destroy(s_type);
    e8_var_destroy(&v_type);

    if (i != -1) {
        e8_function_signature fn = e->types.data[i].fn;
        return fn((void*)d, user_data, argc - 1, argv, result);
    }

    return __err;
}

e8_environment e8_env_new()
{
    __e8_init_vmt_env();
    __e8_init_vmt_unit();

    e8_env_data *res = AALLOC(e8_env_data, 1);
    memset(res, 0, sizeof(e8_env_data));

    res->root_path = gnu_getcwd();

    E8_COLLECTION_INIT(res->fn);
    E8_COLLECTION_INIT(res->properties);
    E8_COLLECTION_INIT(res->types);
    E8_COLLECTION_INIT(res->ext_paths);
    E8_COLLECTION_INIT(res->free_handlers);

    e8_scope_new((e8_scope *)&res->global_scope, 0);
    e8_gc_use_object(res->global_scope);

    res->tr = e8_translator_new();

    e8_init_ticks(res);

    e8_gc_register_object(res, &vmt_env);

    e8_env_add_function_utf(res, "new", e8_new);
    e8_env_add_function_utf(res, "newobject", e8_new);
    e8_env_add_function_utf(res, "новый", e8_new);

    E8_VAR(v_unit_array);
    e8_array_new(0, 0, 0, 0, &v_unit_array);

    e8_env_add_global_value_utf(res, "Модули", &v_unit_array);
    e8_env_add_global_value_utf(res, "Units", &v_unit_array);

    e8_var_destroy(&v_unit_array);

    return (e8_environment)res;
}

void e8_env_close(e8_environment env)
{
    e8_env_data *d = (e8_env_data *)env;

    e8_translator_close(d->tr);

    if (d->root_path)
        free(d->root_path);

    if (d->bin_path)
        free(d->bin_path);

    e8_scope_destroy((e8_scope)d->global_scope);

    int i;

    e8_collection_free(&d->properties);
    e8_collection_free(&d->types);
    e8_collection_free(&d->fn);

    i = d->ext_paths.size;
    while (i--)
        free(d->ext_paths.data[i]);

    e8_collection_free(&d->ext_paths);

    if (d->output_encoding)
        free(d->output_encoding);

    i = d->free_handlers.size;
    while (i--)
        (d->free_handlers.data[i])(env);

    e8_collection_free(&d->free_handlers);

    free(d);
}

e8_unit e8_env_unit_new(e8_environment env, const char *name)
{
    return 0;
}

void e8_env_unit_search_priority(e8_environment env, int priority)
{
    e8_env_data *d = (e8_env_data *)env;
    d->search_priority = priority;
}

struct __r_el {
    FILE                       *f;
    char                       *buf;
    int                         pos;
    int                         size;
    int                         fsize;

    e8_parse_output             data;

    e8_uchar                   *text;

    bool                        r_flag;
    bool                        profile;
    FILE                       *out;

};

static void init_buffer(const char *path, struct __r_el *buf)
{
    buf->f = fopen(path, "rt");
    if (!buf->f)
        return;

    /* */
    fseek(buf->f, 0, SEEK_END);
    long sz = ftell(buf->f);
    fseek(buf->f, 0, SEEK_SET);
    /* */

    buf->buf = malloc(sz + 1);
    buf->text = e8_utf_malloc(sz);
    buf->pos = 0;
    buf->size = 0;
    buf->out = 0;
    buf->fsize = sz;
}

static void done_buffer(struct __r_el *buf)
{
    if (buf->buf) {
        free(buf->buf);
        buf->buf = 0;
    }
    buf->size = 0;
    if (buf->out)
        fclose(buf->out);
    if (buf->text)
        e8_utf_free(buf->text);
}

static
unsigned length(const unsigned char *buf)
{
    if (*buf < 0x80)
        return 1;
    if (*buf >> 5 == 6)
        return 2;
    if (*buf >> 4 == 14)
        return 3;
    if (*buf >> 3 == 30)
        return 4;
    if (*buf >> 2 == 62)
        return 5;
    if (*buf >> 1 == 126)
        return 6;

    return 0;
}

static
unsigned shift(const unsigned char *in, e8_uchar *out)
{
    unsigned l = length(in);

    switch (l) {
        case 1: {
            *out = *in;
            break;
        }
        case 2: {
            e8_uchar cp = *in;
            ++in;
            cp = ((cp << 6) & 0x7ff) + ((*in) & 0x3f);
            ++in;
            *out = cp;
            break;
        }
        case 3: {
            // (3 байта) 1110 xxxx 10xx xxxx 10xx xxxx
            int p1 = *in; ++in;
            int p2 = *in; ++in;
            int p3 = *in; ++in;
            e8_uchar cp = ((p1 & 0x0f) << 12) + ((p2 & 0x3f) << 6) + (p3  & 0x3f);
            *out = cp;
            break;
        }
        default: {
        }
    }
    return l;
}


static e8_uchar shift_utf_point(struct __r_el *buf)
{
    e8_uchar r = 0;
    unsigned l = shift((const unsigned char *)&buf->buf[buf->pos], &r);
    buf->pos += l;
    return r;
}


static void b_read_all(void *data)
{
    struct __r_el *d = (struct __r_el *)data;

    d->size = fread(d->buf, 1, d->fsize, d->f);

    int i;
    i = 0;
    e8_uchar *uc = d->text;
    while (d->pos < d->size) {
        *uc = shift_utf_point(d);
        ++uc;
        ++i;
    }
    while (i++ < d->size)
        *uc = 0;
}


e8_runtime_error *
e8_env_unit_from_source_file(e8_environment env, const char *path, e8_unit *unit)
{
    e8_env_data *d = (e8_env_data *)env;

    struct __r_el data;

    init_buffer(path, &data);
    b_read_all(&data);

    fclose(data.f);

    if (!data.f) {
        E8_THROW_DETAIL(E8_RT_FILE_READ_ERROR, __e8_strdup(path));
    }

    data.profile = d->profile_code;

    {
        e8_syntax_error *__err = e8_translator_parse(d->tr, &data.data, data.text, &data);

        done_buffer(&data);

        if (__err) {

            int i = data.data.commands_count;
            while (i--)
                __e8_command_destroy(&data.data.commands[i]);
            free(data.data.commands);
            E8_THROW_DETAIL(E8_RT_SYNTAX_ERROR, __err);
        }
    }

    e8_runtime_error *__err = e8_env_unit_from_commands(env, data.data.commands, data.data.commands_count, unit);

    free(data.data.commands);
    {
        int i;

        e8_unit_data *uu = (e8_unit_data *)*unit;

        uu->consts.data = data.data.consts;
        uu->consts.size = data.data.consts_count;
        uu->consts.reserved = uu->consts.size;

        e8_collection_init_size(/*DISVOLATILE*/(void*)&uu->subs, sizeof(e8_sub), data.data.symbols_count);
        for (i = 0; i < data.data.symbols_count; ++i) {
            uu->subs.data[i].unit = *unit;
            uu->subs.data[i].name = e8_utf_strdup(data.data.symbols[i].name->s);
            uu->subs.data[i].index = data.data.symbols[i].address;
            uu->subs.data[i]._export_item = data.data.symbols[i]._export_symbol;
        }

        uu->tables.data = data.data.tables;
        uu->tables.size = data.data.tables_count;
        uu->tables.reserved = data.data.tables_count;

        if (uu->root_path)
            free(uu->root_path);

        uu->root_path = __e8_strdup(path);
        extract_directory(path, uu->root_path);

        if (d->profile_code) {

            char profile_output_path[MAX_PATH];
            sprintf(profile_output_path, "%s.cd", path);

            uu->profile_output_path = __e8_strdup(profile_output_path);

        } else {
            uu->profile_output_path = 0;
        }

    }

    e8_gc_register_object(*unit, &vmt_unit);

    return __err;

}


e8_runtime_error *
e8_env_unit_from_file(e8_environment env, const char *path, int file_type, e8_unit *unit)
{
    if (file_type == E8_FILE_AUTO)
        file_type = __e8_detect_filetype(path);
    if (file_type == E8_FILE_AUTO)
        file_type = E8_FILE_SOURCE;


    switch (file_type) {

    case E8_FILE_SOURCE:
        return e8_env_unit_from_source_file(env, path, unit);

    case E8_FILE_COMPILED: {
        char *fname = __e8_env_find_unit(path, 0, env);

        if (!fname)
            E8_THROW_DETAIL(E8_RT_FILE_READ_ERROR, __e8_strdup(path));

        e8_runtime_error *__err = e8_env_unit_from_shared_library(env, fname, unit);

        free(fname);

        return __err;
    }

    default:
        E8_THROW_DETAIL(E8_RT_FILE_READ_ERROR, __e8_strdup(path));
    }

}

e8_runtime_error *
e8_env_unit_from_commands(e8_environment env, const e8_command *commands, int count, e8_unit *r_unit)
{
    e8_env_data *d = (e8_env_data *)env;

    e8_unit_data *unit = AALLOC(e8_unit_data, 1);
    E8_COLLECTION_INIT_SIZE(unit->commands, count);
    int i;
    for(i = 0; i < count; ++i)
        unit->commands.data[i] = commands[i];

    unit->env = env;
    unit->subs.size = 0;
    unit->subs.data = 0;

    if (d->profile_code) {

        long *__ticks = AALLOC(long, unit->commands.size);
        memset(__ticks, 0, sizeof(*__ticks) * unit->commands.size);
        unit->ticks = __ticks;

        long *__counts = AALLOC(long, unit->commands.size);
        memset(__counts, 0, sizeof(*__counts) * unit->commands.size);
        unit->counts = __counts;

    } else {
        unit->ticks = NULL;
        unit->counts = NULL;
    }

    e8_scope tmp;

    e8_scope_new(&tmp, 0);
    unit->scope = (e8_scope_data *)tmp;

    e8_scope_new(&tmp, d->global_scope);
    unit->scope->parent = (e8_scope_data *)tmp;
    unit->vmt_scope = &vmt_scope;

    e8_gc_use_object(unit->scope->parent);
    e8_gc_use_object(unit->scope);

    if (d->root_path)
        unit->root_path = __e8_strdup( d->root_path );
    else
        unit->root_path = 0;

    void *v_unit = (void*)unit;
    e8_gc_register_object(v_unit, &vmt_unit);
    if (r_unit)
        *r_unit = (e8_unit)unit;

    E8_RETURN_OK;
}

e8_runtime_error *
e8_env_add_function(e8_environment env, const e8_uchar *name, e8_function_signature fn)
{
    e8_env_data *d = (e8_env_data *)env;

    e8_collection_add_size(&d->fn, 1);
    e8_fn_struct* f = &d->fn.data[d->fn.size-1];
    f->fn = fn;
    e8_utf_strcpy(f->lname, name);
    e8_utf_lower_case(f->lname);

    E8_RETURN_OK;
}

e8_runtime_error *e8_env_add_function_utf(
                        e8_environment          env,
                        const char             *name,
                        e8_function_signature   fn
            )
{
    e8_uchar *uname = e8_utf_strcdup(name, "utf-8");
    e8_runtime_error *__err = e8_env_add_function(env, uname, fn);
    e8_utf_free(uname);
    return __err;
}


e8_runtime_error *
e8_env_add_global_value(e8_environment env, const e8_uchar *name, const e8_var *value)
{
    e8_env_data *d = (e8_env_data *)env;

    e8_var *nvalue = AALLOC(e8_var, 1);
    e8_var_undefined(nvalue);
    e8_var_assign(nvalue, value);

    e8_property p;
    e8_create_variant_property(&p, nvalue);
    p.can_set = false;
    e8_scope_add(d->global_scope, name, &p, true);

    E8_RETURN_OK;
}

e8_runtime_error *
e8_env_add_global_value_utf(
                        e8_environment          env,
                        const char             *name,
                        const e8_var           *value
            )
{
    e8_uchar *uname = e8_utf_strcdup(name, "utf-8");
    e8_runtime_error *__err = e8_env_add_global_value(env, uname, value);
    e8_utf_free(uname);
    return __err;
}


e8_runtime_error *
e8_env_add_type(e8_environment env, const e8_uchar *name, e8_function_signature fn)
{
    e8_env_data *d = (e8_env_data *)env;

    e8_collection_add_size(&d->types, 1);
    e8_typedata *t = &d->types.data[d->types.size - 1];
    e8_utf_strcpy(t->lname, name);
    e8_utf_lower_case(t->lname);

    t->fn = fn;

    E8_RETURN_OK;
}

e8_runtime_error *
e8_env_add_type_utf(e8_environment env, const char *name, e8_function_signature fn)
{
    e8_uchar *uname = e8_utf_strcdup(name, "utf-8");

    e8_runtime_error *__err = e8_env_add_type(env, uname, fn);

    e8_utf_free(uname);

    return __err;
}


e8_runtime_error *
e8_env_unit_free(e8_unit unit)
{
    e8_gc_free_object(unit);
    E8_RETURN_OK;
}

e8_translator* e8_env_get_translator(e8_environment env)
{
    e8_env_data *d = (e8_env_data *)env;
    return d->tr;
}

e8_runtime_error *e8_env_set_default_input_encoding(
                        e8_environment          env,
                        const char             *encoding
            )
{
    E8_RETURN_OK;
}

e8_runtime_error *e8_env_set_root_path(
                        e8_environment          env,
                        const char             *path
            )
{
    e8_env_data *d = (e8_env_data *)env;

    if (d->root_path)
        free(d->root_path);

    d->root_path = __e8_strdup(path);

    E8_RETURN_OK;
}

void e8_env_set_debug_flag(
                        e8_environment          env,
                        bool                    flag
            )
{
    e8_env_data *d = (e8_env_data *)env;
    d->debug_flag = flag;
}

bool e8_env_get_debug_flag(
                        e8_environment          env
            )
{
    e8_env_data *d = (e8_env_data *)env;
    return d->debug_flag;
}

e8_runtime_error *
e8_env_add_freelib_handler(e8_environment env, e8_env_freelib_t handler)
{
    e8_env_data *d = (e8_env_data *)env;
    e8_collection_add_size(&d->free_handlers, 1);
    d->free_handlers.data[d->free_handlers.size - 1] = handler;
    E8_RETURN_OK;
}

void e8_env_set_bin_path(
                        e8_environment          env,
                        const char             *path
                        )

{
    e8_env_data *d = (e8_env_data *)env;
    if (d->bin_path)
        free(d->bin_path);
    d->bin_path = __e8_strdup(path);
}

void e8_env_add_ext_path(
                        e8_environment          env,
                        const char             *path
                    )
{
    e8_env_data *d = (e8_env_data *)env;
    e8_collection_add_size(&d->ext_paths, 1);

    d->ext_paths.data[d->ext_paths.size - 1] = __e8_strdup(path);
}


const char *
e8_env_get_output_encoding(e8_environment env)
{
    e8_env_data *d = (e8_env_data *)env;
    return d->output_encoding;
}

void
e8_env_set_output_encoding(e8_environment env, const char *encoding)
{
    e8_env_data *d = (e8_env_data *)env;
    if (d->output_encoding)
        free(d->output_encoding);
    d->output_encoding = __e8_strdup(encoding);
}

void e8_env_apply_PATH(e8_environment env, const char *PATH_VAR)
{
    char *v = PATH_VAR ? getenv(PATH_VAR) : getenv("PATH");
    char *tmp = malloc(strlen(v) + 1);

    while (*v) {

        char *stmp = tmp;
        while (*v != 0 && *v != ';')
            *stmp++ = *v++;
        *stmp = 0;

        if (*v != 0)
            ++v;

        e8_env_add_ext_path(env, tmp);
    }
    free(tmp);
}

void
e8_env_set_profile_flag(e8_environment env, bool flag)
{
    e8_env_data *d = (e8_env_data *)env;
    d->profile_code = flag;
}

bool
e8_env_get_profile_flag(e8_environment env)
{
    e8_env_data *d = (e8_env_data *)env;
    return d->profile_code;
}
