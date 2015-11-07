#include "_strings.h"
#include "e8core/variant/variant.h"
#include "e8core/translator/lexer.h"
#include "simpleenum.h"
#include <malloc.h>

e8_vtable vmt_string;
static e8_vtable vmt_string_iterator;

static e8_var v_chars;

#define CR '\r'
#define LF '\n'

static e8_vtable vmt_chars;

#define _CHARACTER_CR   0
#define _CHARACTER_VTAB 1
#define _CHARACTER_NBSP 2
#define _CHARACTER_LF   3
#define _CHARACTER_FF   4
#define _CHARACTER_TAB  5

static
E8_ENUM_START(chars)
    E8_ENUM_ADD("ВК",       "CR",       _CHARACTER_CR)
    E8_ENUM_ADD("ВТаб",     "VTab",     _CHARACTER_VTAB)
    E8_ENUM_ADD("НПП",      "NBsp",     _CHARACTER_NBSP)
    E8_ENUM_ADD("ПС",       "LF",       _CHARACTER_LF)
    E8_ENUM_ADD("ПФ",       "FF",       _CHARACTER_FF)
    E8_ENUM_ADD("Таб",      "Tab",      _CHARACTER_TAB)
E8_ENUM_END

static e8_var v_TextEncoding;
static e8_vtable vmt_TextEncoding;

#define _TEXT_ENCODING_MIN          0
#define _TEXT_ENCODING_ANSI         0
#define _TEXT_ENCODING_OEM          1
#define _TEXT_ENCODING_UTF16        2
#define _TEXT_ENCODING_UTF8         3
#define _TEXT_ENCODING_System       4
#define _TEXT_ENCODING_MAX          5

static
E8_ENUM_START(text_encodings)
    E8_ENUM_ADD("Ansi", "Ansi",         _TEXT_ENCODING_ANSI)
    E8_ENUM_ADD("Oem", "Oem",           _TEXT_ENCODING_OEM)
    E8_ENUM_ADD("Utf16", "Utf16",       _TEXT_ENCODING_UTF16)
    E8_ENUM_ADD("Utf8", "Utf8",         _TEXT_ENCODING_UTF8)
    E8_ENUM_ADD("System", "Системная",  _TEXT_ENCODING_System)
E8_ENUM_END

const char *
e8_get_encoding(int enc)
{
    switch (enc) {
    case _TEXT_ENCODING_ANSI:
        return "cp1251";
    case _TEXT_ENCODING_OEM:
        return "cp866";
    case _TEXT_ENCODING_UTF16:
        return "utf-16";
    case _TEXT_ENCODING_UTF8:
        return "utf-8";
    case _TEXT_ENCODING_System:
    default:
        #ifdef __linux
        return "utf-8";
        #endif // __linux
        #ifdef __WINNT
        return "cp1251";
        #endif
    }
}

E8_DECLARE_SUB(StrLen)
{
    E8_ARG_STRING(s);
    e8_var_long(result, s->len);
    e8_string_destroy(s);
    E8_RETURN_OK;
}

static inline bool is_ws(e8_uchar c)
{
    return c <= 32;
}

static e8_uchar *trim(const e8_uchar *src, bool left, bool right)
{
    e8_uchar *r = (e8_uchar *)src;
    if (left) {
        while (*r && is_ws(*r))
            ++r;
    }
    r = e8_utf_strdup(r);
    if (right) {
        int len = e8_utf_strlen(r);
        while (len && is_ws(r[len - 1]))
            r[--len] = 0;
    }
    return r;
}
static e8_uchar *substr(const e8_uchar *src, int start, int len)
{
    e8_uchar *res = e8_utf_strdup(src + start);
    res[len] = 0;
    return res;
}

E8_DECLARE_SUB(TrimL)
{
    E8_ARG_STRING(s);
    e8_uchar *r = trim(s->s, true, false);
    e8_var_string(result, r);
    e8_utf_free(r);
    e8_string_destroy(s);
    E8_RETURN_OK;
}
E8_DECLARE_SUB(TrimR)
{
    E8_ARG_STRING(s);
    e8_uchar *r = trim(s->s, false, true);
    e8_var_string(result, r);
    e8_utf_free(r);
    e8_string_destroy(s);
    E8_RETURN_OK;
}
E8_DECLARE_SUB(TrimAll)
{
    E8_ARG_STRING(s);
    e8_uchar *r = trim(s->s, true, true);
    e8_var_string(result, r);
    e8_utf_free(r);
    e8_string_destroy(s);
    E8_RETURN_OK;
}

E8_DECLARE_SUB(Left)
{
    E8_ARG_STRING(s);
    E8_ARG_LONG(count);

    e8_uchar *r = e8_utf_strdup(s->s);

    if (count < s->len)
        r[count] = 0;

    e8_var_string(result, r);
    e8_utf_free(r);
    e8_string_destroy(s);

    E8_RETURN_OK;
}

E8_DECLARE_SUB(Right)
{
    E8_ARG_STRING(s);
    E8_ARG_LONG(count);

    e8_uchar *r;

    if (count < s->len) {
        r = e8_utf_strdup(s->s + (s->len - count));
    } else
        r = e8_utf_strdup(s->s);

    e8_var_string(result, r);
    e8_utf_free(r);
    e8_string_destroy(s);

    E8_RETURN_OK;
}

E8_DECLARE_SUB(Mid)
{
    E8_ARG_STRING(s);
    E8_ARG_LONG(start_index);
    --start_index;
    long count = s->len - start_index;

    if (argc > 2) {
        E8_ARG_LONG(__count);
        if (__count < count)
            count = __count;
    }

    e8_uchar *r = substr(s->s, start_index, count);
    e8_var_string(result, r);
    e8_utf_free(r);

    e8_string_destroy(s);

    E8_RETURN_OK;
}

E8_DECLARE_SUB(Find)
{
    E8_ARG_STRING(s_a);
    E8_ARG_STRING(s_b);

    int start_pos = 0;
    if (argc > 2) {
        E8_ARG_LONG(__start);
        --__start;
        if (__start >= 0 && __start < s_a->len)
            start_pos = __start;
    }

    int i = e8_string_find(s_a, s_b, start_pos);
    e8_var_long(result, i + 1);

    e8_string_destroy(s_a);
    e8_string_destroy(s_b);

    E8_RETURN_OK;
}


E8_DECLARE_SUB(Upper)
{
    E8_ARG_STRING(s);
    e8_uchar *ucase = e8_utf_strdup(s->s);
    e8_utf_upper_case(ucase);

    e8_var_string(result, ucase);

    e8_string_destroy(s);
    e8_utf_free(ucase);

    E8_RETURN_OK;
}

E8_DECLARE_SUB(Lower)
{
    E8_ARG_STRING(s);
    e8_uchar *ucase = e8_utf_strdup(s->s);
    e8_utf_lower_case(ucase);

    e8_var_string(result, ucase);

    e8_string_destroy(s);
    e8_utf_free(ucase);

    E8_RETURN_OK;
}


E8_DECLARE_SUB(CharCode)
{
    E8_ARG_STRING(s);
    int index = 0;
    if (argc > 1) {
        E8_ARG_LONG(__index);
        --__index;
        if (__index < s->len)
            index = __index;
    }
    e8_var_long(result, s->s[index]);
    e8_string_destroy(s);
    E8_RETURN_OK;
}

E8_DECLARE_SUB(IsBlankString)
{
    E8_ARG_STRING(s);
    e8_uchar *r = trim(s->s, true, true);
    e8_var_bool(result, *r ? false : true);
    e8_utf_free(r);
    e8_string_destroy(s);

    E8_RETURN_OK;
}

E8_DECLARE_SUB(StrReplace)
{
    E8_ARG_STRING(src);
    E8_ARG_STRING(sub);
    E8_ARG_STRING(rep);

    e8_string_replace(src, sub, rep, 0);

    e8_var_string(result, src->s);

    e8_string_destroy(src);
    e8_string_destroy(sub);
    e8_string_destroy(rep);

    E8_RETURN_OK;
}

E8_DECLARE_SUB(StrLineCount)
{
    E8_ARG_STRING(src);

    long count = 0, i;
    for (i = 0; i < src->len; ++i)
        if (src->s[i] == LF)
            ++count;

    e8_var_long(result, count + 1);
    e8_string_destroy(src);

    E8_RETURN_OK;
}

E8_DECLARE_SUB(StrGetLine)
{
    E8_ARG_STRING(s);
    E8_ARG_LONG(index);

    int len = s->len;
    e8_uchar *uc = e8_utf_strdup(s->s);

    e8_string_destroy(s);

    long count = 0;
    long i, li = -1;

    for (i = 0; i < len; ++i) {
        if (count + 1 == index && li == -1)
            li = i;
        if (uc[i] == LF) {
            ++count;
            if (count == index) {
                uc[i] = 0;
                if (i > 0 && uc[i - 1] == CR)
                    uc[--i] = 0;

                e8_var_string(result, &uc[li]);
                e8_utf_free(uc);

                E8_RETURN_OK;
            }
        }
    }

    if (li == -1) {
        e8_uchar es[] = {0};
        e8_var_string(result, es);
    } else {
        e8_var_string(result, &uc[li]);
    }

    e8_utf_free(uc);

    E8_RETURN_OK;
}

E8_DECLARE_SUB(StrOccurrenceCount)
{
    E8_ARG_STRING(s_src);
    E8_ARG_STRING(s_occ);

    int r = 0;

    if (s_occ->len) {
        int i = e8_string_find(s_src, s_occ, 0);
        while (i != -1) {
            ++r;
            i = e8_string_find(s_src, s_occ, i + s_occ->len);
        }
    }

    e8_var_long(result, r);

    e8_string_destroy(s_src);
    e8_string_destroy(s_occ);

    E8_RETURN_OK;
}

E8_DECLARE_SUB(Title)
{

    E8_ARG_STRING(s);

    int len = s->len;
    e8_uchar *uc = e8_utf_strdup(s->s);

    e8_utf_lower_case(uc);
    e8_string_destroy(s);

    int i;

    bool flag = true;
    for (i = 0; i < len; ++i) {

        if (flag) {
            uc[i] = e8_upcase(uc[i]);
            flag = false;
        } else
            uc[i] = e8_locase(uc[i]);

        if (lexer_is_ws(uc[i]) || lexer_is_nl(uc[i]))
            flag = true;
    }

    e8_var_string(result, uc);

    E8_RETURN_OK;
}

E8_DECLARE_SUB(Char)
{
    E8_ARG_LONG(code);

    e8_uchar us[] = {code, 0};
    e8_var_string(result, us);

    E8_RETURN_OK;
}

#define REGISTER(name, fn) \
{\
    e8_env_add_function_utf(env, name, fn);\
}

static e8_uchar _s2b[] = {0, 0};

static e8_var *__e8_var_char(e8_var *result, e8_uchar c)
{
    _s2b[0] = c;
    e8_var_destroy(result);
    e8_var_string(result, _s2b);
    return result;
}

static void
__e8_strings_free(const e8_environment env)
{
}

#define _STR(name) e8_string *name = (e8_string *)obj

typedef struct __e8_string_iterator_t {
    e8_string      *s;
    size_t          current;
} e8_string_iterator;

#define _S_IT(name) e8_string_iterator *name = (e8_string_iterator *)obj

static
E8_DECLARE_SUB(__e8_string_iterator_has_next)
{
    _S_IT(it);
    e8_var_bool(result, it->current <= it->s->len);
    E8_RETURN_OK;
}

static
E8_DECLARE_SUB(__e8_string_iterator_next)
{
    _S_IT(it);

    e8_uchar current[2];
    current[0] = it->s->s[it->current++];
    current[1] = 0;

    e8_var_string(result, current);

    E8_RETURN_OK;
}

static
E8_DECLARE_DESTRUCTOR(__e8_string_iterator_destroy)
{
    _S_IT(it);
    e8_string_destroy(it->s);
    free(it);
}

#undef _S_IT

static
E8_METHOD_LIST_START(__e8_string_iterator_methods)
    E8_METHOD_LIST_ADD_HAS_NEXT(__e8_string_iterator_has_next)
    E8_METHOD_LIST_ADD_NEXT(__e8_string_iterator_next)
E8_METHOD_LIST_END

static
E8_DECLARE_GET_METHOD(__e8_string_iterator_get_method)
{
    e8_simple_get_method(name, __e8_string_iterator_methods, fn);
    E8_RETURN_OK;
}

static
E8_DECLARE_SUB(__e8_string_iterator)
{
    _STR(s);

    e8_string *cpy = e8_string_copy(s);

    e8_string_iterator *it = malloc(sizeof(*it));
    it->s = cpy;
    it->current = 0;

    e8_gc_register_object(it, &vmt_string_iterator);
    e8_var_object(result, it);

    E8_RETURN_OK;
}

static
E8_DECLARE_DESTRUCTOR(__e8_string_destroy)
{
    _STR(s);
    e8_string_destroy(s);
}

static
E8_DECLARE_SUB(__e8_string_count)
{
    _STR(s);
    e8_var_long(result, s->len);
    E8_RETURN_OK;
}

static
E8_DECLARE_PROPERTY_GET(__e8_string_element_get)
{
    e8_property *p = (e8_property *)property;
    e8_string *s = (e8_string *)p->data;

    e8_var_string(value, s->s);
    e8_string_destroy(s);

    E8_RETURN_OK;
}

static
E8_DECLARE_GET_BY_INDEX(__e8_string_by_index)
{
    _STR(s);

    long l_index;
    if (e8_var_cast_long(index, &l_index))
        E8_THROW(E8_RT_INDEX_OUT_OF_BOUNDS);

    if (l_index < 0 || l_index >= s->len)
        E8_THROW(E8_RT_INDEX_OUT_OF_BOUNDS);

    e8_uchar A[] = {s->s[l_index], 0};
    e8_string *ns = e8_string_init(A);

    property->can_set = false;
    property->get = __e8_string_element_get;
    property->data = ns;

    E8_RETURN_OK;
}

static
E8_METHOD_LIST_START(__e8_string_methods)
    E8_METHOD_LIST_ADD("Количество", "Count", __e8_string_count)
    E8_METHOD_LIST_ADD_ITERATOR(__e8_string_iterator)
E8_METHOD_LIST_END

static
E8_DECLARE_GET_METHOD(__e8_string_get_method)
{
    e8_simple_get_method(name, __e8_string_methods, fn);
    E8_RETURN_OK;
}

#undef _STR

void e8_register_strings(e8_environment env)
{

    e8_prepare_call_elements(__e8_string_methods, "utf-8");
    e8_vtable_template(&vmt_string);
    vmt_string.get_method = __e8_string_get_method;
    vmt_string.by_index = __e8_string_by_index;
    vmt_string.dtor = __e8_string_destroy;

    e8_prepare_call_elements(__e8_string_iterator_methods, "utf-8");
    e8_vtable_template(&vmt_string_iterator);
    vmt_string_iterator.get_method = __e8_string_iterator_get_method;
    vmt_string_iterator.dtor = __e8_string_iterator_destroy;

    REGISTER("StrLen", StrLen);

    REGISTER("TrimL", TrimL);
    REGISTER("TrimR", TrimR);
    REGISTER("TrimAll", TrimAll);

    REGISTER("Left", Left);
    REGISTER("Right", Right);
    REGISTER("Mid", Mid);

    REGISTER("Find", Find);
    REGISTER("Upper", Upper);
    REGISTER("Lower", Lower);

    REGISTER("Char", Char);
    REGISTER("CharCode", Upper);
    REGISTER("IsBlankString", Lower);

    REGISTER("StrReplace", StrReplace);
    REGISTER("StrLineCount", StrLineCount);
    REGISTER("StrGetLine", StrGetLine);
    REGISTER("StrOccurenceCount", StrOccurrenceCount);
    REGISTER("Title", Title);


    REGISTER("СтрДлина", StrLen);

    REGISTER("СокрЛ", TrimL);
    REGISTER("СокрП", TrimR);
    REGISTER("СокрЛП", TrimAll);

    REGISTER("Лев", Left);
    REGISTER("Прав", Right);
    REGISTER("Сред", Mid);

    REGISTER("Найти", Find);
    REGISTER("Врег", Upper);
    REGISTER("Нрег", Lower);

    REGISTER("Символ", Char);
    REGISTER("КодСимвола", Upper);
    REGISTER("ПустаяСтрока", IsBlankString);

    REGISTER("СтрЗаменить", StrReplace);
    REGISTER("СтрКоличествоСтрок", StrLineCount);
    REGISTER("СтрПолучитьСтроку", StrGetLine);
    REGISTER("СтрЧислоВхождений", StrOccurrenceCount);
    REGISTER("ТРег", Title);

    e8_simple_enum_init_vtable(&vmt_chars);
    e8_simple_enum_new(&v_chars, &vmt_chars);
    e8_simple_enum_attach_list(&v_chars, chars);

    e8_var v; e8_var_undefined(&v);
    e8_simple_enum_set_value(&v_chars, _CHARACTER_TAB,  __e8_var_char(&v, 9));
    e8_simple_enum_set_value(&v_chars, _CHARACTER_LF,   __e8_var_char(&v, 10));
    e8_simple_enum_set_value(&v_chars, _CHARACTER_VTAB, __e8_var_char(&v, 11));
    e8_simple_enum_set_value(&v_chars, _CHARACTER_FF,   __e8_var_char(&v, 12));
    e8_simple_enum_set_value(&v_chars, _CHARACTER_CR,   __e8_var_char(&v, 13));
    e8_simple_enum_set_value(&v_chars, _CHARACTER_NBSP, __e8_var_char(&v, 160));
    e8_var_destroy(&v);

    e8_simple_enum_init_vtable(&vmt_TextEncoding);
    e8_simple_enum_new(&v_TextEncoding, &vmt_TextEncoding);
    e8_simple_enum_attach_list(&v_TextEncoding, text_encodings);

    e8_env_add_global_value_utf(env, "Символы", &v_chars);
    e8_env_add_global_value_utf(env, "Chars", &v_chars);
    e8_env_add_global_value_utf(env, "TextEncoding", &v_TextEncoding);
    e8_env_add_global_value_utf(env, "КодировкаТекста", &v_TextEncoding);

    e8_var_destroy(&v_chars);

    e8_env_add_freelib_handler(env, __e8_strings_free);

}

#undef REGISTER
#undef CR
#undef LF
