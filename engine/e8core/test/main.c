#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "e8core/translator/translator.h"
#include "e8core/utf/utf.h"
#include "e8core/env/env.h"
#include "e8core/env/e8stdlib.h"
#include "test_env.h"

static int cindex = 0;
static int len;

static char text[] =
    "a =     1; assert a=1 as \"a=1\";\n"
    "b =     2; assert b=2 as \"b=2\";\n"
    "f =     4; if (f > 2) a = 2;\n"
    "c = a + b; assert c=4 as \"c=4\";\n"
;

static e8_uchar *utext;

static bool r_flag;
static void rprintf(const char *s)
{
    if (!r_flag) {
        r_flag = true;
        printf("%s", s);
    }
}
static void reset_rflag() { r_flag = false; }

static void print_command(const e8_command *c)
{

    e8_string s = {0, 0};
    char *ascii = 0;

    reset_rflag();

    switch (c->cmd) {

    case E8_COMMAND_NOP: rprintf("nop");
    case E8_COMMAND_Exit: rprintf("exit");
    case E8_COMMAND_Call: rprintf("call");
    case E8_COMMAND_ObjectCall: rprintf("ocall");
    case E8_COMMAND_Pop: rprintf("pop");
    case E8_COMMAND_Iterator: rprintf("iterator");
    case E8_COMMAND_ItHasNext: rprintf("ithasnext");
    case E8_COMMAND_ItNext: rprintf("itnext");
    case E8_COMMAND_EnterScope: rprintf("enter");
    case E8_COMMAND_ExitScope: rprintf("exit");
    case E8_COMMAND_EndSub: rprintf("end sub");
    case E8_COMMAND_Return: rprintf("return");
    case E8_COMMAND_EndCritical: rprintf("end critical");
    case E8_COMMAND_Raise: rprintf("raise");
        printf("\n");
        return;

    case E8_COMMAND_Const: rprintf("const: ");
        e8_var_cast_string(&c->value, &s);
        e8_utf_to_8bit(s.s, &ascii, "utf-8");
        printf("%s\n", ascii);
        free(ascii);
        free(s.s);
        return;

    case E8_COMMAND_Load: rprintf("load ");
    case E8_COMMAND_LoadRef: rprintf("load ref ");
    case E8_COMMAND_DeclareVar: rprintf("declare var ");
    case E8_COMMAND_Assert: rprintf("assert ");
    case E8_COMMAND_DeclareSub: rprintf("declare sub ");
        if (c->property.s)
            c->property.s[c->property.len] = 0;
        e8_utf_to_8bit(c->property.s, &ascii, "utf-8");
        printf("%s\n", ascii);
        free(ascii);
        return ;

    case E8_COMMAND_Op:
        {
            printf("op: %d\n", c->op);
            return;
        }

    case E8_COMMAND_Jump: rprintf("jump ");
    case E8_COMMAND_JumpTrue: rprintf("jt ");
    case E8_COMMAND_JumpFalse: rprintf("jf ");
    case E8_COMMAND_Label: rprintf("label ");
    case E8_COMMAND_Critical: rprintf("critical ");
        {
            printf("%d\n", c->label);
            return;
        }
    case E8_COMMAND_DeclareParam:
        {
            printf("declare param ");
            printf("\n");
            return;
        }
    default: return;
    }
}

static void
free_command(e8_command *uc)
{
    e8_var_destroy(&uc->value);

    /* if (uc->property.s) */
        e8_utf_free(uc->property.s);

}


static int push(const e8_command *c, void *data)
{
    /* printf("command received!\n"); */
    print_command(c);

    e8_command *uc = (e8_command *)c;
    free_command(uc);

    return 0;
}

static int read(e8_uchar *c, void *data)
{
    if (cindex >= len)
        return 1;
    *c = utext[cindex++];
    return 0;
}

struct __r_el {
    e8_uchar             *buf;
    e8_script_position_t _index;
    e8_script_position_t _len;
};

static int b_read(e8_uchar *c, void *data)
{
    struct __r_el *d = (struct __r_el *)data;
    if (d->_index >= d->_len)
        return 1;
    *c = d->buf[d->_index++];
    return 0;
}

static int b_push(const e8_command *c, void *data)
{
    e8_command *uc = (e8_command *)c;
    free_command(uc);
    return 0;
}

static e8_command commands[1000];
static int commands_size = 0;

static int
__push(const e8_command *c, void *data)
{
    commands[commands_size++] = *c;
    print_command(c);
    return 0;
}

static void
free_commands()
{
    while (commands_size--)
        free_command(&commands[commands_size]);
}


static int test_file(char *path)
{
    {
        char buf[100];

        sprintf(buf, "Parse test: %s: ... ", path);

        int i = strlen(buf);
        while (i < 60)
            buf[i++] = ' ';
        buf[i] = 0;

        printf("%s", buf);
    }

    FILE *f = fopen(path, "rt");

    if (!f) {
        printf("not found\n");
        return 1;
    }

    #define BUF_SIZE 8192
    char buf[BUF_SIZE];

    char *c = buf;
    int rc;
    while ( (rc = fgetc(f)) != EOF )
        *c++ = rc;
    *c = 0;

    fclose(f);

    e8_uchar *uc = 0;
    e8_utf_from_8bit(buf, &uc, "utf-8");

    struct __r_el el;
    el.buf = uc;
    el._index = 0;
    el._len = e8_utf_strlen(uc);


    commands_size = 0;
    e8_translator tr = e8_translator_new();

    e8_syntax_error *err = e8_translator_parse(tr,
                    b_push, b_read, &el
                );

    e8_translator_close(tr);

    if (err) {
        printf("syntax error %d\n", err->error);

        char *a;

        e8_utf_to_8bit(&uc[err->pos - 1], &a, "cp866");

        printf("from (%ld): %s\n", err->pos, a);

        free(a);
        e8_error_free(err);
    } else
        printf("ok!\n");

    e8_utf_free(uc);

    return 0;
}

static void test_new_env(char *path)
{
    FILE *f = fopen(path, "rt");

    if (!f)
        return;

    {
        char buf[100];

        sprintf(buf, "Run file: %s: ... ", path);

        int i = strlen(buf);
        while (i < 60)
            buf[i++] = ' ';
        buf[i] = 0;

        printf("%s", buf);
    }


    #define BUF_SIZE 8192
    char buf[BUF_SIZE];

    char *c = buf;
    int rc;
    while ( (rc = fgetc(f)) != EOF )
        *c++ = rc;
    *c = 0;

    fclose(f);

    e8_uchar *uc = 0;
    e8_utf_from_8bit(buf, &uc, "utf-8");

    struct __r_el el;
    el.buf = uc;
    el._index = 0;
    el._len = e8_utf_strlen(uc);

    e8_translator tr = e8_translator_new();

    e8_syntax_error *err = e8_translator_parse(tr,
                    __push, b_read, &el
                );

    e8_translator_close(tr);

    if (err) {
        printf("syntax error %d\n", err->error);

        char *a;

        e8_utf_to_8bit(&uc[err->pos - 1], &a, "cp866");

        printf("from (%ld): %s\n", err->pos, a);

        free(a);

        e8_error_free(err);
        e8_utf_free(uc);

        return;
    };

    e8_utf_free(uc);

    e8_environment env = e8_env_new();

    e8_register_stdlib(env);
    e8_register_stdio(env);

    e8_unit u = e8_env_unit_from_commands(env, commands, commands_size);

    e8_runtime_error *__err = e8_env_execute(u);

    e8_env_unit_free(u);
    e8_env_close(env);

    free_commands();

    if (__err) {
        printf("rte(%d)\n", __err->error_no);
        e8_free_exception(__err);
        return;
    }

    printf("ok\n");

}


int main()
{
/*
    printf("%s\n------------\n", text);

    e8_utf_from_8bit(text, &utext, "utf-8");
    len = e8_utf_strlen(utext);

    e8_translator tr = e8_translator_new();

    e8_syntax_error *err = e8_translator_parse(tr, push, read, 0);
    if (err) {
        printf("Syntax error received: %d at index %lu!\n", err->error, err->pos);
        e8_error_free(err);
    }

    e8_translator_close(tr);
    e8_utf_free(utext);

    if (err)
        return 1;
*/
    #define SCRIPT(name) "../../Scripts/" name

    test_file(SCRIPT("altnames.e8s"));
    test_file(SCRIPT("array.e8s"));
    test_file(SCRIPT("array-ecma.e8s"));
    test_file(SCRIPT("assert.e8s"));
    test_file(SCRIPT("binary.e8s"));
    test_file(SCRIPT("byval.e8s"));
    test_file(SCRIPT("custom.e8s"));
    test_file(SCRIPT("dotvar.e8s"));
    test_file(SCRIPT("ecma.e8s"));
    test_file(SCRIPT("files.e8s"));
    test_file(SCRIPT("GtkPort.e8s"));
    test_file(SCRIPT("internet-mail.e8s"));
    test_file(SCRIPT("JustSend.e8s"));
    test_file(SCRIPT("JustSend-nodl.e8s"));
    test_file(SCRIPT("JustZip.e8s"));
    test_file(SCRIPT("map.e8s"));
    test_file(SCRIPT("memleak.e8s"));
    test_file(SCRIPT("none.e8s"));
    test_file(SCRIPT("numerics.e8s"));
    test_file(SCRIPT("plugin.e8s"));
    test_file(SCRIPT("pre-or-and.e8s"));
    test_file(SCRIPT("preproc.e8s"));
    test_file(SCRIPT("script.e8s"));
    test_file(SCRIPT("sort.e8s"));
    test_file(SCRIPT("structure.e8s"));
    test_file(SCRIPT("system.e8s"));
    test_file(SCRIPT("Units-Lib.e8s"));
    test_file(SCRIPT("Units-Main.e8s"));
    test_file(SCRIPT("ValueList.e8s"));
    test_file(SCRIPT("ValueTable.e8s"));

    #undef SCRIPT
    #define SCRIPT(name) "./Scripts/" name

    test_new_env(SCRIPT("altnames.e8s"));
    test_new_env(SCRIPT("array.e8s"));
    test_new_env(SCRIPT("array-ecma.e8s"));
    test_new_env(SCRIPT("assert.e8s"));
    test_new_env(SCRIPT("binary.e8s"));
    test_new_env(SCRIPT("byval.e8s"));
    test_new_env(SCRIPT("custom.e8s"));
    test_new_env(SCRIPT("dotvar.e8s"));
    test_new_env(SCRIPT("ecma.e8s"));
    test_new_env(SCRIPT("files.e8s"));
    test_new_env(SCRIPT("GtkPort.e8s"));
    test_new_env(SCRIPT("internet-mail.e8s"));
    test_new_env(SCRIPT("JustSend.e8s"));
    test_new_env(SCRIPT("JustSend-nodl.e8s"));
    test_new_env(SCRIPT("JustZip.e8s"));
    test_new_env(SCRIPT("map.e8s"));
    test_new_env(SCRIPT("memleak.e8s"));
    test_new_env(SCRIPT("none.e8s"));
    test_new_env(SCRIPT("numerics.e8s"));
    test_new_env(SCRIPT("plugin.e8s"));
    test_new_env(SCRIPT("pre-or-and.e8s"));
    test_new_env(SCRIPT("preproc.e8s"));
    test_new_env(SCRIPT("script.e8s"));
    test_new_env(SCRIPT("sort.e8s"));
    test_new_env(SCRIPT("structure.e8s"));
    test_new_env(SCRIPT("system.e8s"));
    test_new_env(SCRIPT("Units-Lib.e8s"));
    test_new_env(SCRIPT("Units-Main.e8s"));
    test_new_env(SCRIPT("ValueList.e8s"));
    test_new_env(SCRIPT("ValueTable.e8s"));

    /* test_env(); */

    /* e8_gc_free_all(); */

    return 0;
}
