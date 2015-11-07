#ifndef E8_CORE_ENV_KEYVALUE
#define E8_CORE_ENV_KEYVALUE

#include "e8core/plugin/plugin.h"

typedef struct __e8_keyvalue {
    e8_var key;
    e8_var s_key; /*!< Ключ поиска: для структур - строка в нижем регистре */
    e8_var value;

    bool fixed;
} e8_keyvalue;

extern e8_vtable vmt_keyvalue;

e8_keyvalue *e8_keyvalue_new(bool fixed);

#endif // E8_CORE_ENV_KEYVALUE
