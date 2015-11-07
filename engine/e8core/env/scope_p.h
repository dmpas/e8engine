/*!E8::Private*/
/* Служебный файл */
#ifndef E8_CORE_ENV_SCOPE_P_H
#define E8_CORE_ENV_SCOPE_P_H

#include "e8core/variant/variant.h"

typedef struct __e8_named_property {
    e8_uchar               *name;
    //e8_uchar               *lname;
    e8_property             property;
    bool                    local;
} e8_named_property;

typedef struct __e8_scope_data {

    struct {
        E8_COLLECTION(e8_property)
    }                   params;

    struct {
        E8_COLLECTION(e8_named_property)
    }                   properties;

    struct
    __e8_scope_data    *parent;

    e8_object           this_object;
    e8_vtable          *this_vmt;

} e8_scope_data;

extern e8_vtable vmt_scope;

#endif // E8_CORE_ENV_SCOPE_P_H
