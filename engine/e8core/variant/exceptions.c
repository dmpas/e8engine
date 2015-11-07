#include "variant.h"
#include <malloc.h>
#include <string.h>

#ifndef AALLOC
#define AALLOC(type, count) (type*)malloc(sizeof(type)*(count))
#endif // AALLOC


e8_runtime_error *
e8_throw(int error)
{
    e8_runtime_error *r = AALLOC(e8_runtime_error, 1);
    memset(r, 0, sizeof(*r));
    r->error_no = error;
    return r;
}

e8_runtime_error *e8_throw_data(int error, void *data)
{
    e8_runtime_error *r = e8_throw(error);
    r->data = data;
    return r;
}


void e8_free_exception(e8_runtime_error *err)
{
    if (err) {
        if (err->data)
            free(err->data);
        free(err);
    }
}
