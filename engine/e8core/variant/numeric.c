#include "variant.h"
#include <math.h>
#include <stdint.h>

void e8_numeric_add(e8_numeric *a, const e8_numeric *b)
{
    if (a->type == numInt) {
        if (b->type == numFloat) {
            a->type = numFloat;
            a->d_value = a->l_value + b->d_value;
        } else
            a->l_value = a->l_value + b->l_value;
    } else if (a->type == numFloat) {
        if (b->type == numFloat)
            a->d_value = a->d_value + b->d_value;
        else
            a->d_value = a->d_value + b->l_value;
    }
}
void e8_numeric_sub(e8_numeric *a, const e8_numeric *b)
{
    if (a->type == numInt) {
        if (b->type == numFloat) {
            a->type = numFloat;
            a->d_value = a->l_value - b->d_value;
        } else
            a->l_value = a->l_value - b->l_value;
    } else if (a->type == numFloat) {
        if (b->type == numFloat)
            a->d_value = a->d_value - b->d_value;
        else
            a->d_value = a->d_value - b->l_value;
    }
}
void e8_numeric_mul(e8_numeric *a, const e8_numeric *b)
{
    if (a->type == numInt) {
        if (b->type == numFloat) {
            a->type = numFloat;
            a->d_value = a->l_value * b->d_value;
        } else
            a->l_value = a->l_value * b->l_value;
    } else if (a->type == numFloat) {
        if (b->type == numFloat)
            a->d_value = a->d_value * b->d_value;
        else
            a->d_value = a->d_value * b->l_value;
    }
}
void e8_numeric_div(e8_numeric *a, const e8_numeric *b)
{
    if (a->type == numInt) {
        if (b->type == numFloat) {
            a->type = numFloat;
            a->d_value = (double)a->l_value / b->d_value;
        } else {
            if (a->l_value % b->l_value) {
                a->type = numFloat;
                a->d_value = (double)a->l_value / b->l_value;
            }
            else
                a->l_value = a->l_value / b->l_value;
        }
    } else if (a->type == numFloat) {
        if (b->type == numFloat)
            a->d_value = a->d_value / b->d_value;
        else
            a->d_value = a->d_value / b->l_value;
    }
}
void e8_numeric_trunc(e8_numeric *a)
{
    if (a->type == numFloat) {
        a->type = numInt;
        a->l_value = trunc(a->d_value);
    }
}
void e8_numeric_mod(e8_numeric *a, const e8_numeric *b)
{
    e8_numeric t;
    e8_numeric_assign(&t, a);
    e8_numeric_div(&t, b);
    e8_numeric_trunc(&t);
    e8_numeric_mul(&t, b);
    e8_numeric_sub(a, &t);
}

void e8_numeric_assign(e8_numeric *a, const e8_numeric *b)
{
    a->type = b->type;
    if (a->type == numInt)
        a->l_value = b->l_value;
    else
        a->d_value = b->d_value;
}

long e8_numeric_as_long(const e8_numeric *a)
{
    if (a->type == numFloat) {
        e8_numeric t;
        e8_numeric_assign(&t, a);
        e8_numeric_trunc(&t);
        return t.l_value;
    }
    return a->l_value;
}
double e8_numeric_as_double(const e8_numeric *a)
{
    if (a->type == numFloat)
        return a->d_value;
    return a->l_value;
}

void e8_numeric_long(e8_numeric *a, long value)
{
    a->type = numInt;
    a->l_value = value;
}
void e8_numeric_double(e8_numeric *a, double value)
{
    a->type = numFloat;
    a->d_value = value;
}

inline int e8_cmp(double a, double b)
{
    return a == b ? 0 : (a<b?-1:1);
}
inline int e8_numeric_compare(const e8_numeric *a, const e8_numeric *b)
{
    return e8_cmp(
        (a->type == numInt ? a->l_value : a->d_value),
        (b->type == numInt ? b->l_value : b->d_value)
    );
}

int e8_numeric_eq(const e8_numeric *a, const e8_numeric *b)
{
    return e8_numeric_compare(a, b) == 0;
}
int e8_numeric_ne(const e8_numeric *a, const e8_numeric *b)
{
    return e8_numeric_compare(a, b) != 0;
}
int e8_numeric_lt(const e8_numeric *a, const e8_numeric *b)
{
    return e8_numeric_compare(a, b) < 0;
}
int e8_numeric_le(const e8_numeric *a, const e8_numeric *b)
{
    return e8_numeric_compare(a, b) <= 0;
}
int e8_numeric_ge(const e8_numeric *a, const e8_numeric *b)
{
    return e8_numeric_compare(a, b) >= 0;
}
int e8_numeric_gt(const e8_numeric *a, const e8_numeric *b)
{
    return e8_numeric_compare(a, b) > 0;
}

void e8_numeric_neg(e8_numeric *a)
{
    if (a->type == numInt)
        a->l_value = -a->l_value;
    else
        a->d_value = -a->d_value;
}

int e8_numeric_buferize_size(const e8_numeric *value)
{
    return e8_numeric_buferize(value, 0);
}

int e8_numeric_buferize(const e8_numeric *value, void *buf)
{
    int header_size = 1;
    uint8_t *ptr = buf;
    if (ptr)
        *ptr++ = value->type;
    switch (value->type) {
    case numInt:
        {
            *((e8_integer*)ptr) = value->l_value;
            return sizeof(e8_integer) + header_size;
        }
    case numFloat:
        {
            *((e8_float*)ptr) = value->d_value;
            return sizeof(e8_float) + header_size;
        }
    default:
        return 0 + header_size;
    }
}

int e8_numeric_unbuferize(const void *buf, e8_numeric *result)
{
    int header_size = 1;
    const uint8_t *ptr = buf;
    result->type = *ptr++;
    switch (result->type) {
    case numInt:
        {
            result->l_value = *((e8_integer*)ptr);
            return sizeof(e8_integer) + header_size;
        }
    case numFloat:
        {
            result->d_value = *((e8_float*)ptr);
            return sizeof(e8_float) + header_size;
        }
    }
    return 0;
}
