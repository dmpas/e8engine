/*! \file */
#ifndef E8_CORE_ENV_SCOPE
#define E8_CORE_ENV_SCOPE

#include "e8core/variant/variant.h"
#include "runtime_errors.h"

/*! Скоп данных */
typedef e8_object e8_scope;

/*! Создаёт новый скоп данных и выдѣляет под него мѣсто */
e8_runtime_error *e8_scope_new(e8_scope *scope, const e8_scope parent);

/*! Добавляет свойство в скоп данных */
e8_runtime_error *e8_scope_add(
                    e8_scope            scope,
                    const e8_uchar     *name,
                    const e8_property  *property,
                    bool                local      /*!< Показывать, должно ли значение уничтожаться вместе со свойством */
                );

/*! Уничтожает скоп данных */
e8_runtime_error *e8_scope_destroy(e8_scope scope);

/*! Задаёт значение переменной ЭтотОбъект/ThisObject */
e8_runtime_error *
e8_scope_set_this_object(
                    e8_scope            scope,
                    e8_object           This
                        );

#endif
