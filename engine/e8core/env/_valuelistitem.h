#ifndef E8_CORE_STDLIB_VALUELISTITEM
#define E8_CORE_STDLIB_VALUELISTITEM

#include "e8core/plugin/plugin.h"

typedef struct __e8_valuelist_item {
    e8_var  value;

    e8_var  mark;
    e8_var  presentation;

    e8_var  picture;

    long    id;
} e8_valuelist_item;

extern e8_vtable vmt_valuelist_item;

/*! */
e8_valuelist_item *e8_valuelist_item_new();

/*! */
e8_valuelist_item *e8_valuelist_item_new_with_value(
                                const e8_var *value,            /*!< */
                                const e8_var *presentation,     /*!< */
                                const e8_var *mark,             /*!< */
                                const e8_var *picture           /*!< */
                    );


#endif // E8_CORE_STDLIB_VALUELISTITEM
