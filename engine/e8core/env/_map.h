#ifndef E8_CORE_ENV_STDLIB_MAP
#define E8_CORE_ENV_STDLIB_MAP

#include "e8core/plugin/plugin.h"

#include "_keyvalue.h"

#define E8_MAP_CAPACITY_STEP 10

typedef int e8_map_index_t;

typedef struct __e8_map {
    E8_COLLECTION(e8_keyvalue*)
    bool                        structure;
    bool                        fixed;
} e8_map;

typedef struct __e8_map_iterator {
    e8_map               *map;
    e8_map_index_t        current;
} e8_map_iterator;


E8_DECLARE_SUB(e8_map_new);
E8_DECLARE_SUB(e8_fixed_map_new);
E8_DECLARE_SUB(e8_structure_new);
E8_DECLARE_SUB(e8_fixed_structure_new);

#endif // E8_CORE_ENV_STDLIB_MAP
