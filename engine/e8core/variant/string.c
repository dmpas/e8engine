#include "variant.h"
#include <malloc.h>
#include <string.h>
#include <errno.h>

static void __e8_string_check_alloc(e8_string *s)
{
    if (!s->reserved)
        s->s = 0;

    if (s->len >= s->reserved) {

        while (s->len >= s->reserved)
            s->reserved += E8_STRING_ALLOCATION_ALIGN;

        if (s->s) {
            /*  realloc используется только внутри системных вызовов при создании
                новой строки, потому спокойно используем приведение const_cast
             */
            s->s = e8_utf_realloc((e8_uchar *)s->s, s->reserved);
        } else
            s->s = e8_utf_malloc(s->reserved);
    }
}

static void __init_mutex(e8_string *s)
{
    #ifndef E8_NO_THREADS
    pthread_mutex_init(&s->mutex, NULL);
    #endif // E8_NO_THREADS

}

static void __lock(e8_string *s)
{
    #ifndef E8_NO_THREADS
    int r = pthread_mutex_lock(&s->mutex);
    if (r == EINVAL) {
        __init_mutex(s);
        pthread_mutex_lock(&s->mutex);
    }
    #endif // E8_NO_THREADS
}

static void __unlock(e8_string *s)
{
    #ifndef E8_NO_THREADS
    pthread_mutex_unlock(&s->mutex);
    #endif // E8_NO_THREADS
}

e8_string *e8_string_copy(const e8_string *src)
{
    e8_string *dst = malloc(sizeof(*dst));

    dst->len = src->len;
    dst->reserved = dst->len + 1;
    dst->count = 1;
    dst->s = e8_utf_strdup(src->s);

    __init_mutex(dst);

    return dst;
}
e8_string *e8_string_concat(const e8_string *a, const e8_string *b)
{
    e8_string *res = e8_string_copy(a);
    res->len += b->len;
    __e8_string_check_alloc(res);
    e8_utf_strcat((e8_uchar *)res->s, b->s);
    return res;
}
int e8_string_cmp(const e8_string *a, const e8_string *b)
{
    return e8_utf_strcmp(a->s, b->s);
}

e8_string *e8_string_ascii(const char *ascii)
{
    e8_string *str = malloc(sizeof(*str));

    str->len = strlen(ascii);
    str->reserved = 0;
    str->count = 1;

    __init_mutex(str);

    __e8_string_check_alloc(str);

    const char *c = ascii;
    e8_uchar *u = (e8_uchar *)str->s;
    while (*c) {
        *u = *c & 0x7F;
        ++u;
        ++c;
    }
    *u = 0;

    return str;
}


int e8_string_find(const e8_string *str, const e8_string *substr, int from)
{
    if (from < 0)
        return -1;

    while (from + substr->len < str->len) {
        if (e8_utf_strncmp(str->s + from, substr->s, substr->len) == 0)
            return from;
        ++from;
    }
    return -1;
}

static void __e8_char_move(e8_uchar *buf, int count, int delta)
{
    if (delta == 0)
        return;

    if (delta > 0) {
        while (count--)
            buf[count + delta] = buf[count];
    } else {
        int i;
        for (i = 0; i < count + delta; ++i)
            buf[i] = buf[i - delta];
    }
}

e8_string *e8_string_replace(const e8_string *src, const e8_string *find, const e8_string *replace, int from)
{
    e8_string str;

    e8_uchar *res = e8_utf_strdup(src->s);
    int len = src->len;
    str.len = src->len;
    str.reserved = str.len;
    str.s = res;

    if (from < 0) {
        e8_utf_free(res);
        return e8_string_copy(src);
    }

    int d = replace->len - find->len;

    while (1) {

        int index = e8_string_find(&str, find, from);
        if (index < 0)
            break;

        int count_r = len - index;
        len += d;
        if (d > 0)
            __e8_string_check_alloc(&str);

        __e8_char_move(res + index, count_r, d);

        e8_utf_strncpy(res + index, replace->s, replace->len);

        from = index + replace->len;

    } /* while (true) */
    res[len] = 0;

    e8_string *r = e8_string_init(res);
    e8_utf_free(res);
    return r;
}

e8_string *e8_string_init(const e8_uchar *src)
{
    e8_string *res = malloc(sizeof(*res));

    res->len = e8_utf_strlen(src);
    res->reserved = res->len + 1;
    res->s = e8_utf_strdup(src);
    res->count = 1;

    __init_mutex(res);

    return res;
}

e8_string *e8_string_empty()
{
    e8_string *s = malloc(sizeof(*s));

    s->len = 0;
    s->reserved = E8_STRING_ALLOCATION_ALIGN;
    s->s = e8_utf_malloc(s->reserved);
    s->count = 1;

    __init_mutex(s);

    return s;
}
void e8_string_destroy(e8_string *s)
{
    if (!s)
        return;

    __lock(s);

    if (!s->reserved) {
        __unlock(s);
        return;
    }

    if (!s->count) {
        __unlock(s);
        return;
    }

    #ifdef DEBUG
    {
        int i;
        for (i = 0; i < s->reserved; ++i)
            s->s[i] = 0;
    }
    #endif // DEBUG

    if (--s->count) {
        __unlock(s);
        return;
    }

    e8_utf_free((void *)s->s); /* const_cast */
    s->len = 0;
    s->reserved = 0;
    s->s = 0;

    __unlock(s);

    #ifndef E8_NO_THREADS
    pthread_mutex_destroy(&s->mutex);
    #endif // E8_NO_THREADS

    free(s);
}

static inline bool is_ws(e8_uchar c)
{
    return c <= 32;
}
e8_uchar *e8_utf_trim(const e8_uchar *src, bool left, bool right)
{
    e8_uchar *r = (e8_uchar *)src;
    if (left) {
        while (*r && is_ws(*r))
            ++r;
    }
    r = e8_utf_strdup(r);
    if (right) {
        int len = e8_utf_strlen(r);
        while (len && is_ws(r[len - 1]))
            r[--len] = 0;
    }
    return r;
}

e8_string *e8_string_trim(const e8_string *s, bool left, bool right)
{
    return e8_string_init(e8_utf_trim(s->s, left, right));
}
