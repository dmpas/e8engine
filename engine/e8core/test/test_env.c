#include "test_env.h"

#include "e8core/commands/command.h"
#include "e8core/env/env.h"
#include "e8core/translator/translator.h"
#include "e8core/env/e8stdlib.h"

#include <stdio.h>

#include <string.h>

static char buf[] =
    "a =     1; assert a=1 as \"a=1\";"
    "b =     2; assert b=2 as \"b=2\";"
    "f =     4; if (f > 2) a = 2;"
    "c = a + b; assert c=4 as \"c=4\";"
    "mEssage(\"c = \" + c);"
    /*
    "a=1;b=2;c=a+b;"
    "assert c=3 as \"c=3\";"
    */
;

static int b_index = 0, slen;

static e8_command commands[100];
static int commands_size = 0;

static int
__push(const e8_command *c, void *data)
{
    commands[commands_size++] = *c;
    return 0;
}

static int
__read(e8_uchar *c, void *data)
{
    if (b_index >= slen)
        return 1;
    *c = buf[b_index++];
    return 0;
}

void test_env()
{
    slen = strlen(buf);
    e8_translator tr = e8_translator_new();

    e8_translator_parse(tr, __push, __read, 0);

    e8_translator_close(tr);

    e8_environment env;
    e8_unit unit;

    env = e8_env_new();

    e8_register_stdlib(env);
    e8_register_stdio(env);

    unit = e8_env_unit_from_commands(env, commands, commands_size);

    e8_runtime_error *__err = e8_env_execute(unit);
    if (__err) {
        printf("Error: %d\n", __err->error_no);
    }

    e8_env_close(env);

}
