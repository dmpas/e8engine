#include "translator.h"
#include "lexer.h"
#include "e8core/utf/utf.h"
#include "e8core/utf/tables.h"
#include <malloc.h>
#include <stdio.h>

#define true 1
#define false 0


#define NOW_INACTIVE  0
#define NOW_ACTIVE    1
#define WAS_ACTIVE    2

#define NO_SUB   0
#define IN_FUNC  1
#define IN_PROC  2

#define PUSH_LONG(k) { \
            e8_var __var; \
            e8_var_long(&__var, (k)); \
            m_commands_push_back(d, command_push_const(__add_const(d, &__var))); \
        }
#define PUSH_STRING(p) { \
            e8_var __v; \
            e8_var_string(&__v, (p)); \
            m_commands_push_back(d, command_push_const(__add_const(d, &__v))); \
        }
#define PUSH_CSTRING(p) { \
            e8_var __var; \
            e8_uchar __tmp[100]; \
            e8_uchar *__t = __tmp; \
            e8_utf_from_7bit((p), &__t); \
            e8_var_string(&__var, __t); \
            m_commands_push_back(d, command_push_const(__add_const(d, &__var))); \
        }
#define PUSH_VAR(v) m_commands_push_back(d, command_push_const(__add_const(d, &v)))
#define CALL(k) m_commands_push_back(d, command_call(k))
#define OP_INDEX m_commands_push_back(d, command_op(E8_COMMAND_Index))
#define POP m_commands_push_back(d, command_pop())

#define LABEL_HERE(name) put_label_here(d, name);

#define CONST_UNDEF 0
#define CONST_VOID 1
#define CONST_FALSE 2
#define CONST_TRUE 3

static e8_var var_undef;
static e8_var var_void;
static e8_var var_false;
static e8_var var_true;

static inline e8_string *empty_str()
{
    static e8_string r = {0, 0, 0, 0};
    return &r;
}

static
e8_command command_push_const(int const_index) {
    e8_command R = {E8_COMMAND_Const, const_index, 0, false};
    return R;
}
static
e8_command command_load_ref(int const_index) {    e8_command R = {E8_COMMAND_LoadRef, const_index, 0, false};
    return R;
}
static
e8_command command_load_ref_local(const e8_label_index index) {    e8_command R = {E8_COMMAND_LoadRefIndex, 0, index, false};
    return R;
}
static
e8_command command_call(e8_v_command call_type) {    e8_command R = {call_type, 0, 0, false};
    return R;
}
static
e8_command command_op(e8_v_command optype) {    e8_command R = {optype, 0, 0, false};
    return R;
}
static
e8_command command_jump(unsigned label, e8_v_command type) {    e8_command R = {type, 0, label, false};
    return R;
}
static
e8_command command_var(e8_label_index index) {
    e8_command R = {E8_COMMAND_DeclareVar, 0, index, false};
    return R;
}
static e8_command
command_byref(unsigned index, int default_value_index) {
    e8_command R = {E8_COMMAND_DeclareParamByRef, default_value_index, index, 0};
    return R;
}
static e8_command
command_byval(unsigned index, int default_value_index) {
    e8_command R = {E8_COMMAND_DeclareParamByVal, default_value_index, index, 0};
    return R;
}
static
e8_command command_ret() {    e8_command R = {E8_COMMAND_Return, 0, 0, false};
    return R;
}
static
e8_command command_raise() {    e8_command R = {E8_COMMAND_Raise, 0, 0, false};
    return R;
}
static
e8_command command_critical(unsigned catch_label) {    e8_command R = {E8_COMMAND_Critical, 0, catch_label, false};
    return R;
}
static
e8_command command_end_critical() {    e8_command R = {E8_COMMAND_EndCritical, 0, 0, false};
    return R;
}
static
e8_command command_iterator() {    e8_command R = {E8_COMMAND_Iterator, 0, 0, false};
    return R;
}
static
e8_command command_assert_condition(int const_index) {
    e8_command R = {E8_COMMAND_Assert, const_index, 0, false};
    return R;
}
static
e8_command command_pop() {    e8_command R = {E8_COMMAND_Pop, 0, 0, false};
    return R;
}

static e8_command
command_exit() {
    e8_command R = {E8_COMMAND_Exit, 0, 0, false};
    return R;
}

static e8_command
command_stack_dup() {
    e8_command R = {E8_COMMAND_StackDup, 0, 0, false};
    return R;
}

static e8_command
command_table(unsigned table_index)
{
    e8_command R = {E8_COMMAND_Table, 0, table_index, false};
    return R;
}



/*! Not A Char */
#define NAC (e8_uchar)(0)

typedef struct __e8_tr_loop {
    e8_script_position_t loop_label;
    e8_script_position_t out_label;
} e8_tr_loop;

static /*inline*/
e8_tr_loop LoopLabels(e8_script_position_t loop_label, e8_script_position_t out_label) {
    e8_tr_loop r = {loop_label, out_label};
    return r;
}

typedef struct {
    E8_COLLECTION(e8_command)
} __commands_collection;

typedef struct {
    E8_COLLECTION(e8_uchar*)
} __local_vars_collection;

typedef struct {
    E8_COLLECTION(e8_name_element)
} __dynamic_name_table;

typedef struct __e8_tr_data {
    bool                        thick_client;
    bool                        thin_client_ordinary_application;
    bool                        thin_client_managed_application;
    bool                        web_client;
    bool                        server;
    bool                        external_connection;
    bool                        script;
    bool                        strict_syntax;

    bool                        in_lookup;

    bool                        m_preproc_parse;
    bool                        m_line_start;
    e8_label_index              m_label_index;
    unsigned                    m_temp_var_index;
    int                         in_sub;

    const e8_uchar             *buf;
    bool                        _EOF;
    bool                        _EOL;

    e8_script_position_t        pos_in_buf;
    e8_script_position_t        index;
    e8_script_position_t        current_line;

    struct {
        E8_COLLECTION(int)
    }                           active_stack;

    struct {
        E8_COLLECTION(e8_uchar*)
    }                           dot_var;

    __commands_collection       commands;
    __commands_collection       subs_commands;
    __commands_collection      *current_commands;

    struct {
        E8_COLLECTION(e8_tr_loop)
    }                           loops;

    __local_vars_collection     local_vars;
    __local_vars_collection     sub_local_vars;
    __local_vars_collection    *current_local_var;

    struct {
        E8_COLLECTION(e8_var)
    }                           consts;

    struct {
        E8_COLLECTION(e8_unit_symbol)
    }                           symbols;

    struct {
        E8_COLLECTION(unsigned)
    }                           labels;

    struct {
        E8_COLLECTION(__dynamic_name_table)
    }                           tables;

    void                       *data;
} e8_data;

static /*inline*/
void active_stack_push_back(e8_data *d, int value) {
    int i = d->active_stack.size;
    e8_collection_add_size(&d->active_stack, 1);
    d->active_stack.data[i] = value;
}
static /*inline*/
void active_stack_pop_back(e8_data *d) {
    if (d->active_stack.size)
        --d->active_stack.size;
}
static /*inline*/
int active_stack_back(const e8_data *d) {
    return d->active_stack.data[d->active_stack.size - 1];
}
static /*inline*/
e8_uchar *dot_var_back(e8_data *d) {
    return d->dot_var.data[d->dot_var.size - 1];
}
static /*inline*/
void dot_var_pop_back(e8_data *d) {
    if (d->dot_var.size > 0) {
        e8_utf_free(d->dot_var.data[ --d->dot_var.size ]);
    }
}
static /*inline*/
void dot_var_push_back(e8_data *d, e8_uchar *s) {
    int i = d->dot_var.size;
    e8_collection_add_size(&d->dot_var, 1);
    d->dot_var.data[i] = s;
}
static /*inline*/
void loops_push_back(e8_data *d, e8_tr_loop l) {
    int i = d->loops.size;
    e8_collection_add_size(&d->loops, 1);
    d->loops.data[i] = l;
}
static /*inline*/
void loops_pop_back(e8_data *d) {
    if (d->loops.size > 0)
        --d->loops.size;
}

static int
__add_name_table(e8_data *d)
{
    int i = d->tables.size;
    e8_collection_add_size(&d->tables, 1);
    E8_COLLECTION_INIT(d->tables.data[i]);
    return i;
}

static void
__append_name(e8_data *d, const e8_uchar *name, unsigned index)
{
    __dynamic_name_table *t = &d->tables.data[d->tables.size - 1];
    int i = t->size;
    e8_collection_add_size(t, 1);
    t->data[i].name = e8_string_init(name);
    t->data[i].index = index;
}

static int
__add_const(e8_data *d, const e8_var *value)
{
    int i = d->consts.size;
    while (i--) {
        e8_var res;
        e8_var_eq(&d->consts.data[i], value, &res);
        if (res.bv)
            return i;
    }

    i = d->consts.size;
    e8_collection_add_size(&d->consts, 1);
    e8_var_undefined(&d->consts.data[i]);
    e8_var_assign(&d->consts.data[i], value);

    return i;
}
static int
__add_const_text(e8_data *d, const e8_uchar *id)
{
    e8_var v;
    e8_var_string(&v, id);
    return __add_const(d, &v);
}

static e8_syntax_error *tr_wait_for_active(e8_data *d);
static e8_syntax_error *tr_skip_comments(e8_data *d);
static e8_syntax_error *tr_evaluate_preprocessor_condition(e8_data *d, bool *__result);
static e8_syntax_error *tr_check_preprocessor(e8_data *d, bool *__result);
static e8_syntax_error *tr_expect_keyword(e8_data *d, KeyWord kw);
static e8_syntax_error *tr_parse_command(e8_data *d, bool *__result);
static e8_label_index tr_label(e8_data *d);
static e8_syntax_error *tr_parse_expression(e8_data *d);

static e8_label_index
tr_label(e8_data *d)
{
    e8_label_index i = d->labels.size;
    e8_collection_add_size(&d->labels, 1);
    d->labels.data[i] = (unsigned)(-1);
    return i;
}

static int
tr_temp_var(e8_data *d)
{
    return ++d->m_temp_var_index;
}

static e8_uchar *
tr_temp_var_id(e8_data *d)
{
    long lvar = tr_temp_var(d);
    char buf[20];
    sprintf(buf, "var#%ld", lvar);

    e8_uchar *res = e8_utf_strcdup(buf, "latin1");

    return res;
}

static void
init_reader(e8_data *d)
{
    d->index = 0;
    d->current_line = 1;
    d->m_line_start = true;
    d->_EOF = false;
    d->_EOL = false;
    d->in_lookup = false;
    d->pos_in_buf = 0;
    d->m_label_index = 0;
    d->m_temp_var_index = 0;
    d->current_commands = &d->commands;
    d->current_local_var = &d->local_vars;

    E8_COLLECTION_INIT(d->tables);
}

static /*inline*/ e8_uchar
current(e8_data *d, int delta)
{
    e8_uchar c = d->buf[d->pos_in_buf];
    d->_EOF = (c == NAC) || d->_EOF;
    d->_EOL = lexer_is_nl(c) || d->_EOF;

    return d->buf[d->pos_in_buf + delta];
}


static e8_uchar
next(e8_data *d)
{

    if (d->_EOF)
        return NAC;

    if (!lexer_is_ws(current(d, 0)))
        d->m_line_start = false;
    if (lexer_is_nl(current(d, 0))) {
        d->m_line_start = true;
    }

    ++d->pos_in_buf;

    if (!d->in_lookup && lexer_is_nl(current(d, 0)))
        ++d->current_line;


    if (d->buf[d->pos_in_buf] == NAC) {
        d->_EOF = true;
        return NAC;
    } else
        ++d->index;

    e8_uchar r = current(d, 0);

    return r;
}
static inline e8_uchar
shift(e8_data *d)
{
    e8_uchar c = current(d, 0);
    next(d);
    return c;
}

static inline e8_script_position_t
tr_index(e8_data *d)
{
    return d->index;
}

static void
tr_r_index(e8_data *d, e8_script_position_t new_index)
{
    if (d->index > new_index) {
        int delta = d->index - new_index;

        if (d->buf[d->pos_in_buf] == NAC)
            ++delta;

        if (delta <= d->pos_in_buf) {
            d->pos_in_buf -= delta;
            d->index = new_index;
            d->_EOF = false;
        }
    }
}


typedef bool (*__check_fn_t)(const e8_data *d);

static bool is_thick_client(const e8_data *d)
{
    return d->thick_client;
}

static bool is_thin_client_ordinary_application(const e8_data *d)
{
    return d->thin_client_ordinary_application;
}

static bool is_thin_client_managed_application(const e8_data *d)
{
    return d->thin_client_managed_application;
}

static bool is_external_connection(const e8_data *d)
{
    return d->external_connection;
}

static bool is_server(const e8_data *d)
{
    return d->server;
}

static bool is_web_client(const e8_data *d)
{
    return d->web_client;
}

static bool is_thin_client(const e8_data *d)
{
    return
        d->thin_client_managed_application
        || d->thin_client_ordinary_application;
}

static bool is_client(const e8_data *d)
{
    return
        d->thick_client
        || d->web_client
        || d->thin_client_managed_application
        || d->thin_client_ordinary_application
    ;
}

static bool is_strict_syntax(const e8_data *d)
{
    return d->strict_syntax;
}
static bool is_script(const e8_data *d)
{
    return d->script;
}

struct __e8_string_c {
    char            utf8[120];
    e8_uchar        utf32[60];
    __check_fn_t    checker;
};

static
struct __e8_string_c stable[] = {
    {"client", {}, is_client},
    {"atclient", {}, is_client},
    {"клиент", {}, is_client},
    {"наклиенте", {}, is_client},

    {"server", {}, is_server},
    {"atserver", {}, is_server},
    {"сервер", {}, is_server},
    {"насервере", {}, is_server},

    {"webclient", {}, is_web_client},
    {"вебклиент", {}, is_web_client},

    {"strict", {}, is_strict_syntax},
    {"строгий", {}, is_strict_syntax},

    {"externalconnection", {}, is_external_connection},
    {"внешнеесоединение", {}, is_external_connection},

    {"thickclient", {}, is_thick_client},
    {"толстыйклиент", {}, is_thick_client},

    {"thinclient", {}, is_thin_client},
    {"тонкийклиент", {}, is_thin_client},

    {"thinclientmanagedapplication", {}, is_thin_client_managed_application},
    {"тонкийклиентуправляемоеприложение", {}, is_thin_client_managed_application},

    {"thinclientordinaryapplication", {}, is_thin_client_ordinary_application},
    {"тонкийклиентобычноеприложение", {}, is_thin_client_ordinary_application},

    {"скрипт", {}, is_script},
    {"script", {}, is_script},
    {"е8скрипт", {}, is_script},
    {"e8script", {}, is_script},

    {{0}, {}, 0}

};

static bool __init = false;
static void __e8_translator_init_data()
{
    struct __e8_string_c *s = stable;

    while (s->utf8[0]) {
        e8_uchar *r = s->utf32;
        e8_utf_from_8bit(s->utf8, &r, "utf-8");
        ++s;
    }

    e8_var_undefined(&var_undef);
    e8_var_void(&var_void);
    e8_var_bool(&var_false, false);
    e8_var_bool(&var_true, true);

    e8_lexer_init_data();

    __init = true;
}

static /*inline*/
void __fill_data(e8_data *d)
{
    char *c = (char *)d;
    int i = 0;
    for (;i < sizeof(e8_data); ++i)
        *c++ = 0;

    d->script = true;
}

e8_translator e8_translator_new()
{
    if (!__init)
        __e8_translator_init_data();

    e8_data *r = (e8_data *)malloc(sizeof(e8_data));
    __fill_data(r);

    E8_COLLECTION_INIT(r->commands);
    E8_COLLECTION_INIT(r->subs_commands);
    E8_COLLECTION_INIT(r->active_stack);
    E8_COLLECTION_INIT(r->dot_var);
    E8_COLLECTION_INIT(r->loops);

    E8_COLLECTION_INIT(r->local_vars);
    E8_COLLECTION_INIT(r->sub_local_vars);
    E8_COLLECTION_INIT(r->consts);
    E8_COLLECTION_INIT(r->symbols);
    E8_COLLECTION_INIT(r->labels);

    __add_const(r, &var_undef); /* 0 */
    __add_const(r, &var_void);  /* 1 */
    __add_const(r, &var_false); /* 2 */
    __add_const(r, &var_true);  /* 3 */

    return (e8_translator)r;
}

void e8_translator_close(e8_translator tr)
{
    e8_data *r = (e8_data *)tr;

    e8_collection_free(&r->commands);
    e8_collection_free(&r->subs_commands);
    e8_collection_free(&r->active_stack);
    e8_collection_free(&r->dot_var);
    e8_collection_free(&r->loops);

    int i;
    i = r->local_vars.size;
    while (i--)
        e8_utf_free(r->local_vars.data[i]);

    e8_collection_free(&r->local_vars);
    e8_collection_free(&r->sub_local_vars);

    free(tr);
}

void e8_error_free(e8_syntax_error *error)
{
    free(error);
}

static inline void
put_label_here(e8_data *d, e8_label_index label)
{
    d->labels.data[label] = d->current_commands->size;
}

static /*inline*/
void m_commands_push_back(e8_data *d, const e8_command c)
{
    int i = d->current_commands->size;
    e8_collection_add_size(d->current_commands, 1);
    d->current_commands->data[i] = c;
}
static /*inline*/
void m_commands_pop_back(e8_data *d)
{
    if (d->current_commands->size > 0) {
        --d->current_commands->size;
    }
}


static
void tr_skip_ws(e8_data *d)
{
    while (lexer_is_ws( current(d, 0) ) || ( d->_EOL && !d->m_preproc_parse) ) {

        next(d);

        if (d->_EOF)
            break;
    }
}

static inline
e8_uchar *tr_s_cpy(e8_uchar *s, int l)
{
    e8_uchar *r = e8_utf_malloc(l); /* MEMALLOC */
    s[l] = 0;
    e8_utf_strcpy(r, s);
    return r;
}

static
bool tr_skip_trash(e8_data *d)
{
    unsigned old_index;
    do {
        old_index = tr_index(d);
        if (!d->m_preproc_parse)
            tr_wait_for_active(d);
        tr_skip_ws(d);
        tr_skip_comments(d);

        if (d->_EOF)
            break;

    } while (old_index != tr_index(d));
    return !d->_EOF;
}


static
e8_uchar *tr_identifier(e8_data *d)
{
    /*wstring res;*/
    e8_uchar res[E8_IDENTIFIER_MAX_LENGTH];
    int len = 0;

    if (current(d, 0) == '?') {
        res[len++] = shift(d);
        tr_skip_trash(d);
        return tr_s_cpy(res, len);
    }
    if (!lexer_is_id_s(current(d, 0)))
        return 0; /* EMPTY STRING 0 */

    while (lexer_is_id(current(d, 0)))
        res[len++] = shift(d);

    res[len] = 0;

    if (!d->m_preproc_parse)
        tr_skip_trash(d);
    else
        tr_skip_ws(d);

    return tr_s_cpy(res, len);
}

static
e8_uchar *tr_lookup_word(e8_data *d)
{
    long old_index = tr_index(d);
    d->in_lookup = true;

    e8_uchar *id = tr_identifier(d);
    tr_r_index(d, old_index);

    d->in_lookup = false;

    return id;
}

static
void tr_skip_till_eol(e8_data *d)
{
    while (!d->_EOL && !d->_EOF)
        next(d);
}

static
bool tr_evaluate_preprocessor_identifier(const e8_data *d, const e8_uchar *id)
{
    /* return m_script_settings.get(id); */
    struct __e8_string_c *s = stable;
    while (s->checker) {
        e8_uchar sl[100];
        e8_utf_strcpy(sl, id);
        e8_utf_lower_case(sl);
        int r = e8_utf_strcmp(s->utf32, sl);
        if (r == 0)
            return s->checker(d);
        ++s;
    }
    return false;
}

static
e8_syntax_error *throw_error(e8_error_no error, e8_data *d)
{
    e8_syntax_error *r = (e8_syntax_error *)malloc(sizeof(e8_syntax_error));
    r->error = error;
    r->line_no = d->current_line;
    r->pos = d->index;
    return r;
}

static
e8_syntax_error *tr_evaluate_preprocessor_20(e8_data *d, bool *res)
{
    e8_syntax_error *__err = 0;

    e8_uchar *lw = tr_lookup_word(d);
    KeyWord kw = lexer_get_keyword(lw);
    e8_utf_free(lw);

    bool negative = false;
    if (kw == kwNot) {
        negative = true;
        e8_utf_free( tr_identifier(d) );
        tr_skip_ws(d);
    }
    *res = false;
    if (current(d, 0) == '(') {
        next(d);
        tr_skip_ws(d);

        if (( __err = tr_evaluate_preprocessor_condition(d, res) ))
            return __err;

        tr_skip_ws(d);
        if (shift(d) != ')') {
           return throw_error(E8_CLOSE_BRACKET_EXPECTED, d);
        }
    } else {
        e8_uchar *param = tr_identifier(d);
        *res = tr_evaluate_preprocessor_identifier(d, param);
        e8_utf_free(param);
    }
    tr_skip_ws(d);

    if (negative)
        *res = !*res;

    return E8_NO_THROW;
}
static
e8_syntax_error *tr_evaluate_preprocessor_10(e8_data *d, bool *res)
{
    e8_syntax_error *__err;
    e8_uchar *lw;

    if (( __err = tr_evaluate_preprocessor_20(d, res) ))
        return __err;

    tr_skip_ws(d);

    lw = tr_lookup_word(d);
    KeyWord kw = lexer_get_keyword(lw);
    e8_utf_free(lw);

    while (kw == kwAnd) {
        e8_utf_free( tr_identifier(d) );

        bool __r;
        if (( __err = tr_evaluate_preprocessor_20(d, &__r) ))
            return __err;

        if (!__r)
            *res = false;

        tr_skip_ws(d);

        lw = tr_lookup_word(d);
        kw = lexer_get_keyword(lw);
        e8_utf_free(lw);
    } // while wkAnd

    return E8_NO_THROW;
}


static
e8_syntax_error *tr_evaluate_preprocessor_condition(e8_data *d, bool *res)
{
    e8_syntax_error *__err;
    if ((__err = tr_evaluate_preprocessor_10(d, res) ))
        return __err;

    tr_skip_ws(d);

    e8_uchar *lw = tr_lookup_word(d);
    KeyWord kw = lexer_get_keyword(lw);
    e8_utf_free(lw);

    while (kw == kwOr) {
        e8_utf_free( tr_identifier(d) );

        bool __r;
        if (( __err = tr_evaluate_preprocessor_10(d, &__r) ))
            return __err;

        if (__r)
            *res = true;

        tr_skip_ws(d);
        lw = tr_lookup_word(d);
        kw = lexer_get_keyword(lw);
        e8_utf_free(lw);
    } // while wkOr

    return E8_NO_THROW;
}
static
bool tr_text_active(e8_data *d)
{
    int i;
    for (i = 0; i < d->active_stack.size; ++i)
        if (d->active_stack.data[i] != NOW_ACTIVE)
            return false;
    return true;
}
static
e8_syntax_error *tr_wait_for_active(e8_data *d)
{
    bool __r;
    e8_syntax_error *__err = 0;
    if (( __err = tr_check_preprocessor(d, &__r) ))
        return __err;

    while (!tr_text_active(d) && !d->_EOF) {

        if (( __err = tr_check_preprocessor(d, &__r) ))
            return __err;

        if (!__r)
            next(d); // пропускаем символ
    } // while !active
    return E8_NO_THROW;
}

static
e8_syntax_error *tr_check_preprocessor(e8_data *d, bool *__result)
{
    e8_syntax_error *__err;
    e8_uchar *lw;

    d->m_preproc_parse = true;
    if ( d->m_line_start && current(d, 0) == '#' ) {

        next(d);
        tr_skip_ws(d);
        lw = tr_identifier(d);
        KeyWord kw = lexer_get_keyword(lw);
        e8_utf_free(lw);

        if (kw == kwNotKeyWord) {
            return throw_error(E8_KEYWORD_EXPECTED, d);
        }

        if (kw == kwIf) {

            tr_skip_ws(d);

            bool activate_now;
            if (( __err = tr_evaluate_preprocessor_condition(d, &activate_now) ))
                return __err;

            active_stack_push_back(d, activate_now ? NOW_ACTIVE : NOW_INACTIVE);

            tr_skip_ws(d);
            if (( __err = tr_expect_keyword(d, kwThen) ))
                return __err;

        } else if (kw == kwElse) {
            short la = active_stack_back(d);
            active_stack_pop_back(d);

            if (la == NOW_ACTIVE)
                active_stack_push_back(d, NOW_INACTIVE);
            if (la == NOW_INACTIVE)
                active_stack_push_back(d, NOW_ACTIVE);
            if (la == WAS_ACTIVE)
                active_stack_push_back(d, WAS_ACTIVE);

        } else if (kw == kwEndIf) {
            active_stack_pop_back(d);
        } else if (kw == kwElseIf) {
            tr_skip_ws(d);

            bool activate_now;
            if (( __err = tr_evaluate_preprocessor_condition(d, &activate_now) ))
                return __err;

            tr_skip_ws(d);
            if (( __err = tr_expect_keyword(d, kwThen) ))
                return __err;

            short la = active_stack_back(d);
            active_stack_pop_back(d);

            if (la == NOW_ACTIVE)
                active_stack_push_back(d, WAS_ACTIVE);
            if (la == NOW_INACTIVE)
                active_stack_push_back(d, activate_now ? NOW_ACTIVE : NOW_INACTIVE);
            if (la == WAS_ACTIVE)
                active_stack_push_back(d, WAS_ACTIVE);

        }

        tr_skip_till_eol(d);
        d->m_preproc_parse = false;
        tr_skip_ws(d);
        *__result = true;
        return E8_NO_THROW;
    }
    d->m_preproc_parse = false;
    *__result = false;
    return E8_NO_THROW;
}
static
e8_syntax_error *tr_skip_comments(e8_data *d)
{
    while (true) {
        if (current(d, 0) == '&')
            tr_skip_till_eol(d);

        else if (current(d, 0) == '/' && current(d, +1) == '/')
            tr_skip_till_eol(d);
            //++ feature
        else if (current(d, 0) == '/' && current(d, +1) == '*') {
            next(d);
            next(d);
            while ( !(current(d, 0) == '*' && current(d, +1) == '/'  ) ) {
                if (d->_EOF) {
                    return throw_error(E8_UNEXPECTED_END_OF_FILE, d);
                }
                next(d);
            }
            next(d);
            next(d);
        }
            //-- feature
        else
            break;
    }

    return E8_NO_THROW;
}

static
e8_syntax_error *tr_parse_command_block(e8_data *d)
{
    e8_syntax_error *__err;
    bool __r = true;
    while ( __r ) {
        if (( __err = tr_parse_command(d, &__r) ))
            return __err;
    }

    return E8_NO_THROW;
}


static
e8_syntax_error *tr_parse(e8_data *d)
{
    __add_name_table(d);
    e8_syntax_error *__err = tr_parse_command_block(d);
    m_commands_push_back(d, command_exit());
    return __err;
}

static void
__make_output(e8_data *d, e8_parse_output *out)
{
    instruction_index_t ic;
    e8_command *c;
    c = d->commands.data;
    for (ic = 0; ic < d->commands.size; ++ic, ++c) {
        e8_label_index li = c->label;
        if (c->cmd == E8_COMMAND_Jump
            || c->cmd == E8_COMMAND_JumpTrue
            || c->cmd == E8_COMMAND_JumpFalse
            || c->cmd == E8_COMMAND_Critical
            ) {
            d->commands.data[ic].delta = d->labels.data[li] - ic;
            d->commands.data[ic].label = 0;
        }
    }

    c = d->subs_commands.data;
    for (ic = 0; ic < d->subs_commands.size; ++ic, ++c) {
        e8_label_index li = c->label;
        if (c->cmd == E8_COMMAND_Jump
            || c->cmd == E8_COMMAND_JumpTrue
            || c->cmd == E8_COMMAND_JumpFalse
            || c->cmd == E8_COMMAND_Critical
            ) {
            c->delta = d->labels.data[li] - ic;
            c->label = 0;
        }
    }

    int i;

    struct {
        E8_COLLECTION(e8_command)
    } final_commands;

    E8_COLLECTION_INIT_SIZE(final_commands, d->commands.size + d->subs_commands.size);

    memcpy(&final_commands.data[0]
                , d->commands.data, d->commands.size * d->commands.element_size
    );

    memcpy(&final_commands.data[d->commands.size]
                , d->subs_commands.data, d->subs_commands.size * d->subs_commands.element_size
    );

    out->commands = final_commands.data;
    out->commands_count = final_commands.size;

    out->consts = d->consts.data;
    out->consts_count = d->consts.size;

    for (i = 0; i < d->symbols.size; ++i)
        d->symbols.data[i].address += d->commands.size;

    out->symbols = d->symbols.data;
    out->symbols_count = d->symbols.size;

    e8_collection_free(&d->commands);
    e8_collection_free(&d->subs_commands);

    struct {
        E8_COLLECTION(e8_name_table)
    } tables;

    {

        E8_COLLECTION_INIT_SIZE(tables, d->tables.size);
        __dynamic_name_table *T = d->tables.data;
        for (i = 0; i < d->tables.size; ++i, ++T) {
            tables.data[i].size = T->size;
            tables.data[i].names = T->data;
        }

        out->tables_count = tables.size;
        out->tables = tables.data;
    }

}

e8_syntax_error *e8_translator_parse (
                            e8_translator       tr,
                            e8_parse_output    *out,
                            const e8_uchar     *text,
                            void               *data
                        )
{
    e8_data *d = (e8_data *)tr;
    d->buf = text;
    d->data = data;


    init_reader(d);

    out->commands = NULL;
    out->commands_count = 0;

    out->consts_count = 0;
    out->consts = NULL;

    out->symbols_count = 0;
    out->symbols = NULL;

    e8_syntax_error *__err = tr_parse(d);

    if (!__err) {
        __make_output(d, out);
    }

    return __err;
}

static
e8_syntax_error *tr_parse_call_params(e8_data *d)
{
    e8_syntax_error *__err;

    int p_count = 0;

    next(d); // пропустить '('
    tr_skip_trash(d);
    bool was_comma = false;
    while (current(d, 0) != ')') {

        ++p_count;
        was_comma = false;

        if (current(d, 0) == ',') {
            m_commands_push_back(d, command_push_const(CONST_VOID));
        }
        else {
            if (( __err = tr_parse_expression(d) ))
                return __err;
        }
        tr_skip_trash(d);

        if (current(d, 0) == ',') {
            next(d);
            was_comma = true;
        }

        tr_skip_trash(d);
    }

    if (was_comma) {
        m_commands_push_back(d, command_push_const(CONST_VOID));
        ++p_count;
    }

    PUSH_LONG(p_count);

    return E8_NO_THROW;
}

static
bool tr_is_const_expression(e8_data *d)
{

    e8_uchar *lw = tr_lookup_word(d);
    KeyWord kw = lexer_get_keyword(lw);
    e8_utf_free(lw);

    if (kw == kwUndefined)
        return true;
    if (kw == kwNull)
        return true;
    if (kw == kwFalse || kw == kwTrue)
        return true;
    if (current(d, 0) == '\'')
        return true;
    if (current(d, 0) == '"')
        return true;
    if (lexer_is_num(current(d, 0)))
        return true;
    if (current(d, 0) == '.' && lexer_is_num(current(d, +1)))
        return true;

    return false;
}

static
e8_syntax_error *tr_parse_decimal_number(e8_data *d, int k, e8_var *res)
{
    char snum[100];
    int slen = 0;

    while (lexer_is_num(current(d, 0)))
        snum[slen++] = shift(d);

    if (current(d, 0) == '.') {
        // дробная часть
        snum[slen++] = shift(d);
        while (lexer_is_num(current(d, 0)))
            snum[slen++] = shift(d);

        snum[slen] = 0;

        double _dv;
        sscanf(snum, "%lf", &_dv);

        tr_skip_trash(d);

        e8_var_double(res, _dv*k);

        return E8_NO_THROW;

    } else {
        long _lv;

        snum[slen] = 0;

        if (strlen(snum) > 10)
            return throw_error(E8_CONST_EXPRESSION_EXPECTED, d);

        int r = sscanf(snum, "%ld", &_lv);
        if (r == 0)
            return throw_error(E8_CONST_EXPRESSION_EXPECTED, d);

        tr_skip_trash(d);

        e8_var_long(res, _lv*k);

        return E8_NO_THROW;
    }
    return E8_NO_THROW;
}

static int
__dehex(e8_uchar c)
{
    if (c >= '0' && c <= '9')
        return c - '0';

    if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;

    if (c >= 'A' && c <= 'F')
        return c - 'A' + 10;

    switch (c) {
        case cyr_aa: case cyr_AA: return 10;
        case cyr_bb: case cyr_BB: return 11;
        case cyr_cc: case cyr_CC: return 12;
        case cyr_dd: case cyr_DD: return 13;
        case cyr_ye: case cyr_YE: return 14;
        case cyr_ff: case cyr_FF: return 15;

    default:
        return -1;
    }
}

static
e8_syntax_error *tr_parse_hexadecimal_number(e8_data *d, int kf, e8_var *res)
{
    next(d); /* 0 */
    next(d); /* x */

    long value = 0;

    while (lexer_is_hex_num(current(d, 0))) {
        int k = __dehex(shift(d));
        if (k == -1) {
            /* TODO: Внятное исключение */
            return throw_error(E8_CONST_EXPRESSION_EXPECTED, d);
        }
        value = value*16 + k;
    }

    tr_skip_trash(d);

    e8_var_long(res, kf*value);
    return E8_NO_THROW;
}

static
e8_syntax_error *tr_parse_numeric_const_expression(e8_data *d, e8_var *res)
{
    int k = 1;
    if (current(d, 0) == '-') {
        k = -1;
        next(d);
    } else if (current(d, 0) == '+')
        next(d);

    if (current(d, 0) == '0'
        && ( (current(d, 1) == 'x') || current(d, 1) == cyr_sh )
        )
            return tr_parse_hexadecimal_number(d, k, res);

    return tr_parse_decimal_number(d, k, res);
}

static
e8_syntax_error *tr_parse_datetime_const_expression(e8_data *d, e8_var *res)
{
    char s_res[100];
    int slen = 0;
    next(d);
    while (!d->_EOF && current(d, 0) != '\'')
        s_res[slen++] = shift(d);
    s_res[slen] = 0;
    next(d);

    e8_date date;
    e8_date_from_string(s_res, &date);
    e8_var_date(res, date);

    return E8_NO_THROW;
}

static
e8_syntax_error *tr_parse_string_const_expression(e8_data *d, e8_var *v_res)
{
    size_t reserved = E8_MAX_STRING_CONST_LENGTH;
    e8_uchar *res = e8_utf_malloc(reserved);
    size_t len = 0;

    while (true) {
        next(d); // пропустить открывающую кавычку

        while ( !(current(d, 0) == '"' && current(d, +1) != '"') ) {

            if (reserved - len < 5) {
                reserved += E8_MAX_STRING_CONST_LENGTH;
                res = e8_utf_realloc(res, reserved);
            }

            if (lexer_is_nl(current(d, 0))) {
                //skip_ws();
                tr_skip_trash(d);
                if (current(d, 0) != '|') {
                    e8_utf_free(res);
                    return throw_error(E8_LINE_FEED_EXPECTED, d);
                }
                next(d);
                res[len++] = '\n';
                continue;
            }

            res[len++] = current(d, 0);
            if (current(d, 0) == '"' && current(d, +1) == '"')
                next(d);
            next(d);

            if (d->_EOF) {
                e8_utf_free(res);
                return throw_error(E8_UNEXPECTED_END_OF_FILE, d);
            }
        }
        next(d); // пропустить завершающую кавычку

        tr_skip_trash(d);

        if (current(d, 0) == '"') // если тут же новая строка
            res[len++] = '\n';
        else
            break;
    }
    res[len] = 0;
    e8_var_string(v_res, res);

    e8_utf_free(res);

    return E8_NO_THROW;
}


static
e8_syntax_error *tr_parse_const_expression(e8_data *d, e8_var *res)
{
    e8_uchar *lw = tr_lookup_word(d);
    KeyWord kw = lexer_get_keyword(lw);
    e8_utf_free(lw);

    if (kw == kwUndefined) {
        e8_utf_free( tr_identifier(d) );
        e8_var_undefined(res);
        return E8_NO_THROW;
    }
    if (kw == kwNull) {
        e8_utf_free( tr_identifier(d) );
        e8_var_null(res);
        return E8_NO_THROW;
    }
    if (kw == kwTrue) {
        e8_utf_free( tr_identifier(d) );
        e8_var_bool(res, true);
        return E8_NO_THROW;
    }
    if (kw == kwFalse) {
        e8_utf_free( tr_identifier(d) );
        e8_var_bool(res, false);
        return E8_NO_THROW;
    }


    if (current(d, 0) == '\'')
        return tr_parse_datetime_const_expression(d, res);

    if (current(d, 0) == '"')
        return tr_parse_string_const_expression(d, res);

    if (lexer_is_num(current(d, 0))
        || current(d, 0) == '-'
        || current(d, 0) == '+'
        || (current(d, 0) == '.' && lexer_is_num(current(d, +1))) )
        return tr_parse_numeric_const_expression(d, res);

    return throw_error(E8_CONST_EXPRESSION_EXPECTED, d);
}

static e8_label_index
__get_local_index(e8_data *d, const e8_uchar *name)
{
    int i;
    for (i = 0; i < d->current_local_var->size; ++i)
        if (e8_utf_stricmp(d->current_local_var->data[i], name) == 0)
            return i;

    return WRONG_LABEL;
}

static
e8_syntax_error *tr_parse_complex_identifier(e8_data *d, bool *__result)
{
    e8_syntax_error *__err;
    unsigned depth = 0;
    e8_uchar *id = 0;

    bool lvalue = true;

    if (tr_is_const_expression(d)) {

        lvalue = false;
        e8_var const_val;
        if (( __err = tr_parse_const_expression(d, &const_val) ))
            return __err;

        m_commands_push_back(d, command_push_const(__add_const(d, &const_val)));

    } else {

        id = tr_identifier(d);

        if (e8_utf_strlen(id) == 0) {

            e8_utf_free(id);

            if (current(d, 0) == '.' && !lexer_is_num(current(d, +1))) {

                if (d->dot_var.size == 0)
                    return throw_error(E8_IDENTIFIER_EXPECTED, d);

                id = e8_utf_strdup(dot_var_back(d));

                m_commands_push_back(d, command_load_ref(__add_const_text(d, id)));

            }
            else {
                return throw_error(E8_IDENTIFIER_EXPECTED, d);
            }
        } else {

            e8_label_index __local = WRONG_LABEL;

            __local = __get_local_index(d, id);

            if (__local == WRONG_LABEL)
                m_commands_push_back(d, command_load_ref(__add_const_text(d, id)));
            else
                m_commands_push_back(d, command_load_ref_local(__local));
        }
    }

    while (true) {

        if (current(d, 0) == '.') {

            next(d);
            tr_skip_trash(d);

            if (id)
                e8_utf_free(id);

            id = tr_identifier(d);
            if (e8_utf_strlen(id) == 0)
                break;

            PUSH_STRING(id);
            OP_INDEX;

            lvalue = true;
            ++depth;

        } else if (current(d, 0) == '[') {

            next(d);
            tr_skip_trash(d);

            if (( __err = tr_parse_expression(d) )) {

                if (id)
                    e8_utf_free(id);

                return __err;
            }

            if (shift(d) != ']') {

                if (id)
                    e8_utf_free(id);

                return throw_error(E8_CLOSE_BRACKET_EXPECTED, d);
            }

            tr_skip_trash(d);

            OP_INDEX;

            if (id)
                e8_utf_free(id);

            id = 0;
            lvalue = true;
            ++depth;

        } else if (current(d, 0) == '(') {

            m_commands_pop_back(d); // Убираем лишний OP_INDEX
            if (!depth) {
                PUSH_STRING(id);
            }

            if (( __err = tr_parse_call_params(d) )) {

                if (id)
                    e8_utf_free(id);

                return __err;
            }

            tr_skip_trash(d);

            if (shift(d) != ')') {

                if (id)
                    e8_utf_free(id);

                return throw_error(E8_CLOSE_BRACKET_EXPECTED, d);
            }

            //m_commands_push_back(d, command_push_const(Variant(id)));
            if (depth)
                m_commands_push_back(d, command_call(E8_COMMAND_ObjectCall));
            else
                m_commands_push_back(d, command_call(E8_COMMAND_Call));

            tr_skip_trash(d);

            if (id)
                e8_utf_free(id);

            id = 0;
            lvalue = false;
            ++depth;

        } else {
            break;
        }
    } // while true

    if (id)
        e8_utf_free(id);

    *__result = lvalue;
    return E8_NO_THROW;
} // parse_complex_identifier



// [+|-|НЕ]{СложныйИдентификатор|КонстантноеВыражение|(Выражение)|Новый Тип[(Параметры)]}|[ЭлементыМассива]
/* СложныйИдентификатор ::= идентификатор, содержащий полный путь к необходимой области памяти:
 * полный["путь"].объекта[1]["с"].индексами
 * ЭлементыМассива ::= Выражение{,Выражение}|
 */
static
e8_syntax_error *tr_parse_060(e8_data *d)
{
    bool num_negative = false;
    bool bool_negative = false;

    e8_syntax_error *__err = 0;
    e8_uchar *lw;

    lw = tr_lookup_word(d);
    KeyWord kw = lexer_get_keyword(lw);
    e8_utf_free(lw);

    if (kw == kwNew) {
        e8_utf_free( tr_identifier(d) ); // Новый

        PUSH_CSTRING("NewObject");
        if (current(d, 0) == '(') {
            // Новый ("ИмяТипа"[,Параметры])

            if (( __err = tr_parse_call_params(d) ))
                return __err;

            if (current(d, 0) != ')') {
                return throw_error(E8_CLOSE_BRACKET_EXPECTED, d);
            }
            next(d);
            tr_skip_trash(d);

        } else {
            // Новый <Тип>[(Параметры)]

            e8_uchar *type_name = tr_identifier(d);

            if (current(d, 0) == '.') {
                int idlen = e8_utf_strlen(type_name);

                while (current(d, 0) == '.') {
                    next(d);

                    int oldlen = idlen;

                    idlen += 1;

                    tr_skip_trash(d);
                    e8_uchar *_sub = tr_identifier(d);

                    idlen += e8_utf_strlen(_sub);

                    type_name = e8_utf_realloc(type_name, idlen);
                    type_name[oldlen] = '.';

                    e8_utf_strcpy(&type_name[oldlen + 1], _sub);
                    e8_utf_free(_sub);
                }
            }

            e8_var var_type_name;
            e8_var_string(&var_type_name, type_name);
            m_commands_push_back(d, command_push_const(__add_const(d, &var_type_name)));

            if (current(d, 0) == '(') {

                if (( __err = tr_parse_call_params(d) ))
                    return __err;

                if (current(d, 0) != ')') {
                    return throw_error(E8_CLOSE_BRACKET_EXPECTED, d);
                }
                next(d);
                tr_skip_trash(d);

                // TODO: затычка
                PUSH_LONG(1);
                m_commands_push_back(d, command_op(E8_COMMAND_Add));
            } else {
                // TODO: затычка
                PUSH_LONG(1);
            }

            e8_utf_free(type_name);
        }

        CALL(E8_COMMAND_Call);

        return E8_NO_THROW;
    }

    if (current(d, 0) == '[' && !d->strict_syntax) {
        // Упрощённая запись массива
        next(d);
        tr_skip_trash(d);

        PUSH_CSTRING("NewObject");
        PUSH_CSTRING("Array");
        PUSH_LONG(1);
        CALL(E8_COMMAND_Call);

        PUSH_CSTRING("#append");

        int k = 0;
        if (current(d, 0) != ']') {
            do {

                if (current(d, 0) == ',') {
                    next(d);
                    tr_skip_trash(d);
                }

                ++k;

                if (current(d, 0) == ',' || current(d, 0) == ']')
                    m_commands_push_back(d, command_push_const(CONST_UNDEF));
                else {
                    if (( __err = tr_parse_expression(d) ))
                        return __err;
                }

            } while (current(d, 0) != ']');
        }
        next(d);
        tr_skip_trash(d);

        PUSH_LONG(k);
        CALL(E8_COMMAND_ObjectCall);

        return E8_NO_THROW;
    }
    if (current(d, 0) == '{' && !d->strict_syntax) {
        // Упрощённая запись структуры
        next(d);
        tr_skip_trash(d);

        PUSH_CSTRING("NewObject");
        PUSH_CSTRING("Structure");
        PUSH_LONG(1);
        CALL(E8_COMMAND_Call);

        PUSH_CSTRING("#append");

        int k = 0;
        if (current(d, 0) != '}') {
            do {
                if (current(d, 0) == ',') {
                    next(d);
                    tr_skip_trash(d);
                }
                if (current(d, 0) == ',' || current(d, 0) == ']');
                else {
                    ++k;
                    e8_var id;
                    if (( __err = tr_parse_string_const_expression(d, &id) ))
                        return __err;

                    PUSH_VAR(id);

                    if (current(d, 0) != ':') {
                        return throw_error(E8_COLON_EXPECTED, d);
                    }
                    next(d);
                    tr_skip_trash(d);

                    ++k;
                    tr_parse_expression(d);
                }
            } while (current(d, 0) != '}');
        }
        next(d);
        tr_skip_trash(d);

        PUSH_LONG(k);
        CALL(E8_COMMAND_ObjectCall);

        return E8_NO_THROW;
    }

    if (current(d, 0) == '+') {
        next(d);
        tr_skip_trash(d);
    } else if (current(d, 0) == '-') {
        num_negative = true;
        next(d);
        tr_skip_trash(d);
    } else {
        lw = tr_lookup_word(d);

        if (lexer_get_keyword(lw) == kwNot) {
            bool_negative = true;
            e8_utf_free( tr_identifier(d) );
            tr_skip_trash(d);
        }

        e8_utf_free(lw);
    }

    if (current(d, 0) == '(') {

        next(d);
        tr_skip_trash(d);

        if (( __err = tr_parse_expression(d) ))
            return __err;

        if (current(d, 0) != ')') {
            return throw_error(E8_CLOSE_BRACKET_EXPECTED, d);
        }
        next(d);
        tr_skip_trash(d);

    } else if (current(d, 0) == '?') {
        next(d);
        tr_skip_trash(d);

        next(d); /* '(' */
        tr_skip_trash(d);

        if (( __err = tr_parse_expression(d) ))
            return __err;

        int lfalse = tr_label(d);
        int lout = tr_label(d);

        m_commands_push_back(d, command_jump(lfalse, E8_COMMAND_JumpFalse));

        next(d); /* ',' */
        tr_skip_trash(d);

        if (( __err = tr_parse_expression(d) ))
            return __err;

        m_commands_push_back(d, command_jump(lout, E8_COMMAND_Jump));

        next(d); /* ',' */
        tr_skip_trash(d);

        LABEL_HERE((lfalse));

        if (( __err = tr_parse_expression(d) ))
            return __err;

        next(d); /* ')' */
        tr_skip_trash(d);

        LABEL_HERE((lout));


    } else {
        // идентификатор
        bool __r;
        if (( __err = tr_parse_complex_identifier(d, &__r) ))
            return __err;
    }

    if (bool_negative)
        m_commands_push_back(d, command_op(E8_COMMAND_Not));
    if (num_negative)
        m_commands_push_back(d, command_op(E8_COMMAND_Neg));

    return E8_NO_THROW;
}

// А[{*|/|%}Б]
static
e8_syntax_error *tr_parse_050(e8_data *d)
{
    e8_syntax_error *__err;

    e8_v_command ot = E8_COMMAND_Nop;
    do {
        if (( __err = tr_parse_060(d) ))
            return __err;

        if (ot != E8_COMMAND_Nop)
            m_commands_push_back(d, command_op(ot));

        tr_skip_trash(d);
        if (current(d, 0) == '*') {
            ot = E8_COMMAND_Mul;
            next(d);
            tr_skip_trash(d);
        } else if (current(d, 0) == '/') {
            ot = E8_COMMAND_Div;
            next(d);
            tr_skip_trash(d);
        } else if (current(d, 0) == '%') {
            ot = E8_COMMAND_Mod;
            next(d);
            tr_skip_trash(d);
        } else
            break;
    } while (true);
    return E8_NO_THROW;
}

// А[{+|-}Б]
static
e8_syntax_error *tr_parse_040(e8_data *d)
{
    e8_syntax_error *__err;
    e8_v_command ot = E8_COMMAND_Nop;
    do {
        if (( __err = tr_parse_050(d) ))
            return __err;

        if (ot != E8_COMMAND_Nop)
            m_commands_push_back(d, command_op(ot));

        tr_skip_trash(d);
        if (current(d, 0) == '+') {
            ot = E8_COMMAND_Add;
            next(d);
            tr_skip_trash(d);
        } else if (current(d, 0) == '-') {
            ot = E8_COMMAND_Sub;
            next(d);
            tr_skip_trash(d);
        } else
            break;
    } while (true);
    return E8_NO_THROW;
}

// A[{<|<=|>=|>}Б]
static
e8_syntax_error *tr_parse_030(e8_data *d)
{
    e8_syntax_error *__err;

    if (( __err = tr_parse_040(d) ))
        return __err;

    tr_skip_trash(d);

    if (current(d, 0) == '<' && current(d, +1) != '>') {

        e8_v_command ot = E8_COMMAND_Less;

        next(d);
        if (current(d, 0) == '=') {
            next(d);
            ot = E8_COMMAND_LessEqual;
        }

        tr_skip_trash(d);

        if (( __err = tr_parse_040(d) ))
            return __err;

        tr_skip_trash(d);

        m_commands_push_back(d, command_op(ot));

    } else if (current(d, 0) == '>') {

        e8_v_command ot = E8_COMMAND_Greater;

        next(d);
        if (current(d, 0) == '=') {
            next(d);
            ot = E8_COMMAND_GreaterEqual;
        }

        tr_skip_trash(d);

        if (( __err = tr_parse_040(d) ))
            return __err;

        tr_skip_trash(d);

        m_commands_push_back(d, command_op(ot));

    }
    return E8_NO_THROW;
}

// А[ {=|<>}Б]
static
e8_syntax_error *tr_parse_020(e8_data *d)
{
    e8_syntax_error *__err;

    if (( __err = tr_parse_030(d) ))
        return __err;

    tr_skip_trash(d);

    if (current(d, 0) == '=') {
        next(d);
        tr_skip_trash(d);

        if (( __err = tr_parse_030(d) ))
            return __err;

        tr_skip_trash(d);

        m_commands_push_back(d, command_op(E8_COMMAND_Equal));

    } else if (current(d, 0) == '<' && current(d, +1) == '>') {
        next(d); next(d);
        tr_skip_trash(d);

        if (( __err = tr_parse_030(d) ))
            return __err;

        tr_skip_trash(d);

        m_commands_push_back(d, command_op(E8_COMMAND_NotEqual));
    }
    return E8_NO_THROW;
}

// [И ] А[ И Б]
static
e8_syntax_error *tr_parse_010(e8_data *d)
{
    e8_syntax_error *__err;
    bool and_chain = false, need_false_label = false;
    { // пропускаем впереди идущее И
        e8_uchar *lw = tr_lookup_word(d);
        KeyWord kw = lexer_get_keyword(lw);
        e8_utf_free(lw);
        if (kw == kwAnd) {
            e8_utf_free( tr_identifier(d) );
            and_chain = true;
        }
    }
    e8_label_index
        false_label = tr_label(d),
        out_label = tr_label(d)
    ;
    do {

        if (( __err = tr_parse_020(d) ))
            return __err;

        tr_skip_trash(d);

        e8_uchar *lw = tr_lookup_word(d);
        KeyWord kw = lexer_get_keyword(lw);
        e8_utf_free(lw);

        if (kw == kwAnd || and_chain) {

            m_commands_push_back(d, command_jump(false_label, E8_COMMAND_JumpFalse));
            need_false_label = true;

            if (kw == kwAnd) {
                e8_utf_free( tr_identifier(d) );
                and_chain = true;
            } else
                break;

        } else
            break;

    } while (true);

    if (need_false_label) {
        m_commands_push_back(d, command_push_const(CONST_TRUE));
        m_commands_push_back(d, command_jump(out_label, E8_COMMAND_Jump));
        LABEL_HERE((false_label));
        m_commands_push_back(d, command_push_const(CONST_FALSE));
        LABEL_HERE((out_label));
    }

    return E8_NO_THROW;
}

// [ИЛИ ]А[ ИЛИ Б]
static
e8_syntax_error *tr_parse_expression(e8_data *d)
{
    e8_syntax_error *__err;
    bool or_chain = false, need_true_label = false;
    { // пропускаем впереди идущее ИЛИ
        e8_uchar *lw = tr_lookup_word(d);
        KeyWord kw = lexer_get_keyword(lw);
        e8_utf_free(lw);
        if (kw == kwOr) {
            e8_utf_free( tr_identifier(d) );
            or_chain = true;
        }
    }
    e8_label_index
        true_label = tr_label(d),
        out_label = tr_label(d)
    ;
    do {

        if (( __err = tr_parse_010(d) ))
            return __err;

        tr_skip_trash(d);

        e8_uchar *lw = tr_lookup_word(d);
        KeyWord kw = lexer_get_keyword(lw);
        e8_utf_free(lw);
        if (kw == kwOr || or_chain) {

            m_commands_push_back(d, command_jump(true_label, E8_COMMAND_JumpTrue));
            need_true_label = true;

            if (kw == kwOr) {
                e8_utf_free( tr_identifier(d) );
                or_chain = true;
            } else
                break;

        } else
            break;

    } while (true);

    if (need_true_label) {
        m_commands_push_back(d, command_push_const(CONST_FALSE));
        m_commands_push_back(d, command_jump(out_label, E8_COMMAND_Jump));
        LABEL_HERE((true_label));
        m_commands_push_back(d, command_push_const(CONST_TRUE));
        LABEL_HERE((out_label));
    }

    return E8_NO_THROW;
}

static
e8_syntax_error *tr_expect_keyword(e8_data *d, KeyWord kw)
{
    e8_uchar *word = tr_identifier(d);
    KeyWord fkw = lexer_get_keyword(word);
    e8_utf_free(word);
    if (fkw != kw) {
        return throw_error(E8_KEYWORD_EXPECTED, d);
    }

    if (!d->m_preproc_parse)
        tr_skip_trash(d);

    return E8_NO_THROW;
}

static
e8_syntax_error *tr_parse_declared_param(e8_data *d)
{

    e8_syntax_error *__err = 0;

    bool by_val = false;
    e8_var default_value;
    e8_var_void(&default_value);

    e8_uchar *lw = tr_lookup_word(d);
    KeyWord kw = lexer_get_keyword(lw);
    e8_utf_free(lw);

    if (kw == kwVal) {
        e8_utf_free( tr_identifier(d) );
        by_val = true;
    }
    e8_uchar *param_name = tr_identifier(d);
    if (current(d, 0) == '=') {
        next(d);
        tr_skip_trash(d);

        if (( __err = tr_parse_const_expression(d, &default_value) ))
            return __err;
    }

    e8_label_index lindex = d->current_local_var->size;
    e8_collection_add_size(d->current_local_var, 1);
    d->current_local_var->data[lindex] = e8_utf_strdup(param_name);

    __append_name(d, param_name, lindex);
    e8_utf_free(param_name);

    if (by_val) {
        m_commands_push_back(d, command_byval(lindex, __add_const(d, &default_value)));
    } else
        m_commands_push_back(d, command_byref(lindex, __add_const(d, &default_value)));


    return E8_NO_THROW;
}

static
e8_syntax_error *tr_parse_declared_params(e8_data *d)
{
    e8_syntax_error *__err;

    while (current(d, 0) != ')') {

        if (( __err = tr_parse_declared_param(d) ))
            return __err;

        tr_skip_trash(d);
        if (current(d, 0) != ',' && current(d, 0) != ')') {
            return throw_error(E8_CLOSE_BRACKET_EXPECTED, d);
        }
        if (current(d, 0) == ',') {
            next(d);
            tr_skip_trash(d);
        }
    }
    return E8_NO_THROW;
}

static
e8_syntax_error *tr_kw_if(e8_data *d, bool *__result)
{
    e8_syntax_error *__err;
    e8_uchar *lw;

            if (( __err = tr_parse_expression(d) )) /* булево выраженіе */
                return __err;


            lw = tr_lookup_word(d);
            KeyWord kw_st = lexer_get_keyword(lw);
            e8_utf_free(lw);

            bool ecma = kw_st != kwThen && !d->strict_syntax;

            if (!ecma) {
                if (( __err = tr_expect_keyword(d, kwThen) ))
                    return __err;
            }

            int false_label = tr_label(d);
            int true_label = tr_label(d);
            m_commands_push_back(d, command_jump(false_label, E8_COMMAND_JumpFalse));

            if (ecma) {
                bool __r;
                if ( (__err = tr_parse_command(d, &__r)) )
                    return __err;
            }
            else {
                if ( (__err = tr_parse_command_block(d)) )
                    return __err;
            }

            m_commands_push_back(d, command_jump(true_label, E8_COMMAND_Jump));
            LABEL_HERE((false_label));

            lw = tr_lookup_word(d);
            KeyWord lkw = lexer_get_keyword(lw);
            e8_utf_free(lw);

            if (!ecma) {
                while (lkw == kwElseIf) {
                    e8_utf_free( tr_identifier(d) ); // пропустить ElseIf

                    tr_skip_trash(d);
                    if (( __err = tr_parse_expression(d) ))
                        return __err;

                    if (( __err = tr_expect_keyword(d, kwThen) ))
                        return __err;

                    false_label = tr_label(d);
                    m_commands_push_back(d, command_jump(false_label, E8_COMMAND_JumpFalse));

                    if (( __err = tr_parse_command_block(d) ))
                        return __err;

                    m_commands_push_back(d, command_jump(true_label, E8_COMMAND_Jump));

                    LABEL_HERE((false_label));

                    lw = tr_lookup_word(d);
                    lkw = lexer_get_keyword(lw);
                    e8_utf_free(lw);
                }
            }
            if (lkw == kwElse) {
                e8_utf_free( tr_identifier(d) ); // пропустить Else
                if (ecma) {
                    bool __r;
                    if (( __err = tr_parse_command(d, &__r) ))
                        return __err;
                }
                else {
                    if (( __err = tr_parse_command_block(d) ))
                        return __err;
                }
            }

            if (!ecma) {
                if (( __err = tr_expect_keyword(d, kwEndIf) ))
                    return __err;
            }

            LABEL_HERE((true_label));

            if (current(d, 0) == ';') {
                next(d);
                tr_skip_trash(d);
            }
    return E8_NO_THROW;
}

static
e8_syntax_error *tr_kw_while(e8_data *d, bool *__result)
{
    e8_syntax_error *__err;
    e8_uchar *lw;

            unsigned loop_label = tr_label(d);

            unsigned out_label = tr_label(d);

            loops_push_back(d, LoopLabels(loop_label, out_label));

            LABEL_HERE((loop_label));

            if (( __err = tr_parse_expression(d) ))
                return __err;

            tr_skip_trash(d);

            lw = tr_lookup_word(d);
            KeyWord kw_st = lexer_get_keyword(lw);
            e8_utf_free(lw);

            bool ecma = kw_st != kwDo && !d->strict_syntax;

            if (!ecma) {
                if (( __err = tr_expect_keyword(d, kwDo) ))
                    return __err;
            }

            m_commands_push_back(d, command_jump(out_label, E8_COMMAND_JumpFalse));

            if (ecma) {
                bool __r;
                if (( __err = tr_parse_command(d, &__r) ))
                    return __err;
            }
            else {
                if (( __err = tr_parse_command_block(d) ))
                    return __err;

                if (( __err = tr_expect_keyword(d, kwEndDo) ))
                    return __err;
            }

            m_commands_push_back(d, command_jump(loop_label, E8_COMMAND_Jump));
            LABEL_HERE((out_label));

            if (current(d, 0) == ';') {
                next(d);
                tr_skip_trash(d);
            }

            loops_pop_back(d);

    return E8_NO_THROW;
}

static
e8_syntax_error *tr_kw_for(e8_data *d, bool *__result)
{
    e8_syntax_error *__err;
    e8_uchar *lw;

            lw = tr_lookup_word(d);
            KeyWord kw_each = lexer_get_keyword(lw);
            e8_utf_free(lw);

            if (kw_each == kwEach
                || (kw_each == kwNotKeyWord && current(d, 0) == '(') ) {

                e8_uchar *loop_var;

                if (kw_each == kwEach) {
                    // For Each ... Do
                    e8_utf_free( tr_identifier(d) );
                    tr_skip_trash(d);

                    loop_var = tr_identifier(d);

                    if (( __err = tr_expect_keyword(d, kwIn) ))
                        return __err;

                } else {
                    // For (<Collection>)
                    loop_var = tr_temp_var_id(d);
                    next(d);
                    tr_skip_trash(d);
                }

                dot_var_push_back(d, loop_var);
                e8_uchar *loop_collection = tr_temp_var_id(d);
                m_commands_push_back(d, command_load_ref(__add_const_text(d, loop_collection)));

                if (( __err = tr_parse_expression(d) ))
                    return __err;

                m_commands_push_back(d, command_op(E8_COMMAND_Assign));

                if (kw_each != kwEach) {
                    next(d); // )
                    tr_skip_trash(d);
                }

                lw = tr_lookup_word(d);
                KeyWord kw_st = lexer_get_keyword(lw);
                e8_utf_free(lw);

                bool ecma = kw_st != kwDo && !d->strict_syntax;

                if (!ecma) {
                    if (( __err = tr_expect_keyword(d, kwDo) ))
                        return __err;
                }

                e8_uchar *it_var = tr_temp_var_id(d);

                m_commands_push_back(d, command_load_ref(__add_const_text(d, it_var)));
                m_commands_push_back(d, command_load_ref(__add_const_text(d, loop_collection)));
                m_commands_push_back(d, command_iterator());
                m_commands_push_back(d, command_op(E8_COMMAND_Assign));

                unsigned loop_label = tr_label(d);
                unsigned out_label = tr_label(d);
                loops_push_back(d, LoopLabels(loop_label, out_label));

                LABEL_HERE((loop_label));
                m_commands_push_back(d, command_load_ref(__add_const_text(d, it_var)));
                PUSH_CSTRING("__HasNext");
                PUSH_LONG(0);
                CALL(E8_COMMAND_ObjectCall);
                m_commands_push_back(d, command_jump(out_label, E8_COMMAND_JumpFalse));

                m_commands_push_back(d, command_load_ref(__add_const_text(d, loop_var)));
                m_commands_push_back(d, command_load_ref(__add_const_text(d, it_var)));
                PUSH_CSTRING("__Next");
                PUSH_LONG(0);
                CALL(E8_COMMAND_ObjectCall);
                m_commands_push_back(d, command_op(E8_COMMAND_Assign));

                e8_utf_free(loop_collection);

                if (!ecma) {

                    if (( __err = tr_parse_command_block(d) ))
                        return __err;

                    if (( __err = tr_expect_keyword(d, kwEndDo) ))
                        return __err;

                } else {
                    bool __r;
                    if (( __err = tr_parse_command(d, &__r) ))
                        return __err;
                }

                m_commands_push_back(d, command_jump(loop_label, E8_COMMAND_Jump));
                LABEL_HERE((out_label));

                m_commands_push_back(d, command_load_ref(__add_const_text(d, it_var)));
                m_commands_push_back(d, command_push_const(CONST_UNDEF));
                m_commands_push_back(d, command_op(E8_COMMAND_Assign));

                e8_utf_free(it_var);

                if (current(d, 0) == ';') {
                    next(d);
                    tr_skip_trash(d);
                }

                loops_pop_back(d);
                dot_var_pop_back(d);

            } else {
                // For ... To ... Do
                e8_uchar *loop_var = tr_identifier(d);
                if (current(d, 0) != '=') {
                    return throw_error(E8_ASSIGN_CHAR_EXPECTED, d);
                }

                next(d);
                tr_skip_trash(d);

                e8_label_index i_loop_var = WRONG_LABEL;

                i_loop_var = __get_local_index(d, loop_var);

                if (i_loop_var == WRONG_LABEL)
                    m_commands_push_back(d, command_load_ref(__add_const_text(d, loop_var)));
                else
                    m_commands_push_back(d, command_load_ref_local(i_loop_var));

                tr_skip_trash(d);

                if (( __err = tr_parse_expression(d) ))
                    return __err;

                tr_skip_trash(d);

                m_commands_push_back(d, command_op(E8_COMMAND_Assign));

                unsigned loop_label = tr_label(d);
                unsigned out_label = tr_label(d);
                unsigned continue_label = tr_label(d);

                loops_push_back(d, LoopLabels(continue_label, out_label));

                if (( __err = tr_expect_keyword(d, kwTo) ))
                    return __err;

                e8_uchar *final_var = tr_temp_var_id(d);

                e8_label_index i_final_var = WRONG_LABEL;
                i_final_var = d->current_local_var->size;
                e8_collection_add_size(d->current_local_var, 1);
                d->current_local_var->data[i_final_var] = e8_utf_strdup(final_var);

                m_commands_push_back(d, command_load_ref_local(i_final_var));

                if (( __err = tr_parse_expression(d) ))
                    return __err;

                m_commands_push_back(d, command_op(E8_COMMAND_Assign));

                LABEL_HERE((loop_label));

                if (i_loop_var == WRONG_LABEL)
                    m_commands_push_back(d, command_load_ref(__add_const_text(d, loop_var)));
                else
                    m_commands_push_back(d, command_load_ref_local(i_loop_var));

                if (i_final_var == WRONG_LABEL)
                    m_commands_push_back(d, command_load_ref(__add_const_text(d, final_var)));
                else
                    m_commands_push_back(d, command_load_ref_local(i_final_var));

                m_commands_push_back(d, command_op(E8_COMMAND_LessEqual));
                m_commands_push_back(d, command_jump(out_label, E8_COMMAND_JumpFalse));

                lw = tr_lookup_word(d);
                KeyWord kw_st = lexer_get_keyword(lw);
                e8_utf_free(lw);

                e8_utf_free(final_var);
                if (i_final_var != WRONG_LABEL) {
                    if (d->in_sub == NO_SUB)
                        d->local_vars.size--;
                    else
                        d->sub_local_vars.size--;
                }

                bool ecma = kw_st != kwDo && !d->strict_syntax;

                if (!ecma) {

                    if (( __err = tr_expect_keyword(d, kwDo) ))
                        return __err;

                    if (( __err = tr_parse_command_block(d) ))
                        return __err;

                    if (( __err = tr_expect_keyword(d, kwEndDo) ))
                        return __err;

                } else {
                    bool __r;
                    if (( __err = tr_parse_command(d, &__r) ))
                        return __err;
                }

                LABEL_HERE((continue_label));

                if (i_loop_var == WRONG_LABEL)
                    m_commands_push_back(d, command_load_ref(__add_const_text(d, loop_var)));
                else
                    m_commands_push_back(d, command_load_ref_local(i_loop_var));

                m_commands_push_back(d, command_op(E8_COMMAND_Inc));

                m_commands_push_back(d, command_jump(loop_label, E8_COMMAND_Jump));
                LABEL_HERE((out_label));

                if (current(d, 0) == ';') {
                    next(d);
                    tr_skip_trash(d);
                }
                loops_pop_back(d);
                e8_utf_free(loop_var);
            } // for i =

    return E8_NO_THROW;
}

static
e8_syntax_error *tr_kw_sub(e8_data *d, bool *__result, KeyWord kw)
{
    e8_syntax_error *__err;
    e8_uchar *lw;
    e8_uchar *sub_name = tr_identifier(d);

    d->current_commands = &d->subs_commands;
    d->current_local_var = &d->sub_local_vars;

    int sbase = d->symbols.size, snow = sbase;
    int cbase = d->subs_commands.size;

    unsigned table_index = __add_name_table(d);
    m_commands_push_back(d, command_table(table_index));

            e8_collection_add_size(&d->symbols, 1);
            d->symbols.data[snow].name = e8_string_init(sub_name);
            d->symbols.data[snow].address = cbase;
            d->symbols.data[snow]._export_symbol = false;
            snow++;

            e8_utf_free(sub_name);

            lw = tr_lookup_word(d);
            KeyWord _kw = lexer_get_keyword(lw);
            e8_utf_free(lw);

            while (_kw == kwOr) {
                e8_utf_free( tr_identifier(d) );
                e8_uchar *alt_name = tr_identifier(d);

                e8_collection_add_size(&d->symbols, 1);
                d->symbols.data[snow].name = e8_string_init(alt_name);
                d->symbols.data[snow].address = cbase;
                d->symbols.data[snow]._export_symbol = false;
                snow++;

                e8_utf_free(alt_name);

                lw = tr_lookup_word(d);
                _kw = lexer_get_keyword(lw);
                e8_utf_free(lw);
            }

            tr_skip_trash(d);
            if (current(d, 0) != '(') {
                return throw_error(E8_OPEN_BRACKET_EXPECTED, d);
            }
            next(d);
            tr_skip_trash(d);

            if (( __err = tr_parse_declared_params(d) ))
                return __err;

            tr_skip_trash(d);
            if (current(d, 0) != ')') {
                return throw_error(E8_CLOSE_BRACKET_EXPECTED, d);
            }
            next(d);
            tr_skip_trash(d);

            lw = tr_lookup_word(d);
            if (lexer_get_keyword(lw) == kwExport) {

                /* Помечаем символы экспортными */
                while (sbase < snow) {
                    d->symbols.data[sbase]._export_symbol = true;
                    ++sbase;
                }

                e8_utf_free( tr_identifier(d) ); // пропускаем слово "Экспорт"
                tr_skip_trash(d);
            }
            e8_utf_free(lw);

            if (kw == kwProcedure)
                d->in_sub = IN_PROC;
            else
                d->in_sub = IN_FUNC;

            if (( __err = tr_parse_command_block(d) ))
                return __err;

            if (kw == kwProcedure) {
                if (( __err = tr_expect_keyword(d, kwEndProcedure) ))
                    return __err;
            }
            else {

                if (( __err = tr_expect_keyword(d, kwEndFunction) ))
                    return __err;
            }

            m_commands_push_back(d, command_push_const(CONST_UNDEF));
            m_commands_push_back(d, command_ret());

            d->in_sub = NO_SUB;
            d->current_commands = &d->commands;

            int i = d->sub_local_vars.size;
            while (i--)
                e8_utf_free(d->sub_local_vars.data[i]);

            e8_collection_free(&d->sub_local_vars);
            d->current_local_var = &d->local_vars;

    return E8_NO_THROW;
}

static
e8_syntax_error *tr_kw_assert(e8_data *d, bool *__result)
{
    e8_syntax_error *__err;
    e8_uchar *lw;

            e8_script_position_t i_start = tr_index(d);

            if (( __err = tr_parse_expression(d) ))
                return __err;

            e8_script_position_t i_end = tr_index(d);
            /* wstring msg = r->substr(i_start, i_end); */
            (void)i_end; (void)i_start;
            /* TODO: Отмѣнённая фича */
            e8_uchar *msg = 0;

            {
                lw = tr_lookup_word(d);
                KeyWord _kw = lexer_get_keyword(lw);
                e8_utf_free(lw);

                if (_kw == kwAs) {

                    e8_utf_free( tr_identifier(d) );

                    e8_var vm;
                    if (( __err = tr_parse_string_const_expression(d, &vm) ))
                        return __err;

                    e8_string *svm;
                    e8_var_cast_string(&vm, &svm);
                    e8_var_destroy(&vm);

                    msg = e8_utf_strdup(svm->s);
                    e8_string_destroy(svm);
                }
            }

            if (current(d, 0) == ';') {
                next(d);
                tr_skip_trash(d);
            }

            e8_var v_msg;
            e8_var_string(&v_msg, msg);
            e8_utf_free(msg);

            m_commands_push_back(d, command_assert_condition(__add_const(d, &v_msg)));

    return E8_NO_THROW;
}

static
e8_syntax_error *tr_kw_break_continue(e8_data *d, bool *__result, KeyWord kw)
{
    e8_syntax_error *__err;
            long depth = 1;
            if (tr_is_const_expression(d)) {

                e8_var v;
                if (( __err = tr_parse_numeric_const_expression(d, &v) ))
                    return __err;

                e8_numeric n_depth = v.num;
                /* TODO: e8_var_cast_numeric */
                depth = e8_numeric_as_long(&n_depth);
            }
            if (depth < 1 || depth > d->loops.size) {
                return throw_error(E8_WRONG_LOOP_DEPTH, d);
            }

            unsigned jump_label;
            if (kw == kwBreak)
                jump_label = d->loops.data[d->loops.size - depth].out_label;
            else
                jump_label = d->loops.data[d->loops.size - depth].loop_label;

            m_commands_push_back(d, command_jump(jump_label, E8_COMMAND_Jump));
            tr_skip_trash(d);
            if (current(d, 0) == ';') {
                next(d);
                tr_skip_trash(d);
            }
    return E8_NO_THROW;
}

static
e8_syntax_error *tr_kw_try(e8_data *d, bool *__result)
{
    e8_syntax_error *__err;
            unsigned catch_label = tr_label(d);
            unsigned out_label = tr_label(d);
            unsigned end_critical = tr_label(d);

            m_commands_push_back(d, command_critical(catch_label));
            if (( __err = tr_parse_command_block(d) ))
                return __err;

            m_commands_push_back(d, command_jump(end_critical, E8_COMMAND_Jump));

            if (( __err = tr_expect_keyword(d, kwException) ))
                return __err;

            LABEL_HERE((catch_label));

            if (( __err = tr_parse_command_block(d) ))
                return __err;

            if (( __err = tr_expect_keyword(d, kwEndTry) ))
                return __err;

            LABEL_HERE((end_critical));
            m_commands_push_back(d, command_end_critical());
            LABEL_HERE((out_label));

            if (current(d, 0) == ';') {
                next(d);
                tr_skip_trash(d);
            }
    return E8_NO_THROW;
}

static
e8_syntax_error *tr_parse_command(e8_data *d, bool *__result)
{
    if (!tr_skip_trash(d)) {
        *__result = false;
        return E8_NO_THROW;
    }

    if (current(d, 0) == ';') {
        next(d);
        tr_skip_trash(d);
        *__result = true;
        return E8_NO_THROW;
    }

    unsigned long old_pos = tr_index(d), old_line = d->current_line;
    e8_uchar *l_id = tr_identifier(d);
    if (e8_utf_strlen(l_id) == 0
            && !(current(d, 0) == '.' && !lexer_is_num(current(d, +1)))
        ) {
        return throw_error(E8_IDENTIFIER_EXPECTED, d);
    }

    e8_syntax_error *__err = 0;
    e8_uchar *lw;

    // сначала проверить на ключевые слова
    KeyWord kw = lexer_get_keyword(l_id);
    e8_utf_free(l_id);

    if (kw != kwNotKeyWord) {

        // If .... Then
        if (kw == kwIf) {

            if (( __err = tr_kw_if(d, __result) ))
                return __err;

        } else if (lexer_is_end_keyword(kw)) {

            // Конец блока
            tr_r_index(d, old_pos);
            d->current_line = old_line;
            *__result = false;

            return E8_NO_THROW;

        } else if (kw == kwElse || kw == kwElseIf || kw == kwException) {

            // Конец-начало блока
            tr_r_index(d, old_pos);
            d->current_line = old_line;
            *__result = false;

            return E8_NO_THROW;

        } else

        // Var name{,name};
        if (kw == kwVar) {

            // TODO: разбор списка переменных
            e8_uchar *var_name = tr_identifier(d);
            bool is_export = false;
            {
                e8_uchar *_export_item = tr_lookup_word(d);
                KeyWord kw_ex = lexer_get_keyword(_export_item);
                if (kw_ex == kwExport) {
                    e8_utf_free(tr_identifier(d));
                    is_export = true;
                }
                e8_utf_free(_export_item);
            }

            e8_label_index lindex = d->current_local_var->size;
            e8_collection_add_size(d->current_local_var, 1);
            d->current_local_var->data[lindex] = e8_utf_strdup(var_name);

            __append_name(d, var_name, lindex);

            m_commands_push_back(d, command_var(lindex));

            if (is_export) {
                int i = d->symbols.size;
                e8_collection_add_size(&d->symbols, 1);
                d->symbols.data[i].address = WRONG_ADDRESS;
                d->symbols.data[i].name = e8_string_init(var_name);
                d->symbols.data[i]._export_symbol = true;
            }

            e8_utf_free(var_name);

            while (current(d, 0) == ',') {
                next(d);
                tr_skip_trash(d);
                var_name = tr_identifier(d);

                is_export = false;
                {
                    e8_uchar *_export_item = tr_lookup_word(d);
                    KeyWord kw_ex = lexer_get_keyword(_export_item);
                    if (kw_ex == kwExport) {
                        e8_utf_free(tr_identifier(d));
                        is_export = true;
                    }
                    e8_utf_free(_export_item);
                }

                e8_label_index lindex = d->current_local_var->size;
                e8_collection_add_size(d->current_local_var, 1);
                d->current_local_var->data[lindex] = e8_utf_strdup(var_name);

                __append_name(d, var_name, lindex);

                m_commands_push_back(d, command_var(lindex));

                if (is_export) {
                    int i = d->symbols.size;
                    e8_collection_add_size(&d->symbols, 1);
                    d->symbols.data[i].address = WRONG_ADDRESS;
                    d->symbols.data[i].name = e8_string_init(var_name);
                    d->symbols.data[i]._export_symbol = true;
                }

                e8_utf_free(var_name);
            }
            if (current(d, 0) == ';') {
                next(d);
                tr_skip_trash(d);
            }

        } else
        // Execute StringExpression;
        if (kw == kwExecute) {

            if (( __err = tr_parse_expression(d) ))
                return __err;

            PUSH_LONG(1);
            PUSH_CSTRING("execute");
            CALL(E8_COMMAND_Call);

            if (current(d, 0) == ';') {
                next(d);
                tr_skip_trash(d);
            }

        } else
        // While ... Do
        if (kw == kwWhile) {

            if (( __err = tr_kw_while(d, __result) ))
                return __err;

        } else
        // For ...
        if (kw == kwFor) {

            if (( __err = tr_kw_for(d, __result)))
                return __err;

        } else
        // Procedure ....     Function .....
        if (kw == kwProcedure || kw == kwFunction) {

            if (( __err = tr_kw_sub(d, __result, kw) ))
                return __err;

        } else if (kw == kwReturn) {

            if (d->in_sub == IN_PROC) {
                /* Процедура возвращает Неопределено */
                m_commands_push_back(d, command_push_const(CONST_UNDEF));
            } else if (d->in_sub == IN_FUNC) {

                if (( __err = tr_parse_expression(d) ))
                    return __err;

            }
            tr_skip_trash(d);
            if (current(d, 0) == ';') {
                next(d);
                tr_skip_trash(d);
            }
            m_commands_push_back(d, command_ret());

        } else
        // Raise
        if (kw == kwRaise) {

            lw = tr_lookup_word(d);
            KeyWord _kw = lexer_get_keyword(lw);
            e8_utf_free(lw);

            if (_kw == kwNotKeyWord && current(d, 0) != ';') {

                if (( __err = tr_parse_expression(d) ))
                    return __err;

            }
            else {
                /* m_commands_push_back(d, command_push_const(Variant(L"Исключение"))); */
                PUSH_CSTRING("Exception");
            }
            m_commands_push_back(d, command_raise());
            tr_skip_trash(d);
            if (current(d, 0) == ';') {
                next(d);
                tr_skip_trash(d);
            }
        } else

        // Assert
        if (kw == kwAssert) {

            if (( __err = tr_kw_assert(d, __result) ))
                return __err;

        } else
        // Break | Continue
        if (kw == kwBreak || kw == kwContinue) {

            if (( __err = tr_kw_break_continue(d, __result, kw) ))
                return __err;

        } else
        // Try .... Exception ...  EndTry
        if (kw == kwTry) {

            if (( __err = tr_kw_try(d, __result) ))
                return __err;

        } else {
            *__result = false;
            return E8_NO_THROW;
        }

        tr_skip_trash(d);
        *__result = true;
        return E8_NO_THROW;
    }

    tr_r_index(d, old_pos);
    d->current_line = old_line;
    bool is_lvalue;

    if (( __err = tr_parse_complex_identifier(d, &is_lvalue) ))
        return __err;

    if (!is_lvalue) {
        POP;
    } else {

        int assign_addition = 0;
        {
            e8_uchar assign_add_char = current(d, 0);
            switch (assign_add_char) {
            case '+':
                assign_addition = E8_COMMAND_Add;
                break;
            case '-':
                assign_addition = E8_COMMAND_Sub;
                break;
            case '/':
                assign_addition = E8_COMMAND_Div;
                break;
            case '*':
                assign_addition = E8_COMMAND_Mul;
                break;
            case '%':
                assign_addition = E8_COMMAND_Mod;
                break;
            default:
                break;
            }
            if (assign_addition)
                next(d);
        }

        if (current(d, 0) != '=') {
            return throw_error(E8_ASSIGN_CHAR_EXPECTED, d);
        }

        /*
            Здѣсь у нас выраженіе типа a = .....;
            Транслируется в :
            LoadRef a
            *** вычислить выраженіе справа
            Op :=
        */

        next(d);
        tr_skip_trash(d);

        if (assign_addition)
            m_commands_push_back(d, command_stack_dup());

        if (( __err = tr_parse_expression(d) ))
            return __err;

        if (assign_addition)
            m_commands_push_back(d, command_op(assign_addition));

        e8_command c = command_op(E8_COMMAND_Assign);
        m_commands_push_back(d, c);

    }

    tr_skip_trash(d);

    if (current(d, 0) == ';') {
        next(d);
        tr_skip_trash(d);
    }

    *__result = true;
    return E8_NO_THROW;
} // parse_command


void e8_translator_set_server(e8_translator tr, bool flag)
{
    ((e8_data*)tr)->server = flag;
}
void e8_translator_set_script(e8_translator tr, bool flag)
{
    ((e8_data*)tr)->script = flag;
}
void e8_translator_set_thick_client(e8_translator tr, bool flag)
{
    ((e8_data*)tr)->thick_client = flag;
}
void e8_translator_set_web_client(e8_translator tr, bool flag)
{
    ((e8_data*)tr)->web_client = flag;
}
void e8_translator_set_thin_client_ordinary_application(e8_translator tr, bool flag)
{
    ((e8_data*)tr)->thin_client_ordinary_application = flag;
}
void e8_translator_set_thin_client_managed_application(e8_translator tr, bool flag)
{
    ((e8_data*)tr)->thin_client_managed_application = flag;
}
void e8_translator_set_strict_syntax(e8_translator tr, bool flag)
{
    ((e8_data*)tr)->strict_syntax = flag;
}

bool e8_translator_get_server(e8_translator tr)
{
    return ((e8_data*)tr)->server;
}
bool e8_translator_get_script(e8_translator tr)
{
    return ((e8_data*)tr)->script;
}
bool e8_translator_get_thick_client(e8_translator tr)
{
    return ((e8_data*)tr)->thick_client;
}
bool e8_translator_get_web_client(e8_translator tr)
{
    return ((e8_data*)tr)->web_client;
}
bool e8_translator_get_thin_client_ordinary_application(e8_translator tr)
{
    return ((e8_data*)tr)->thin_client_ordinary_application;
}
bool e8_translator_get_thin_client_managed_application(e8_translator tr)
{
    return ((e8_data*)tr)->thin_client_managed_application;
}
bool e8_translator_get_strict_syntax(e8_translator tr)
{
    return ((e8_data*)tr)->strict_syntax;
}


#undef PUSH_LONG
#undef PUSH_STRING
#undef CALL
#undef POP
