#ifndef E8_CORE_VARIANT_H
#define E8_CORE_VARIANT_H

#ifdef __cplusplus
extern "C" {
#else

#include <stdbool.h>

#endif

#include <stdint.h>
#include <pthread.h>
#include "e8core/utf/utf.h"


#define E8_RT_NO_ERROR                  0

/*! Внутренняя ошибка */
#define E8_RT_ERROR_INTERNAL_FAIL       1

/*! Исключеніе в кодѣ */
#define E8_RT_EXCEPTION_RAISED          2

/*! Нехватка памяти */
#define E8_RT_ALLOCATION_ERROR          3

/*! Переполненіе стёка */
#define E8_RT_ERROR_STACK_OVERFLOW     20
/*! Чтеніе пустого стёка */
#define E8_RT_ERROR_STACK_EMPTY        21

/*! Невѣрно заданы параметры для вызова */
#define E8_RT_UNDEFINED_CALL_PARAMS    40
/*! Значеніе не может быть прочитано */
#define E8_RT_VALUE_CANNOT_BE_READ     41
/*! Значеніе не может быть записано */
#define E8_RT_VALUE_CANNOT_BE_WRITTEN  42

/*! Значеніе не может быть приведено к булевому типу */
#define E8_RT_CANNOT_CAST_TO_BOOL      43
/*! Значеніе не может быть приведено к числовому типу */
#define E8_RT_CANNOT_CAST_TO_NUMERIC   44

/*! Голова стёка не является ссылкой */
#define E8_RT_STACK_NOT_A_REFERENCE    60

/*! Сущность не является набором (ядро не может получить бегунок) */
#define E8_RT_NOT_A_COLLECTION         62

/*! Метод сущности не найден */
#define E8_RT_METHOD_NOT_FOUND         63

/*! Свойство сущности не найдено */
#define E8_RT_PROPERTY_NOT_FOUND       64

/*! Для сущности не опредѣлено полученіе значенія по индеѯу */
#define E8_RT_UNDEFINED_GET_BY_INDEX   65

/*! Индеѯ выходит за допустимые предѣлы */
#define E8_RT_INDEX_OUT_OF_BOUNDS      66

/*! Тип не найден */
#define E8_RT_TYPE_NOT_FOUND           67

/*! Сущность не объектнаго типа */
#define E8_RT_NOT_AN_OBJECT            68

/*! Утвержденіе не истинно */
#define E8_RT_ASSERTION_FAILED         80

/*! Ошибка в исходном теѯтѣ */
#define E8_RT_SYNTAX_ERROR             90

/*! Ошибка чтенія файла */
#define E8_RT_FILE_READ_ERROR          91


typedef struct __e8_runtime {
    int         error_no;           /*!< Номер ошибки */
    int         command_index;      /*!< Индеѯ указанія */
    void       *data;               /*!< Подробныя данные об ошибкѣ */
} e8_runtime_error;

#define E8_RETURN_OK return 0

/*! Создаёт новое исключеніе */
e8_runtime_error *e8_throw(int error);

/*! Создаёт новое исключеніе с данными */
e8_runtime_error *e8_throw_data(int error, void *data);

/*! Освобождает гумно, связанное с исключеніем */
void e8_free_exception(e8_runtime_error *err);

#define E8_THROW(num) return e8_throw(num)
#define E8_THROW_DETAIL(num, data) return e8_throw_data(num, (void*)data)


/*! Дата - количество секунд, прошедших с полуночи 1 января 1 года */
typedef uint64_t e8_date;

/*! Прибавить секунды к датѣ */
void e8_date_add            (e8_date *date, long delta);

/*! Отнять секунды от даты */
void e8_date_sub_long       (e8_date *date, long delta);

/*! Получить разность дат в секундах */
long e8_date_sub_date       (e8_date a, e8_date b);

/*! Извлечь мѣсяц из даты */
int e8_date_get_month       (e8_date m_data);

/*! Извлечь год из даты */
int e8_date_get_year        (e8_date m_data);

/*! Извлечь день мѣсяца из даты */
int e8_date_get_day         (e8_date m_data);

/*! Извлечь час из даты */
int e8_date_get_hour        (e8_date m_data);

/*! Извлечь минуту из даты */
int e8_date_get_min         (e8_date m_data);

/*! Извлечь секунду из даты */
int e8_date_get_sec         (e8_date m_data);

/*! Получить дату из составляющих:
    Y  - 1-based
    M  - 1-based
    D  - 1-based
    hh - 0-based
    mm - 0-based
    ss - 0-based
*/
int e8_date_construct       (int Y, int M, int D, int hh, int mm, int ss, e8_date *d);

/*! Получить дату из строки ггггММдд[ЧЧммсс]*/
int e8_date_from_string     (const char *s, e8_date *d);


#define E8_STRING_ALLOCATION_ALIGN  16
typedef struct _e8_string {
    const e8_uchar     *s;          /*!< Указатель на строку */
    int                 len;        /*!< Длина строки */
    int                 reserved;   /*!< Количество символов, под которое зарезервирована память*/
    volatile int        count;      /*!< Счётчик ссылок для строки */

    pthread_mutex_t     mutex;

} e8_string;

/*! Создать новую пустую строку */
e8_string*
e8_string_empty();

/*! Уничтожить строку */
void
e8_string_destroy
            (
                        e8_string          *s
             );

/*! Слиять двѣ строки */
e8_string *
e8_string_concat
            (
                        const e8_string    *a,
                        const e8_string    *b
             );

/*! Создать копію строки */
e8_string *
e8_string_copy
            (
                        const e8_string    *src
             );

/*! Сравнить строки на больше-меньше (с учётом регистра) */
int e8_string_cmp               (const e8_string *a, const e8_string *b);

/*! Найти подстроку в строкѣ */
int e8_string_find              (const e8_string *str, const e8_string *substr, int from);

/*! Найти и замѣнить подстроку */
e8_string *
e8_string_replace
            (
                        const e8_string    *str,
                        const e8_string    *find,
                        const e8_string    *replace,
                        int                 from
             );

/*! Получить строку из строки в кодировкѣ Latin1 */
e8_string *
e8_string_ascii
            (
                        const char         *ascii
            );

/*! Получить строку из строки в кодировке UTF-32 */
e8_string *
e8_string_init
            (
                        const e8_uchar     *src
             );

/*! Получить усечённую строку */
e8_uchar  *e8_utf_trim           (const e8_uchar *src, bool left, bool right);

/*! Получить усечённую строку */
e8_string *e8_string_trim        (const e8_string *s, bool left, bool right);

typedef enum _e8_numeric_type {numInt, numFloat} e8_numeric_type;

typedef long    e8_integer;
typedef double  e8_float;

typedef struct _e8_numeric {
    e8_numeric_type     type;
    union {
        e8_integer      l_value;
        e8_float        d_value;
    };
} e8_numeric;

void e8_numeric_long        (e8_numeric *a, long value);
void e8_numeric_double      (e8_numeric *a, double value);
void e8_numeric_assign      (e8_numeric *a, const e8_numeric *b);

void e8_numeric_add         (e8_numeric *a, const e8_numeric *b);
void e8_numeric_sub         (e8_numeric *a, const e8_numeric *b);
void e8_numeric_mul         (e8_numeric *a, const e8_numeric *b);
void e8_numeric_div         (e8_numeric *a, const e8_numeric *b);
void e8_numeric_mod         (e8_numeric *a, const e8_numeric *b);
void e8_numeric_neg         (e8_numeric *a);

long e8_numeric_as_long     (const e8_numeric *a);
double e8_numeric_as_double (const e8_numeric *a);

int e8_numeric_eq           (const e8_numeric *a, const e8_numeric *b); // ==
int e8_numeric_ne           (const e8_numeric *a, const e8_numeric *b); // !=
int e8_numeric_lt           (const e8_numeric *a, const e8_numeric *b); // <
int e8_numeric_le           (const e8_numeric *a, const e8_numeric *b); // <=
int e8_numeric_ge           (const e8_numeric *a, const e8_numeric *b); // >=
int e8_numeric_gt           (const e8_numeric *a, const e8_numeric *b); // >

int e8_numeric_buferize_size(const e8_numeric *value);
int e8_numeric_buferize     (const e8_numeric *value, void *buf);
int e8_numeric_unbuferize   (const void *buf, e8_numeric *result);


typedef enum _e8_var_type {
        varVoid = 0,
        varNull, varUndefined, varNumeric, varString, varBool, varDateTime, varObject,
        varEnum,
        varLast
} e8_var_type;

typedef int8_t e8_enum_index;

typedef struct _e8_md_provider {
    int32_t     r0;
} e8_metadata_provider;

typedef struct _e8_type {
    e8_metadata_provider    *md;
} e8_type;



/*! Объект Е8 */
typedef void *e8_object;

/*! Структура вариантнаго значения */
typedef struct {
    e8_var_type                 type;
    union {
        bool                    bv;
        e8_date                 date;
        e8_numeric              num;
        e8_string              *str;
        struct {
            e8_object               obj;
            void                   *gcl;
        };
        struct {
            void               *md;
            e8_enum_index       index;
        }                      _enum;
    };
} e8_var;

#define E8_VAR_OK           0
#define E8_VAR_WRONG_CAST   1
#define E8_EXCEPTION        2


e8_var * e8_var_void             (e8_var *);
e8_var * e8_var_undefined        (e8_var *);
e8_var * e8_var_null             (e8_var *);

e8_var * e8_var_long             (e8_var *, long);
e8_var * e8_var_double           (e8_var *, double);
e8_var * e8_var_date             (e8_var *, e8_date);
e8_var * e8_var_string           (e8_var *, const e8_uchar *);
e8_var * e8_var_string_utf       (e8_var *, const char *);
e8_var * e8_var_bool             (e8_var *, bool);
e8_var * e8_var_numeric          (e8_var *v, const e8_numeric *num);
e8_var * e8_var_object           (e8_var *v, e8_object obj);

e8_var * e8_var_assign           (e8_var *, const e8_var *);

e8_string *
e8_numeric_to_string            (const e8_numeric *);

int e8_var_cast_string      (const e8_var *, e8_string **);

int e8_var_add              (e8_var *, const e8_var *);
int e8_var_sub              (e8_var *, const e8_var *);
int e8_var_mul              (e8_var *, const e8_var *);
int e8_var_div              (e8_var *, const e8_var *);
int e8_var_mod              (e8_var *, const e8_var *);

int e8_var_not              (e8_var *);
int e8_var_neg              (e8_var *);

/* Тип итога вычисленія не совпадает с типом членов,
 * потому полученіе итога сдѣлано через отдѣльную перемѣнную
 */
int e8_var_eq               (const e8_var *a, const e8_var *b, e8_var *res); // ==
int e8_var_ne               (const e8_var *a, const e8_var *b, e8_var *res); // !=
int e8_var_lt               (const e8_var *a, const e8_var *b, e8_var *res); // <
int e8_var_le               (const e8_var *a, const e8_var *b, e8_var *res); // <=
int e8_var_ge               (const e8_var *a, const e8_var *b, e8_var *res); // >=
int e8_var_gt               (const e8_var *a, const e8_var *b, e8_var *res); // >

/* Булева алгебра */
int e8_var_or               (e8_var *a, const e8_var *b);
int e8_var_and              (e8_var *a, const e8_var *b);
int e8_var_xor              (e8_var *a, const e8_var *b);


int e8_var_destroy          (e8_var *);

e8_var * e8_var_enum        (e8_var *r, void *md, e8_enum_index index);



bool e8_var_can_buferize    (const e8_var *value);
size_t e8_var_buferize_size (const e8_var *value);

int e8_var_buferize         (const e8_var *value, void *buf);
int e8_var_unbuferize       (const void *buf, e8_var *result);

int e8_var_cast_bool        (const e8_var *value, bool *result);

int e8_var_cast_long        (const e8_var *value, long *result);


typedef void *e8_object;

typedef e8_runtime_error* (*e8_property_set_handler)(void *property, const e8_var *value);
typedef e8_runtime_error* (*e8_property_get_handler)(void *property, e8_var *value);

typedef struct _e8_property {
    bool                        can_set;
    bool                        can_get;
    e8_property_get_handler     get;
    e8_property_set_handler     set;
    void                       *data;
} e8_property;

typedef bool (*e8_has_next_handler)(e8_object obj);
typedef void (*e8_next_handler)(e8_object obj, e8_var *value);

typedef int16_t e8_property_index_t;
typedef int16_t e8_method_index_t;

typedef e8_runtime_error*   (*e8_function_signature)(void *context, const void *user_data, int argc, e8_property *argv, e8_var *result);
typedef e8_runtime_error*   (*e8_get_property_t)    (e8_object obj, const e8_uchar *name, e8_property *property);
typedef e8_runtime_error*   (*e8_get_method_t)      (e8_object obj, const e8_uchar *name, e8_function_signature *method, void **user_data);
typedef const e8_uchar*     (*e8_to_string_t)       (const e8_object obj);

typedef e8_runtime_error*   (*e8_get_by_index_t)    (e8_object obj, const e8_var *index, e8_property *property);
typedef void                (*e8_destroy_t)         (e8_object);


typedef struct _e8_type_info {
    e8_to_string_t          to_string;
    e8_function_signature   constructor;
    void                   *data;
} e8_type_info;

/*! Описаніе виртуальной таблицы методов */
typedef struct _e8_virtual_table {
    e8_to_string_t          to_string;
    e8_get_property_t       get_property;
    e8_get_method_t         get_method;
    e8_get_by_index_t       by_index;
    e8_destroy_t            dtor;
    e8_type_info           *type;
} e8_vtable;

/*! Получает строковое представленіе объекта */
void e8_obj_to_string               (const e8_object ref, e8_var *str);

/*! Создаёт свойство-обёртку для ссылки на значеніе */
void e8_create_variant_property     (e8_property *property, e8_var *value);

/*! Уничтожает свойство-обёртку для ссылки на значеніе */
void e8_destroy_variant_property    (e8_property *property);

/*! Заполняет виртуальную таблицу методами по-умолчанію */
void e8_vtable_template             (e8_vtable *vmt);


#define E8_POOL_OK 0


typedef /*volatile */struct __list_element {
    e8_object                       obj;
    e8_vtable                      *vmt;
    volatile int                    count;

    pthread_mutex_t                 mutex;

    struct __list_element          *next;
    struct __list_element          *prev;
} gc_list_element;


void e8_gc_register_object      (const void *obj, e8_vtable *vmt);
void e8_gc_use_object           (const void *obj);
void e8_gc_free_object          (const void *obj);

void e8_gcl_use_object          (gc_list_element *gcl);
void e8_gcl_free_object         (gc_list_element *gcl);

void e8_gc_free_all             ();
void e8_gc_done                 ();

e8_vtable *e8_get_vtable        (const e8_object obj);
void *e8_get_gc_link            (const e8_object obj);


#define E8_COLLECTION(data_type) \
    data_type          *data; \
    int                 size; \
    int                 reserved; \
    int                 element_size; \
    int                 capacity_step;

#define E8_ITERATOR(base_type) \
    base_type          *ref; \
    int                 current;


struct __e8_collection_template {
    E8_COLLECTION(void)
};

struct __e8_collection_iterator_template {
    E8_ITERATOR(struct __e8_collection_template)
};


void e8_collection_set_default_capacity_step(int step);

e8_runtime_error *e8_collection_init(void *collection, int element_size);
e8_runtime_error *e8_collection_init_size(void *collection, int element_size, int size);
e8_runtime_error *e8_collection_resize(void *collection, int new_size);
e8_runtime_error *e8_collection_add_size(void *collection, int delta);
e8_runtime_error *e8_collection_insert(void *collection, int index);

void e8_collection_free(void *collection);

#define E8_COLLECTION_INIT_SIZE(coll, size) \
    e8_collection_init_size((void*)&coll, sizeof(*((coll).data)), (size))

#define E8_COLLECTION_INIT(coll) \
    e8_collection_init((void*)&coll, sizeof(*((coll).data)))

bool  e8_iterator_has_next(void *iterator);
void  e8_iterator_move_next(void *iterator);
void *e8_iterator_current(void *iterator);
void *e8_iterator_next(void *iterator);

#define E8_ITR_NEXT(it, data_type) *((data_type*)e8_iterator_next(it))
#define E8_ITR_INIT(it, source) { \
        it = malloc(sizeof(*it));\
        it->ref = source;\
        it->current = 0;\
    }


#define E8_CLUSTERED_LIST(data_type) \
    data_type          *data; \
    int                 size; \
    int                 reserved; \
    int                 element_size; \
    void               *next; \
    void               *tail;

struct __e8_clustered_list_template {
    E8_CLUSTERED_LIST(void)
};

e8_runtime_error *e8_clustered_list_init(void *list, int element_size);
e8_runtime_error *e8_clustered_list_append(void *list, void **address);
e8_runtime_error *e8_clustered_list_dump(const void *list, void *collection);
void e8_clustered_list_free(void *list);

int e8_clustered_list_size(const void *list);

#ifdef __cplusplus
} // extern "C"
#endif


#endif // E8_CORE_VARIANT_H
