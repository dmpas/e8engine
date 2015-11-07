/*! \file e8core/env/env.h */

#ifndef E8_CORE_ENVIRONMENT
#define E8_CORE_ENVIRONMENT

#include "e8core/plugin/plugin.h"
#include "e8core/commands/command.h"
#include "e8core/translator/translator.h"
#include "runtime_errors.h"

#define E8_MAX_SUB_NAME 128
#define E8_MAX_TYPE_NAME 32


/*! Самостоятельное опредѣленіе типа файла по расширенію */
#define E8_FILE_AUTO            0
/*! Файл является исходным текстом. Текст должен быть в кодировкѣ UTF-8. */
#define E8_FILE_SOURCE          1
/*! Файл является байт-кодом в формате E8 */
#define E8_FILE_BYTECODE        2
/*! Файл является собранной библіотекой или плагином (dll, so) */
#define E8_FILE_COMPILED        3

typedef void* e8_environment;

typedef e8_object e8_unit;

/*! Создаёт новое окруженіе исполненія */
e8_environment e8_env_new();

/*! Освобождает добро, выдѣленное под среду исполненія */
void e8_env_close(
                        e8_environment      env
                );

/*! Создаёт модуль по имени. Берёт файлы в соотвѣтствіи с установленным правилом */
e8_unit e8_env_unit_new(
                        e8_environment      env,
                        const char         *name
                        );

/*! Задаёт правило порядка поиска модулей по имени */
void e8_env_unit_search_priority(
                        e8_environment      env,
                        int                 priority
                        );

/*! e8s -> e8c -> dll/so */
#define E8_UNIT_PRIORITY_SOURCE      0

/*! dll/so -> e8c -> e8s */
#define E8_UNIT_PRIORITY_COMPILED    1

/*! e8c -> dll/so -> e8s */
#define E8_UNIT_PRIORITY_BYTECODE    2

/*! e8c -> e8s -> dll/so */
#define E8_UNIT_PRIORITY_DEBUG       3

/*! Создаёт модуль из файла */
e8_runtime_error *e8_env_unit_from_file(
                        e8_environment      env,
                        const char         *path,
                        int                 file_type,
                        e8_unit            *unit
            );

/*! Создаёт модуль по байт-коду */
e8_runtime_error *e8_env_unit_from_commands(
                        e8_environment      env,
                        const e8_command   *commands,
                        int                 count,
                        e8_unit            *unit
            );


/*! Выполняет подгруженный модуль */
e8_runtime_error *e8_env_execute(
                        e8_unit             unit,
                        bool                isolated /*!< Не завершать работу при вызове
                                                        в модуле функций Exit и Terminate*/
            );

/*! Добавляет функцию в среду исполнения */
e8_runtime_error *e8_env_add_function(
                        e8_environment          env,
                        const e8_uchar         *name,
                        e8_function_signature   fn
            );

/*! Добавляет функцию в среду исполнения. Версия UTF-8. */
e8_runtime_error *e8_env_add_function_utf(
                        e8_environment          env,
                        const char             *name,
                        e8_function_signature   fn
            );

/*! Добавляет общее свойство из значения */
e8_runtime_error *e8_env_add_global_value(
                        e8_environment          env,
                        const e8_uchar         *name,
                        const e8_var           *value
            );

/*! Добавляет общее свойство из значения. Версия UTF-8 */
e8_runtime_error *e8_env_add_global_value_utf(
                        e8_environment          env,
                        const char             *name,
                        const e8_var           *value
            );

/*! Добавляет конструктор типа в среду исполнения */
e8_runtime_error *e8_env_add_type(
                        e8_environment          env,
                        const e8_uchar         *name,
                        e8_function_signature   fn
            );

/*! Добавляет конструктор типа в среду исполнения. Версия UTF-8 */
e8_runtime_error *e8_env_add_type_utf(
                        e8_environment          env,
                        const char             *name,
                        e8_function_signature   fn
            );

/*! Освобождает ссылку на модуль */
e8_runtime_error *e8_env_unit_free(
                        e8_unit                 unit
            );


/*! Получает транслятор среды исполнения */
e8_translator* e8_env_get_translator(e8_environment env);


/*! Устанавливает кодировку, используемую по-умолчанию для скриптов из файлов */
e8_runtime_error *e8_env_set_default_input_encoding(
                        e8_environment          env,
                        const char             *encoding
            );

e8_runtime_error *e8_env_set_root_path(
                        e8_environment          env,
                        const char             *path
            );

void e8_env_set_debug_flag(
                        e8_environment          env,
                        bool                    flag
            );

bool e8_env_get_debug_flag(
                        e8_environment          env
            );

typedef void (*e8_env_freelib_t)(const e8_environment env);

e8_runtime_error *e8_env_add_freelib_handler(
                        e8_environment          env,
                        e8_env_freelib_t        handler
            );

/*! Устанавливает путь к исполняемому файлу */
void e8_env_set_bin_path(
                        e8_environment          env,
                        const char             *path
                    );

/*! Добавляет путь поиска расширений */
void e8_env_add_ext_path(
                        e8_environment          env,
                        const char             *path
                    );

/*! Добавляет пути из переменной PATH в список путей поиска расширений */
void e8_env_apply_PATH(
                        e8_environment          env,
                        const char             *PATH_VAR    /*!< Имя переменной или NULL для PATH */
                    );

const char *
e8_env_get_output_encoding(
                        e8_environment          env
                    );
void
e8_env_set_output_encoding(
                        e8_environment          env,
                        const char             *encdoing
                    );

/*! Возвращает конструктор для типа */
e8_function_signature
e8_get_constructor(void *env, const char *utf_type);

/*! Устанавливает признак профилирования кода */
void
e8_env_set_profile_flag(
                        e8_environment          env,
                        bool                    flag        /*!< Признак профилирования */
                    );

/*! Возвращает признак профилирования кода */
bool
e8_env_get_profile_flag(
                        e8_environment env
                    );

#endif
