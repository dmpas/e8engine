#ifndef E8_CORE_ENVIRONMENT_UTILS
#define E8_CORE_ENVIRONMENT_UTILS

#include "env_p.h"
#include "unit.h"
#include <stdio.h>

long e8_ticks(const e8_env_data *env);
void e8_init_ticks(e8_env_data *env);

void e8_count_operation(e8_unit_data *unit, instruction_index_t index, long ticks);



void print_command(FILE *out, const e8_command *c);

char       *e8_sprintf (char *buf, const char *encoding, const char *format, ...);
e8_uchar   *e8_usprintf(e8_uchar *buf, const char *format, ...);

void        e8_delimit_string(const e8_uchar *s, e8_var *result, const char *delimiter);

char       *__e8_strdup(const char *s);

void        __e8_command_destroy(e8_command *c);

char       *gnu_getcwd();

void        dump_var(const e8_var *value, char **buf, const char *encoding);


#endif // E8_CORE_ENVIRONMENT_UTILS
