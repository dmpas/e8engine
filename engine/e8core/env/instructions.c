#include "instructions.h"
#include <malloc.h>
#include "macro.h"
#include "utils.h"

static const e8_uchar u_iterator[] = {'_', '_', 'i', 't', 'e', 'r', 'a', 't', 'o', 'r', 0};

static void
__clone_property(e8_property *src, e8_property *dst)
{
    *src = *dst;
}

static e8_runtime_error *
__var_property_get(void *data, e8_var *value)
{
    e8_var *v = (e8_var *) ((e8_property *)data)->data;
    e8_var_assign(value, v);
    return 0;
}

static e8_runtime_error *
__var_property_set(void *data, const e8_var *value)
{
    e8_var *v = (e8_var *) ((e8_property *)data)->data;
    e8_var_assign(v, value);
    return 0;
}
static void
__var_property(e8_property *dst, e8_var *src)
{
    dst->get = __var_property_get;
    dst->set = __var_property_set;
    dst->can_get = true;
    dst->can_set = true;
    dst->data = src;
}

static const e8_string *
__search_in_name_table(const e8_name_table *T, unsigned index)
{
    if (T == NULL)
        return NULL;

    if (T->size == 0)
        return NULL;

    int l = 0, r = T->size - 1;

    while (l < r) {

        int h = (l + r) / 2;
        const e8_name_element *E = &T->names[h];

        if (E->index == index)
            return E->name;

        if (E->index > index) {
            r = h - 1;
        } else {
            l = h + 1;
        }

    }

    if (T->names[l].index == index)
        return T->names[l].name;

    return NULL;
}

static e8_runtime_error *
__e8_unit_objectcall(e8_unit_runtime_info_t *rti)
{
    e8_var      v_argc;
    e8_stack_pop_value(rti->stack, &v_argc);
    long        i_argc, i;
    i_argc = v_argc.num.l_value;

    /* +1 на случай i_argc == 0*/
    e8_property *params = AALLOC(e8_property, i_argc + 1);
    e8_var      *__var = AALLOC(e8_var, i_argc + 1);

    for (i = i_argc; i ; --i) {
        int j= i-1;
        e8_stack_element el;
        e8_stack_pop(rti->stack, &el);

        if (el.type == E8_STACK_REFERENCE) {
            e8_var_undefined(&__var[j]);
            __clone_property(&params[j], &el.p);
        } else {
            __var[j] = el.v;
            __var_property(&params[j], &__var[j]);
        }
    }

    e8_runtime_error *__err = 0;

    e8_var v_method;
    if (( __err = e8_stack_pop_value(rti->stack, &v_method) ))
        return __err;

    const e8_uchar *s_method = v_method.str->s;

    e8_var v_obj;
    e8_stack_pop_value(rti->stack, &v_obj);

    e8_vtable  *env_vt;
    e8_object   obj;

    if (v_obj.type == varString) {

        obj = v_obj.str;
        env_vt = &vmt_string;

    } else {

        if (v_obj.gcl)
            env_vt = ((gc_list_element*)v_obj.gcl)->vmt;
        else
            env_vt = e8_get_vtable(v_obj.obj);

        obj = v_obj.obj;
    }

    if (!env_vt) {
        __err = e8_throw_data(E8_RT_NOT_AN_OBJECT, e8_utf_strdup(s_method));
        goto lb_free1;
    }

    E8_VAR(result);

    e8_function_signature fn;
    void *user_data = 0;
    if (( env_vt->get_method(obj, s_method, &fn, &user_data) )) {
        __err = e8_throw_data(E8_RT_METHOD_NOT_FOUND,
                              e8_usprintf(NULL, "%v has no method %S", v_obj, s_method)
                );
        goto lb_free1;
    }

    if (!fn) {
        __err = e8_throw_data(E8_RT_METHOD_NOT_FOUND,
                              e8_usprintf(NULL, "%v has no method %S", v_obj, s_method)
                );
        goto lb_free1;
    }

    __err = fn(obj, user_data, i_argc, params, &result);

    if (__err)
        goto lb_free1;

    e8_stack_push_value(rti->stack, &result);
    e8_var_destroy(&result);

lb_free1:
    e8_var_destroy(&v_method);
    e8_var_destroy(&v_obj);

    for (i = 0; i < i_argc; ++i)
        e8_var_destroy(&__var[i]);

    free(__var);
    free(params);

    return __err;
}


static e8_runtime_error *
__e8_local_call(e8_unit_runtime_info_t *rti)
{
    e8_runtime_error *__err = 0;

    e8_var      v_argc;
    if (( __err = e8_stack_pop_value(rti->stack, &v_argc) ))
        return __err;

    long        i_argc, i;
    i_argc = v_argc.num.l_value;

    e8_property *params = AALLOC(e8_property, i_argc + 1);
    e8_var      *__var = AALLOC(e8_var, i_argc + 1);

    for (i = i_argc; i ; --i) {
        int j= i-1;
        e8_stack_element el;
        if (( __err = e8_stack_pop(rti->stack, &el) ))
            return __err;

        if (el.type == E8_STACK_REFERENCE) {
            e8_var_undefined(&__var[j]);
            __clone_property(&params[j], &el.p);
        } else {
            __var[j] = el.v;
            __var_property(&params[j], &__var[j]);
        }
    }

    e8_var v_method;
    if (( __err = e8_stack_pop_value(rti->stack, &v_method) ))
        goto lb_free1;

    const e8_uchar *s_method = v_method.str->s;

    bool found = false;

    {
        /* Поиске в скопе */
        e8_function_signature fn;
        void *user_data;
        __err = rti->vmt_scope->get_method(rti->scope, s_method, &fn, &user_data);
        if (__err)
            e8_free_exception(__err);
        else {

            if (fn) {

                E8_VAR(result);
                __err = fn((void*)rti->unit, user_data, i_argc, params, &result);

                if (__err == NULL) {
                    e8_stack_push_value(rti->stack, &result);
                    e8_var_destroy(&result);
                }

                found = true;
                goto lb_free1;
            }

        }
    }


    for (i = 0; i < rti->unit->subs.size; ++i) {
        if (e8_utf_stricmp(rti->unit->subs.data[i].name, s_method) == 0) {

            found = true;

            E8_VAR(result);
            __err = __e8_unit_call_inner((void*)rti->unit, &rti->unit->subs.data[i], i_argc, params, &result);

            if (__err)
                goto lb_free1;

            e8_stack_push_value(rti->stack, &result);
            e8_var_destroy(&result);

            break;
        }
    }

    if (!found) {
        e8_vtable  *env_vt = e8_get_vtable(rti->unit->env);
        E8_VAR(result);

        e8_function_signature fn;
        void *user_data = 0;

        if (( __err = env_vt->get_method(rti->unit->env, s_method, &fn, &user_data) ))
            goto lb_free1;

        if (( __err = fn((void*)rti->unit, user_data, i_argc, params, &result) ))
            goto lb_free1;

        e8_stack_push_value(rti->stack, &result);
        e8_var_destroy(&result);
    }

lb_free1:
    e8_var_destroy(&v_method);
    for (i = 0; i < i_argc; ++i)
        e8_var_destroy(&__var[i]);
    free(__var);
    free(params);

    return __err;
}



DECLARE_OPERATOR(__e8_operator_nop)
{
    E8_RETURN_OK;
}


DECLARE_OPERATOR(__e8_operator_Raise) {

	e8_string *s;

	E8_VAR(error_text);
	e8_stack_pop_value(rti->stack, &error_text);
	e8_var_cast_string(&error_text, &s);

	e8_uchar *uc = e8_utf_strdup(s->s);
	e8_string_destroy(s);

	char *utf = 0;
	e8_utf_to_8bit(uc, &utf, "utf-8");
	e8_utf_free(uc);

	E8_THROW_DETAIL(E8_RT_EXCEPTION_RAISED, utf);
}

DECLARE_OPERATOR(__e8_operator_Critical) {

	instruction_index_t catch_label = rti->current + rti->unit->commands.data[rti->current].delta;

	++rti->current;

	e8_runtime_error *__err;
	__err = __e8_unit_iterate(rti);

	if (__err) {
		rti->current = catch_label;
		e8_free_exception(__err); /* TODO: Пересылка исключения в обработчик */
	}

	E8_RETURN_OK;
}

DECLARE_OPERATOR(__e8_operator_EndCritical)
{
	E8_RETURN_OK;
}

DECLARE_OPERATOR(__e8_operator_Assert)
{
	e8_runtime_error *__err = NULL;
	e8_var cond;
	if (( __err = e8_stack_pop_value(rti->stack, &cond) ))
		return __err;

	const e8_command *c = &rti->unit->commands.data[rti->current];

	bool b_cond;
	if (e8_var_cast_bool(&cond, &b_cond))
		E8_THROW(E8_RT_CANNOT_CAST_TO_BOOL);

	if (!b_cond) {
        e8_var value = rti->unit->consts.data[c->const_index];
		E8_THROW_DETAIL(E8_RT_ASSERTION_FAILED, e8_utf_strdup(value.str->s));
	}
	E8_RETURN_OK;
}
DECLARE_OPERATOR(__e8_operator_Pop)
{
	e8_stack_pop(rti->stack, 0);
	E8_RETURN_OK;
}

DECLARE_OPERATOR(__e8_operator_Iterator)
{
	e8_var coll;
	e8_runtime_error *__err = NULL;
	if (( __err = e8_stack_pop_value(rti->stack, &coll) ))
		return __err;

	e8_vtable  *coll_vmt;
	e8_object   obj;

	if (coll.type == varString) {
		obj = coll.str;
		coll_vmt = &vmt_string;
	} else {
		coll_vmt = ((gc_list_element*)coll.gcl)->vmt;
		obj = coll.obj;
	}

	if (!coll_vmt)
		E8_THROW_DETAIL(E8_RT_NOT_AN_OBJECT, e8_utf_strcdup("Iterator", "utf-8"));

	e8_function_signature fn;
	void *user_data;
	if (( __err = coll_vmt->get_method(obj, u_iterator, &fn, &user_data) ))
		return __err;

	if (!fn)
		E8_THROW_DETAIL(E8_RT_NOT_A_COLLECTION, e8_usprintf(0, "%v not a collection!", coll) );

	e8_var it;
	if (( __err = fn(obj, user_data, 0, 0, &it) ))
		return __err;

	e8_stack_push_value(rti->stack, &it);

	e8_var_destroy(&it);
	e8_var_destroy(&coll);

	E8_RETURN_OK;
}
DECLARE_OPERATOR(__e8_operator_Jump)
{
	rti->current += rti->unit->commands.data[rti->current].delta;
	E8_RETURN_OK;
}

DECLARE_OPERATOR(__e8_operator_JumpTrue)
{
	e8_var cond;
	e8_stack_pop_value(rti->stack, &cond);
	bool b_cond;
	if (e8_var_cast_bool(&cond, &b_cond) )
		E8_THROW(E8_RT_CANNOT_CAST_TO_BOOL);

	if (b_cond)
		rti->current += rti->unit->commands.data[rti->current].delta;
	E8_RETURN_OK;
}
DECLARE_OPERATOR(__e8_operator_JumpFalse)
{
	e8_var cond;
	e8_stack_pop_value(rti->stack, &cond);
	bool b_cond;
	if (e8_var_cast_bool(&cond, &b_cond) )
		E8_THROW(E8_RT_CANNOT_CAST_TO_BOOL);

	if (!b_cond)
		rti->current += rti->unit->commands.data[rti->current].delta;
	E8_RETURN_OK;
}
DECLARE_OPERATOR(__e8_operator_Const)
{
	e8_stack_push_value(rti->stack, &rti->unit->consts.data[rti->unit->commands.data[rti->current].const_index]);
	E8_RETURN_OK;
}
DECLARE_OPERATOR(__e8_operator_Table)
{
    const e8_command *c = &rti->unit->commands.data[rti->current];

    if (rti->unit->tables.size > c->label)
        rti->table = &rti->unit->tables.data[c->label];
    else
        rti->table = 0;

    E8_RETURN_OK;
}
DECLARE_OPERATOR(__e8_operator_LoadRef)
{
	e8_property prop;
	e8_runtime_error *__err = NULL;
    const e8_command *c = &rti->unit->commands.data[rti->current];

    e8_var *v = &rti->unit->consts.data[c->const_index];

	if ( (__err = rti->vmt_scope->get_property(
				rti->scope,
				v->str->s,
				&prop
		)) )
		return __err;

	e8_stack_push_reference(rti->stack, &prop);

	E8_RETURN_OK;
}
DECLARE_OPERATOR(__e8_operator_DeclareVar)
{

	e8_property prop;
	const e8_command *c = &rti->unit->commands.data[rti->current];
	e8_runtime_error *__err = NULL;

	e8_var *v = malloc(sizeof(*v));
	e8_var_undefined(v);

	__var_property(&prop, v);

	if (c->label != WRONG_LABEL) {
		rti->local_data[c->label] = prop;
		rti->declared_local[c->label] = true;
	}

	const e8_string *name = __search_in_name_table(rti->table, c->label);
	if (name) {
        __err = e8_scope_add(rti->scope->parent, name->s, &prop, true);
	}

	return __err;
}
DECLARE_OPERATOR(__e8_operator_LoadRefIndex)
{
	e8_property prop;
	e8_command *c = &rti->unit->commands.data[rti->current];
	if (!rti->declared_local[c->label]) {
		e8_var *v = malloc(sizeof(*v));
		e8_var_undefined(v);
		e8_create_variant_property(&prop, v);
		rti->local_data[c->label] = prop;
		rti->declared_local[c->label] = true;
		rti->created_local[c->label] = true;
	}
	prop = rti->local_data[c->label];
	e8_stack_push_reference(rti->stack, &prop);
	E8_RETURN_OK;
}
DECLARE_OPERATOR(__e8_operator_Call)
{
	e8_runtime_error *__err = NULL;
	if (( __err = __e8_local_call(rti) ))
		return __err;
	E8_RETURN_OK;
}
DECLARE_OPERATOR(__e8_operator_ObjectCall)
{
	e8_runtime_error *__err = NULL;
	if (( __err = __e8_unit_objectcall(rti) ))
		return __err;

	E8_RETURN_OK;
}
DECLARE_OPERATOR(__e8_operator_StackDup)
{
	int i = rti->stack->size;
	if (i == 0)
		E8_THROW(E8_RT_ERROR_STACK_EMPTY);

	e8_stack_element *el = &rti->stack->data[i - 1];

	if (el->type == E8_STACK_REFERENCE)
		e8_stack_push_reference(rti->stack, &el->p);

	if (el->type == E8_STACK_VALUE)
		e8_stack_push_value(rti->stack, &el->v);
	E8_RETURN_OK;
}

DECLARE_OPERATOR(__e8_operator_DeclareParamByVal)
{

    e8_command *c = &rti->unit->commands.data[rti->current];
    e8_runtime_error *__err = NULL;

    size_t params_count = rti->scope->params.size;

    e8_var *p_value = malloc(sizeof(*p_value));
    e8_var_undefined(p_value);

    e8_var_assign(p_value, &rti->unit->consts.data[c->const_index]);

    if (rti->param_index < params_count) {
        E8_VAR(p_set);
        e8_property *m_p = &rti->scope->params.data[rti->param_index];
        e8_property_get_handler m_get = m_p->get;

        if (( __err = m_get(m_p, &p_set) ))
            return __err;

        if (p_set.type != varVoid)
            e8_var_assign(p_value, &p_set);

        e8_var_destroy(&p_set);
    }

    if (p_value->type == varVoid) {
        E8_THROW(E8_RT_UNDEFINED_CALL_PARAMS);
    }

    e8_property m_p;
    __var_property(&m_p, p_value);

	if (c->label != WRONG_LABEL) {
		rti->local_data[c->label] = m_p;
		rti->declared_local[c->label] = true;
	}

	const e8_string *name = __search_in_name_table(rti->table, c->label);
	if (name) {
        __err = e8_scope_add(rti->scope->parent, name->s, &m_p, true);
	}

    ++rti->param_index;

    E8_RETURN_OK;
}

DECLARE_OPERATOR(__e8_operator_DeclareParamByRef)
{
    e8_command *c = &rti->unit->commands.data[rti->current];
    e8_runtime_error *__err = NULL;

    size_t params_count = rti->scope->params.size;

	const e8_string *name = __search_in_name_table(rti->table, c->label);

    if (rti->param_index < params_count) {
        e8_property *m_p = &rti->scope->params.data[rti->param_index];
        if (m_p->get == __var_property_get) {
            /* Это внутреннее преобразование значения в свойство */
            e8_var p_value;
            e8_var_undefined(&p_value);

            m_p->get(m_p, &p_value);
            if (p_value.type == varVoid) {
                /* Не указан параметр */
                e8_var_assign(&p_value, &rti->unit->consts.data[c->const_index]);
                m_p->set(m_p, &p_value);
            }
            e8_var_destroy(&p_value);
        }

        if (c->label != WRONG_LABEL) {
            rti->local_data[c->label] = *m_p;
            rti->declared_local[c->label] = true;
        }
        if (name)
            e8_scope_add(rti->scope, name->s, m_p, false);
    } else {

        /* Незаданный параметр по ссылке */
        e8_var *p_value = malloc(sizeof(*p_value));
        e8_var_undefined(p_value);

        e8_var_assign(p_value, &rti->unit->consts.data[c->const_index]);
        if (p_value->type == varVoid) {
            E8_THROW(E8_RT_UNDEFINED_CALL_PARAMS);
        }

        e8_property m_p;
        __var_property(&m_p, p_value);

        if (c->label != WRONG_LABEL) {
            rti->local_data[c->label] = m_p;
            rti->declared_local[c->label] = true;
        }

        if (name)
            e8_scope_add(rti->scope, name->s, &m_p, false);
    }

    ++rti->param_index;

    E8_RETURN_OK;
}

DECLARE_OPERATOR(__e8_operator_Add)
{
	e8_var a, b;
	e8_runtime_error *__err;
	if (( __err = e8_stack_pop_value(rti->stack, &b) ))
		return __err;
	if (( __err = e8_stack_pop_value(rti->stack, &a) ))
		return __err;

	e8_var_add(&a, &b);

	e8_stack_push_value(rti->stack, &a);

	e8_var_destroy(&a);
	e8_var_destroy(&b);
	E8_RETURN_OK;
}

DECLARE_OPERATOR(__e8_operator_Sub)
{
	e8_var a, b;
	e8_stack_pop_value(rti->stack, &b);
	e8_stack_pop_value(rti->stack, &a);

	e8_var_sub(&a, &b);

	e8_stack_push_value(rti->stack, &a);

	e8_var_destroy(&a);
	e8_var_destroy(&b);

	E8_RETURN_OK;
}


DECLARE_OPERATOR(__e8_operator_Mul)
{
	e8_var a, b;
	e8_stack_pop_value(rti->stack, &b);
	e8_stack_pop_value(rti->stack, &a);

	e8_var_mul(&a, &b);

	e8_stack_push_value(rti->stack, &a);

	e8_var_destroy(&a);
	e8_var_destroy(&b);

	E8_RETURN_OK;
}


DECLARE_OPERATOR(__e8_operator_Div)
{
	e8_var a, b;
	e8_stack_pop_value(rti->stack, &b);
	e8_stack_pop_value(rti->stack, &a);

	int r = e8_var_div(&a, &b);

	if (r) {
		E8_THROW_DETAIL(
				E8_RT_EXCEPTION_RAISED,
				e8_sprintf(0, "utf-8", "Division error: [%v] / [%v].", a, b)
		);
	} else
		e8_stack_push_value(rti->stack, &a);

	e8_var_destroy(&a);
	e8_var_destroy(&b);

	E8_RETURN_OK;
}


DECLARE_OPERATOR(__e8_operator_Neg)
{
	e8_var a;
	e8_stack_pop_value(rti->stack, &a);
	e8_var_neg(&a);
	e8_stack_push_value(rti->stack, &a);
	e8_var_destroy(&a);
	E8_RETURN_OK;
}

DECLARE_OPERATOR(__e8_operator_Assign)
{
	e8_runtime_error *__err;
	e8_var v;
	e8_stack_pop_value(rti->stack, &v);

	e8_property p;
	if (( __err = e8_stack_pop_ref(rti->stack, &p) ))
		return __err;

	if (!p.can_set)
		E8_THROW(E8_RT_VALUE_CANNOT_BE_WRITTEN);

	p.set(&p, &v);

	e8_var_destroy(&v);
	E8_RETURN_OK;
}

DECLARE_OPERATOR(__e8_operator_Mod)
{
	e8_var a, b;
	e8_stack_pop_value(rti->stack, &b);
	e8_stack_pop_value(rti->stack, &a);

	int r = e8_var_mod(&a, &b);
	if (r) {
		E8_THROW_DETAIL(
				E8_RT_EXCEPTION_RAISED,
				e8_sprintf(0, "utf-8", "Division error: [%v] / [%v].", a, b)
		);
	} else
		e8_stack_push_value(rti->stack, &a);

	e8_var_destroy(&a);
	e8_var_destroy(&b);
	E8_RETURN_OK;
}

DECLARE_OPERATOR(__e8_operator_Or)
{
	e8_var a, b;
	e8_stack_pop_value(rti->stack, &b);
	e8_stack_pop_value(rti->stack, &a);

	e8_var_or(&a, &b);

	e8_stack_push_value(rti->stack, &a);

	e8_var_destroy(&a);
	e8_var_destroy(&b);
	E8_RETURN_OK;
}

DECLARE_OPERATOR(__e8_operator_And)
{
	e8_var a, b;
	e8_stack_pop_value(rti->stack, &b);
	e8_stack_pop_value(rti->stack, &a);

	e8_var_and(&a, &b);

	e8_stack_push_value(rti->stack, &a);

	e8_var_destroy(&a);
	e8_var_destroy(&b);
	E8_RETURN_OK;
}

DECLARE_OPERATOR(__e8_operator_Not)
{
	e8_var a;
	e8_stack_pop_value(rti->stack, &a);

	e8_var_not(&a);
	e8_stack_push_value(rti->stack, &a);

	e8_var_destroy(&a);
	E8_RETURN_OK;
}

DECLARE_OPERATOR(__e8_operator_Equal)
{
	e8_var a, b, r;

	e8_stack_pop_value(rti->stack, &b);
	e8_stack_pop_value(rti->stack, &a);
	e8_var_eq(&a, &b, &r);

	e8_stack_push_value(rti->stack, &r);

	e8_var_destroy(&a);
	e8_var_destroy(&b);
	e8_var_destroy(&r);
	E8_RETURN_OK;
}

DECLARE_OPERATOR(__e8_operator_NotEqual)
{
	e8_var a, b, r;

	e8_stack_pop_value(rti->stack, &b);
	e8_stack_pop_value(rti->stack, &a);
	e8_var_ne(&a, &b, &r);

	e8_stack_push_value(rti->stack, &r);

	e8_var_destroy(&a);
	e8_var_destroy(&b);
	e8_var_destroy(&r);
	E8_RETURN_OK;
}

DECLARE_OPERATOR(__e8_operator_Less)
{
	e8_var a, b, r;

	e8_stack_pop_value(rti->stack, &b);
	e8_stack_pop_value(rti->stack, &a);
	e8_var_lt(&a, &b, &r);

	e8_stack_push_value(rti->stack, &r);

	e8_var_destroy(&a);
	e8_var_destroy(&b);
	e8_var_destroy(&r);
	E8_RETURN_OK;
}

DECLARE_OPERATOR(__e8_operator_Greater)
{
	e8_var a, b, r;

	e8_stack_pop_value(rti->stack, &b);
	e8_stack_pop_value(rti->stack, &a);
	e8_var_gt(&a, &b, &r);

	e8_stack_push_value(rti->stack, &r);

	e8_var_destroy(&a);
	e8_var_destroy(&b);
	e8_var_destroy(&r);
	E8_RETURN_OK;
}

DECLARE_OPERATOR(__e8_operator_LessEqual)
{
	e8_var a, b, r;

	e8_stack_pop_value(rti->stack, &b);
	e8_stack_pop_value(rti->stack, &a);
	e8_var_le(&a, &b, &r);

	e8_stack_push_value(rti->stack, &r);

	e8_var_destroy(&a);
	e8_var_destroy(&b);
	e8_var_destroy(&r);
	E8_RETURN_OK;

}
DECLARE_OPERATOR(__e8_operator_GreaterEqual)
{
	e8_var a, b, r;

	e8_stack_pop_value(rti->stack, &b);
	e8_stack_pop_value(rti->stack, &a);
	e8_var_ge(&a, &b, &r);

	e8_stack_push_value(rti->stack, &r);

	e8_var_destroy(&a);
	e8_var_destroy(&b);
	e8_var_destroy(&r);
	E8_RETURN_OK;
}

DECLARE_OPERATOR(__e8_operator_Inc)
{
	e8_property p;
	e8_stack_pop_ref(rti->stack, &p);

	if (!p.can_set)
		E8_THROW(E8_RT_VALUE_CANNOT_BE_WRITTEN);

	if (!p.can_get)
		E8_THROW(E8_RT_VALUE_CANNOT_BE_READ);

	e8_var v, one;
	e8_var_undefined(&v);
	e8_var_long(&one, 1);
	p.get(&p, &v);
	e8_var_add(&v, &one);
	p.set(&p, &v);
	E8_RETURN_OK;
}
DECLARE_OPERATOR(__e8_operator_Dec)
{
	e8_property p;
	e8_stack_pop_ref(rti->stack, &p);
	if (!p.can_set)
		E8_THROW(E8_RT_VALUE_CANNOT_BE_WRITTEN);
	if (!p.can_get)
		E8_THROW(E8_RT_VALUE_CANNOT_BE_READ);

	e8_var v, one;
	e8_var_undefined(&v);
	e8_var_long(&one, 1);
	p.get(&p, &v);
	e8_var_sub(&v, &one);
	p.set(&p, &v);
	E8_RETURN_OK;
}

DECLARE_OPERATOR(__e8_operator_Index)
{
	e8_runtime_error *__err;
	e8_var index, obj;
	e8_property nr;

	if (( __err = e8_stack_pop_value(rti->stack, &index) ))
		return __err;

	if (( __err = e8_stack_pop_value(rti->stack, &obj) ))
		return __err;

	e8_object  hobj;
	e8_vtable *vmt = NULL;

	if (obj.type == varString) {

		vmt = &vmt_string;
		hobj = obj.str;

	} else if (obj.type == varObject) {

		if (obj.gcl)
			vmt = ((gc_list_element*)obj.gcl)->vmt;

		hobj = obj.obj;

	} else
		vmt = NULL;

	if (!vmt)
		E8_THROW_DETAIL(E8_RT_NOT_A_COLLECTION, e8_usprintf(0, "%v not a collection!", obj) );

	if (!vmt->by_index)
		E8_THROW(E8_RT_UNDEFINED_GET_BY_INDEX);

	if (( __err = vmt->by_index(hobj, &index, &nr) ))
		return __err;

	e8_stack_push_reference(rti->stack, &nr);

	e8_var_destroy(&index);
	e8_var_destroy(&obj);

	E8_RETURN_OK;
}
