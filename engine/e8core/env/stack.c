#include "stack.h"
#include <malloc.h>
#include "macro.h"
#include "e8core/plugin/plugin.h"

e8_runtime_error *__e8_throw(int error);

e8_runtime_error *e8_stack_new(
                    e8_stack  **stack,
                    int         reserve_size
            )
{
    *stack = AALLOC(e8_stack, 1);
    (*stack)->size = 0;
    (*stack)->data = AALLOC(e8_stack_element, reserve_size);
    E8_RETURN_OK;
}

e8_runtime_error *e8_stack_push_value(
                    e8_stack           *stack,
                    const e8_var       *value
                    )
{
    stack->data[stack->size].type = E8_STACK_VALUE;
    e8_var_undefined(&stack->data[stack->size].v);
    e8_var_assign(&stack->data[stack->size].v, value);
    ++stack->size;
    E8_RETURN_OK;
}

e8_runtime_error *e8_stack_push_reference(
                    e8_stack           *stack,
                    const e8_property  *property
                    )
{
    stack->data[stack->size].type = E8_STACK_REFERENCE;
    stack->data[stack->size].p = *property;
    ++stack->size;
    E8_RETURN_OK;
}

e8_runtime_error *e8_stack_pop(
                    e8_stack *stack,
                    e8_stack_element *element
                    )
{
    if (stack->size == 0)
        E8_THROW(E8_RT_ERROR_STACK_EMPTY);

    --stack->size;
    if (element)
        *element = stack->data[stack->size];
    else {
        if (stack->data[stack->size].type == E8_STACK_VALUE)
            e8_var_destroy(&stack->data[stack->size].v);
    }

    E8_RETURN_OK;
}

e8_runtime_error *e8_stack_pop_value(e8_stack *stack, e8_var *result)
{
    e8_runtime_error *__err;
    e8_stack_element el;

    e8_var_undefined(result);

    if (( __err = e8_stack_pop(stack, &el) ))
        return __err;

    if (el.type == E8_STACK_VALUE) {
        e8_var_assign(result, &el.v);
        e8_var_destroy(&el.v);
        E8_RETURN_OK;
    }
    if (!el.p.can_get)
        E8_THROW(E8_RT_VALUE_CANNOT_BE_READ);

    if ( el.p.get(&el.p, result) )
        E8_THROW(E8_RT_VALUE_CANNOT_BE_READ);

    E8_RETURN_OK;
}

e8_runtime_error *
e8_stack_pop_ref(e8_stack *stack, e8_property *property)
{
    e8_runtime_error *__err;
    e8_stack_element el;
    el.type = 0; /* W: not initialized*/

    if (( __err = e8_stack_pop(stack, &el) ))
        return __err;

    if (el.type == E8_STACK_VALUE) {
        E8_THROW(E8_RT_STACK_NOT_A_REFERENCE);
    }

    *property = el.p;
    E8_RETURN_OK;
}


e8_runtime_error *e8_stack_destroy(e8_stack *stack)
{

    while (stack->size--) {
        if (stack->data[stack->size].type == E8_STACK_VALUE)
            e8_var_destroy(&stack->data[stack->size].v);
    }

    free(stack->data);
    free(stack);
    return 0;
}
