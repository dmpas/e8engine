/*! \file
 * Описаніе взаимодѣйствія с транслятором
 */
#ifndef E8_CORE_TRANSLATOR
#define E8_CORE_TRANSLATOR

#include "e8core/commands/command.h"
#include "syntax_error.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void* e8_translator;

typedef struct __e8_parse_output {

    unsigned                    commands_count;
    unsigned                    consts_count;
    unsigned                    symbols_count;
    unsigned                    tables_count;

    e8_command                 *commands;
    e8_var                     *consts;
    e8_unit_symbol             *symbols;
    e8_name_table              *tables;

} e8_parse_output;

/*! Выдѣляет гумно под транслятор */
e8_translator e8_translator_new();

/*! выполняет преобразованіе текста в набор команд.
 *  возвращает нулевой указатель в случаѣ успѣха или
 *  ссылку на описаніе ошибки.
 */
e8_syntax_error *e8_translator_parse (
                            e8_translator       tr,
                            e8_parse_output    *out,
                            const e8_uchar     *text,
                            void               *data
                        );

/*! Освобождает гумно, выдѣленное под описаніе ошибки */
void e8_error_free(e8_syntax_error *error);

void e8_translator_set_server(e8_translator tr, bool flag);
void e8_translator_set_script(e8_translator tr, bool flag);
void e8_translator_set_thick_client(e8_translator tr, bool flag);
void e8_translator_set_web_client(e8_translator tr, bool flag);
void e8_translator_set_thin_client_ordinary_application(e8_translator tr, bool flag);
void e8_translator_set_thin_client_managed_application(e8_translator tr, bool flag);
void e8_translator_set_strict_syntax(e8_translator tr, bool flag);

bool e8_translator_get_server(e8_translator tr);
bool e8_translator_get_script(e8_translator tr);
bool e8_translator_get_thick_client(e8_translator tr);
bool e8_translator_get_web_client(e8_translator tr);
bool e8_translator_get_thin_client_ordinary_application(e8_translator tr);
bool e8_translator_get_thin_client_managed_application(e8_translator tr);
bool e8_translator_get_strict_syntax(e8_translator tr);


/*! Включает/выключает признак необходимости добавленія
 *  отладочных свѣденій в вывод.
 */
void e8_translate_produce_debug_info(
                        e8_translator           tr,
                        bool                    enable
            );

/*! Освобождает выдѣленное для транслятора гумно */
void e8_translator_close(e8_translator tr);

#ifdef __cplusplus
}
#endif

#endif
