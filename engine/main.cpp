#include <locale.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include "version.h"

extern "C" {

#include "e8core/env/env.h"
#include "e8core/env/options.h"
#include "e8core/env/e8stdlib.h"
#include "e8core/variant/variant.h"
#include "e8core/utf/utf.h"
#include "e8core/env/_array.h"
#include <errno.h>

}

#ifdef __WINNT
#include <windows.h>
#endif // __WINDOWS

static bool
get_file_name(int argc, char **argv, const char **fname, int *fname_index)
{
    *fname_index = 1;
    while (*fname_index < argc) {
        if (argv[*fname_index][0] == '-'
                && argv[*fname_index][1] == '-')
            ++*fname_index;
        else {
            *fname = argv[*fname_index];
            return true;
        }
    }
    *fname_index = 0;
    return false;
}

void VERSION()
{
    printf("%s rev %s", PRODUCT_VERSION, PRODUCT_REVISION);
}

void USAGE()
{
    printf("E8::Script ");
    VERSION();
    printf("\n");

    printf(
        "USAGE: e8script [PARAMS] filename [script-params]\n"
        "Available params:\n"
        "\t--profile                out profile code\n"
        "\t--no-run                 do not execute file, just compile (and profile)\n"
        "\t--dl=<unit>              attach unit before script compiling\n"
        "\t--ext-path=<path>        add path to search units\n"
        "\t--input-encoding=<enc>   set encoding of input files\n"
        "\t--output-encoding=<enc>  set encoding of standart output\n"
        "Preprocessor params:\n"
        "\t--server\n"
        "\t--thin-client\n"
        "\t--web-client\n"
        "\t--thick-client-managed-application\n"
        "\t\t--thick-managed\n"
        "\t--thick-client-ordinary-application\n"
        "\t\t--thick-ordinary\n"
        "\t--no-script\n"
        "\t--strict-syntax\n"
    );
}

inline bool
exists(const char *path)
{
    return access(path, R_OK) != -1;
}

static void
settings_from_file(const char *path, e8_environment env, e8_translator tr)
{

    if (!exists(path))
        return;

    FILE *f = fopen(path, "r");

    #define MAX_BUF 100

    char s[MAX_BUF], enc[MAX_BUF];
    if (fgets(enc, MAX_BUF, f)) {

        while (fgets(s, MAX_BUF, f)) {
            int l = strlen(s);
            if (s[0] == '#')
                continue;

            if (l)
                while (s[l - 1] == '\n' || s[l - 1] == '\r') {
                    s[--l] = 0;
                    if (!l)
                        break;
                }
            e8_runtime_error *__err = e8_option_apply_cli_option(s, env, tr);
            if (__err)
                e8_free_exception(__err);
        }

    }

    fclose(f);

    #undef MAX_BUF
}

static char *
gnu_getcwd ()
{
    size_t size = 100;
    while (1) {

        char *buffer = (char *) malloc (size);

        if (!buffer)
            return 0;

        if (getcwd (buffer, size) == buffer)
            return buffer;

        free (buffer);

        if (errno != ERANGE)
            return 0;

        size *= 2;
    }
}

static void
print_error(const e8_runtime_error *__err, const char *enc)
{
    int res = __err->error_no;
    fprintf(stderr, "Error (%d) at offset (%d)\n", res, __err->command_index);

    if (res == E8_RT_METHOD_NOT_FOUND) {
        e8_uchar *method = (e8_uchar *)__err->data;

        if (method) {
            char *t = 0;
            e8_utf_to_8bit(method, &t, enc);
            fprintf(stderr, "Method not found: %s\n", t);
            free(t);
        }
    }
    if (res == E8_RT_ALLOCATION_ERROR) {
        fprintf(stderr, "Allocation error. Not enough memory.");
    }
    if (res == E8_RT_EXCEPTION_RAISED) {
        char *utf = (char *)__err->data;
        e8_uchar *us = 0;
        e8_utf_from_8bit(utf, &us, "utf-8");
        //free(utf);
        utf = 0;

        e8_utf_to_8bit(us, &utf, enc);

        fprintf(stderr, "Exception: %s\n", utf);
        e8_utf_free(us);
        free(utf);
    }

    if (res == E8_RT_TYPE_NOT_FOUND) {
        e8_uchar *method = (e8_uchar *)__err->data;

        if (method) {
            char *t = 0;
            e8_utf_to_8bit(method, &t, enc);
            fprintf(stderr, "Type not found: %s\n", t);
            free(t);
        }
    }

    if (res == E8_RT_FILE_READ_ERROR) {
        char *fname = (char *)__err->data;
        if (fname) {
            fprintf(stderr, "File access error: %s\n", fname);
            free(fname);
        } else
            fprintf(stderr, "File access error.\n");
    }

    if (res == E8_RT_NOT_AN_OBJECT) {
        e8_uchar *udata = (e8_uchar *)__err->data;
        char *t = 0;
        e8_utf_to_8bit(udata, &t, enc);
        fprintf(stderr, "Value not an object: %s\n", t);
        free(t);
    }

    if (res == E8_RT_PROPERTY_NOT_FOUND) {
        e8_uchar *udata = (e8_uchar *)__err->data;
        char *t = 0;
        e8_utf_to_8bit(udata, &t, enc);
        fprintf(stderr, "Property not found: %s\n", t);
        free(t);
    }
    if (res == E8_RT_ASSERTION_FAILED) {
        e8_uchar *udata = (e8_uchar *)__err->data;
        if (udata) {
            char *t = 0;
            e8_utf_to_8bit(udata, &t, enc);
            fprintf(stderr, "Assertion failed: %s\n", t);
            free(t);
        } else
            fprintf(stderr, "Assertion failed!\n");
    }
    if (res == E8_RT_CANNOT_CAST_TO_NUMERIC) {
        e8_uchar *udata = (e8_uchar *)__err->data;
        if (udata) {
            char *t = 0;
            e8_utf_to_8bit(udata, &t, enc);
            fprintf(stderr, "Value cannot be cast to numeric: %s\n", t);
            free(t);
        } else
            fprintf(stderr, "Cannot cast to numeric\n");
    }
    if (res == E8_RT_NOT_A_COLLECTION) {
        e8_uchar *udata = (e8_uchar *)__err->data;
        if (udata) {
            char *t = 0;
            e8_utf_to_8bit(udata, &t, enc);
            fprintf(stderr, "Error: %s\n", t);
            free(t);
        } else
            fprintf(stderr, "Value not a collection\n");
    }
    if (res == E8_RT_SYNTAX_ERROR) {

        e8_syntax_error *s_err = (e8_syntax_error *)__err->data;

        printf("Syntax error");
        if (s_err)
            printf(": (%d) at line (%lu)", s_err->error, s_err->line_no);

        printf("\n");
    }
    if (res == E8_RT_INDEX_OUT_OF_BOUNDS) {
        printf("Index out of bounds");
        printf("\n");
    }
}

static char *
extract_dir(const char *fname)
{
    char *fdir = strdup(fname);
    int l = strlen(fdir);
    while (l) {
        --l;
        if (fdir[l] == '/' || fdir[l] == '\\') {
            fdir[l] = 0;
            break;
        }
        fdir[l] = 0;
    }
    return fdir;
}

static char *
get_global_config_path()
{
    #ifdef __linux
    return strdup("/etc/e8script");
    #endif // __linux
    #ifdef __WINNT
    TCHAR buffer[MAX_PATH] = {0};
    DWORD bufSize = sizeof(buffer) / sizeof(*buffer);

    if (GetModuleFileName(NULL, buffer, bufSize) == bufSize)
        return strdup("");

    return extract_dir(buffer);
    #endif // __WINDOWS
}

static void
load_global_settings(e8_environment env, e8_translator tr)
{
    char *bp = get_global_config_path();
    char *cfg = static_cast<char*>( malloc(strlen(bp) + strlen("e8script.cfg") + 2) );
    sprintf(cfg, "%s/%s", bp, "e8script.cfg");

    free(bp);

    settings_from_file(cfg, env, tr);

    free(cfg);

    /* Пытаемся подгрузить дополнительные настройки из пользовательских каталогов */
    #ifdef __WINNT
    settings_from_file("%AppData\\e8script\\e8script.cfg", env, tr);
    settings_from_file("%LocalAppData\\e8script\\e8script.cfg", env, tr);
    #endif
    #ifdef __linux
    settings_from_file("~/.e8script/e8script.cfg", env, tr);
    settings_from_file("~/e8script/e8script.cfg", env, tr);
    #endif // __linux

}

static int
exec_cli(int argc, char **argv)
{
    int fni;

    char **s_params;
    (void)s_params;
    const char *fname;
    if (!get_file_name(argc, argv, &fname, &fni)) {
        USAGE();
        return 0;
    }

    e8_environment env = e8_env_new();
    e8_translator tr = e8_env_get_translator(env);
    e8_env_set_debug_flag(env, true);

    #ifdef __WINNT
    e8_env_set_output_encoding(env, "cp866");
    #endif // __WINNT
    #ifdef __linux
    e8_env_set_output_encoding(env, "utf-8");
    #endif // __linux

    char *source_cwd = gnu_getcwd();

    load_global_settings(env, tr);

    /* Настройки из папки скрипта */

    settings_from_file("./e8script.cfg", env, tr);

    e8_env_apply_PATH(env, "PATH");

    bool no_run = false;
    /* settings_from_cli: */
    {
        int i = 1;
        while (i < fni) {

            if (strcmp(argv[i], "--no-run") == 0) {
                no_run = true;
                ++i;
                continue;
            }

            e8_runtime_error *__err;
            __err = e8_option_apply_cli_option(argv[i], env, tr);
            if (__err) {
                print_error(__err, e8_env_get_output_encoding(env));
                e8_free_exception(__err);
                __err = 0;
            }
            ++i;
        }
        s_params = &argv[fni + 1];
    }

    setlocale(LC_NUMERIC, "C");
    setlocale(LC_MONETARY, "C");

    e8_register_stdlib(env);
    e8_register_stdio(env);

    int first_argument_index = fni + 1;

    {
        /* Создаём массив Аргументы командной строки */
        e8_array *a_args = __e8_new_array(false, 0);

        E8_VAR(va_args);
        e8_var_object(&va_args, a_args);

        for (int i = first_argument_index; i < argc; i++) {
            e8_string *s_arg = e8_string_ascii(argv[i]);

            E8_VAR(vs_arg);
            e8_var_string(&vs_arg, s_arg->s);
            e8_string_destroy(s_arg);

            e8_array_add_nvalues(&va_args, 1, &vs_arg);

            e8_var_destroy(&vs_arg);
        }

        e8_env_add_global_value_utf(env, "АргументыКоманднойСтроки", &va_args);
        e8_env_add_global_value_utf(env, "CommandLineArguments", &va_args);

        e8_var_destroy(&va_args);
    }

    e8_runtime_error *__err;
    e8_unit u;
    int res = 0;

    char *used_filename = (char*)malloc(strlen(source_cwd) + strlen(fname) + 3);
    sprintf(used_filename, "%s/%s", source_cwd, fname);

    __err = e8_env_unit_from_file(env, used_filename, E8_FILE_AUTO, &u);

    free(used_filename);

    {
        /* Устанавливаем текущий каталог по первому файлу */
        char *fdir = extract_dir(fname);
        chdir(fdir);
        free(fdir);
    }

    if (!__err) {

        e8_gc_use_object(u);

        if (!no_run)
            __err = e8_env_execute(u, false);

        e8_env_unit_free(u);

    }
    const char *enc = e8_env_get_output_encoding(env);
    if (__err) {
        res = __err->error_no;
        print_error(__err, enc);
        e8_free_exception(__err);
    }

    e8_env_close(env);

    e8_gc_done();

    chdir(source_cwd);
    free(source_cwd);

    return res;

} // exec_cli

int main(int argc, char **argv)
{
    if (argc == 2 && strcmp(argv[1], "--version") == 0) {
        VERSION();
        return 0;
    }
    int r = exec_cli(argc, argv);
    return r;
}
