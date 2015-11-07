#include "utils.h"
#include <sys/timeb.h>
#include <malloc.h>
#include <stdarg.h>
#include "_array.h"
#include <errno.h>
#include <unistd.h>

char *
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
long e8_ticks(const e8_env_data *env)
{
    struct timeb tmb;
    ftime(&tmb);

    long
        ds = tmb.time - env->time_b_sec,
        dms = tmb.millitm - env->time_b_ms
    ;

    if (dms < 0) {
        --ds;
        dms += 1000;
    }

    return ds*1000 + dms;
}
void e8_init_ticks(e8_env_data *env)
{
    struct timeb tmb;
    ftime(&tmb);
    env->time_b_sec = tmb.time;
    env->time_b_ms = tmb.millitm;
}

void e8_count_operation(e8_unit_data *unit, instruction_index_t index, long ticks)
{
    if (unit->ticks && unit->counts) {
        unit->ticks[index] += ticks;
        unit->counts[index] += 1;
    }
}

static inline void rprintf(bool *r_flag, FILE *out, const char *s)
{
    if (!*r_flag) {
        *r_flag = true;
        fprintf(out, "%s", s);
    }
}
static inline void reset_rflag(bool *r_flag) { *r_flag = false; }

void
dump_var(const e8_var *value, char **buf, const char *encoding)
{
    switch (value->type) {
    case varBool:
        {
            *buf = malloc(10);
            if (value->bv)
                strcpy(*buf, "b: true");
            else
                strcpy(*buf, "b: false");
            break;
        }
    case varString:
        {
            e8_uchar *escaped = e8_utf_malloc(value->str->len*2 + 4),
                     *me = escaped
            ;
            const e8_uchar *orig = value->str->s;

            *me++ = '"';
            while (*orig) {
                if (*orig == '\n') {
                    *me++ = '"';
                    *me++ = ' ';
                    *me++ = '"';
                } else if (*orig == '"') {
                    *me++ = '"';
                    *me++ = '"';
                } else {
                    *me++ = *orig;
                }
                ++orig;
            }
            *me++ = '"';
            *me = 0;

            char *pbuf = 0;
            e8_utf_to_8bit(escaped, &pbuf, encoding);
            e8_utf_free(escaped);

            *buf = malloc(strlen(pbuf) + 4);
            sprintf(*buf, "s: %s", pbuf);

            free(pbuf);

            break;
        }

    case varNumeric:
        {
            e8_string *s = e8_numeric_to_string(&value->num);

            char *pbuf = 0;
            e8_utf_to_8bit(s->s, &pbuf, encoding);
            e8_string_destroy(s);

            *buf = malloc(strlen(pbuf) + 4);
            if (value->num.type == numFloat)
                sprintf(*buf, "f: %s", pbuf);
            else
                sprintf(*buf, "i: %s", pbuf);

            free(pbuf);

            break;
        }

    case varUndefined:
        {
            *buf = __e8_strdup("Undefined");
            break;
        }
    case varNull:
        {
            *buf = __e8_strdup("Null");
            break;
        }

    case varObject:
    case varDateTime:
        {
            e8_string *s;
            e8_var_cast_string(value, &s);

            char *pbuf = 0;
            e8_utf_to_8bit(s->s, &pbuf, encoding);

            *buf = malloc(strlen(pbuf) + 4);
            if (value->type == varObject)
                sprintf(*buf, "o: %s", pbuf);
            else
                sprintf(*buf, "d: %s", pbuf);

            e8_string_destroy(s);
            free(pbuf);

            break;
        }
    case varVoid:
        {
            *buf = __e8_strdup("<VOID>");
            break;
        }
    default:
        {
            *buf = __e8_strdup("<Error value>");
            break;
        }
    }
}

void print_command(FILE *out, const e8_command *c)
{
    char *ascii = 0;

    bool r;
    bool *d = &r;

    reset_rflag(d);

    switch (c->cmd) {

    case E8_COMMAND_NOP: rprintf(d, out, "nop");
    case E8_COMMAND_Exit: rprintf(d, out, "exit");
    case E8_COMMAND_Call: rprintf(d, out, "call");
    case E8_COMMAND_ObjectCall: rprintf(d, out, "ocall");
    case E8_COMMAND_Pop: rprintf(d, out, "pop");
    case E8_COMMAND_Iterator: rprintf(d, out, "iterator");
    case E8_COMMAND_ItHasNext: rprintf(d, out, "ithasnext");
    case E8_COMMAND_ItNext: rprintf(d, out, "itnext");
    case E8_COMMAND_Return: rprintf(d, out, "return");
    case E8_COMMAND_EndCritical: rprintf(d, out, "end critical");
    case E8_COMMAND_Raise: rprintf(d, out, "raise");
        return;

    case E8_COMMAND_Const: fprintf(out, "const: #%d", c->const_index); break;
    case E8_COMMAND_LoadRefIndex: fprintf(out, "load ref $%d", c->label); break;
    case E8_COMMAND_Assert: fprintf(out, "assert #%d", c->const_index); break;

    case E8_COMMAND_LoadRef: rprintf(d, out, "load ref ");
    case E8_COMMAND_DeclareParamByRef: rprintf(d, out, "byref ");
    case E8_COMMAND_DeclareParamByVal: rprintf(d, out, "byval ");

        fprintf(out, "$%u", c->label);

        if ((c->cmd == E8_COMMAND_DeclareParamByRef || c->cmd == E8_COMMAND_DeclareParamByVal)
            && c->const_index != 1) {
            fprintf(out, " default: #%d", c->const_index);
        }

        return ;

    case E8_COMMAND_DeclareVar: fprintf(out, "var as $%u", c->label); break;
    case E8_COMMAND_Table: fprintf(out, "table %d", c->label); break;
    case E8_COMMAND_Jump: rprintf(d, out, "jump ");
    case E8_COMMAND_JumpTrue: rprintf(d, out, "jt ");
    case E8_COMMAND_JumpFalse: rprintf(d, out, "jf ");
    case E8_COMMAND_Critical: rprintf(d, out, "critical ");
        fprintf(out, "%d", c->delta);
        return;
    case E8_COMMAND_Add: rprintf(d, out, "add"); break;
    case E8_COMMAND_Sub: rprintf(d, out, "sub"); break;
    case E8_COMMAND_Mul: rprintf(d, out, "mul"); break;
    case E8_COMMAND_Div: rprintf(d, out, "div"); break;
    case E8_COMMAND_Neg: rprintf(d, out, "neg"); break;
    case E8_COMMAND_Assign: rprintf(d, out, "assign"); break;
    case E8_COMMAND_Mod: rprintf(d, out, "mod"); break;
    case E8_COMMAND_Or: rprintf(d, out, "or"); break;
    case E8_COMMAND_And: rprintf(d, out, "and"); break;
    case E8_COMMAND_Not: rprintf(d, out, "not"); break;
    case E8_COMMAND_Equal: rprintf(d, out, "eq"); break;
    case E8_COMMAND_NotEqual: rprintf(d, out, "ne"); break;
    case E8_COMMAND_Less: rprintf(d, out, "lt"); break;
    case E8_COMMAND_Greater: rprintf(d, out, "gt"); break;
    case E8_COMMAND_LessEqual: rprintf(d, out, "le"); break;
    case E8_COMMAND_GreaterEqual: rprintf(d, out, "ge"); break;
    case E8_COMMAND_Inc: rprintf(d, out, "inc"); break;
    case E8_COMMAND_Dec: rprintf(d, out, "dec"); break;
    case E8_COMMAND_Index: rprintf(d, out, "index"); break;

    default: return;
    }
}

char *e8_sprintf (char *buf, const char *encoding, const char *format, ...)
{
    va_list l;
    va_start(l, format);

    if (!buf)
        buf = malloc(1000);

    char *pb = buf;
    const char *pf = format;
    while (*pf) {
        *pb = 0;
        if (*pf == '%') {
            ++pf;

            switch (*pf) {

            case 'd':
            {
                int d = va_arg(l, int);
                sprintf(pb, "%d", d);
                break;
            }
            case 's':
            {
                char *s = va_arg(l, char*);
                sprintf(pb, "%s", s);
                break;
            }
            case 'v':
            {
                e8_var v = va_arg(l, e8_var);
                char *s = 0;
                dump_var(&v, &s, encoding);
                sprintf(pb, "%s", s);
                free(s);
                break;
            }
            case 'S':
            {
                e8_uchar *us = va_arg(l, e8_uchar*);
                char *s = 0;
                e8_utf_to_8bit(us, &s, encoding);
                sprintf(pb, "%s", s);
                free(s);
                break;
            }
            default: break;

            }
            pb += strlen(pb) - 1;
        } else
            *pb = *pf;

        ++pb;
        ++pf;
    }
    *pb = 0;

    va_end(l);

    return buf;
}
e8_uchar *e8_usprintf(e8_uchar *buf, const char *format, ...)
{
    va_list l;
    va_start(l, format);

    if (!buf)
        buf = e8_utf_malloc(1000);

    e8_uchar *pb = buf;
    const char *pf = format;
    while (*pf) {
        *pb = 0;
        if (*pf == '%') {
            ++pf;

            switch (*pf) {

            case 'd':
            {
                int d = va_arg(l, int);
                char mbuf[20], *mmb = mbuf;
                sprintf(mbuf, "%d", d);
                while (*mmb) {
                    *pb = *mmb;
                    ++pb;
                    ++mmb;
                }
                *pb = 0;
                break;
            }
            case 's':
            {
                char *mmb = va_arg(l, char*);
                while (*mmb) {
                    *pb = *mmb;
                    ++pb;
                    ++mmb;
                }
                *pb = 0;
                break;
            }
            case 'v':
            {
                e8_var v = va_arg(l, e8_var);
                char *s = 0;
                dump_var(&v, &s, "utf-8");

                e8_uchar *us = 0;
                e8_utf_from_8bit(s, &us, "utf-8");
                free(s);

                e8_uchar *mc = us;
                while (*mc) {
                    *pb = *mc;
                    ++pb;
                    ++mc;
                }
                *pb = 0;

                e8_utf_free(us);

                break;
            }
            case 'S':
            {
                e8_uchar *us = va_arg(l, e8_uchar*);

                e8_uchar *mc = us;
                while (*mc) {
                    *pb = *mc;
                    ++pb;
                    ++mc;
                }
                *pb = 0;

                break;
            }
            default: break;

            }
            pb += e8_utf_strlen(pb) - 1;
        } else
            *pb = *pf;

        ++pb;
        ++pf;
    }
    *pb = 0;

    va_end(l);

    return buf;
}


void e8_delimit_string(const e8_uchar *s, e8_var *result, const char *delimiter)
{
    e8_array_new(0, 0, 0, 0, result);

    e8_string es = {s, 0, 0};
    es.len = e8_utf_strlen(s);

    e8_string *dl = e8_string_ascii(delimiter);

    int index = 0;
    index = e8_string_find(&es, dl, 0);
    while (index != -1) {

        e8_uchar *subc = e8_utf_malloc(index);
        e8_utf_strncpy(subc, es.s, index);
        subc[index] = 0;

        e8_var el;
        e8_var_string(&el, subc);
        e8_utf_free(subc);

        e8_array_add_nvalues(result, 1, el);

        es.s = &es.s[index + 1];
        es.len -= index + 1;

        index = e8_string_find(&es, dl, 0);
    }

    if (es.len > 0) {
        e8_uchar *subc = e8_utf_strdup(es.s);

        e8_var el;
        e8_var_string(&el, subc);
        e8_utf_free(subc);

        e8_array_add_nvalues(result, 1, el);
    }

}

char *__e8_strdup(const char *s)
{
    char *r = malloc(strlen(s) + 1);
    strcpy(r, s);
    return r;
}

void
__e8_command_destroy(e8_command *c)
{
}

