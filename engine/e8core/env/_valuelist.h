#ifndef E8_CORE_STDLIB_VALUELIST
#define E8_CORE_STDLIB_VALUELIST

#include "e8core/plugin/plugin.h"

#include "_valuelistitem.h"

#define E8_LIST_CAPACITY_STEP 10

typedef int e8_list_index_t;

typedef struct __e8_valuelist {
    E8_COLLECTION(e8_valuelist_item*)
    long                        max_id;
} e8_valuelist;

typedef struct __e8_valuelist_iterator {
    e8_valuelist               *list;
    e8_list_index_t             current;
} e8_valuelist_iterator;


E8_DECLARE_SUB(e8_valuelist_new);

#endif // E8_CORE_STDLIB_VALUELIST
