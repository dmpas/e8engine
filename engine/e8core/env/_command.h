#ifndef E8_ACTION_COMMAND
#define E8_ACTION_COMMAND

#include "e8core/plugin/plugin.h"

#include <pthread.h>


typedef struct {
    e8_object               obj;
    e8_vtable              *vmt;
    e8_uchar               *method;
} e8_action_command;

extern e8_vtable   action_command_vmt;



struct __thread_data {
    e8_object               obj;
    void                   *user_data;
    e8_function_signature   fn;
    int                     argc;
    e8_property            *argv;
    e8_runtime_error       *err;
    e8_var                  result;
    e8_var                 *vars;
    bool                    auto_free;
};


typedef struct {
    pthread_t                       thread;
    bool                            done;
    volatile struct __thread_data  *data;
} e8_action_command_thread;

extern e8_vtable   action_command_result_vmt;


E8_DECLARE_SUB(e8_action_command_call);
E8_DECLARE_SUB(e8_action_command_new);

#endif // E8_ACTION_COMMAND
