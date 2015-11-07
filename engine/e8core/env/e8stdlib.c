#include "e8stdlib.h"
#include "e8core/plugin/plugin.h"
#include "env_p.h"
#include "env.h"
#include "unit.h"
#include "utils.h"

#include <stdio.h>
#include <malloc.h>
#include <time.h>

#include "_array.h"
#include "_map.h"
#include "_numerics.h"
#include "_strings.h"
#include "_valuelist.h"
#include "_command.h"
#include "_valuetable.h"
#include "sharedlibs.h"

#ifdef __WINNT
#include <windows.h>
#endif // __WINNT

#ifdef __linux
#include <unistd.h>
#endif // __linux

static char *
__get_tempdir()
{

    #ifdef __WINNT
    return __e8_strdup(getenv("TEMP"));
    #endif

    return __e8_strdup("/tmp");

}

E8_DECLARE_SUB(e8_sleep)
{
    E8_ARG_LONG(time);

    #ifdef __WINNT
    Sleep(time);
    #endif // __WINNT

    #ifdef __linux

    struct timespec tim, tim2;
    memset(&tim, 0, sizeof(tim));
    tim.tv_sec = time / 1000;
    tim.tv_nsec = (time % 1000) * 1000000L;

    nanosleep(&tim , &tim2);
    #endif // __linux

    E8_RETURN_OK;
}

E8_DECLARE_SUB(e8_temp_dir)
{
    char *res = __get_tempdir();
    e8_var_string_utf(result, res); /* TODO: Env-encoding */
    free(res);
    E8_RETURN_OK;
}

E8_DECLARE_SUB(e8_bin_dir)
{
    e8_unit_data *d = (e8_unit_data *)obj;
    e8_env_data *e = (e8_env_data *)d->env;

    if (e->bin_path)
        e8_var_string_utf(result, e->bin_path); // TODO: Env-encoding
    else
        e8_var_string_utf(result, "");

    E8_RETURN_OK;
}

#ifndef E8_NO_THREADS

static bool mutex_init = false;
static pthread_mutex_t mutex_output;

static void init_mutex()
{
    mutex_init = true;
    pthread_mutex_init(&mutex_output, NULL);
}

#define CHECK_MULTY_THREADS if (!mutex_init) init_mutex();
#define LOCK(name) pthread_mutex_lock(&mutex_##name)
#define UNLOCK(name) pthread_mutex_unlock(&mutex_##name)


#else

#define CHECK_MULTY_THREADS
#define LOCK(name)
#define UNLOCK(name)

#endif // E8_NO_THREADS

E8_DECLARE_SUB(e8_threads_enabled)
{
    #ifdef E8_NO_THREADS
        e8_var_bool(result, false);
    #else
        e8_var_bool(result, true);
    #endif // E8_NO_THREADS

    E8_RETURN_OK;
}

static int __indent = 0;
static e8_var __indent_value;

E8_DECLARE_SUB(e8_indent)
{
    CHECK_MULTY_THREADS;
    LOCK(output);

    E8_ARG_LONG(l_indent);
    if (l_indent >= 0 && l_indent <= 10)
        __indent = l_indent;

    UNLOCK(output);

    E8_RETURN_OK;
}

E8_DECLARE_SUB(e8_indent_value)
{
    e8_var_assign(result, &__indent_value);

    if (argc) {
        E8_ARGUMENT(new_value);
        e8_var_assign(&__indent_value, &new_value);
        e8_var_destroy(&new_value);
    }

    E8_RETURN_OK;
}

typedef struct __msg_line {
    e8_unit_data       *unit;
    e8_string          *str;
} __msg_line;


static void
__output_string(const e8_string *s, const e8_unit_data *d)
{
    if (s->len) {

        char *ascii = 0;
        e8_utf_to_8bit(s->s, &ascii, ((e8_env_data*)d->env)->output_encoding);
        printf("%s", ascii);

        free(ascii);
    }
}

static void
__output_value(__msg_line *ln, const e8_var *value, const e8_unit_data *d)
{
    e8_string *s;
    e8_var_cast_string(value, &s);

    if (ln) {

        /* копим строку */
        e8_string *tmp = ln->str;
        ln->str = e8_string_concat(ln->str, s);
        e8_string_destroy(tmp);

    } else {

        /* выводим сразу */
        __output_string(s, d);

    }
    e8_string_destroy(s);
}

static e8_vtable vmt_message_line;
static e8_uchar u_MessageLine[] = {'M', 'e', 's', 's', 'a', 'g', 'e', 'L', 'i', 'n', 'e', 0};

static
E8_DECLARE_TO_STRING(__message_line_to_string)
{
    return u_MessageLine;
}

static
E8_DECLARE_SUB(__message_line_message)
{
    LOCK(output);

    __msg_line *ln = (__msg_line *)obj;
    e8_unit_data *d = ln->unit;

    while (argc--) {

        E8_ARGUMENT(p);
        __output_value(ln, &p, d);
        e8_var_destroy(&p);

    } // argc

    UNLOCK(output);

    e8_var_object(result, obj);

    E8_RETURN_OK;
}

static void
__message_line_flush_nolock(__msg_line *ln)
{
    __output_string(ln->str, ln->unit);
    fflush(stdout);

    e8_string_destroy(ln->str);
}

static
E8_DECLARE_SUB(__message_line_flush)
{
    __msg_line *ln = (__msg_line *)obj;
    LOCK(output);

    __message_line_flush_nolock(ln);
    ln->str = e8_string_ascii("");

    if (result)
        e8_var_object(result, ln);

    UNLOCK(output);
    E8_RETURN_OK;
}

static
E8_DECLARE_DESTRUCTOR(__message_line_destroy)
{

    __msg_line *ln = (__msg_line *)obj;

    LOCK(output);
    __message_line_flush_nolock(ln);

    printf("\n");
    fflush(stdout);

    UNLOCK(output);

    free(ln);
}

static
E8_METHOD_LIST_START(__message_line_methods)
    E8_METHOD_LIST_ADD("Сообщить", "Message", __message_line_message)
    E8_METHOD_LIST_ADD("Вывести", "Flush", __message_line_flush)
E8_METHOD_LIST_END

static
E8_DECLARE_GET_METHOD(__message_get_method)
{
    e8_simple_get_method(name, __message_line_methods, fn);
    E8_RETURN_OK;
}

E8_DECLARE_SUB(e8_message)
{
    CHECK_MULTY_THREADS;

    e8_unit_data *d = (e8_unit_data *)obj;

    __msg_line *R = malloc(sizeof(*R));
    R->unit = d;
    R->str = e8_string_ascii("");

    if (argc) {

        /* Вывести отступ */
        int i = __indent;
        while (i--)
            __output_value(R, &__indent_value, d);
    }

    e8_gc_register_object(R, &vmt_message_line);

    return __message_line_message(R, NULL, argc, argv, result);

}

E8_DECLARE_SUB(e8_value_is_filled)
{
    E8_ARGUMENT(value);

    switch (value.type) {
        case varNull: e8_var_bool(result, false);
            break;
        case varDateTime: e8_var_bool(result, value.date != 0);
            break;
        case varNumeric:
            {
                if (value.num.type == numInt)
                    e8_var_bool(result, value.num.l_value != 0);
                else
                    e8_var_bool(result, value.num.d_value != 0);
            }
            break;
        case varString:
            {
                e8_string *vs;
                e8_var_cast_string(&value, &vs);
                vs = e8_string_trim(vs, true, true);
                e8_var_bool(result, vs->len != 0);
                e8_string_destroy(vs);
            }
            break;
        case varBool: e8_var_bool(result, value.bv);
            break;
        default:
            e8_var_bool(result, true);
    }

    e8_var_destroy(&value);
    E8_RETURN_OK;
}


#ifndef FULL_STATIC
E8_DECLARE_SUB(e8_attach_unit)
{
    E8_ARGUMENT(v_name);

    e8_string *s_name;
    e8_var_cast_string(&v_name, &s_name);

    e8_var_destroy(&v_name);

    char *buf = 0;
    #ifdef __WINNT
    e8_utf_to_8bit(s_name->s, &buf, "cp1251");
    #else
    e8_utf_to_8bit(s_name->s, &buf, "utf-8");
    #endif

    e8_string_destroy(s_name);

    e8_unit_data *d = 0;
    e8_env_data *e = 0;

    e8_vtable *m_vmt_obj = e8_get_vtable(obj);
    if (m_vmt_obj == &vmt_env)
        e = (e8_env_data *)obj;
    else
        d = (e8_unit_data *)obj;

    char *fname = __e8_env_find_unit(buf, d, e);

    if (!fname)
        E8_THROW_DETAIL(E8_RT_FILE_READ_ERROR, buf);

    free(buf);

    e8_runtime_error *__err;
    e8_unit u;

    if ((
         __err = e8_env_unit_from_file(
                                        (d != NULL) ? (d->env) : (e8_environment)e,
                                       fname,
                                       E8_FILE_AUTO, &u
                                    )
         ))
        return __err;

    char *cwd = gnu_getcwd();

    char *mpath = strdup(fname);
    size_t l = strlen(mpath);
    if (l) {

        while (--l) {

            if (mpath[l] == '/' || mpath[l] == '\\') {
                chdir(mpath);
                break;
            }

            mpath[l] = 0;
        }

    }
    free(mpath);

    free(fname);

    bool isolated_call = false;
    if (argc > 1) {
        E8_ARG_BOOL(b_isolated);
        isolated_call = b_isolated;
    }

    e8_var_object(result, u);

    if (e8_get_vtable(u) != &vmt_shared)
        if (( __err = e8_env_execute(u, isolated_call) )) {
            e8_var_destroy(result);
            chdir(cwd);
            free(cwd);
            return __err;
        }

    if (!isolated_call && d != 0) {
        e8_vtable *uvt = e8_get_vtable(u);
        if (uvt != &vmt_shared) {
            e8_unit_data *ud = (e8_unit_data *)u;

            if (ud->exit_flag)
                d->exit_flag = true;

            if (ud->terminate_flag)
                d->terminate_flag = true;
        }
    }

    chdir(cwd);
    free(cwd);

    E8_RETURN_OK;
}
#endif // FULL_STATIC

E8_DECLARE_SUB(e8_global_unit)
{
    e8_unit_data *d = 0;
    e8_env_data *e = 0;

    e8_vtable *m_vmt_obj = e8_get_vtable(obj);
    if (m_vmt_obj == &vmt_env)
        e = (e8_env_data *)obj;
    else {
        d = (e8_unit_data *)obj;
        e = (e8_environment)d->env;
    }

    E8_ARGUMENT(v_unit);
    E8_ARG_STRING(s_name);

    e8_runtime_error *__err = e8_env_add_global_value(e, s_name->s, &v_unit);

    e8_string_destroy(s_name);
    e8_var_assign(result, &v_unit);
    e8_var_destroy(&v_unit);

    return __err;;
}

E8_DECLARE_SUB(e8_exit)
{
    e8_unit_data *d = (e8_unit_data *)obj;

    d->exit_flag = true;

    E8_RETURN_OK;
}

E8_DECLARE_SUB(e8_terminate)
{
    e8_unit_data *d = (e8_unit_data *)obj;

    d->terminate_flag = true;

    E8_RETURN_OK;
}

E8_DECLARE_SUB(e8_current_date)
{
    time_t t = time(NULL);
    struct tm *ts = localtime(&t);

    e8_date d;

    e8_date_construct(ts->tm_year + 1900, ts->tm_mon + 1, ts->tm_mday,
                        ts->tm_hour, ts->tm_min, ts->tm_sec,
                        &d
    );

    e8_var_date(result, d);
    E8_RETURN_OK;
}

static void
__e8_stdio_free(const e8_environment env)
{
    e8_var_destroy(&__indent_value);
}


#define REGISTER(name, fn) e8_env_add_function_utf(env, name, fn);

void e8_register_stdio(e8_environment env)
{

    e8_prepare_call_elements(__message_line_methods, "utf-8");
    e8_vtable_template(&vmt_message_line);

    vmt_message_line.dtor = __message_line_destroy;
    vmt_message_line.to_string = __message_line_to_string;
    vmt_message_line.get_method = __message_get_method;

    e8_var_string_utf(&__indent_value, " ");

    REGISTER("Message",             e8_message);
    REGISTER("Сообщить",            e8_message);

    REGISTER("Indent",              e8_indent);
    REGISTER("Отступ",              e8_indent);

    REGISTER("IndentValue",         e8_indent_value);
    REGISTER("ЗначениеОтступа",     e8_indent_value);

    #ifndef FULL_STATIC
    REGISTER("AttachUnit",          e8_attach_unit);
    REGISTER("dl",                  e8_attach_unit); /* Для совместимости с 0.2.4 */
    REGISTER("ПодключитьМодуль",    e8_attach_unit);

    REGISTER("DeclareCommonUnit",   e8_global_unit);
    REGISTER("ОбъявитьОбщийМодуль", e8_global_unit);
    #endif

    REGISTER("ТекущаяДата",         e8_current_date);
    REGISTER("CurrentDate",         e8_current_date);

    #ifndef FULL_STATIC
    REGISTER("TempDir",             e8_temp_dir);
    REGISTER("КаталогВременныхФайлов",e8_temp_dir);
    #endif // FULL_STATIC

    REGISTER("BinDir",              e8_bin_dir);
    REGISTER("КаталогПрограммы",    e8_bin_dir);

    REGISTER("Ждать",               e8_sleep);
    REGISTER("Спать",               e8_sleep);
    REGISTER("Sleep",               e8_sleep);

    e8_env_add_freelib_handler(env, __e8_stdio_free);
}

#define REGISTER_TYPE(name, fn) e8_env_add_type_utf(env, name, fn);

void e8_register_stdlib(e8_environment env)
{
    REGISTER_TYPE("Array",                      e8_array_new);
    REGISTER_TYPE("Массив",                     e8_array_new);
    REGISTER_TYPE("FixedArray",                 e8_fixed_array_new);
    REGISTER_TYPE("ФиксированныйМассив",        e8_fixed_array_new);

    REGISTER_TYPE("Соответствие",               e8_map_new);
    REGISTER_TYPE("Структура",                  e8_structure_new);
    REGISTER_TYPE("ФиксированноеСоответствие",  e8_fixed_map_new);
    REGISTER_TYPE("ФиксированнаяСтруктура",     e8_fixed_structure_new);

    REGISTER_TYPE("Map",                        e8_map_new);
    REGISTER_TYPE("Structure",                  e8_structure_new);
    REGISTER_TYPE("FixedMap",                   e8_map_new);
    REGISTER_TYPE("FixedStructure",             e8_structure_new);

    REGISTER_TYPE("ValueList",                  e8_valuelist_new);
    REGISTER_TYPE("СписокЗначений",             e8_valuelist_new);

    REGISTER_TYPE("ValueTable",                 e8_valuetable_new);
    REGISTER_TYPE("ТаблицаЗначений",            e8_valuetable_new);

    REGISTER("ЗавершитьРаботуСистемы",          e8_exit);
    REGISTER("Exit",                            e8_exit);

    REGISTER("ПрекратитьРаботуСистемы",         e8_terminate);
    REGISTER("Terminate",                       e8_terminate);

    REGISTER("ЗначениеЗаполнено",               e8_value_is_filled);
    REGISTER("ValueIsFilled",                   e8_value_is_filled);

    REGISTER("РазрешеныПотоки",                 e8_threads_enabled);
    REGISTER("ThreadsEnabled",                  e8_threads_enabled);

    REGISTER_TYPE("Действие",                        e8_action_command_new);
    REGISTER_TYPE("Action",                          e8_action_command_new);


    e8_register_numerics(env);
    e8_register_strings(env);
}

#undef REGISTER
#undef REGISTER_TYPE
