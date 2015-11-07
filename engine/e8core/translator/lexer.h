#ifndef E8_CORE_TRANSLATOR_LEXER
#define E8_CORE_TRANSLATOR_LEXER

#include "e8core/utf/utf.h"

#define kwNotKeyWord        0
#define kwVar               1
#define kwOr		        13
#define kwNew		        16
#define kwProcedure		    18
#define kwUndefined		    25
#define kwTry		        27
#define kwTrue		        31
#define kwExecute		    33
#define kwBreak		        34
#define kwAssert		    36
#define kwMaxKeyWord        38

typedef int KeyWord;

KeyWord lexer_get_keyword(const e8_uchar *s);
int lexer_is_ws(e8_uchar c);
int lexer_is_nl(e8_uchar c);
int lexer_is_ins(e8_uchar c, e8_uchar s[]);
int lexer_is_dl(e8_uchar c);
int lexer_is_id_s(e8_uchar c);
int lexer_is_id(e8_uchar c);
int lexer_is_num(e8_uchar c);
int lexer_is_end_keyword(KeyWord kw);
int lexer_is_hex_num(e8_uchar c);

void e8_lexer_init_data();


#endif