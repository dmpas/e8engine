#ifndef E8_CORE_VARIANT_PLUGIN_H
#define E8_CORE_VARIANT_PLUGIN_H

#include "e8core/variant/variant.h"
#include <stdarg.h>

#ifdef __cplusplus
    #define E8_CALLING_NO_THROW throw()
#else
    #define E8_CALLING_NO_THROW
#endif // __cplusplus

#define E8_DECLARE_SUB(name) \
e8_runtime_error *\
name (void *obj, const void *user_data, int argc, e8_property *argv, e8_var *result) \
E8_CALLING_NO_THROW

#define E8_ARGUMENT(name) e8_var name; \
    e8_var_undefined(&name); \
    argv->get(argv, &name); \
    ++argv

#define E8_ARG_STRING(name) e8_string *name; \
    { \
        E8_ARGUMENT(__v_##name); \
        e8_var_cast_string(&__v_##name, &name); \
        e8_var_destroy(&__v_##name); \
    }

#define E8_ARG_UTF_STRING(name) char *name = 0; \
    { \
        E8_ARGUMENT(__v_##name); \
        e8_string *__s_##name; \
        e8_var_cast_string(&__v_##name, &__s_##name);\
        \
        e8_utf_to_8bit(__s_##name ->s, &name, "utf-8"); \
        \
        e8_string_destroy( __s_##name ); \
        e8_var_destroy(& __v_##name ); \
        \
    }

#define E8_ARG_BOOL(name) bool name; \
    { \
        E8_ARGUMENT(__v_##name); \
        e8_var_cast_bool(&__v_##name, &name); \
        e8_var_destroy(&__v_##name); \
    }

#define E8_ARG_DOUBLE(name) double name; \
    { \
        E8_ARGUMENT(v_##name); \
        name = e8_numeric_as_double(&v_##name.num); \
    }

#define E8_ARG_LONG(name) long name;\
    { \
        E8_ARGUMENT(v_##name); \
        name = e8_numeric_as_long(&v_##name.num); \
    }

#define E8_VAR(name) e8_var name; e8_var_undefined(&name)


#define E8_REGISTER_CLASS(Class) env->vmt->register_type(env, E8_TO_STRING(Class)(0), &Info_##Class)
#define E8_GET(x, v) (x).get(&(x), v)
#define E8_DECLARE_GET_PROPERTY(fname) e8_runtime_error* fname(void *obj, const e8_uchar* name, e8_property *property)
#define E8_DECLARE_TO_STRING(name) const e8_uchar* name(const e8_object obj)

#define E8_DECLARE_GET_BY_INDEX(name) e8_runtime_error* name(e8_object obj, const e8_var *index, e8_property *property)

#define E8_DECLARE_PROPERTY_GET(__name) \
e8_runtime_error* __name(void *property, e8_var *value)

#define E8_DECLARE_PROPERTY_SET(__name) \
e8_runtime_error* __name(void *property, const e8_var *value)

/* Макросы определения типов */
#define E8_PLUGIN_REGISTER_TYPE(__name, __vmt) env->vmt->register_type(env, __name, __vmt)
#define E8_WITH_TYPE(__vmt_name) { e8_type_info *vmt = &__vmt_name;
#define E8_REGISTER_ALIAS(__type_name) env->vmt->register_type(env, __type_name, vmt);
#define E8_END_WITH }

#define E8_DECLARE_TYPE(__name) e8_type_info __name = {

#ifdef __cplusplus

#define E8_SET_TO_STRING(__name) __name,
#define E8_SET_CONSTRUCTOR(__name) __name,

#else

#define E8_SET_TO_STRING(__name) .to_string = __name,
#define E8_SET_CONSTRUCTOR(__name) .constructor = __name,

#endif // __cplusplus

#define E8_END_TYPE };



#define E8_DECLARE_GET_METHOD(sub_name) \
e8_runtime_error * sub_name (e8_object obj, const e8_uchar *name, e8_function_signature *fn, void **user_data)

#define E8_DECLARE_DESTRUCTOR(sub_name) \
void sub_name(e8_object obj)

#define E8_PLUGIN_MAIN int e8_entry(e8_env *env)

#ifdef __cplusplus
extern "C" {
#endif

// функция такого вида подаётся на вход модулю
typedef int  (*e8_register_type_method)  (e8_uchar *name, e8_type_info *type);

// сигнатура функции-регистратора типов
typedef void (*e8_plugin_type_signature) (e8_register_type_method M);

typedef int (*e8_register_global_property) (/* e8_env * */void *env, const char* utf_name, e8_property *property);
typedef int (*e8_register_global_function) (/* e8_env * */void *env, const char* utf_name, e8_function_signature fn);
typedef int (*e8_register_global_type)     (/* e8_env * */void *env, const char* utf_name, e8_type_info *type);
typedef e8_function_signature (*e8_get_constructor_t) (/* e8_env * */void *env, const char* utf_name);

typedef struct _e8_env_vtable {
    e8_register_global_function         register_function;
    e8_register_global_property         register_property;
    e8_register_global_type             register_type;
    e8_get_constructor_t                get_ctor;
} e8_env_vtable;

typedef struct _e8_env_struct {
    e8_env_vtable          *vmt;
    void                   *data;
} e8_env;

/*! Сигнатура функции, вызываемой в подключаемом модуле */
typedef int (*e8_plugin_entry) (e8_env *env);

typedef struct __e8_call_element {
    char                        a_rus[128], a_eng[128];
    e8_uchar                    u_rus[128], u_eng[128];
    e8_function_signature       fn;
} e8_call_element;

#define E8_METHOD_LIST_START(name) e8_call_element name[] = {
#define E8_METHOD_LIST_ADD(eng, rus, fn) {eng, rus, {}, {}, fn},
#define E8_METHOD_LIST_END  {{}, {}, {}, {}, 0}};

#define E8_METHOD_LIST_ADD_HAS_NEXT(fn) {"__HasNext", "__ЕстьСледующий", {}, {}, fn},
#define E8_METHOD_LIST_ADD_NEXT(fn) {"__Next", "__Следующий", {}, {}, fn},
#define E8_METHOD_LIST_ADD_ITERATOR(fn) {"__Iterator", "__Итератор", {}, {}, fn},

#define E8_VAR_READ_WRITE true, true
#define E8_VAR_READ_ONLY  true, false
#define E8_VAR_WRITE_ONLY false, true
typedef struct __e8_property_element {
    char                        a_rus[64], a_eng[64];
    e8_uchar                    u_rus[64], u_eng[64];
    int                         index;
} e8_property_element;

void e8_prepare_call_elements(e8_call_element *table, const char *encoding);

void e8_simple_get_method(const e8_uchar *name, const e8_call_element *table, e8_function_signature *fn);

void e8_prepare_property_elements(e8_property_element *table, const char *encoding);

int e8_find_simple_property(const e8_uchar *name, const e8_property_element *table);

/*! Создать список свойств по списку e8_var* */
e8_property *e8_arg_list(int argc, ...);

/*! Создать список свойств по списку e8_var* . Версия va_list */
e8_property *e8_arg_vlist(int argc, va_list list);

/*! Вызов функции с простой передачей списка значений e8_var */
e8_runtime_error *e8_simple_call(
                            const e8_function_signature         fn,
                            void                               *obj,
                            e8_var                             *result,
                            int                                 argc,
                            ...                                 /* e8_var[] */
                        );

/*! Вызов функции с простой передачей списка e8_var. Версия va_list */
e8_runtime_error *e8_simple_vcall(
                            const e8_function_signature         fn,
                            void                               *obj,
                            e8_var                             *result,
                            int                                 argc,
                            va_list                             argv
                        );

/*! Вызов функции с простой передачей массива e8_var */
e8_runtime_error *e8_simple_pcall(
                            const e8_function_signature         fn,
                            void                               *obj,
                            e8_var                             *result,
                            int                                 argc,
                            e8_var                             *argv
                        );

/*! Вызов функции по UTF-8-имени с простой передачей списка e8_var */
e8_runtime_error *e8_simple_call_utf(
                            const char                         *fn_name,
                            void                               *obj,
                            e8_var                             *result,
                            int                                 argc,
                            //...                                 /* e8_var[] */
                            e8_var                             *argv
                        );

/*! Получает свойство объекта по имени UTF-8 */
e8_var *e8_get_property_value(const e8_var *object, const char *utf_name, e8_var *result);

#ifdef __cplusplus
}
#endif


#endif // E8_CORE_VARIANT_PLUGIN_H
