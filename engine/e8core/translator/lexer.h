#ifndef E8_CORE_TRANSLATOR_LEXER
#define E8_CORE_TRANSLATOR_LEXER

#include "e8core/utf/utf.h"

#define kwNotKeyWord        0
#define kwVar               1#define kwIf		        2#define kwWhile		        3#define kwDo		        4#define kwThen		        5#define kwFor		        6#define kwEach		        7#define kwTo		        8#define kwEndIf		        9#define kwEndDo		        10#define kwElse		        11#define kwElseIf		    12
#define kwOr		        13#define kwAnd		        14#define kwNot		        15
#define kwNew		        16#define kwIn		        17
#define kwProcedure		    18#define kwFunction		    19#define kwVal		        20#define kwExport		    21#define kwEndProcedure		22#define kwEndFunction		23#define kwReturn		    24
#define kwUndefined		    25#define kwNull		        26
#define kwTry		        27#define kwException		    28#define kwEndTry		    29#define kwRaise		        30
#define kwTrue		        31#define kwFalse		        32
#define kwExecute		    33
#define kwBreak		        34#define kwContinue		    35
#define kwAssert		    36#define kwAs		        37
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
