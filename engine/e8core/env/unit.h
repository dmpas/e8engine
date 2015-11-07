/*!E8::Private*/
/* Служебный файл */
#ifndef E8_CORE_ENV_UNIT
#define E8_CORE_ENV_UNIT

#include "e8core/commands/command.h"
#include "scope.h"
#include "scope_p.h"
#include "stack.h"
#include "env.h"

#include <stdio.h>

typedef struct __e8_sub_info {
    e8_uchar                   *name;
    bool                       _export_item;
    instruction_index_t         index;
    volatile void              *unit;
} e8_sub;

typedef volatile struct __e8_unit_data {

    struct {
        E8_COLLECTION(e8_command)
    }                                  commands;

    struct {
        E8_COLLECTION(e8_var)
    }                                  consts;

    struct {
        E8_COLLECTION(e8_sub)
    }                                  subs;

    struct {
        E8_COLLECTION(e8_name_table)
    }                                  tables;

    e8_scope_data            *volatile scope;
    e8_vtable                *volatile vmt_scope;

    char                              *root_path;

    volatile long                     *ticks;
    volatile long                     *counts;
    char                              *profile_output_path;
    FILE                              *profile_out;

    volatile bool                      exit_flag;
    volatile bool                      terminate_flag;

    /* final */e8_environment         *env;
} e8_unit_data;

typedef struct __e8_unit_runtime_info_t {
    instruction_index_t     current;
    e8_scope_data          *scope;
    e8_vtable              *vmt_scope;
    e8_stack               *stack;
    bool                    exit_flag;
    bool                    terminate_flag;
    bool                    finish_flag;

    volatile e8_unit_data  *unit;
    const e8_name_table    *table;

    #define MAX_LOCAL_VARS  128
    e8_property             local_data[MAX_LOCAL_VARS];
    bool                    declared_local[MAX_LOCAL_VARS];
    bool                    created_local[MAX_LOCAL_VARS];

    int                     param_index;

} e8_unit_runtime_info_t;

e8_runtime_error *e8_unit_execute(e8_unit_data *unit);

extern e8_vtable vmt_unit;
void __e8_init_vmt_unit();

e8_runtime_error *
__e8_unit_iterate(e8_unit_runtime_info_t *rti);

E8_DECLARE_SUB(__e8_unit_call_inner);

#endif
