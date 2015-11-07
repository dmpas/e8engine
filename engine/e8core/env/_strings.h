#ifndef E8_CORE_ENV_STRINGS
#define E8_CORE_ENV_STRINGS
#include "env.h"

/*! Включить в среду исполнения стандартые методы работы со строками */
void e8_register_strings(e8_environment env);

extern e8_vtable vmt_string;

#endif // E8_CORE_ENV_STRINGS
