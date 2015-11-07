#ifndef E8_CORE_ENV_OPTIONS
#define E8_CORE_ENV_OPTIONS

#include "e8core/translator/translator.h"
#include "e8core/env/env.h"
#include <stdbool.h>

/*! Применяет настройку из параметра командной строки в транслятор или среду */
e8_runtime_error *
e8_option_apply_cli_option(
                const char         *option,
                e8_environment      env,
                e8_translator       tr
            );

#endif // E8_CORE_ENV_OPTIONS
