#ifndef E8_CORE_ENV_STDLIB_ARRAY
#define E8_CORE_ENV_STDLIB_ARRAY

#include "e8core/plugin/plugin.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef int e8_array_index_t;

typedef struct __e8_array {
    E8_COLLECTION(e8_var)
    bool                    fixed;
} e8_array;

typedef struct __e8_array_iterator {
    e8_array               *array;
    e8_array_index_t        current;
} e8_array_iterator;


E8_DECLARE_SUB(e8_array_new);
E8_DECLARE_SUB(e8_fixed_array_new);

e8_array *__e8_new_array(bool fixed, int size);


/*! Раздел для плагинов */

void e8_array_add_nvalues(e8_var *array, int n, ...);

void e8_array_insert_nvalues(e8_var *array, int index, int n, ...);

int e8_array_index_of(const e8_var *array, const e8_var *value);

int e8_array_size(const e8_var *array);

void e8_array_clear(e8_var *array);

e8_var *e8_array_get(const e8_var *array, int index, e8_var *result);
void e8_array_set(e8_var *array, int index, const e8_var *value);


#ifdef __cplusplus
} // extern "C"
#endif

#endif // E8_CORE_ENV_STDLIB_ARRAY
