/*! \file
 * Описаніе байт-кода движка E8
 */

#ifndef E8_CORE_COMMANDS
#define E8_CORE_COMMANDS

#include "e8core/variant/variant.h"

#ifdef __cplusplus
extern "C" {
#endif

/*! Команда байт-кода, 1 байт */
typedef uint8_t e8_v_command;

/*! Бѣздѣйствіе */
#define E8_COMMAND_NOP          0
/*! Завершеніе работы программы */
#define E8_COMMAND_Exit         1
/*! Помѣщеніе в стёк значенія */
#define E8_COMMAND_Const        2
/*! Помѣщеніе в стёк ссылки на свойство

 * Получает из стёка имя свойства и помѣщает обратно ссылку на свойство
 */
#define E8_COMMAND_LoadRef      4
/*! Вызов

 * Состав данных в стёке:
 * (1) количество параметров, переданных процедурѣ
 * (2-...) параметры в обратном порядке
 * (...) имя процедуры
 */
#define E8_COMMAND_Call         6
/*! Объектный вызов

 * Состав данных в стёке:
 * (1) количество параметров, переданных процедурѣ
 * (2-...) параметры в обратном порядке
 * (...) имя процедуры
 * (...) объект вызова
 */
#define E8_COMMAND_ObjectCall   7
/*! Бѣзусловный переход */
#define E8_COMMAND_Jump         8
/*! Условный истинный переход

 * Берёт из стёка значеніе и выполняет переход, если оно истинно
 */
#define E8_COMMAND_JumpTrue     9
/*! Условный ложный переход

 * Берёт из стёка значеніе и выполняет переход, если оно ложно
 */
#define E8_COMMAND_JumpFalse    10
/*! Возврат из процедуры */
#define E8_COMMAND_Return       11
/*! Задаёт таблицу имён для текущего вызова */
#define E8_COMMAND_Table        12
/*! Убирает из стёка одну запись */
#define E8_COMMAND_Pop          13
/*! Объявить параметр вызова по значенію */
#define E8_COMMAND_DeclareParamByVal 15
/*! Объявить перемѣнную в текущем скопѣ */
#define E8_COMMAND_DeclareVar   16
/*! Объявить процедуру в текущем скопѣ */
#define E8_COMMAND_DeclareSub   17
/*! Объявить завершеніе процедуры */
#define E8_COMMAND_EndSub       18
/*! Объявить параметр вызова по ссылкѣ */
#define E8_COMMAND_DeclareParamByRef 19
/*! Начать отслѣживаніе исключеній */
#define E8_COMMAND_Critical     20
/*! Завершить отслѣживаніе исключеній */
#define E8_COMMAND_EndCritical  21
/*! Вызвать исключеніе */
#define E8_COMMAND_Raise        22
/*! Получить бѣгунок объекта */
#define E8_COMMAND_Iterator     23
/*! (Старьё) Опредѣленіе, есть ли в бѣгунке слѣдующій элемент */
#define E8_COMMAND_ItHasNext    24
/*! (Старьё) Полученіе слѣдующаго элемента в бѣгункѣ */
#define E8_COMMAND_ItNext       25
/*! Провѣрить утвержденіе */
#define E8_COMMAND_Assert       26
/*! Начать кусок единой команды */
#define E8_COMMAND_StackDup     29
/*! Загрузить в стёк локальную перменную по номеру */
#define E8_COMMAND_LoadRefIndex 30


/*! Бѣздѣйствіе */
#define E8_COMMAND_Nop               31
/*! Сумма двух значеній в стёкѣ. */
#define E8_COMMAND_Add               32
/*! Разность двух значеній в стёкѣ */
#define E8_COMMAND_Sub               33
/*! Произведеніе двух значеній в стёкѣ */
#define E8_COMMAND_Mul               34
/*! Частное двух значеній в стёкѣ */
#define E8_COMMAND_Div               35
/*! Унарный минус */
#define E8_COMMAND_Neg               36
/*! Присвоить ссылкѣ значеніе */
#define E8_COMMAND_Assign            37
/*! Остаток от дѣленія двух значеній в стёкѣ */
#define E8_COMMAND_Mod               38
/*! Логическое НЕ */
#define E8_COMMAND_Not               39
/*! Логическое ИЛИ */
#define E8_COMMAND_Or                40
/*! Логическое И */
#define E8_COMMAND_And               41
/*! Равенство двух элементов из стёка */
#define E8_COMMAND_Equal             42
/*! Неравеноство двух элементов из стёка */
#define E8_COMMAND_NotEqual          43
/*! Сравненіе "Меньше" */
#define E8_COMMAND_Less              44
/*! Сравненіе "Больше" */
#define E8_COMMAND_Greater           45
/*! Сравненіе "Меньше или равно" */
#define E8_COMMAND_LessEqual         46
/*! Сравненіе "Больше или равно" */
#define E8_COMMAND_GreaterEqual      47
/*! Увеличеніе значенія в стёкѣ на 1 */
#define E8_COMMAND_Inc               48
/*! Уменьшеніе значенія в стёкѣ на 1 */
#define E8_COMMAND_Dec               49
/*! Полученіе ссылки по индексу */
#define E8_COMMAND_Index             50


/*! Наибольшій код команды +1 */
#define E8_COMMAND_OperandMax   51


typedef uint16_t e8_label_index;
#define WRONG_LABEL UINT16_MAX

/*! Описаніе указа байт-кода E8 */
typedef struct _e8_command {
    e8_v_command        cmd;      /*!< Указ */
    int                 const_index;
    e8_label_index      label;    /*!< Мѣтка */
    int                 delta;    /*!< Смѣщеніе прыжка */
} e8_command;

typedef unsigned long instruction_index_t ;
#define WRONG_ADDRESS (instruction_index_t)(-1)

typedef struct _e8_symbol {
    e8_string              *name;
    bool                   _export_symbol;
    instruction_index_t     address;
} e8_unit_symbol;

typedef struct _e8_name_element {
    unsigned                index;
    e8_string              *name;
} e8_name_element;

typedef struct __e8_name_table {
    unsigned                size;
    e8_name_element        *names;
} e8_name_table;


#ifdef __cplusplus
}
#endif

#endif
