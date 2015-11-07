#ifndef E8_CORE_UTF_H
#define E8_CORE_UTF_H

#include <string.h>

#include <stdint.h>

#define E8_UTFLIB_CP866  1
#define E8_UTFLIB_CP1251 2
#define E8_UTFLIB_KOI8R  3

#define cyr_aa 0x0430
#define cyr_bb 0x0431
#define cyr_vv 0x0432
#define cyr_gg 0x0433
#define cyr_dd 0x0434
#define cyr_ye 0x0435
#define cyr_eo 0x0451
#define cyr_zh 0x0436
#define cyr_zz 0x0437
#define cyr_ii 0x0438
#define cyr_jj 0x0439
#define cyr_kk 0x043A
#define cyr_ll 0x043B
#define cyr_mm 0x043C
#define cyr_nn 0x043D
#define cyr_oo 0x043E
#define cyr_pp 0x043F
#define cyr_rr 0x0440
#define cyr_ss 0x0441
#define cyr_tt 0x0442
#define cyr_uu 0x0443
#define cyr_ff 0x0444
#define cyr_hh 0x0445
#define cyr_cc 0x0446
#define cyr_ch 0x0447
#define cyr_sh 0x0448
#define cyr_sc 0x0449
#define cyr_er 0x044A
#define cyr_yi 0x044B
#define cyr_ir 0x044C
#define cyr_ee 0x044D
#define cyr_yu 0x044E
#define cyr_ya 0x044F


#define cyr_AA 0x0410
#define cyr_BB 0x0411
#define cyr_VV 0x0412
#define cyr_GG 0x0413
#define cyr_DD 0x0414
#define cyr_YE 0x0415
#define cyr_EO 0x0401
#define cyr_ZH 0x0416
#define cyr_ZZ 0x0417
#define cyr_II 0x0418
#define cyr_JJ 0x0419
#define cyr_KK 0x041A
#define cyr_LL 0x041B
#define cyr_MM 0x041C
#define cyr_NN 0x041D
#define cyr_OO 0x041E
#define cyr_PP 0x041F
#define cyr_RR 0x0420
#define cyr_SS 0x0421
#define cyr_TT 0x0422
#define cyr_UU 0x0423
#define cyr_FF 0x0424
#define cyr_HH 0x0425
#define cyr_CC 0x0426
#define cyr_CH 0x0427
#define cyr_SH 0x0428
#define cyr_SC 0x0429
#define cyr_ER 0x042A
#define cyr_YI 0x042B
#define cyr_IR 0x042C
#define cyr_EE 0x042D
#define cyr_YU 0x042E
#define cyr_YA 0x042F

#define cyr_EAT 0x0462
#define cyr_eat 0x0463
#define cyr_I 0x0406
#define cyr_i 0x0456
#define cyr_THETA 0x0472
#define cyr_theta 0x0473



#ifdef __cplusplus
extern "C" {
#endif

/*! e8_uchar - код символа уникода (UTF-32) */
typedef uint32_t e8_uchar;

/*! Определяет, поддерживается ли кодировка */
int
e8_utf_encoding_support(
                        const char         *encoding /*!< кодировка */
                        );

/*! Возвращает длину строки */
size_t
e8_utf_strlen(
                        const e8_uchar     *utf /*!< Строка с 0 символом на конце */
              );

/*! Создаёт строку UTF-32 из строки ASCII-Latin */
void
e8_utf_from_7bit(
                    const char             *ascii, /*!< Строка latin */
                    e8_uchar              **utf    /*!< Указатель на буффер для строки UTF-32 */
                );

/*! Создаёт строку UTF-32 из строки 8-битной строки в указанной */
void
e8_utf_from_8bit(
                    const char             *ascii,  /*!< 8-битная строка */
                    e8_uchar              **utf,    /*!< указатель на буффер для строки UTF-32 */
                    const char *encoding            /*!< Кодировка */
                 );


/*! Преобразует строку UTF-32 в 8-битную строку */
void
e8_utf_to_8bit(
                    const e8_uchar         *utf,    /*!< Строка UTF-32 */
                    char                  **ascii,  /*!< указательна буффер для 8-битной строки */
                    const char *encoding            /*!< кодировка */
                );

/*! Дописывает строку src в конец строки dst */
void
e8_utf_strcat(
                    e8_uchar               *dst,    /*!< Итоговая строка */
                    const e8_uchar         *src     /*!< Присоединяемая строка */
            );

/*! Сравнивает строки:
    0 - A == B
   -1 - A < B
    1 - A > B
 */
int
e8_utf_strcmp(
                    const e8_uchar         *a,      /*!< Строка A */
                    const e8_uchar         *b       /*!< Строка B */
              );

/*! Сравнивает строки, но не более N символов */
int
e8_utf_strncmp(
                    const e8_uchar         *a,      /*!< Строка A */
                    const e8_uchar         *b,      /*!< Строка B */
                    int                     n       /*!< Количество символов */
                );

/*! Сравнивает строки без учёта регистра */
int
e8_utf_stricmp(
                    const e8_uchar         *a,      /*!< Строка A */
                    const e8_uchar         *b       /*!< Строка Б */
               );

/*! Копирует содержимое строки src в строку dst */
void
e8_utf_strcpy(
                    e8_uchar               *dst,    /*!< Приёмник */
                    const e8_uchar         *src     /*!< Источник */
                    );

/*! Копирует содержимое строки src в строку dst не более N символов */
void
e8_utf_strncpy(
                    e8_uchar               *dst,    /*!< Приёмник */
                    const e8_uchar         *src,    /*!< Источник*/
                    int                     n       /*!< Количество символов */
               );

/*! Приводит строку в верхнему регистру */
void
e8_utf_upper_case(
                    e8_uchar               *us      /*!< Строка */
                  );

/*! Приводит строку к нижнему регистру */
void
e8_utf_lower_case(
                    e8_uchar               *us      /*!< Строка */
                  );

/*! Выделяет память, достаточную для строки UTF-32 указанной длины */
e8_uchar *
e8_utf_malloc(
                    size_t                  len     /*!< Длина строки */
            );

/*! Изменяет размер выделенной памяти под строку UTF-32 указанной новой длины */
e8_uchar *
e8_utf_realloc(
                    e8_uchar               *c,      /*!< Существующая строка */
                    size_t                  len     /*!< Новая длина */
               );

/*! Освобождает память, выделененую под строку UTF-32 */
void
e8_utf_free(
                    e8_uchar               *us      /*!< Строка */
            );

/*! Создаёт строку UTF-32 из строки UTF-16 или UTF-32 */
e8_uchar *
e8_utf_from_mbstring(
                    const void             *mbs,        /*!< Исходная строка */
                    int                     length,     /*!< Длина строки */
                    int                     size        /*!< Размер символа */
                    );

/*! Определяет длину строки в кодировке UTF-8 */
size_t
e8_utf_get_utf8_length(
                    const e8_uchar         *s           /*!< Строка */
                );

/*! Создаёт копию строки UTF-32, выделяет память */
e8_uchar *
e8_utf_strdup(
                    const e8_uchar         *src         /*!< Исходная строка */
              );

/*! Создаёт строку UTF-32 из строки в указанной кодировке, выделяет память */
e8_uchar *
e8_utf_strcdup(
                    const char             *src,        /*!< Исходная строка */
                    const char             *encoding    /*!< Кодировка */
                );

/*! Приводит символ к верхему регистру */
e8_uchar
e8_upcase(
                    e8_uchar                uc          /*!< Символ */
          );

/*! Приводит символ к нижнему регистру */
e8_uchar
e8_locase(
                    e8_uchar                uc          /*!< Символ */
            );

#ifdef __cplusplus
} // extern "C"
#endif


#endif // E8_CORE_UTF_H
