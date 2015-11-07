#include "variant.h"
#include <string.h>
#include <malloc.h>
#include <stdio.h>

static inline int OK(int r)
{
    return r == E8_VAR_OK;
}

e8_var * e8_var_bool(e8_var *v, bool value)
{
    v->type = varBool;
    v->bv = value;
    return v;
}
static inline void __e8_zero_mem(char *v, size_t l)
{
    memset(v, 0, l);
}
e8_var * e8_var_void(e8_var *v)
{
    __e8_zero_mem((char *)v, sizeof(e8_var));
    v->type = varVoid;
    return v;
}
e8_var * e8_var_undefined(e8_var *v)
{
    v->type = varUndefined;
    return v;
}
e8_var * e8_var_long(e8_var *v, long value)
{
    v->type = varNumeric;
    e8_numeric_long(&v->num, value);
    return v;
}
e8_var * e8_var_double(e8_var *v, double value)
{
    v->type = varNumeric;
    e8_numeric_double(&v->num, value);
    return v;
}
e8_var * e8_var_date(e8_var *v, e8_date value)
{
    v->type = varDateTime;
    v->date = value;
    return v;
}
e8_var * e8_var_string(e8_var *v, const e8_uchar *value)
{
    v->type = varString;
    v->str = e8_string_init(value);
    return v;
}
e8_var * e8_var_string_utf(e8_var *v, const char *value)
{
    e8_uchar *uc = e8_utf_strcdup(value, "utf-8");
    v->type = varString;
    v->str = e8_string_init(uc);
    e8_utf_free(uc);
    return v;
}
int e8_var_destroy(e8_var *v)
{
    if (v->type == varString) {
        e8_string_destroy(v->str);
        v->str = NULL;
    }

    if (v->type == varObject) {
        e8_gcl_free_object(v->gcl);
        v->obj = NULL;
    }

    v->type = varNull;

    return E8_VAR_OK;
}

e8_var * e8_var_assign(e8_var *dst, const e8_var *src)
{
    e8_var_destroy(dst);
    dst->type = src->type;
    if (dst->type == varBool)
        dst->bv = src->bv;

    if (dst->type == varDateTime)
        dst->date = src->date;

    if (dst->type == varNumeric)
        e8_numeric_assign(&dst->num, &src->num);

    if (dst->type == varString) {
        dst->str = e8_string_copy(src->str);
    }

    if (dst->type == varObject) {

        dst->obj = src->obj;
        dst->gcl = src->gcl;

        e8_gcl_use_object(dst->gcl);
    }

    if (dst->type == varEnum)
        dst->_enum = src->_enum;
    // TODO: [E8::Core::Variant] var_assign
    return dst;
}

int e8_var_add(e8_var *a, const e8_var *b)
{
    if (a->type == varNumeric) {
        if (b->type == varNumeric) {
            e8_numeric_add(&a->num, &b->num);
            return E8_VAR_OK;
        }
    } else if (a->type == varString) {

        e8_string *s, *ns;

        int r = e8_var_cast_string(b, &s);
        if (r != E8_VAR_OK)
            return r;

        ns = e8_string_concat(a->str, s);
        e8_string_destroy(s);
        e8_string_destroy(a->str);
        a->str = ns;

        return E8_VAR_OK;

    } else if (a->type == varDateTime) {

        if (b->type == varNumeric) {
            e8_date_add(&a->date, e8_numeric_as_long(&b->num));
            return E8_VAR_OK;
        }

    }
    return E8_VAR_WRONG_CAST;
}
int e8_var_sub(e8_var *a, const e8_var *b)
{
    if (a->type == varNumeric && b->type == varNumeric) {
        e8_numeric_sub(&a->num, &b->num);
        return E8_VAR_OK;
    }
    if (a->type == varDateTime && b->type == varNumeric) {
        e8_date_sub_long(&a->date, e8_numeric_as_long(&b->num));
        return E8_VAR_OK;
    }
    if (a->type == varDateTime && b->type == varDateTime) {
        long r = e8_date_sub_date(a->date, b->date);
        e8_var_long(a, r);
        return E8_VAR_OK;
    }
    return E8_VAR_OK;
}
int e8_var_mul(e8_var *a, const e8_var *b)
{
    if (a->type != varNumeric || b->type != varNumeric)
        return E8_VAR_WRONG_CAST;
    e8_numeric_mul(&a->num, &b->num);
    return E8_VAR_OK;
}
int e8_var_div(e8_var *a, const e8_var *b)
{
    if (a->type != varNumeric || b->type != varNumeric)
        return E8_VAR_WRONG_CAST;

    if (b->num.type == numInt && b->num.l_value == 0)
        return E8_VAR_WRONG_CAST;

    if (b->num.type == numFloat && b->num.d_value == 0)
        return E8_VAR_WRONG_CAST;

    e8_numeric_div(&a->num, &b->num);

    return E8_VAR_OK;
}
int e8_var_mod(e8_var *a, const e8_var *b)
{
    if (a->type != varNumeric || b->type != varNumeric)
        return E8_VAR_WRONG_CAST;

    if (b->num.type == numInt && b->num.l_value == 0)
        return E8_VAR_WRONG_CAST;

    if (b->num.type == numFloat && b->num.d_value == 0)
        return E8_VAR_WRONG_CAST;

    e8_numeric_mod(&a->num, &b->num);

    return E8_VAR_OK;
}

int e8_var_not(e8_var *a)
{
    if (a->type == varBool) {
        a->bv = !a->bv;
        return E8_VAR_OK;
    }
    return E8_VAR_WRONG_CAST;
}

int e8_var_neg(e8_var *a)
{
    if (a->type == varNumeric) {
        e8_numeric_neg(&a->num);
        return E8_VAR_OK;
    }
    return E8_VAR_WRONG_CAST;
}

e8_string *e8_gregorian_cast_string(e8_date date)
{
    char buf[60];
    sprintf(buf, "%02d.%02d.%04d %02d:%02d:%02d"
                    , e8_date_get_day(date), e8_date_get_month(date), e8_date_get_year(date)
                    , e8_date_get_hour(date), e8_date_get_min(date), e8_date_get_sec(date)
    );

    return e8_string_ascii(buf);
}

int e8_var_cast_string(const e8_var *src, e8_string **dst)
{
    if (src->type == varObject) {
        e8_var v;
        e8_obj_to_string(src->obj, &v);
        *dst = e8_string_copy(v.str);
        return E8_VAR_OK;
    }
    if (src->type == varNumeric) {
        *dst = e8_numeric_to_string(&src->num);
        return E8_VAR_OK;
    }
    if (src->type == varString) {
        *dst = e8_string_copy(src->str);
        return E8_VAR_OK;
    }
    if (src->type == varNull) {
        *dst = e8_string_ascii("Null");
        return E8_VAR_OK;
    }
    if (src->type == varUndefined) {
        *dst = e8_string_ascii("Undefined");
        return E8_VAR_OK;
    }
    if (src->type == varVoid) {
        *dst = e8_string_ascii("Void");
        return E8_VAR_OK;
    }
    if (src->type == varBool) {

        if (src->bv)
            *dst = e8_string_ascii("True");
        else
            *dst = e8_string_ascii("False");

        return E8_VAR_OK;
    }
    if (src->type == varEnum) {
        char buf[20];
        sprintf(buf, "Enum(%d)", src->_enum.index);
        *dst = e8_string_ascii(buf);
        return E8_VAR_OK;
    }

    if (src->type == varDateTime) {
        *dst = e8_gregorian_cast_string(src->date);
        return E8_VAR_OK;
    }

    // TODO: [E8::Core::Variant] e8_var_cast_string
    *dst = e8_string_ascii("Error");
    return E8_VAR_OK;
}

int e8_var_eq(const e8_var *a, const e8_var *b, e8_var *res)
{
    if (a->type == b->type) {
        if (a->type == varVoid || a->type == varNull || a->type == varUndefined) {
            e8_var_bool(res, true);
            return E8_VAR_OK;
        }
        if (a->type == varNumeric) {
            e8_var_bool(res, e8_numeric_eq(&a->num, &b->num));
            return E8_VAR_OK;
        }
        if (a->type == varString) {
            e8_var_bool(res, e8_string_cmp(a->str, b->str) == 0);
            return E8_VAR_OK;
        }
        if (a->type == varDateTime) {
            e8_var_bool(res, (a->date == b->date));
            return E8_VAR_OK;
        }
        if (a->type == varBool) {
            e8_var_bool(res, (a->bv == b->bv));
            return E8_VAR_OK;
        }
        if (a->type == varEnum) {
            e8_var_bool(res, (a->_enum.md == b->_enum.md && a->_enum.index == b->_enum.index));
            return E8_VAR_OK;
        }
    }
    e8_var_bool(res, false);
    //return E8_VAR_WRONG_CAST;
    return E8_VAR_OK;
}
int e8_var_ne(const e8_var *a, const e8_var *b, e8_var *res)
{
    int r = e8_var_eq(a, b, res);
    if (!OK(r))
        return r;

    r = e8_var_not(res);
    if (!OK(r))
        return r;

    return E8_VAR_OK;
}
int e8_var_lt(const e8_var *a, const e8_var *b, e8_var *res)
{
    if (a->type == b->type) {
        if (a->type == varNumeric) {
            e8_var_bool(res, e8_numeric_lt(&a->num, &b->num));
            return E8_VAR_OK;
        }
        if (a->type == varString) {
            e8_var_bool(res, e8_string_cmp(a->str, b->str) < 0);
            return E8_VAR_OK;
        }
        if (a->type == varDateTime) {
            e8_var_bool(res, (a->date < b->date));
            return E8_VAR_OK;
        }
    }
    return E8_VAR_WRONG_CAST;
}
int e8_var_le(const e8_var *a, const e8_var *b, e8_var *res)
{
    if (a->type == b->type) {
        if (a->type == varNumeric) {
            e8_var_bool(res, e8_numeric_le(&a->num, &b->num));
            return E8_VAR_OK;
        }
        if (a->type == varString) {
            e8_var_bool(res, e8_string_cmp(a->str, b->str) <= 0);
            return E8_VAR_OK;
        }
        if (a->type == varDateTime) {
            e8_var_bool(res, (a->date <= b->date));
            return E8_VAR_OK;
        }
    }
    return E8_VAR_WRONG_CAST;
}
int e8_var_ge(const e8_var *a, const e8_var *b, e8_var *res)
{
    if (a->type == b->type) {
        if (a->type == varNumeric) {
            e8_var_bool(res, e8_numeric_ge(&a->num, &b->num));
            return E8_VAR_OK;
        }
        if (a->type == varString) {
            e8_var_bool(res, e8_string_cmp(a->str, b->str) >= 0);
            return E8_VAR_OK;
        }
        if (a->type == varDateTime) {
            e8_var_bool(res, (a->date >= b->date));
            return E8_VAR_OK;
        }
    }
    return E8_VAR_WRONG_CAST;
}
int e8_var_gt(const e8_var *a, const e8_var *b, e8_var *res)
{
    if (a->type == b->type) {
        if (a->type == varNumeric) {
            e8_var_bool(res, e8_numeric_gt(&a->num, &b->num));
            return E8_VAR_OK;
        }
        if (a->type == varString) {
            e8_var_bool(res, e8_string_cmp(a->str, b->str) > 0);
            return E8_VAR_OK;
        }
        if (a->type == varDateTime) {
            e8_var_bool(res, (a->date > b->date));
            return E8_VAR_OK;
        }
    }
    return E8_VAR_WRONG_CAST;
}

e8_string *e8_numeric_to_string(const e8_numeric *n)
{
    char buf[80];

    if (n->type == numInt)
        sprintf(buf, "%ld", n->l_value);
    else {
        // TODO: [e8core::variant]сделать толковое преобразование числа в строку
        sprintf(buf, "%.6f", n->d_value);
        if (strstr(buf, ".")) {
            // Если число дробное, убираем незначащие нули в конце
            size_t l = strlen(buf) - 1;
            while (buf[l] == '0')
                buf[l--] = 0;
            if (buf[l] == '.')
                buf[l] = 0;
        }
    }

    return e8_string_ascii(buf);
}
e8_var * e8_var_numeric(e8_var *v, const e8_numeric *num)
{
    v->type = varNumeric;
    e8_numeric_assign(&v->num, num);
    return v;
}

e8_var * e8_var_enum(e8_var *r, void *md, e8_enum_index index)
{
    r->type = varEnum;
    r->_enum.md = md;
    r->_enum.index = index;
    return r;
}

e8_var * e8_var_object(e8_var *v, e8_object obj)
{
    v->type = varObject;
    v->obj = obj;

    v->gcl = e8_get_gc_link(obj);
    e8_gcl_use_object(v->gcl);

    return v;
}

bool e8_var_can_buferize(const e8_var *value)
{
    return e8_var_buferize_size(value) != 0;
}

size_t e8_var_buferize_size(const e8_var *value)
{
    return e8_var_buferize(value, 0);
}

int e8_var_buferize(const e8_var *value, void *buf)
{
    int header_size = 1;
    uint8_t *ptr = buf;

    if (ptr)
        *ptr++ = value->type;

    switch (value->type) {

    case varBool: {
            if (ptr)
                *ptr = value->bv;
            return sizeof(bool) + header_size;
    }

    case varDateTime: {
        if (ptr)
            *((e8_date*)ptr) = value->date;
        return sizeof(e8_date) + header_size;
    }

    case varVoid:
    case varNull:
    case varUndefined: return 0 + header_size;
    case varString:
        return 0 + header_size;
    case varNumeric:
        return e8_numeric_buferize(&value->num, ptr) + header_size;
    default: return 0;
    } // switch value->type
}

int e8_var_unbuferize(const void *buf, e8_var *result)
{
    size_t header_size = 0;
    const uint8_t *ptr = buf;
    result->type = *ptr;
    ++ptr;
    switch (result->type) {

    case varBool: {
        result->bv = *ptr;
        return sizeof(bool) + header_size;
    }

    case varDateTime: {
        result->date = *((e8_date*)ptr);
        return sizeof(e8_date) + header_size;
    }

    case varVoid:
    case varNull:
    case varUndefined: return 0 + header_size;
    case varString:
        return 0 + header_size;
    case varNumeric:
        return e8_numeric_unbuferize(ptr, &result->num) + header_size;
    default: return 0;
    }
}

e8_var * e8_var_null(e8_var *res)
{
    res->type = varNull;
    return res;
}

int e8_var_cast_bool(const e8_var *value, bool *result)
{
    if (value->type == varBool) {
        *result = value->bv;
        return 0;
    }
    return 1;
}

int e8_var_or (e8_var *a, const e8_var *b)
{
    int r;
    bool ba, bb;

    if (( r = e8_var_cast_bool(a, &ba) ))
        return r;

    if (( r = e8_var_cast_bool(b, &bb) ))
        return r;

    e8_var_bool(a, ba || bb);
    return 0;
}

int e8_var_and(e8_var *a, const e8_var *b)
{
    int r;
    bool ba, bb;

    if (( r = e8_var_cast_bool(a, &ba) ))
        return r;

    if (( r = e8_var_cast_bool(b, &bb) ))
        return r;

    e8_var_bool(a, ba && bb);
    return 0;
}

int e8_var_xor(e8_var *a, const e8_var *b)
{
    int r;
    bool ba, bb;

    if (( r = e8_var_cast_bool(a, &ba) ))
        return r;

    if (( r = e8_var_cast_bool(b, &bb) ))
        return r;

    e8_var_bool(a, ba ^ bb);
    return 0;
}

int e8_var_cast_long(const e8_var *value, long *result)
{
    if (value->type == varBool) {
        *result = value->bv;
    } else
    if (value->type == varNumeric) {
        if (value->num.type == numInt)
            *result = value->num.l_value;
        else
            *result = value->num.d_value;
    } else
        return 1;

    return 0;
}
