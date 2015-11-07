#include "variant.h"
#include <malloc.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#include <errno.h>

#ifndef E8_NO_THREADS

#define INIT_POOL_MUTEX pthread_mutex_init(&mutex_pool, NULL);
#define LOCK_POOL_MUTEX pthread_mutex_lock(&mutex_pool);
#define UNLOCK_POOL_MUTEX pthread_mutex_unlock(&mutex_pool);

#else

#define INIT_POOL_MUTEX
#define LOCK_POOL_MUTEX
#define UNLOCK_POOL_MUTEX

#endif


#define E8_GC_CHECK_CLEAR \
    /* При первом обращении обнуляем хранилище*/ \
    if (need_to_clear) { \
        e8_clear_pool(); \
        INIT_POOL_MUTEX\
        need_to_clear = false; \
    }


#define GC_POOL_SIZE 256
static /* volatile */gc_list_element pool[GC_POOL_SIZE];
static volatile bool need_to_clear = true;

#ifndef E8_NO_THREADS
static pthread_mutex_t mutex_pool;
#endif


static void __init_mutex(gc_list_element *gcl)
{
    #ifndef E8_NO_THREADS
    pthread_mutex_init(&gcl->mutex, NULL);
    #endif // E8_NO_THREADS
}


static void __lock(gc_list_element *gcl)
{
    #ifndef E8_NO_THREADS
    int r = pthread_mutex_lock(&gcl->mutex);
    if (r == EINVAL) {
        __init_mutex(gcl);
        r = pthread_mutex_lock(&gcl->mutex);
        if (r == EINVAL)
            return;
    }
    #endif // E8_NO_THREADS
}

static void __unlock(gc_list_element *gcl)
{
    #ifndef E8_NO_THREADS
    pthread_mutex_unlock(&gcl->mutex);
    #endif // E8_NO_THREADS
}

static void __destroy_mutex(gc_list_element *gcl)
{
    #ifndef E8_NO_THREADS
    pthread_mutex_destroy(&gcl->mutex);
    #endif // E8_NO_THREADS
}

static inline
void e8_clear_pool()
{
    memset(&pool, 0, sizeof(pool));
}

static unsigned int
e8_hash_object(const void *obj)
{
    unsigned R = 0;
    int K = sizeof(obj);
    unsigned char *a = (unsigned char *)&obj;
    while (K--)
        R ^= *a++;
    return R % GC_POOL_SIZE;
}

static
gc_list_element *e8_ref_register_in_list(const void *obj, e8_vtable *vmt, unsigned K)
{
    LOCK_POOL_MUTEX;

    gc_list_element *el = &pool[K];
    while (el->obj != obj) {
        if (!el->next) {
            // не нашли объект в списке - добавляем
            el->next = malloc(sizeof(gc_list_element));
            el->next->prev = el;
            el = el->next;

            el->next = 0;
            el->vmt = vmt;
            el->obj = (void*)obj;
            el->count = 0;

            __init_mutex(el);

            break;
        } else
            el = el->next;
    }

    UNLOCK_POOL_MUTEX;

    return el;
}

static
int e8_gc_inc_use(gc_list_element *ref)
{
    int r = ++ref->count;
    return r;
}

static
int e8_gc_dec_use(gc_list_element *ref)
{
    int r = --ref->count;
    return r;
}

static
gc_list_element *e8_find_in_list(const void *obj)
{
    E8_GC_CHECK_CLEAR;

    LOCK_POOL_MUTEX;

    unsigned K = e8_hash_object(obj);
    gc_list_element *el = &pool[K];
    while (el->obj != obj) {
        if (!el->next) {
            el = NULL;
            break;
        }
        el = el->next;
    }

    UNLOCK_POOL_MUTEX;

    return el;
}

void e8_gc_register_object(const void *obj, e8_vtable *vmt)
{
    E8_GC_CHECK_CLEAR;
    e8_ref_register_in_list(obj, vmt, e8_hash_object(obj));
}

void e8_gcl_use_object(gc_list_element *gcl)
{

    if (gcl) {
        __lock(gcl);
        e8_gc_inc_use(gcl);
        __unlock(gcl);
    }

}
void e8_gcl_free_object(gc_list_element *gcl)
{
    if (!gcl)
        return;

    __lock(gcl);

    if (!gcl->obj) {
        /* TODO: Разбираться с такими случаями */
        __unlock(gcl);
        return;
    }

    E8_GC_CHECK_CLEAR;

    if (!e8_gc_dec_use(gcl)) {
        e8_vtable *vmt = gcl->vmt;

        if (vmt)
            if (vmt->dtor)
                vmt->dtor(gcl->obj);

        gcl->obj = 0;

        __unlock(gcl);
        __destroy_mutex(gcl);

    } else
        __unlock(gcl);

}


void e8_gc_use_object(const void *obj)
{
    gc_list_element *r = e8_find_in_list(obj);
    e8_gcl_use_object(r);
}

void e8_gc_destroy_object(void *obj)
{
    gc_list_element *el = e8_find_in_list(obj);

    if (!el)
        return;

    e8_gcl_free_object(el);
}

void e8_gc_free_object(const void *obj)
{
    gc_list_element *r = e8_find_in_list(obj);

    if (!r)
        return; /* TODO: надо отрабатывать эти случаи! */

    e8_gcl_free_object(r);
}

e8_vtable *e8_get_vtable(const e8_object obj)
{
    gc_list_element *el = e8_find_in_list(obj);
    if (el)
        return el->vmt;
    return 0;
}

void e8_gc_done()
{

    LOCK_POOL_MUTEX;

    int i;
    for (i = 0; i < GC_POOL_SIZE; ++i) {
        gc_list_element *el = &pool[i], *t;

        if (!el->next)
            continue;

        while (el->next) {

            t = el->next;
            el->next = t->next;
            if (el->next)
                el->next->prev = el;

            free((void*)t);
        }
    } /* for i */

    UNLOCK_POOL_MUTEX;
}

void e8_gc_free_all()
{
    LOCK_POOL_MUTEX ;

    int i;
    for (i = 0; i < GC_POOL_SIZE; ++i) {
        gc_list_element *el = &pool[i], *t;

        if (!el->next)
            continue;

        el = el->next;

        while (el) {

            if (el->count && el->vmt && el->obj)
                el->vmt->dtor(el->obj);

            t = el;
            el = el->next;

            free((void*)t);
        }
    } /* for i */

    UNLOCK_POOL_MUTEX ;
}
void *e8_get_gc_link(const e8_object obj)
{
    return (void*)e8_find_in_list(obj);
}

#undef GC_POOL_SIZE

#undef INIT_POOL_MUTEX
#undef LOCK_POOL_MUTEX
#undef UNLOCK_POOL_MUTEX
