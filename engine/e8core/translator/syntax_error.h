#ifndef SYNTAX_ERROR_H_INCLUDED
#define SYNTAX_ERROR_H_INCLUDED

/*! Наибольшая длина идентификатора */
#define E8_IDENTIFIER_MAX_LENGTH 64
/*! Наибольшая вложенность указаній предобработчика */
#define E8_PREPROC_DEPTH_MAX 16
/*! Наибольшая длина константной строки */
#define E8_MAX_STRING_CONST_LENGTH 10240

/*! Исключеніе не возникало */
#define E8_NO_THROW 0
/*! Identifier expected */
#define E8_IDENTIFIER_EXPECTED 1
/*! '=' expected */
#define E8_ASSIGN_CHAR_EXPECTED 2
/*! '(' expected */
#define E8_OPEN_BRACKET_EXPECTED 3
/*! ')' expected */
#define E8_CLOSE_BRACKET_EXPECTED 4
/*! Wrong loop depth */
#define E8_WRONG_LOOP_DEPTH 5
/*! | expected */
#define E8_LINE_FEED_EXPECTED 6
/*! Unexpected end-of-file */
#define E8_UNEXPECTED_END_OF_FILE 7
/*! Constant expression expected */
#define E8_CONST_EXPRESSION_EXPECTED 8
/*! Colon expected */
#define E8_COLON_EXPECTED 9
/*! Keyword expected */
#define E8_KEYWORD_EXPECTED 10

typedef unsigned long e8_script_position_t;
typedef int e8_error_no;

typedef struct __e8_syntax_error {
    e8_error_no             error;
    e8_script_position_t    pos;
    e8_script_position_t    line_no;
} e8_syntax_error;

#endif // SYNTAX_ERROR_H_INCLUDED
