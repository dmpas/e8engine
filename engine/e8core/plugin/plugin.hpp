/*!file*/
#ifndef E8_CORE_PLUGIN_HPP_INCLUDED
#define E8_CORE_PLUGIN_HPP_INCLUDED

#define E8_IN  const E8::Variant &
#define E8_OUT       E8::Variant &

#ifdef __cplusplus

#include <string>
#include <map>
#include <vector>
#include <e8core/plugin/plugin.h>
#include <e8core/utf/utf.h>
#include <malloc.h>

#ifndef E8_DEFAULT_OUTPUT_ENCODING
#define E8_DEFAULT_OUTPUT_ENCODING "utf-8"
#endif // E8_DEFAULT_OUTPUT_ENCODING

#include <ctime>

namespace E8 {

template<typename T>
std::wstring utf_to_ws(const T *us)
{
    std::wstring r;
    while (*us) {
        r += static_cast<wchar_t>(*us);
        ++us;
    }
    return r;
}


class string : public std::basic_string<e8_uchar> {

public:

    string(const char *utf_text)
        : std::basic_string<e8_uchar>()
    {
        e8_uchar *text = e8_utf_strcdup(utf_text, "utf-8");
        this->append(text);
        e8_utf_free(text);
    }
    string (const wchar_t *wtext)
        : std::basic_string<e8_uchar>()
    {
        std::wstring ws(wtext);
        e8_uchar *r = e8_utf_from_mbstring(ws.c_str(), ws.size(), sizeof(wchar_t));
        this->append(r);
        e8_utf_free(r);
    }
    string (const std::wstring &ws)
        : std::basic_string<e8_uchar>()
    {
        e8_uchar *r = e8_utf_from_mbstring(ws.c_str(), ws.size(), sizeof(wchar_t));
        this->append(r);
        e8_utf_free(r);
    }
    string (const e8_uchar *utext)
        : std::basic_string<e8_uchar>()
    {
        this->append(utext);
    }
    string ()
        : std::basic_string<e8_uchar>()
    {

    }

    std::string to_string(const char *encoding) const
    {
        char *b = 0;
        e8_utf_to_8bit(this->c_str(), &b, encoding);
        std::string s(b);
        free(b);
        return s;
    }
    std::string to_string(const std::string &encoding) const
    {
        return to_string(encoding.c_str());
    }

    std::string to_string() const
    {
        return to_string("utf-8");
    }

    std::wstring to_wstring() const
    {
        return utf_to_ws(this->c_str());
    }

    string Lower() const
    {
        e8_uchar *cp = e8_utf_strdup(c_str());
        e8_utf_lower_case(cp);
        string R(cp);
        e8_utf_free(cp);
        return R;
    }

}; // class string

class Exception : public std::exception {
public:
    Exception()
        : std::exception(), m_error(E8_RT_EXCEPTION_RAISED), m_msg(""), m_cmsg("") {}

    explicit Exception(int error)
        : std::exception(), m_error(error), m_msg(""), m_cmsg("") {}

    Exception(int error, const string &msg)
        : std::exception(), m_error(error), m_msg(msg)
    {
        m_cmsg = m_msg.to_string();
    }

    Exception(int error, const std::string &msg)
        : std::exception(), m_error(error), m_msg(msg.c_str()), m_cmsg(msg)
    {}

    explicit Exception(const std::string &msg)
        : std::exception(), m_error(E8_RT_EXCEPTION_RAISED), m_msg(msg.c_str()), m_cmsg(msg)
    {}

    Exception(const string &msg)
        : std::exception(), m_error(E8_RT_EXCEPTION_RAISED), m_msg(msg)
    {
        m_cmsg = m_msg.to_string();
    }

    virtual e8_runtime_error *__e_raise() const
    {
        return e8_throw_data(m_error, strdup(m_cmsg.c_str()));
    }

    virtual ~Exception() throw() {}
    virtual const char* what() const throw()
    {
        return m_cmsg.c_str();
    }

private:
    int             m_error;
    string          m_msg;
    std::string     m_cmsg;
};

class CoreException : public Exception {
public:
    CoreException(e8_runtime_error *err) : err(err) {}
    virtual e8_runtime_error *__e_raise() const
    {
        return err;
    }
    virtual const char* what() const throw()
    {
        return (const char *)err->data;
    }
private:
    e8_runtime_error *err;
};

class VariantException : public std::exception {
public: VariantException() : std::exception() {}
};

class Variant {
public:
    Variant() { e8_var_undefined(&m_value); }
    ~Variant() { e8_var_destroy(&m_value); }

    Variant(const e8_var &value) { e8_var_undefined(&m_value); assign(value); }
    Variant(const Variant &V) { e8_var_undefined(&m_value); assign(V.plain()); }

    static Variant Object(void *obj)
    {
        Variant R;
        e8_var_object(&R.plain(), obj);
        return R;
    }
    static Variant Object(void *obj, e8_vtable *vmt)
    {
        Variant R;
        e8_gc_register_object(obj, vmt);
        e8_var_object(&R.plain(), obj);
        return R;
    }
    static Variant Null()
    {
        e8_var r;
        e8_var_null(&r);
        return Variant(r);
    }
    static Variant False()
    {
        e8_var r;
        e8_var_bool(&r, false);
        return Variant(r);
    }
    static Variant True()
    {
        e8_var r;
        e8_var_bool(&r, false);
        return Variant(r);
    }

    explicit Variant(long value) { e8_var_long(&m_value, value); }
    explicit Variant(int value) { e8_var_long(&m_value, value); }
    explicit Variant(double value) { e8_var_double(&m_value, value); }
    explicit Variant(bool value) { e8_var_bool(&m_value, value); }
    explicit Variant(uintmax_t value) { e8_var_long(&m_value, value); }
    #ifndef _WIN64
	#ifndef __x86_64
    explicit Variant(size_t value) { e8_var_long(&m_value, value); }
	#endif
    #endif
    explicit Variant(const tm *fd)
    {
        e8_date d;
        e8_date_construct(fd->tm_year + 1900, fd->tm_mon + 1, fd->tm_mday,
                          fd->tm_hour, fd->tm_min, fd->tm_sec, &d);
        e8_var_date(&plain(), d);
    }
    Variant(const string &value) { e8_var_string(&m_value, value.c_str()); }
	Variant(void *md, int value)
	{
		e8_var_enum(&plain(), md, value);
	}

    tm get_tm() const
    {
        tm s;

        e8_date d = plain().date;
        s.tm_sec = e8_date_get_sec(d);
        s.tm_min = e8_date_get_min(d);
        s.tm_hour = e8_date_get_hour(d);
        s.tm_mday = e8_date_get_day(d);
        s.tm_mon = e8_date_get_month(d);
        s.tm_year = e8_date_get_year(d) - 1900;

        return s;
    }

    const e8_var &plain() const { return m_value; }
    e8_var &plain() { return m_value; }

    #define VARIANT_OPERATOR(op) \
    { \
        Variant r(*this); \
        if (e8_var_##op(&r.plain(), &b.plain())) \
            throw VariantException(); \
        return r; \
    }

    Variant operator + (const Variant &b) const
    VARIANT_OPERATOR(add)

    Variant operator - (const Variant &b) const
    VARIANT_OPERATOR(sub)

    Variant operator * (const Variant &b) const
    VARIANT_OPERATOR(mul)

    Variant operator / (const Variant &b) const
    VARIANT_OPERATOR(div)

    Variant operator % (const Variant &b) const
    VARIANT_OPERATOR(mod)

    #undef VARIANT_OPERATOR

    #define VARIANT_OPERATOR(op) \
    { \
        if (e8_var_##op(&plain(), &b.plain())) \
            throw VariantException(); \
        return *this; \
    }

    Variant operator += (const Variant &b)
    VARIANT_OPERATOR(add)

    Variant operator -= (const Variant &b)
    VARIANT_OPERATOR(sub)

    Variant operator *= (const Variant &b)
    VARIANT_OPERATOR(mul)

    Variant operator /= (const Variant &b)
    VARIANT_OPERATOR(div)

    Variant operator %= (const Variant &b)
    VARIANT_OPERATOR(mod)

    #undef VARIANT_OPERATOR

    Variant &operator ++()
    {
        e8_var one;
        e8_var_long(&one, 1);
        if (e8_var_add(&plain(), &one))
            throw VariantException();
        return *this;
    }

    Variant operator ++ (int)
    {
        Variant R(*this);

        e8_var one;
        e8_var_long(&one, 1);
        if (e8_var_add(&plain(), &one))
            throw VariantException();

        return R;
    }

    Variant &operator --()
    {
        e8_var one;
        e8_var_long(&one, 1);
        if (e8_var_sub(&plain(), &one))
            throw VariantException();
        return *this;
    }

    Variant operator -- (int)
    {
        Variant R(*this);

        e8_var one;
        e8_var_long(&one, 1);
        if (e8_var_sub(&plain(), &one))
            throw VariantException();

        return R;
    }

    #define VARIANT_COMPARE(op) \
    {\
        e8_var r;\
        if (e8_var_##op(&plain(), &b.plain(), &r))\
            throw VariantException();\
        if (r.type != varBool)\
            throw VariantException();\
        return r.bv;\
    }

    bool operator == (const Variant &b) const
    VARIANT_COMPARE(eq)

    bool operator != (const Variant &b) const
    VARIANT_COMPARE(ne)

    bool operator >= (const Variant &b) const
    VARIANT_COMPARE(ge)

    bool operator <= (const Variant &b) const
    VARIANT_COMPARE(le)

    bool operator > (const Variant &b) const
    VARIANT_COMPARE(gt)

    bool operator < (const Variant &b) const
    VARIANT_COMPARE(lt)

    #undef VARIANT_COMPARE

    void assign(const Variant &value)
    {
        assign(value.plain());
    }
    void assign(const e8_var &value)
    {
        e8_var_assign(&plain(), &value);
    }

    Variant &operator = (const Variant &value)
    {
        assign(value);
        return *this;
    }

    template<typename Type>
    Variant &operator = (Type value)
    {
        Variant NewValue(value);
        this->assign(NewValue);
        return *this;
    }

    Variant operator - () const
    {
        Variant r(*this);
        e8_var_neg(&r.plain());
        return r;
    }

    Variant operator ! () const
    {
        Variant r(*this);
        e8_var_not(&r.plain());
        return r;
    }

    string to_string() const
    {
        e8_string *s;
        e8_var_cast_string(&plain(), &s);
        string r(s->s);
        e8_string_destroy(s);
        return r;
    }

    bool to_bool() const
    {
        bool value;
        if (e8_var_cast_bool(&plain(), &value))
            throw VariantException();
        return value;
    }

    long to_long() const
    {
        if (of_type(varNumeric))
            return e8_numeric_as_long(&plain().num);
        throw VariantException();
    }

    double to_double() const
    {
        if (of_type(varNumeric))
            return e8_numeric_as_double(&plain().num);
        throw VariantException();
    }

    operator bool() const
    {
        return to_bool();
    }

    bool of_type(e8_var_type type) const
    {
        return plain().type == type;
    }

    Variant __VCall(const string &name, int argc = 0, ...)
    {
        std::string utf_name = name.to_string("utf-8");
        E8_VAR(r);

        e8_var *argv = new e8_var[argc + 1];

        va_list params;
        va_start(params, argc);
        for (int i = 0; i < argc; ++i)
            argv[i] = va_arg(params, e8_var);
        va_end(params);

        e8_runtime_error *__err = e8_simple_call_utf(utf_name.c_str(), m_value.obj, &r, argc, argv);
        if (__err)
            throw CoreException(__err);

        delete [] argv;

        Variant V(r);
        e8_var_destroy(&r);

        return V;
    }

    Variant __Call(const string &name)
    {
        return __VCall(name, 0);
    }

    template <typename T1>
    Variant __Call(const string &name, T1 p1)
    {
        Variant v1(p1);
        return __VCall(name, 1, v1.plain());
    }

    template <typename T1, typename T2>
    Variant __Call(const string &name, T1 p1, T2 p2)
    {
        Variant v1(p1), v2(p2);
        return __VCall(name, 2, v1.plain(), v2.plain());
    }

    template <typename T1, typename T2, typename T3>
    Variant __Call(const string &name, T1 p1, T2 p2, T3 p3)
    {
        Variant v1(p1), v2(p2), v3(p3);
        return __VCall(name, 3, v1.plain(), v2.plain(), v3.plain());
    }

    Variant __get(const string &name) const
    {
        std::string utf = name.to_string("utf-8");
        E8_VAR(r);

        e8_get_property_value(&m_value, utf.c_str(), &r);

        Variant R(r);

        e8_var_destroy(&r);

        return R;
    }

    /* Частые операторы */
    Variant Iterator()
    {
        return __Call(string("Iterator"));
    }

    bool HasNext()
    {
        return __Call(string("HasNext")).to_bool();
    }

    Variant Next()
    {
        return __Call(string("Next"));
    }

    Variant Size()
    {
        return __Call(string("Size"));
    }
    Variant Count()
    {
        return __Call(string("Count"));
    }

    template<typename T>
    Variant Get(T index)
    {
        return __Call("Get", index);
    }

static Variant Undefined() { return Variant(); }

private:
    e8_var      m_value;
};

inline Variant Undefined() { return Variant(); }

class PropertyException : public std::exception {
public:
    PropertyException() {}
};


class Property {
public:
    Property(const e8_property &property)
    {
        m_property = property;
    }

    bool can_get() const { return m_property.can_get; }
    bool can_set() const { return m_property.can_set; }

    Property &set(const Variant &value)
    {
        if (!can_set())
            throw PropertyException();
        m_property.set(&m_property, &value.plain());
        return *this;
    }

    Variant get() const
    {
        if (!can_get())
            throw PropertyException();
        e8_var value;
        e8_var_undefined(&value);
        m_property.get(&m_property, &value);
        return Variant(value);
    }

private:
    mutable e8_property m_property;
};

class EnvironmentException : public std::exception {
public:
    EnvironmentException() {}
};

namespace GC {
    inline void Register(void *ref, e8_vtable *vmt)
    {
        e8_gc_register_object(ref, vmt);
    }

    inline void Use(void *ref)
    {
        e8_gc_use_object(ref);
    }

    inline void Free(void *ref)
    {
        e8_gc_free_object(ref);
    }
};

class Environment {
public:
    Environment() {}

    void assign(e8_env *env)
    {
        m_env = env;
    }

    Variant NewObject_v(const string &type, int argc, va_list params)
    {

        std::string utf = type.to_string("utf-8");

        e8_function_signature fn = m_env->vmt->get_ctor(m_env, utf.c_str());

        if (!fn)
            throw EnvironmentException();

        e8_var *argv = new e8_var[argc + 1];

        for (int i = 0; i < argc; ++i)
            argv[i] = va_arg(params, e8_var);

        va_end(params);

        E8_VAR(r);

        if (e8_simple_pcall(fn, NULL, &r, argc, argv))
            throw VariantException();

        delete [] argv;

        Variant V(r);
        e8_var_destroy(&r);

        return V;
    }

    Variant NewObject(const string &type, int argc = 0, ...)
    {
        va_list params;
        va_start(params, argc);

        return NewObject_v(type, argc, params);
    }


    Variant __VCall(const string &name, int argc = 0, ...)
    {
        std::string utf_name = name.to_string("utf-8");
        E8_VAR(r);

        e8_var *argv = new e8_var[argc + 1];

        va_list params;
        va_start(params, argc);
        for (int i = 0; i < argc; ++i)
            argv[i] = va_arg(params, e8_var);
        va_end(params);

        if (e8_simple_call_utf(utf_name.c_str(), m_env->data, &r, argc, argv))
            throw VariantException();

        delete [] argv;

        Variant V(r);
        e8_var_destroy(&r);

        return V;
    }

    Variant __Call(const string &name)
    {
        return __VCall(name, 0);
    }

    template <typename T1>
    Variant __Call(const string &name, T1 p1)
    {
        Variant v1(p1);
        return __VCall(name, 1, v1.plain());
    }

    template <typename T1, typename T2>
    Variant __Call(const string &name, T1 p1, T2 p2)
    {
        Variant v1(p1), v2(p2);
        return __VCall(name, 2, v1.plain(), v2.plain());
    }

    template <typename T1, typename T2, typename T3>
    Variant __Call(const string &name, T1 p1, T2 p2, T3 p3)
    {
        Variant v1(p1), v2(p2), v3(p3);
        return __VCall(name, 3, v1.plain(), v2.plain(), v3.plain());
    }

    template<typename T1>
    Variant AttachUnit(const std::basic_string<T1> &name)
    {
        Variant unit_name(name);
        return __VCall(string("AttachUnit"), 1, unit_name.plain());
    }

    Variant AttachUnit(const string &name)
    {
        Variant unit_name(name);
        return __VCall(string("AttachUnit"), 1, unit_name.plain());
    }

    template<typename T1>
    void Message(T1 p1)
    {
        Variant v1(p1);
        __VCall(string("Message"), 1, v1.plain());
    }
    template<typename T1, typename T2>
    void Message(T1 p1, T2 p2)
    {
        Variant v1(p1), v2(p2);
        __VCall(string("Message"), 2, v1.plain(), v2.plain());
    }

    void RegisterGlobalValue(const std::string &name, Variant &Value, bool read = true, bool write = false)
    {
        e8_property *mp = new e8_property;
        e8_create_variant_property(mp, &Value.plain());
        mp->can_get = read;
        mp->can_set = write;
        m_env->vmt->register_property(m_env, name.c_str(), mp);

        m_props.push_back(mp);
    }

    ~Environment()
    {
        std::vector<e8_property *>::iterator it;
        for (it = m_props.begin(); it != m_props.end(); ++it)
            delete (*it);
        m_props.clear();
    }


private:
    e8_env                                 *m_env;
    std::vector<e8_property *>              m_props;

public:
    std::map<std::string, e8_vtable*>       VMT;
};

extern Environment Env;

template<typename T>
Variant __NewObject(const string &type, int argc, ...)
{
    va_list params;
    va_start(params, argc);

    return Env.NewObject_v(type, argc, params);
}

template<typename T>
Variant NewObject(T type)
{
    string s(type);
    return __NewObject<void>(s, 0);
}

template<typename T1>
Variant NewObject(const string &type, T1 p1)
{
    Variant v1(p1);
    return __NewObject<void>(type, 1, v1.plain());
}
template<typename T1, typename T2>
Variant NewObject(const string &type, T1 p1, T2 p2)
{
    Variant v1(p1), v2(p2);
    return __NewObject<void>(type, 2, v1.plain(), v2.plain());
}
template<typename T1, typename T2, typename T3>
Variant NewObject(const string &type, T1 p1, T2 p2, T3 p3)
{
    Variant v1(p1), v2(p2), v3(p3);
    return __NewObject<void>(type, 3, v1.plain(), v2.plain(), v3.plain());
}


} // namespace E8

template<typename T>
std::basic_ostream<T> &operator << (std::basic_ostream<T> &stream, const E8::string &utext)
{
    stream << utext.to_string(E8_DEFAULT_OUTPUT_ENCODING);
    return stream;
}

template<typename T>
std::basic_ostream<T> &operator << (std::basic_ostream<T> &stream, const E8::Variant &value)
{
    stream << value.to_string();
    return stream;
}

template<typename Type>
bool operator < (const E8::Variant &A, Type B)
{
    E8::Variant V(B);
    return A < V;
}

template<typename Type>
bool operator > (const E8::Variant &A, Type B)
{
    E8::Variant V(B);
    return A > V;
}

template<typename Type>
bool operator <= (const E8::Variant &A, Type B)
{
    E8::Variant V(B);
    return A <= V;
}

template<typename Type>
bool operator >= (const E8::Variant &A, Type B)
{
    E8::Variant V(B);
    return A >= V;
}

template<typename Type>
E8::Variant operator + (const E8::Variant &A, Type B)
{
    E8::Variant R(A), Value(B);
    return R + Value;
}

template<typename Type>
E8::Variant operator - (const E8::Variant &A, Type B)
{
    E8::Variant R(A), Value(B);
    return R - Value;
}

template<typename Type>
E8::Variant operator * (const E8::Variant &A, Type B)
{
    E8::Variant R(A), Value(B);
    return R * Value;
}

template<typename Type>
E8::Variant operator / (const E8::Variant &A, Type B)
{
    E8::Variant R(A), Value(B);
    return R / Value;
}

template<typename Type>
E8::Variant operator % (const E8::Variant &A, Type B)
{
    E8::Variant R(A), Value(B);
    return R % Value;
}


#endif // __cplusplus

#endif // E8_CORE_PLUGIN_HPP_INCLUDED
