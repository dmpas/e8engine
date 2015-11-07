/*!E8::Private*/
/* Служебный файл */

#ifndef E8_CORE_ENV_STACK
#define E8_CORE_ENV_STACK

#include "e8core/variant/variant.h"
#include "runtime_errors.h"

#define E8_STACK_VALUE      0
#define E8_STACK_REFERENCE  1

typedef struct __e8_stack_element {
    short       type;
    union {
        e8_property     p;
        e8_var          v;
    };
} e8_stack_element;

typedef struct __e8_stack {
    e8_stack_element       *data;
    int                     size;
} e8_stack;

e8_runtime_error *e8_stack_new(
                    e8_stack  **stack,
                    int         reserve_size
                    );

e8_runtime_error *e8_stack_push_value(
                    e8_stack           *stack,
                    const e8_var       *value
                    );

e8_runtime_error *e8_stack_push_reference(
                    e8_stack           *stack,
                    const e8_property  *property
                    );

e8_runtime_error *e8_stack_pop(
                    e8_stack           *stack,
                    e8_stack_element   *element
                    );

e8_runtime_error *e8_stack_pop_value(
                    e8_stack           *stack,
                    e8_var             *result
                    );

e8_runtime_error *e8_stack_pop_ref(
                    e8_stack           *stack,
                    e8_property        *property
                    );

e8_runtime_error *e8_stack_destroy(
                    e8_stack           *stack
                    );

#endif
