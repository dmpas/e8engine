#ifndef E8_CORE_STDLIB_VALUETABLE
#define E8_CORE_STDLIB_VALUETABLE

#include "e8core/plugin/plugin.h"

struct __e8_valuetable;

typedef struct __e8_valuetable_column {
    e8_var                          name;
    e8_var                        s_name;
    e8_var                          title;
    e8_var                          value_type;
    e8_var                          width;
} e8_valuetable_column;

typedef struct __e8_valuetable_columns {
    E8_COLLECTION(e8_valuetable_column*)
    struct __e8_valuetable         *owner;
} e8_valuetable_columns;

typedef struct __e8_valuetable_index {
} e8_valuetable_index;

typedef struct __e8_valuetable_indexes {
    E8_COLLECTION(e8_valuetable_index*)
} e8_valuetable_indexes;

typedef struct __e8_valuetable_row_value {
    const e8_valuetable_column     *column;
    e8_var                          value;
} e8_valuetable_row_value;

typedef struct __e8_valuetable_row {
    E8_COLLECTION(e8_valuetable_row_value*)
    struct __e8_valuetable         *owner;
} e8_valuetable_row;

typedef struct __e8_valuetable_rows {
    E8_COLLECTION(e8_valuetable_row*)
} e8_valuetable_rows;

typedef struct __e8_valuetable {

    e8_valuetable_rows              rows;

    e8_valuetable_columns           columns;
    e8_valuetable_indexes           indexes;

} e8_valuetable;


E8_DECLARE_SUB(e8_valuetable_new);

#endif // E8_CORE_STDLIB_VALUETABLE
