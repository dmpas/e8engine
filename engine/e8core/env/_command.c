#include "_command.h"
#include "unit.h"
#include <malloc.h>
#include "utils.h"

#ifndef E8_NO_THREADS
#include <pthread.h>
#include <signal.h>
#endif // E8_NO_THREADS

#define _COMMAND(name) e8_action_command *name = (e8_action_command *)obj
#define _ARESULT(name) e8_action_command_thread *name = (e8_action_command_thread *)obj

e8_vtable action_command_vmt;

static E8_DECLARE_DESTRUCTOR(e8_action_command_destroy)
{
    _COMMAND(c);
    e8_utf_free(c->method);
    e8_gc_free_object(c->obj);
    free(c);
}

E8_DECLARE_SUB(e8_action_command_call)
{
    _COMMAND(c);

    e8_function_signature fn;

    void *m_user_data;
    e8_runtime_error *__err = c->vmt->get_method(c->obj, c->method, &fn, &m_user_data);
    if (__err)
        return __err;

    return fn(c->obj, m_user_data, argc, argv, result);
}

#ifndef E8_NO_THREADS


e8_vtable action_command_result_vmt;

static void
__destroy_thread_data(volatile struct __thread_data *data)
{
    if (!data)
        return;

    e8_var_destroy((e8_var*)&data->result);
    if (data->argv) {
        e8_property *p = data->argv;
        int i = data->argc;
        while (i--) {
            e8_var_destroy((e8_var*)(p++)->data);
        }
        free(data->argv);
        free(data->vars);
        data->vars = NULL;
        data->argv = NULL;
    }

    free((void*)data);
}

E8_DECLARE_SUB(e8_action_command_result_await)
{
    _ARESULT(th);

    if (th->done)
        E8_THROW_DETAIL(E8_RT_EXCEPTION_RAISED, e8_utf_strcdup("Already done!", "utf-8"));

    void *mdata;
    int r = pthread_join(th->thread, &mdata);
    if (r) {
        char buf[100];
        sprintf(buf, "Thread join error: %d", r);
        E8_THROW_DETAIL(E8_RT_EXCEPTION_RAISED, __e8_strdup(buf));
    }

    th->done = true;
    th->data = NULL;

    struct __thread_data *data = (struct __thread_data *)mdata;

    if (!data->err) {
        if (result)
            e8_var_assign(result, &data->result);
    }

    e8_runtime_error *__err = data->err;
    __destroy_thread_data(data);

    return __err;
}

static E8_DECLARE_DESTRUCTOR(e8_action_command_result_destroy)
{
    _ARESULT(ar);

    if (ar->done) {
        __destroy_thread_data(ar->data);
    } else
        ar->data->auto_free = true;

    free(ar);
}


E8_METHOD_LIST_START(e8_action_command_result_cs)
    E8_METHOD_LIST_ADD("Ожидать", "await", e8_action_command_result_await)
E8_METHOD_LIST_END

static E8_DECLARE_GET_METHOD(e8_action_command_result_get_method)
{
    e8_simple_get_method(name, e8_action_command_result_cs, fn);
    E8_RETURN_OK;
}

static void* thread_fn(void *d)
{
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);

    struct __thread_data *data = (struct __thread_data *)d;
    data->err = data->fn(data->obj, data->user_data, data->argc, data->argv, &data->result);

    if (data->auto_free) {
        __destroy_thread_data(data);
        pthread_exit(NULL);
        return NULL;
    }

    return d;
}

E8_DECLARE_SUB(e8_action_command_async)
{
    _COMMAND(c);

    e8_function_signature fn;
    void *m_user_data;

    e8_runtime_error *__err = c->vmt->get_method(c->obj, c->method, &fn, &m_user_data);
    if (__err)
        return __err;

    e8_action_command_thread *th = malloc(sizeof(*th));
    struct __thread_data *data = malloc(sizeof(*data));

    data->obj = c->obj;
    data->user_data = m_user_data;
    data->fn = fn;
    data->err = NULL;
    data->auto_free = false;
    e8_var_undefined(&data->result);

    e8_var *v = 0;

    if (argc) {

        e8_property *props = calloc(argc, sizeof(*props));
        v = calloc(argc, sizeof(*v));
        data->argc = argc;

        int i;
        for (i = 0; i < argc; ++i) {
            e8_var_undefined(&v[i]);
            argv[i].get(&argv[i], &v[i]);
            /* props[i] = argv[i]; */
            e8_create_variant_property(&props[i], &v[i]);
        }

        data->argv = props;

    } else {
        data->argc = 0;
        data->argv = 0;
    }

    data->vars = v;

    th->done = false;
    th->data = data;
    int r = pthread_create(&th->thread, NULL, thread_fn, data);
    if (r) {
        /* TODO: Leak */
        th->data = NULL;
        th->done = true;
        char buf[100];
        sprintf(buf, "Thread create error: %d!", r);
        E8_THROW_DETAIL(E8_RT_EXCEPTION_RAISED, __e8_strdup(buf));
    }

    e8_gc_register_object(th, &action_command_result_vmt);
    e8_var_object(result, th);

    E8_RETURN_OK;
}

static const e8_uchar NAME_RESULT[] = {'A', 's', 'y', 'n', 'c', 'R', 'e', 's', 'u', 'l', 't', 0};
static E8_DECLARE_TO_STRING(e8_action_command_result_to_string)
{
    return NAME_RESULT;
}

#endif


E8_METHOD_LIST_START(e8_action_command_cs)
    E8_METHOD_LIST_ADD("Вызвать", "call", e8_action_command_call)

    #ifndef E8_NO_THREADS
    E8_METHOD_LIST_ADD("Фоном", "async", e8_action_command_async)
    #endif

E8_METHOD_LIST_END

static E8_DECLARE_GET_METHOD(e8_action_command_get_method)
{
    e8_simple_get_method(name, e8_action_command_cs, fn);
    E8_RETURN_OK;
}

static const e8_uchar NAME_COMMAND[] = {'A', 'c', 't', 'i', 'o', 'n', 0};
static E8_DECLARE_TO_STRING(e8_action_command_to_string)
{
    return NAME_COMMAND;
}


static bool __init = false;

void init_command_vmt()
{
    __init = true;

    e8_prepare_call_elements(e8_action_command_cs, "utf-8");
    e8_vtable_template(&action_command_vmt);
    action_command_vmt.get_method = e8_action_command_get_method;
    action_command_vmt.dtor = e8_action_command_destroy;
    action_command_vmt.to_string = e8_action_command_to_string;

    #ifndef E8_NO_THREADS
    e8_prepare_call_elements(e8_action_command_result_cs, "utf-8");
    e8_vtable_template(&action_command_result_vmt);
    action_command_result_vmt.get_method = e8_action_command_result_get_method;
    action_command_result_vmt.dtor = e8_action_command_result_destroy;
    action_command_result_vmt.to_string = e8_action_command_result_to_string;
    #endif
}

E8_DECLARE_SUB(e8_action_command_new)
{
    if (!__init)
        init_command_vmt();

    E8_ARG_STRING(s_method);

    e8_action_command *c = malloc(sizeof(*c));

    c->obj = obj;
    c->vmt = e8_get_vtable(obj);
    c->method = e8_utf_strdup(s_method->s);

    e8_gc_use_object(obj);

    e8_gc_register_object(c, &action_command_vmt);

    e8_string_destroy(s_method);

    e8_var_object(result, c);

    E8_RETURN_OK;
}

#undef _COMMAND
