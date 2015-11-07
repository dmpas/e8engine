/* НАДО: Пересмотреть работу с IEnum */

#include "e8core/plugin/plugin.h"
#include "e8core/utf/utf.h"
#include "disphelper.h"
#include <oleauto.h>

#include <malloc.h>
#include <wchar.h>
#include <stdbool.h>
#include <stdio.h>

struct dispatch {
    IDispatch *obj;
};
#define _DISP(name) struct dispatch *name = (struct dispatch *)obj

#define MAX_PROPERTY_NAME_LENTH 80
struct property {
    struct dispatch            *disp;
    char                        name[MAX_PROPERTY_NAME_LENTH];
    wchar_t                     call_name[MAX_PROPERTY_NAME_LENTH];
};
#define _PROP(name) struct property *name = (struct property *)((e8_property *)property)->data

struct method {
    struct dispatch            *disp;
    wchar_t                     call_name[MAX_PROPERTY_NAME_LENTH];
};
#define _METH(name) struct method *name = (struct method *)user_data


e8_vtable vmt_Dispatcher;
static bool __init = false;
static void __init_table();

#define HR_TRY(func) \
if (FAILED(func)) E8_THROW_DETAIL(E8_RT_EXCEPTION_RAISED, "OLE error!");

static E8_DECLARE_DESTRUCTOR(Dispatch_Destroy)
{
    _DISP(d);

    SAFE_RELEASE(d->obj);

    free(d);
}

E8_DECLARE_SUB(Dispatch_Constructor)
{
    if (!__init)
        __init_table();

    E8_ARG_UTF_STRING(ClassName);

    wchar_t
        *w_ClassName = malloc(sizeof(wchar_t) * (strlen(ClassName) + 1)),
        *w_MachineName = 0
    ;

    wsprintfW(w_ClassName, L"%hs", ClassName);
    free(ClassName);

    if (argc > 1) {
        E8_ARG_UTF_STRING(MachineName);
        w_MachineName = malloc(sizeof(wchar_t) * (strlen(MachineName) + 1));

        wsprintfW(w_MachineName, L"%hs", MachineName);

        free(MachineName);
    }


    struct dispatch *r = malloc(sizeof(*r));

    r->obj = NULL;

    HR_TRY ( dhCreateObject(w_ClassName, w_MachineName, &r->obj) );

    if (w_ClassName)
        free(w_ClassName);
    if (w_MachineName)
        free(w_MachineName);

    e8_gc_register_object(r, &vmt_Dispatcher);

    e8_var_object(result, r);

    E8_RETURN_OK;
}

static e8_uchar NAME[] = {'C', 'o', 'm', 'O', 'b', 'j', 'e', 'c', 't', 0};
E8_DECLARE_TO_STRING(Dispatch_ToString)
{
    return NAME;
}

static void
olechar_to_e8(const wchar_t *W, e8_uchar *U)
{
    const wchar_t *w = W;
    e8_uchar *u = U;
    while (*w) {
        *u = *w;
        ++w;
        ++u;
    }
    *u = 0;
}
static void
e8_to_olechar(const e8_uchar *U, wchar_t *W)
{
    const e8_uchar *u = U;
    wchar_t *w = W;
    while (*u) {
        *w = *u;
        ++u;
        ++w;
    }
    *w = 0;
}

static void
Dispatch_Cast(const VARIANT *v, e8_var *value)
{
    if (v->vt == VT_INT
        || v->vt == VT_UINT
        || v->vt == VT_I1
        || v->vt == VT_I2
        || v->vt == VT_I4
        || v->vt == VT_I8
        || v->vt == VT_UI1
        || v->vt == VT_UI2
        || v->vt == VT_UI4
        || v->vt == VT_UI8
        ) {
            e8_var_long(value, v->lVal);
    }
    else
    if (v->vt == VT_R4) {
        e8_var_double(value, v->fltVal);
    }
    else
    if (v->vt == VT_R8) {
        e8_var_double(value, v->dblVal);
    }
    else
    if (v->vt == VT_BOOL) {
        e8_var_bool(value, v->boolVal);
    }
    else
    if (v->vt == VT_EMPTY) {
        char buf[] = "EMPTY";
        e8_var_string_utf(value, buf);
    }
    else
    if (v->vt == VT_DISPATCH) {
        struct dispatch *D = malloc(sizeof(*D));
        D->obj = v->pdispVal;
        e8_gc_register_object(D, &vmt_Dispatcher);
        e8_var_object(value, D);
    }
    else
    if (v->vt == VT_BSTR) {
        e8_uchar *buf = e8_utf_malloc(SysStringLen(v->bstrVal));
        olechar_to_e8(v->bstrVal, buf);

        e8_var_string(value, buf);
        e8_utf_free(buf);
    }
    else
    if (v->vt == VT_DATE) {
        e8_date base;
        e8_date_construct(1899, 12, 30, 00, 00, 00, &base);

        e8_date ed = base + v->date * 24 * 3600;
        e8_var_date(value, ed);
    }
    else {
        char buf[40];
        sprintf(buf, "Wrong value: (%d)", v->vt);
        e8_var_string_utf(value, buf);
    }
}

static void
Cast_Dispatch(const e8_var *value, VARIANT *v)
{
    switch (value->type) {
    case varBool:
        v->vt = VT_BOOL;
        v->boolVal = value->bv;
        break;
    case varNull:
        v->vt = VT_NULL;
        break;
    case varNumeric:
        {
            if (value->num.type == numFloat) {
                v->vt = VT_R8;
                v->dblVal = value->num.d_value;
            } else {
                v->vt = VT_I8; /* TODO: Platform! */
                v->lVal = value->num.l_value;
            }
        }
        break;
    case varString:
        {
            int l = value->str->len;
            wchar_t *wbuf = malloc(sizeof(wchar_t) * (l + 1));

            e8_to_olechar(value->str->s, wbuf);

            v->vt = VT_BSTR;
            v->bstrVal = SysAllocString(wbuf);

            free(wbuf);
        }
        break;
    case varDateTime:
        {
            e8_date base;
            e8_date_construct(1899, 12, 30, 00, 00, 00, &base);
            v->vt = VT_DATE;
            v->date = (double)(value->date - base) / (24*60*60);
        }
        break;
    case varObject:
        {
            /* Считаем, что здесь наш dispatch */
            struct dispatch *r = (struct dispatch *)value->obj;
            v->vt = VT_DISPATCH;
            v->pdispVal = r->obj;
        }
        break;
    default:
        v->vt = VT_EMPTY;
    }
}

static e8_vtable vmt_IEnum;

struct Dispatch_Enum {
    struct dispatch        *d;
    IEnumVARIANT           *en;
    e8_var                  next;
};
#define _ENUM(name) struct Dispatch_Enum *name = (struct Dispatch_Enum *)obj

E8_DECLARE_SUB(Dispatch_Enum_Iterator)
{
    e8_var_object(result, obj);
    E8_RETURN_OK;
}

E8_DECLARE_SUB(Dispatch_Enum_HasNext)
{
    _ENUM(ien);

    VARIANT val;
    VariantInit(&val);

    HRESULT res = dhEnumNextVariant(ien->en, &val);
    if (res == NOERROR) {
        Dispatch_Cast(&val, &ien->next);
        e8_var_bool(result, true);
    } else
        e8_var_bool(result, false);

    E8_RETURN_OK;
}

E8_DECLARE_SUB(Dispatch_Enum_Next)
{
    _ENUM(ien);

    e8_var_assign(result, &ien->next);
    e8_var_destroy(&ien->next);

    E8_RETURN_OK;
}

static E8_METHOD_LIST_START(Dispatch_Enum_methods)
    E8_METHOD_LIST_ADD_HAS_NEXT(Dispatch_Enum_HasNext)
    E8_METHOD_LIST_ADD_NEXT(Dispatch_Enum_Next)
E8_METHOD_LIST_END

static E8_DECLARE_GET_METHOD(Dispatch_Enum_GetMethod)
{
    e8_simple_get_method(name, Dispatch_Enum_methods, fn);
    E8_RETURN_OK;
}

static E8_DECLARE_PROPERTY_GET(DispatchProperty_get_handler)
{
    _PROP(p);

    VARIANT VR;

    HRESULT res = dhGetValue(L"%v", &VR, p->disp->obj, p->call_name);

    /* Этот кусок кода под вопросом */
    if (FAILED(res)) {
        /* Попытаемся получить IEnum */
        IEnumVARIANT *ienum = NULL;

        wchar_t wbuf[100];
        wsprintfW(wbuf, L".%s", p->call_name);

        res = dhEnumBegin(&ienum, p->disp->obj, wbuf);
        if (SUCCEEDED(res)) {

            struct Dispatch_Enum *en = malloc(sizeof(*en));
            e8_gc_register_object(en, &vmt_IEnum);
            e8_var_object(value, en);

            E8_RETURN_OK;
        }
    }


    if (FAILED(res)) {
        E8_THROW_DETAIL(E8_RT_EXCEPTION_RAISED, strdup("OLE Error!"));
    }


    Dispatch_Cast(&VR, value);
    /* VariantClear(&VR); */

    E8_RETURN_OK;
}

static E8_DECLARE_PROPERTY_SET(DispatchProperty_set_handler)
{
    _PROP(p);

    VARIANT VR;

    Cast_Dispatch(value, &VR);

    wchar_t buf[MAX_PROPERTY_NAME_LENTH + 10];
    wsprintfW(buf, L"%s = %%v", p->call_name);

    HRESULT res = dhPutValue(p->disp->obj, buf, &VR);

    /* VariantClear(&VR); */

    if (FAILED(res)) {
        E8_THROW_DETAIL(E8_RT_EXCEPTION_RAISED, strdup("OLE Error!"));
    }

    E8_RETURN_OK;
}



static void
create_dispatch_property(e8_property *p)
{
    p->get = DispatchProperty_get_handler;
    p->set = DispatchProperty_set_handler;
    p->can_get = true;
    p->can_set = true;
}

static E8_DECLARE_GET_PROPERTY(Dispatch_GetProperty)
{
    _DISP(d);

    struct property *p = malloc(sizeof(*p)); /* TODO: Утечка */
    p->disp = d;
    e8_to_olechar(name, p->call_name);

    create_dispatch_property(property);
    property->data = p;

    E8_RETURN_OK;
}

E8_DECLARE_SUB(Dispatch_Invoke)
{
    _METH(m);

    VARIANT *vargs = malloc(sizeof(VARIANT) * (argc + 1));
    int i;

    char argbuf[80] = {0};
    char *cindex = argbuf;

    e8_property *P = argv;
    VARIANT *varg = vargs;
    for (i = 0; i < argc; ++i, ++P, ++varg) {

        if (i)
            strcpy(cindex, ", %v");
        else
            strcpy(cindex, "%v");

        cindex += strlen(cindex);

        E8_VAR(arg);

        P->get(P, &arg);
        Cast_Dispatch(&arg, varg);

        /* e8_var_destroy(&arg); */
    }

    VARIANT VR;
    HRESULT res;
    res = dhInvokeArray(DISPATCH_METHOD,
                            &VR, argc, m->disp->obj, m->call_name, vargs
    );

    free(vargs);
    free(m); /* TODO: Действительно ли? */

    if (SUCCEEDED(res)) {
        Dispatch_Cast(&VR, result);
        /* VariantClear(&VR); */
        E8_RETURN_OK;
    }

    E8_THROW_DETAIL(E8_RT_EXCEPTION_RAISED, "Some fail!");
}

struct Dispatch_Iterator {
    struct dispatch            *d;      /*!< DISPATCH */
    IEnumVARIANT               *en;     /*!< Enum */
    e8_var                      next;
};
#define _DIT(name) struct Dispatch_Iterator *name = (struct Dispatch_Iterator *)obj

E8_DECLARE_SUB(Dispatch_Iterator_HasNext)
{
    _DIT(dit);

    VARIANT val;
    VariantInit(&val);

    HRESULT res = dhEnumNextVariant(dit->en, &val);
    if (res == NOERROR) {
        Dispatch_Cast(&val, &dit->next);
        e8_var_bool(result, true);
    } else
        e8_var_bool(result, false);

    E8_RETURN_OK;
}

E8_DECLARE_SUB(Dispatch_Iterator_Next)
{
    _DIT(dit);
    e8_var_assign(result, &dit->next);
    e8_var_destroy(&dit->next);
    E8_RETURN_OK;
}

static E8_METHOD_LIST_START(Dispatch_Iterator_methods)
    E8_METHOD_LIST_ADD_HAS_NEXT(Dispatch_Iterator_HasNext)
    E8_METHOD_LIST_ADD_NEXT(Dispatch_Iterator_Next)
E8_METHOD_LIST_END

static E8_DECLARE_GET_METHOD(Dispatch_Iterator_GetMethod)
{
    e8_simple_get_method(name, Dispatch_Iterator_methods, fn);
    E8_RETURN_OK;
}

static e8_vtable vmt_Dispatch_Iterator;

E8_DECLARE_SUB(Dispatch_Iterator)
{
    _DISP(d);

    IEnumVARIANT *enum_var = NULL;
    HRESULT res = dhEnumBegin(&enum_var, d->obj, NULL);
    if (FAILED(res))
        E8_THROW_DETAIL(E8_RT_NOT_A_COLLECTION, "Object has no iterator!");

    struct Dispatch_Iterator *dit = malloc(sizeof(*dit));

    dit->d = d;
    dit->en = enum_var;

    e8_gc_register_object(dit, &vmt_Dispatch_Iterator);
    e8_var_object(result, dit);

    E8_RETURN_OK;
}

static E8_METHOD_LIST_START(Dispatch_methods)
    E8_METHOD_LIST_ADD_ITERATOR(Dispatch_Iterator)
E8_METHOD_LIST_END

E8_DECLARE_GET_METHOD(Dispatch_GetMethod)
{

    e8_simple_get_method(name, Dispatch_methods, fn);
    if (*fn)
        E8_RETURN_OK;

    _DISP(d);

    struct method *m = malloc(sizeof(*m));
    *user_data = m;
    m->disp = d;
    e8_to_olechar(name, m->call_name);

    *fn = Dispatch_Invoke;

    E8_RETURN_OK;
}

E8_DECLARE_TYPE(Dispatcher_type)
    E8_SET_TO_STRING    (Dispatch_ToString)
    E8_SET_CONSTRUCTOR  (Dispatch_Constructor)
E8_END_TYPE

E8_PLUGIN_MAIN
{

    E8_WITH_TYPE(Dispatcher_type)
        E8_REGISTER_ALIAS("ComObject")
        E8_REGISTER_ALIAS("ComОбъект")
        E8_REGISTER_ALIAS("КомОбъект")
    E8_END_WITH

    E8_RETURN_OK;
}

static void __init_table()
{
	dhInitialize(TRUE);
	dhToggleExceptions(FALSE);

    e8_prepare_call_elements(Dispatch_methods, "utf-8");
    e8_prepare_call_elements(Dispatch_Iterator_methods, "utf-8");
    e8_prepare_call_elements(Dispatch_Enum_methods, "utf-8");

    e8_vtable_template(&vmt_Dispatcher);
    vmt_Dispatcher.dtor = Dispatch_Destroy;
    vmt_Dispatcher.to_string = Dispatch_ToString;
    vmt_Dispatcher.get_property = Dispatch_GetProperty;
    vmt_Dispatcher.get_method = Dispatch_GetMethod;

    e8_vtable_template(&vmt_Dispatch_Iterator);
    vmt_Dispatch_Iterator.get_method = Dispatch_Iterator_GetMethod;

    e8_vtable_template(&vmt_IEnum);
    vmt_IEnum.get_method = Dispatch_Enum_GetMethod;

    __init = true;
}

/*!
From disphelper's doc.

Identifier	Type	Compatible Types	        Note
%d	        LONG	long, int, INT
%u	        ULONG	unsigned long, unsigned int, UINT, DWORD
%e	        DOUBLE	double
%b	        BOOL
%v	        VARIANT
%B	        BSTR	 	                        1
%s	        LPSTR	char *	                    1
%S	        LPWSTR	WCHAR *	                    1
%T	        LPTSTR	TCHAR *	                    1
%o	        IDispatch *	DISPATCH_OBJ(var)	    2, 3
%O	        IUnknown *	 	                    3
%t	        time_t	 	                        4
%W	        SYSTEMTIME *	 	                4
%f	        FILETIME *	 	                    4
%D	        DATE	 	                        4
%p	        LPVOID	Use for HANDLEs, HWNDs, etc
%m	        Missing Argument	 	            5

Note 1: When a string is returned using dhGetValue() it should be freed using dhFreeString().
Note 2: When an object is declared using DISPATCH_OBJ(var) it is an IDispatch *.
Note 3: When an IDispatch * or IUnknown * is returned from dhGetValue() it should be released using the SAFE_RELEASE() macro.
Note 4: A variant DATE(%D) is always in local time. When passing in or receiving a FILETIME(%f) or SYSTEMTIME(%W) no time zone translation is performed. Therefore, you should pass in and expect to receive SYSTEMTIMEs and FILETIMEs in local time. However, as a time_t(%t) is always in GMT time, time zone translation is performed by DispHelper.
Note 5: %m can only be used as an input argument. It specifies a missing optional argument. It does not have a corresponding entry in the argument list.
dhCallMethod(myObj, L".DoSomething(%m, %s)", "test");
*/
