#include "unit.h"
#include "env.h"
#include "env_p.h"
#include "stack.h"
#include "e8core/variant/variant.h"
#include "e8core/utf/utf.h"
#include <malloc.h>

#include "macro.h"
#include "utils.h"
#include "_strings.h"
#include "instructions.h"

static bool __init = false;
e8_vtable vmt_unit;


static e8_runtime_error *
__e8_unit_iterate_with_rti(e8_unit_runtime_info_t *rti, e8_var *result);


/*! Ошибка виртуальной машины.
 *
 *  Возможныя причины:
 *  несовмѣстимыя команды, несовмѣстимыя версіи
 */
static e8_runtime_error *
__e8_vm_error()
{
    return e8_throw(E8_RT_ERROR_INTERNAL_FAIL);
}
E8_DECLARE_SUB(__e8_unit_call_inner);




/*! Выполняет одну команду */
static e8_runtime_error *
__e8_unit_command(e8_unit_runtime_info_t *rti)
{
    e8_runtime_error *__err;

    e8_command *c = &rti->unit->commands.data[rti->current];

    switch (c->cmd) {
        case E8_COMMAND_Exit: {
            rti->exit_flag = true;
            rti->finish_flag = true;
            E8_RETURN_OK;
        }

        case E8_COMMAND_EndSub: {
            /* EndSub без Return - возвращаем Неопределено */
            E8_VAR(undef);
            e8_stack_push_value(rti->stack, &undef);
        }

        case E8_COMMAND_Return:
            rti->finish_flag = true;
            E8_RETURN_OK;

        case E8_COMMAND_DeclareParamByRef:
            return __e8_operator_DeclareParamByRef(rti);

        case E8_COMMAND_DeclareParamByVal:
            return __e8_operator_DeclareParamByVal(rti);

        case E8_COMMAND_DeclareSub: {

            /* Пропускаем процедуру */
            do {
                ++c;
                ++rti->current;
            } while (c->cmd != E8_COMMAND_EndSub);

            ++c;
            ++rti->current;

            E8_RETURN_OK;
        }

        case E8_COMMAND_Raise:
            return __e8_operator_Raise(rti);

        case E8_COMMAND_Critical:
            return __e8_operator_Critical(rti);

        case E8_COMMAND_EndCritical:
            return __e8_operator_EndCritical(rti);

        case E8_COMMAND_Assert:
            return __e8_operator_Assert(rti);

        case E8_COMMAND_Table:
            return __e8_operator_Table(rti);

        case E8_COMMAND_Pop:
            return __e8_operator_Pop(rti);

        case E8_COMMAND_Iterator:
            return __e8_operator_Iterator(rti);

        case E8_COMMAND_Jump:
            return __e8_operator_Jump(rti);

        case E8_COMMAND_JumpTrue:
            return __e8_operator_JumpTrue(rti);

        case E8_COMMAND_JumpFalse:
            return __e8_operator_JumpFalse(rti);

        case E8_COMMAND_Const:
            return __e8_operator_Const(rti);

        case E8_COMMAND_LoadRef:
            return __e8_operator_LoadRef(rti);

        case E8_COMMAND_DeclareVar:
            return __e8_operator_DeclareVar(rti);

        case E8_COMMAND_LoadRefIndex:
            return __e8_operator_LoadRefIndex(rti);

        case E8_COMMAND_Call:
            return __e8_operator_Call(rti);

        case E8_COMMAND_ObjectCall:
            return __e8_operator_ObjectCall(rti);

        case E8_COMMAND_StackDup:
            return __e8_operator_StackDup(rti);

        case E8_COMMAND_Add:
            return __e8_operator_Add(rti);

        case E8_COMMAND_Sub:
            return __e8_operator_Sub(rti);

        case E8_COMMAND_Mul:
            return __e8_operator_Mul(rti);

        case E8_COMMAND_Div:
            return __e8_operator_Div(rti);

        case E8_COMMAND_Neg:
            return __e8_operator_Neg(rti);

        case E8_COMMAND_Assign:
            return __e8_operator_Assign(rti);

        case E8_COMMAND_Mod:
            return __e8_operator_Mod(rti);

        case E8_COMMAND_Or:
            return __e8_operator_Or(rti);

        case E8_COMMAND_And:
            return __e8_operator_And(rti);

        case E8_COMMAND_Not:
            return __e8_operator_Not(rti);

        case E8_COMMAND_Equal:
            return __e8_operator_Equal(rti);

        case E8_COMMAND_NotEqual:
            return __e8_operator_NotEqual(rti);

        case E8_COMMAND_Less:
            return __e8_operator_Less(rti);

        case E8_COMMAND_Greater:
            return __e8_operator_Greater(rti);

        case E8_COMMAND_LessEqual:
            return __e8_operator_LessEqual(rti);

        case E8_COMMAND_GreaterEqual:
            return __e8_operator_GreaterEqual(rti);

        case E8_COMMAND_Inc:
            return __e8_operator_Inc(rti);

        case E8_COMMAND_Dec:
            return __e8_operator_Dec(rti);

        case E8_COMMAND_Index:
            return __e8_operator_Index(rti);

    default:
        return __e8_vm_error();
    } // switch cmd

    return 0;
}

static bool
__e8_is_iteration_end(const e8_command *c)
{
    if (c->cmd == E8_COMMAND_EndSub)
        return true;
    if (c->cmd == E8_COMMAND_EndCritical)
        return true;
    return false;
}

/*! Выполняет один отладочный шаг */
e8_runtime_error *
__e8_unit_iterate(e8_unit_runtime_info_t *rti)
{
    e8_runtime_error *__err;
    rti->finish_flag = false;

    while (!rti->finish_flag) {
        int __ie = __e8_is_iteration_end(&rti->unit->commands.data[rti->current]);

        long start_tick, end_tick, delta;
        instruction_index_t debug_instruction;

        if (rti->unit->profile_out) {
            start_tick = e8_ticks((const e8_env_data *)rti->unit->env);
        }

        debug_instruction = rti->current;

        instruction_index_t current = rti->current;
        __err = __e8_unit_command(rti);

        if (rti->unit->profile_out) {
            end_tick = e8_ticks((const e8_env_data *)rti->unit->env);
            delta = end_tick - start_tick;
            e8_count_operation(rti->unit, debug_instruction, delta);
        }

        if ( __err ) {

            if (!__err->command_index)
                __err->command_index = current;

            return __err;
        }

        if (rti->current == current) {
            /* Команда не совершала переход */
            ++rti->current;
        }

        if (__ie)
            break;

        if (rti->unit->terminate_flag || rti->unit->exit_flag)
            rti->finish_flag = true;
    }

    E8_RETURN_OK;
}

#define FREE_NOT_NULL(name) if (name) free(name)

static void
__e8_unit_destroy(e8_object obj)
{
    e8_unit_data *d = (e8_unit_data *)obj;

    if (d->profile_output_path) {
        FILE *out = fopen(d->profile_output_path, "wt");

        instruction_index_t i;
        long total = 0, total_ticks = 0;
        const e8_command *c = d->commands.data;
        for (i = 0; i < d->commands.size; ++i, ++c) {
            if (d->ticks && d->counts) {
                fprintf(out, "[%4ld]:c[%12ld]t[%8ld]: ", i, d->counts[i], d->ticks[i]);
                total += d->ticks[i];
                total_ticks += d->counts[i];
            } else {
                fprintf(out, "[    ]:c[            ]t[        ]: ");
            }
            print_command(out, c);

            if (c->cmd == E8_COMMAND_Const) {
                fprintf(out, "\t\t\t\t\t\t; ");
                char *t = 0;

                dump_var(&d->consts.data[c->const_index], &t, "utf-8");

                fprintf(out, "%s", t);
                free(t);
            } else
            if (c->cmd == E8_COMMAND_DeclareParamByRef || c->cmd == E8_COMMAND_DeclareParamByVal) {
                if (c->const_index != 1) {
                    fprintf(out, "\t\t\t; ");
                    char *t = 0;

                    dump_var(&d->consts.data[c->const_index], &t, "utf-8");

                    fprintf(out, "%s", t);
                    free(t);
                }
            }

            fprintf(out, "\n");
        }

        fprintf(out, "Total :c[%12ld]t[%8ld]\n", total_ticks, total);

        fprintf(out, "Symbols:\n");
        for (i = 0; i < d->subs.size; ++i) {

            char *t = 0;
            e8_utf_to_8bit(d->subs.data[i].name, &t, "utf-8");

            if (d->subs.data[i]._export_item)
                fprintf(out, "\t.export ");
            else
                fprintf(out, "\t.local  ");

            fprintf(out, "%s $%d", t, d->subs.data[i].index);

            fprintf(out, "\n");
        }

        fprintf(out, "Consts:\n");

        for (i = 0; i < d->consts.size; ++i) {
            char *t = 0;
            dump_var(&d->consts.data[i], &t, "utf-8");
            fprintf(out, "%4ld: %s\n", i, t);
            free(t);
        }

        fprintf(out, "Names:\n");

        for (i = 0; i < d->tables.size; ++i) {

            fprintf(out, ".table %d\n", i);

            int j;
            e8_name_table *T = &d->tables.data[i];

            for (j = 0; j < T->size; ++j) {
                char *t = 0;
                e8_utf_to_8bit(T->names[j].name->s, &t, "utf-8");
                fprintf(out, "\t%4u: %s\n", T->names[j].index, t);
                free(t);
            }
        }

        fclose(out);
    }


    while (d->subs.size--) {
        e8_utf_free(d->subs.data[d->subs.size].name);
    }

    e8_collection_free((void*)&d->subs);
    FREE_NOT_NULL(d->root_path);

    int i = d->commands.size;
    while (i--)
        __e8_command_destroy(&d->commands.data[i]);
    e8_collection_free((void*)&d->commands);

    e8_gc_free_object(d->scope);
    d->scope = NULL;

    FREE_NOT_NULL(d->profile_output_path);
    FREE_NOT_NULL((void *)d->ticks); /* Disvolatile */
    FREE_NOT_NULL((void *)d->counts); /* Disvolatile */

    free((void *)d); /* Disvolatile */
}
#undef FREE_NOT_NULL

static
void __e8_unit_prepare(e8_unit_data *unit)
{
    unit->terminate_flag = false;
    unit->exit_flag = false;
}

static e8_runtime_error *
__e8_unit_iterate_with_rti(e8_unit_runtime_info_t *rti, e8_var *result)
{
    e8_runtime_error *__err;

    rti->finish_flag = false;
    while (!rti->finish_flag) {

        if (( __err = __e8_unit_iterate(rti) ))
            return __err;

    }

    if (result) {
        /* Ожидается, что исполнение предоставит итог вычислений */
        if (( __err = e8_stack_pop_value(rti->stack, result) ))
            return __err;
    }

    E8_RETURN_OK;
}

E8_DECLARE_SUB(__e8_unit_call_inner)
{
    if (!user_data)
        E8_THROW(E8_RT_ERROR_INTERNAL_FAIL);

    e8_sub *sub = (e8_sub *)user_data;
    e8_unit_data *d = (e8_unit_data *)sub->unit;

    e8_unit_runtime_info_t rti;
    memset(&rti, 0, sizeof(rti));

    rti.scope = 0;
    rti.unit = d;

    e8_scope_new((e8_scope *)&rti.scope, d->scope);
    e8_scope_set_this_object((e8_scope)rti.scope, obj);

    e8_scope_new((e8_scope *)&rti.scope, rti.scope);
    e8_gc_use_object(rti.scope);

    e8_collection_free(&rti.scope->params);

    rti.scope->params.data = argv;
    rti.scope->params.size = argc;
    rti.scope->params.reserved = argc;

    e8_runtime_error *__err;

    rti.stack = 0;
    e8_stack_new(&rti.stack, 100);

    rti.current = sub->index;
    rti.vmt_scope = &vmt_scope;

    int i;

    __err = __e8_unit_iterate_with_rti(&rti, result);

    i = MAX_LOCAL_VARS;
    while (i--) {
        if (rti.created_local[i]) {
            e8_destroy_variant_property((e8_property *)&rti.local_data[i]);
            free(rti.local_data[i].data);
        }
    }

    e8_stack_destroy(rti.stack);
    rti.scope->params.data = 0;
    e8_scope_destroy(rti.scope);

    return __err;
}

static E8_DECLARE_GET_METHOD(__e8_unit_get_method)
{
    e8_unit_data *d = (e8_unit_data *)obj;
    int i;
    for (i = 0; i < d->subs.size; ++i) {
        if (e8_utf_stricmp(d->subs.data[i].name, name) == 0) {
            *fn = __e8_unit_call_inner;
            *user_data = &d->subs.data[i];
            E8_RETURN_OK;
        }
    }

    E8_THROW_DETAIL(E8_RT_METHOD_NOT_FOUND, e8_utf_strdup(name));
}


void __e8_init_vmt_unit()
{
    e8_vtable_template(&vmt_unit);
    vmt_unit.get_method = __e8_unit_get_method;
    vmt_unit.dtor = __e8_unit_destroy;
    __init = true;
}

e8_runtime_error *
e8_unit_execute(e8_unit_data *unit)
{

    if (!__init)
        __e8_init_vmt_unit();

    e8_runtime_error *__err;

    __e8_unit_prepare(unit);

    e8_unit_runtime_info_t rti;
    memset(&rti, 0, sizeof(rti));
    rti.unit = unit;

    rti.stack = 0;
    e8_stack_new(&rti.stack, 100);

    e8_scope_new((e8_scope *)&rti.scope, unit->scope);

    e8_gc_use_object(rti.scope);

    rti.current = 0;
    rti.vmt_scope = &vmt_scope;
    rti.table = unit->tables.data;

    int i;

    __err = __e8_unit_iterate_with_rti(&rti, 0);

    i = MAX_LOCAL_VARS;
    while (i--) {
        if (rti.created_local[i]) {
            e8_destroy_variant_property((e8_property *)&rti.local_data[i]);
            free(rti.local_data[i].data);
        }
    }



    e8_stack_destroy(rti.stack);
    e8_scope_destroy(rti.scope);

    return __err;
}
