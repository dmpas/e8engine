#include "options.h"
#include <string.h>
#include "env_p.h"
#include <stdio.h>
#include <malloc.h>


e8_runtime_error *
e8_option_apply_cli_option(
                const char         *option,
                e8_environment      env,
                e8_translator       tr)


{
    if (option[0] != '-' || option[1] != '-' || option[2] == 0)
        return false;

    e8_runtime_error *__err = 0;

    int o_len = strlen(option);
    char *s_eq = strstr(option, "=");
    char *lvalue=0, *rvalue=0;
    if (s_eq) {

        /* --option=value */

        int l_size = s_eq - option;

        lvalue = malloc(sizeof(*lvalue) * (l_size + 1));
        rvalue = malloc(sizeof(*rvalue) * (o_len - l_size));

        strncpy(lvalue, option, l_size);
        lvalue[l_size] = 0;

        strcpy(rvalue, s_eq + 1);

    } else {

        /* --option */

        lvalue = malloc(sizeof(*lvalue) * (o_len + 1));
        strcpy(lvalue, option);

    }

    if (tr) {
        if (strcmp(lvalue, "--server") == 0) {
            e8_translator_set_server(tr, true);
            E8_RETURN_OK;
        }
        if (strcmp(lvalue, "--thin-client") == 0) {
            e8_translator_set_thick_client(tr, true);
            E8_RETURN_OK;
        }
        if (strcmp(lvalue, "--web-client") == 0) {
            e8_translator_set_web_client(tr, true);
            E8_RETURN_OK;
        }
        if (strcmp(lvalue, "--thick-client-managed-application") == 0
            || strcmp(lvalue, "--thick-managed") == 0
            ) {
            e8_translator_set_thin_client_managed_application(tr, true);
            E8_RETURN_OK;
        }
        if (strcmp(lvalue, "--thick-client-ordinary-application") == 0
            || strcmp(lvalue, "--thick-ordinary") == 0
            ) {
            e8_translator_set_thin_client_managed_application(tr, true);
            E8_RETURN_OK;
        }
        if (strcmp(lvalue, "--strict-syntax") == 0) {
            e8_translator_set_strict_syntax(tr, true);
            E8_RETURN_OK;
        }
        if (strcmp(lvalue, "--no-script") == 0) {
            e8_translator_set_script(tr, false);
            E8_RETURN_OK;
        }
    }

    if (env) {

        if (strcmp(lvalue, "--dl") == 0) {
            char buf[MAX_PATH];
            sprintf(buf, "lib%s.%s", rvalue, SHARED_EXT);
            __err = e8_env_unit_from_file(env, buf, E8_FILE_COMPILED, 0);
            goto lb_free1;
        }

        if (strcmp(lvalue, "--input-encoding") == 0) {
            e8_env_set_default_input_encoding(env, rvalue);
            goto lb_free1;
        }

        if (strcmp(lvalue, "--output-encoding") == 0) {
            e8_env_set_output_encoding(env, rvalue);
            goto lb_free1;
        }

        if (strcmp(lvalue, "--ext-path") == 0) {
            e8_env_add_ext_path(env, rvalue);
            goto lb_free1;
        }

        if (strcmp(lvalue, "--profile") == 0) {
            e8_env_set_profile_flag(env, true);
            goto lb_free1;
        }

    }

    E8_RETURN_OK;

lb_free1:

    if (lvalue)
        free(lvalue);

    if (rvalue)
        free(rvalue);

    return __err;
}
