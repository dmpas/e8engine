#include "e8core/plugin/plugin.h"
#include "e8core/variant/variant.h"
#include "e8core/utf/utf.h"
#include <malloc.h>
#include <zip.h>

// TODO: [JustZip] переписать плагин под новую модель мусорщика

static e8_vtable vmt_ZipFileWriter;

typedef struct _ZipFileWriter_object {
    struct zip *z;
} ZipFileWriter;

#define _ZIP(name) ZipFileWriter *name = (ZipFileWriter *)obj;

E8_DECLARE_SUB(ZipFileWriter_Add)
{
    _ZIP(z);
    e8_var_object(result, z); /* return This */

    E8_ARGUMENT(v_filename);
    e8_string s_filename;
    e8_var_cast_string(&v_filename, &s_filename);
    e8_var_destroy(&v_filename);

    char *fname = 0;
    e8_utf_to_8bit(s_filename.s, &fname, "cp1251");

    struct zip_source *src = zip_source_file(z->z, fname, 0, 0);

    if (!src) {
        E8_THROW_DETAIL(E8_RT_FILE_READ_ERROR, fname);
    } else {
        zip_file_add(z->z, fname, src, 0);
    }

    //zip_source_free(src);
    free(fname);
    E8_RETURN_OK;
}
E8_DECLARE_SUB(ZipFileWriter_Write)
{
    _ZIP(z);
    if (!z->z) {
        E8_THROW_DETAIL(E8_RT_EXCEPTION_RAISED, "Zip not opened!");
    }

    e8_var_object(result, z); /* return This */

    zip_close(z->z);
    E8_RETURN_OK;
}
E8_DECLARE_SUB(ZipFileWriter_Open)
{
    _ZIP(z);

    E8_ARGUMENT(v_filename);
    e8_string s_filename;
    e8_var_cast_string(&v_filename, &s_filename);
    e8_var_destroy(&v_filename);

    char *fname = 0;
    e8_utf_to_8bit(s_filename.s, &fname, "cp1251");

    int err;
    z->z = zip_open(fname, ZIP_CREATE | ZIP_TRUNCATE, &err);

    if (!z->z) {
        E8_THROW_DETAIL(E8_RT_FILE_READ_ERROR, fname);
    }

    e8_var_object(result, z); /* return This */

    free(fname);
    E8_RETURN_OK;
}

E8_METHOD_LIST_START(ZipFileWriter_methods)
    E8_METHOD_LIST_ADD("Add",   "Добавить", ZipFileWriter_Add)
    E8_METHOD_LIST_ADD("Open",  "Открыть",  ZipFileWriter_Open)
    E8_METHOD_LIST_ADD("Write", "Записать", ZipFileWriter_Write)
E8_METHOD_LIST_END

E8_DECLARE_GET_METHOD(ZipFileWriter_GetMethod)
{
    e8_simple_get_method(name, ZipFileWriter_methods, fn);
    E8_RETURN_OK;
}

E8_DECLARE_SUB(ZipFileWriter_Constructor)
{
    ZipFileWriter *z = malloc(sizeof(ZipFileWriter));
    z->z = 0;

    e8_gc_register_object(z, &vmt_ZipFileWriter);
    e8_var_object(result, z);

    if (argc) {
        e8_var R;
        return ZipFileWriter_Open(z, user_data, argc, argv, &R);
    }

    E8_RETURN_OK;
}

static const e8_uchar NAME[] = {'Z', 'i', 'p', 'F', 'i', 'l', 'e', 'W', 'r', 'i', 't', 'e', 'r', 0};
E8_DECLARE_TO_STRING(ZipFileWriter_ToString)
{
    return NAME;
}

static e8_type_info writer_class = {ZipFileWriter_ToString, ZipFileWriter_Constructor, 0};

E8_PLUGIN_MAIN
{
    e8_vtable_template(&vmt_ZipFileWriter);
    vmt_ZipFileWriter.get_method = ZipFileWriter_GetMethod;
    vmt_ZipFileWriter.to_string = ZipFileWriter_ToString;

    e8_prepare_call_elements(ZipFileWriter_methods, "utf-8");

    env->vmt->register_type(env, "ZipFileWriter", &writer_class);
    env->vmt->register_type(env, "ПисательZip", &writer_class);

    E8_RETURN_OK;
}

