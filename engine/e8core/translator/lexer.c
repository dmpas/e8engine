#include "lexer.h"
#include <malloc.h>
#include "e8core/utf/tables.h"

static char u8_delimiters[60] = ",./<>?;':\"[]{}\\|=+-)(*&^%$#@!`~";
static char u8_rus_small[200] = "абвгдеёжзийклмнопрстуфхцчшщъыьэюя";
static char u8_rus_large[200] = "АБВГДЕЁЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯ";
static e8_uchar delimiters[60];
static e8_uchar RUS_SMALL[60];
static e8_uchar RUS_LARGE[60];

static e8_uchar HEX[] = {
        'a', 'b', 'c', 'd', 'e', 'f',
        'A', 'B', 'C', 'D', 'E', 'F',
         cyr_aa,  cyr_bb,  cyr_cc,  cyr_dd,  cyr_ye,  cyr_ff,
         cyr_AA,  cyr_BB,  cyr_CC,  cyr_DD,  cyr_YE,  cyr_FF
        };

typedef struct __e8_kw_list {
    char eng[80];
    char rus[80];
    e8_uchar ueng[20];
    e8_uchar urus[20];
    KeyWord kw;
} e8_kw_list;

static
e8_kw_list key_words[] = {
{"if", "если", {}, {}    , kwIf},
{"var", "перем", {}, {}    , kwVar},
{"while", "пока", {}, {}    , kwWhile},
{"do", "цикл", {}, {}    , kwDo},
{"enddo", "конеццикла", {}, {}    , kwEndDo},
{"then", "тогда", {}, {}    , kwThen},
{"for", "для", {}, {}    , kwFor},
{"each", "каждого", {}, {}    , kwEach},
{"to", "по", {}, {}    , kwTo},
{"endif", "конецесли", {}, {}    , kwEndIf},
{"else", "иначе", {}, {}    , kwElse},
{"elseif", "иначеесли", {}, {}    , kwElseIf},
{"or", "или", {}, {}    , kwOr},
{"and", "и", {}, {}    , kwAnd},
{"not", "не", {}, {}    , kwNot},
{"new", "новый", {}, {}    , kwNew},
{"in", "из", {}, {}    , kwIn},
{"procedure", "процедура", {}, {}    , kwProcedure},
{"endprocedure", "конецпроцедуры", {}, {}    , kwEndProcedure},
{"function", "функция", {}, {}    , kwFunction},
{"endfunction", "конецфункции", {}, {}    , kwEndFunction},
{"export", "экспорт", {}, {}    , kwExport},
{"return", "возврат", {}, {}    , kwReturn},
{"undefined", "неопределено", {}, {}    , kwUndefined},
{"null", "null", {}, {}    , kwNull},
{"val", "знач", {}, {}    , kwVal},
{"true", "истина", {}, {}    , kwTrue},
{"false", "ложь", {}, {}    , kwFalse},
{"break", "прервать", {}, {}    , kwBreak},
{"continue", "продолжить", {}, {}    , kwContinue},

{"try", "попытка", {}, {}    , kwTry},
{"exception", "исключение", {}, {}    , kwException},
{"endtry", "конецпопытки", {}, {}    , kwEndTry},
{"raise", "вызватьисключение", {}, {}    , kwRaise},

{"execute", "выполнить", {}, {}    , kwRaise},

{"assert", "заявить", {}, {}    , kwAssert},

{"as", "как", {}, {}    , kwAs},
{{}, {}, {}, {}, kwNotKeyWord}
};



void e8_lexer_init_data()
{
    e8_uchar *d;

    d = delimiters;
    e8_utf_from_8bit(u8_delimiters, &d, "utf-8");

    d = RUS_SMALL;
    e8_utf_from_8bit(u8_rus_small, &d, "utf-8");

    d = RUS_LARGE;
    e8_utf_from_8bit(u8_rus_large, &d, "utf-8");

    e8_kw_list *s = key_words;
    while (s->kw != kwNotKeyWord) {
        d = s->ueng;
        e8_utf_from_8bit(s->eng, &d, "utf-8");

        d = s->urus;
        e8_utf_from_8bit(s->rus, &d, "utf-8");

        ++s;
    }

}

int lexer_is_ws(e8_uchar c)
{
    return c == 32 || c == '\t' || c == 11 || c == 12
        || c == 0xFEFF // На случай Byte-Order-Mark
        ; // почему 11 и 12 ?
}
int lexer_is_nl(e8_uchar c)
{
    return c == 13 || c == 10;
}
int lexer_is_ins(e8_uchar c, e8_uchar s[])
{
    e8_uchar *dc = s;
    while (*dc) {
        if (*dc == c)
            return 1;
        ++dc;
    }
    return 0;
}

int lexer_is_end_keyword(KeyWord kw)
{
    return
            kw == kwEndIf
        ||  kw == kwEndDo
        ||  kw == kwEndProcedure
        ||  kw == kwEndFunction
        ||  kw == kwEndTry
    ;
}

int lexer_is_dl(e8_uchar c)
{
    return lexer_is_ins(c, delimiters);
}
int lexer_is_id_s(e8_uchar c)
{

    return c == '_'
        || (c >= 'a' && c <= 'z')
        || (c >= 'A' && c <= 'Z')
        || lexer_is_ins(c, RUS_SMALL)
        || lexer_is_ins(c, RUS_LARGE)
    ;
}
int lexer_is_id(e8_uchar c)
{
    return lexer_is_id_s(c) || (c >= '0' && c <= '9') || (c == ':');
}
int lexer_is_num(e8_uchar c)
{
    return c >= '0' && c <= '9';
}
int lexer_is_hex_num(e8_uchar c)
{
    return lexer_is_num(c)
        || lexer_is_ins(c, HEX);
}

KeyWord lexer_get_keyword(const e8_uchar *sb)
{
    e8_uchar *sl = e8_utf_malloc(e8_utf_strlen(sb));
    e8_utf_strcpy(sl, sb);
    e8_utf_lower_case(sl);

    KeyWord res = kwNotKeyWord;

    e8_kw_list *s = key_words;
    while (s->kw != kwNotKeyWord) {
        if (e8_utf_strcmp(sl, s->ueng) == 0) {
            res = s->kw;
            break;
        }
        if (e8_utf_strcmp(sl, s->urus) == 0) {
            res = s->kw;
            break;
        }

        ++s;
    }
    free(sl);
    return res;
}
