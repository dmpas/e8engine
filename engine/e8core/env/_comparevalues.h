#ifndef E8_CORE_COMPAREVALUES
#define E8_CORE_COMPAREVALUES

#include "e8core/plugin/plugin.h"


typedef struct __compare_values {
    bool            built_in;
    bool            invert;
} e8_compare_values;

extern e8_vtable vmt_comparevalues;

E8_DECLARE_SUB(e8_comparevalues_new);

typedef int (*e8_compare_fn_t)(
                        const e8_compare_values        *cv,
                        const e8_var                   *value1,
                        const e8_var                   *value2
                    );

int e8_compare_values_compare(
                        const e8_compare_values        *cv,
                        const e8_var                   *value1,
                        const e8_var                   *value2
                    );

#endif // E8_CORE_COMPAREVALUES
