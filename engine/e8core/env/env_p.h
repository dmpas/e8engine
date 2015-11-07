/*!E8::Private*/
#ifndef E8_CORE_ENV_P_H_INCLUDED
#define E8_CORE_ENV_P_H_INCLUDED

#include "e8core/plugin/plugin.h"
#include "e8core/translator/translator.h"
#include "stack.h"
#include "scope_p.h"
#include "unit.h"
#include "env.h"

#ifndef MAX_PATH
/* *nix-patch */
#define MAX_PATH 1000
#endif

#ifdef __WINNT
#define SHARED_EXT "dll"
#endif
#ifdef __linux
#define SHARED_EXT "so"
#endif


typedef struct __e8_fn_struct {
    e8_uchar                lname[E8_MAX_SUB_NAME]; /*!< Имя функции */
    e8_function_signature   fn;                     /*!< Функция */
} e8_fn_struct;

typedef struct __e8_typedata_struct {
    e8_uchar                lname[E8_MAX_TYPE_NAME]; /*!< Имя типа */
    e8_function_signature   fn;                      /*!< Функция-Конструктор */
} e8_typedata;

typedef struct __e8_env_data {

    e8_translator       tr;

    e8_stack            stack;
    int                 search_priority;

    struct {
        E8_COLLECTION(e8_fn_struct)
    }                   fn;

    struct {
        E8_COLLECTION(e8_named_property)
    }                   properties;

    struct {
        E8_COLLECTION(e8_typedata)
    }                   types;

    char               *root_path;
    bool                debug_flag;

    struct {
        E8_COLLECTION(e8_env_freelib_t)
    }                   free_handlers;

    char               *bin_path;

    e8_scope            global_scope;

    long                time_b_sec;
    long                time_b_ms;

    struct {
        E8_COLLECTION(char*)
    }                   ext_paths;

    bool                exit_flag;
    bool                terminate_flag;
    bool                profile_code;

    char               *output_encoding;

} e8_env_data;


extern e8_vtable vmt_env;

void __e8_init_vmt_env();

/*! Ищет модуль в папках поиска:
 *  (1) Unit::RootPath
 *  (2) Env::RootPath
 *  (3) Env::ExtPaths
 */
char *__e8_env_find_unit(const char *name, e8_unit_data *d, e8_env_data *env);


/*! Определяет тип файла по расширению */
int
__e8_detect_filetype(const char *path);

#endif // E8_CORE_ENV_P_H_INCLUDED
