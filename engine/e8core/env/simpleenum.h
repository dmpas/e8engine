#ifndef E8_CORE_ENV_SIMPLEENUM
#define E8_CORE_ENV_SIMPLEENUM

#include "e8core/plugin/plugin.h"

typedef struct __e8_simple_enum_element {
    char                rus[128]; /*!< имя utf-8 */
    char                eng[128]; /*!< имя utf-8*/
    int                 index;    /*!< определяющий номер */
} e8_simple_enum_element;

#define E8_ENUM_START(name) e8_simple_enum_element name[] = {
#define E8_ENUM_ADD(rus, eng, index) {rus, eng, index},
#define E8_ENUM_END {{}, {}, -1}};

/*! Внести в таблицу методов обработчики перечисления */
void e8_simple_enum_init_vtable(e8_vtable *vmt);

/*! Создать новое перечисление */
void e8_simple_enum_new(e8_var *result, e8_vtable *vmt);

/*! Внести список членов перечисления */
void e8_simple_enum_attach_list(
                        e8_var                             *_enum,
                        const e8_simple_enum_element       *t
                    );

/*! Установить новое значение члену перечисления */
void e8_simple_enum_set_value(e8_var *_enum, int element, const e8_var *value);

int e8_simple_enum_find_value(const e8_vtable *vmt, const e8_var *value);

#endif // E8_CORE_ENV_SIMPLEENUM
