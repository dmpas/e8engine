#include "variant.h"
#include <malloc.h>

typedef struct __e8_collection_template __tmpl;
typedef struct __e8_collection_iterator_template __tmpl_it;

static int capacity_step = 10;


void e8_collection_set_default_capacity_step(int step)
{
    capacity_step = step;
}

static e8_runtime_error *
e8_collection_realloc(__tmpl *col)
{
    if (col->reserved >= col->size)
        E8_RETURN_OK;

    while (col->reserved < col->size)
        col->reserved += col->capacity_step;

    size_t new_size = col->element_size * col->reserved;

    if (col->data)
        col->data = realloc(col->data, new_size);
    else
        col->data = malloc(new_size);

    if (col->data)
        E8_RETURN_OK;

    E8_THROW(E8_RT_ALLOCATION_ERROR);
}

e8_runtime_error *e8_collection_init(void *collection, int element_size)
{
    __tmpl *d = (__tmpl *)collection;
    d->data = 0;
    d->capacity_step = capacity_step;
    d->size = 0;
    d->reserved = 0;
    d->element_size = element_size;

    E8_RETURN_OK;
}

e8_runtime_error *e8_collection_init_size(void *collection, int element_size, int size)
{
    e8_collection_init(collection, element_size);
    return e8_collection_resize(collection, size);
}

e8_runtime_error *e8_collection_resize(void *collection, int new_size)
{
    __tmpl *d = (__tmpl *)collection;
    d->size = new_size;
    return e8_collection_realloc(d);
}

e8_runtime_error *e8_collection_add_size(void *collection, int delta)
{
    __tmpl *d = (__tmpl *)collection;
    d->size += delta;
    return e8_collection_realloc(d);
}

e8_runtime_error *e8_collection_insert(void *collection, int index)
{
    e8_runtime_error *__err = e8_collection_add_size(collection, 1);
    if (__err)
        return __err;

    __tmpl *d = (__tmpl *)collection;
    size_t tomove = (d->size - index - 1) * d->element_size;

    if (tomove) {

        char *src = (char *)d->data;

        src += index * d->element_size;
        char *dst = src + d->element_size;

        memmove(dst, src, tomove);
    }

    E8_RETURN_OK;
}

void e8_collection_free(void *collection)
{
    __tmpl *d = (__tmpl *)collection;
    if (d->data) {

        free(d->data);

        d->data = 0;
        d->size = 0;
        d->reserved = 0;
    }
}

bool e8_iterator_has_next(void *iterator)
{
    __tmpl_it *it = (__tmpl_it *)iterator;
    return it->current < it->ref->size;
}

void e8_iterator_move_next(void *iterator)
{
    __tmpl_it *it = (__tmpl_it *)iterator;
    if (it->current < it->ref->size)
        ++it->current;
}

void *e8_iterator_current(void *iterator)
{
    __tmpl_it *it = (__tmpl_it *)iterator;
    if (it->current < it->ref->size) {
        char *cv = (char *)it->ref;
        cv += it->ref->element_size * it->current;
        return cv;
    }
    return 0;
}
void *e8_iterator_next(void *iterator)
{
    __tmpl_it *it = (__tmpl_it *)iterator;
    if (it->current < it->ref->size) {
        ++it->current;
        char *cv = (char *)it->ref;
        cv += it->ref->element_size * it->current;
        return cv;
    }
    return 0;
}

typedef struct __e8_clustered_list_template __clist;

e8_runtime_error *e8_clustered_list_init(void *list, int element_size)
{
    __clist *c = (__clist *)list;

    void *data = malloc(element_size * capacity_step);

    if (!data)
        E8_THROW(E8_RT_ALLOCATION_ERROR);

    c->reserved = capacity_step;
    c->element_size = element_size;
    c->size = 0;
    c->next = NULL;
    c->tail = list;
    c->data = data;

    E8_RETURN_OK;
}

e8_runtime_error *e8_clustered_list_append(void *list, void **address)
{
    __clist *c = (__clist *)list;
    __clist *t = (__clist *)c->tail;

    if (t->size == t->reserved) {
        /* Заполнили хвост - надо выделять новый кусок памяти */
        __clist *new_element = malloc(sizeof(*new_element));

        e8_runtime_error *__err = e8_clustered_list_init(new_element, c->element_size);

        if ( __err )
            return __err;

        new_element->tail = NULL;
        t->next = new_element;
        t = t->next;
        c->tail = t;
    }

    int i = t->size++;
    *address = &((char*)t->data)[i * t->element_size];

    E8_RETURN_OK;
}

e8_runtime_error *e8_clustered_list_dump(const void *list, void *collection)
{
    int size = e8_clustered_list_size(list);
    e8_runtime_error *__err;

    const __clist *c = (const __clist *)list;

    if (( __err = e8_collection_init_size(collection, c->element_size, size) ))
        return __err;

    int index = 0;

    __tmpl *d = (__tmpl *)collection;
    char *dst = (char *)d->data;
    while (c) {
        int data_size = c->element_size * c->size;
        memcpy(dst, c->data, data_size);
        index += c->size;
        dst += data_size;
        c = c->next;
    }

    E8_RETURN_OK;
}

void e8_clustered_list_free(void *list)
{
    __clist *c = (__clist *)list;

    free(c->data);
    c->reserved = 0;
    c->size = 0;
    c->tail = 0;

    __clist *n = (__clist *)c->next;
    c->next = NULL;
    c = n;

    while (c) {
        n = c->next;

        free(c->data);
        c->reserved = 0;
        c->size = 0;

        free(c);

        c = n;
    }
}

int e8_clustered_list_size(const void *list)
{
    __clist *c = (__clist *)list;
    int r = c->size;

    while (c->next) {
        c = c->next;
        r += c->size;
    }
    return r;
}
