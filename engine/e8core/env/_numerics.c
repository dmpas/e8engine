#include "_numerics.h"
#include "env_p.h"
#include <math.h>
#include "simpleenum.h"
#include <malloc.h>
#include "utils.h"


static e8_var v_round_mode;

#define _ENUM_ROUND_15_AS_10 0
#define _ENUM_ROUND_15_AS_20 1

static e8_vtable vmt_round_modes;
E8_ENUM_START(round_modes)
    E8_ENUM_ADD("Окр15Как10", "Round15As10", _ENUM_ROUND_15_AS_10)
    E8_ENUM_ADD("Окр15Как20", "Round15As20", _ENUM_ROUND_15_AS_20)
E8_ENUM_END

static double frac(double x)
{
    return fabs(x - trunc(x));
}

static E8_DECLARE_SUB(Int)
{
    E8_ARG_DOUBLE(n);
    e8_var_long(result, trunc(n));
    E8_RETURN_OK;
}
static E8_DECLARE_SUB(Round)
{
    E8_ARG_DOUBLE(n);
    double base = 1;
    if (argc > 1) {

        E8_ARG_LONG(k);
        if (k < 0)
            while (k++)
                base /= 10;
        else
            while (k--)
                base *= 10;

    }
    int mode = 0;
    if (argc > 2) {

        E8_ARGUMENT(v_rv);

        if (e8_simple_enum_find_value(&vmt_round_modes, &v_rv) == _ENUM_ROUND_15_AS_10)
            mode = 1;
    }
    double res = n*base;
    if (mode && frac(res) == 0.5)
        res = trunc(res);

    e8_var_double(result, round(res) / base);

    E8_RETURN_OK;
}
static E8_DECLARE_SUB(Log)
{
    E8_ARG_DOUBLE(n);
    e8_var_double(result, log(n));
    E8_RETURN_OK;
}
static E8_DECLARE_SUB(Log10)
{
    E8_ARG_DOUBLE(n);
    e8_var_double(result, log10(n));
    E8_RETURN_OK;
}
static E8_DECLARE_SUB(Sin)
{
    E8_ARG_DOUBLE(n);
    e8_var_double(result, sin(n));
    E8_RETURN_OK;
}
static E8_DECLARE_SUB(Cos)
{
    E8_ARG_DOUBLE(n);
    e8_var_double(result, cos(n));
    E8_RETURN_OK;
}

static E8_DECLARE_SUB(Tan)
{
    E8_ARG_DOUBLE(n);
    e8_var_double(result, tan(n));
    E8_RETURN_OK;
}
static E8_DECLARE_SUB(ASin)
{
    E8_ARG_DOUBLE(n);
    e8_var_double(result, asin(n));
    E8_RETURN_OK;
}
static E8_DECLARE_SUB(ACos)
{
    E8_ARG_DOUBLE(n);
    e8_var_double(result, acos(n));
    E8_RETURN_OK;
}
static E8_DECLARE_SUB(ATan)
{
    E8_ARG_DOUBLE(n);
    e8_var_double(result, atan(n));
    E8_RETURN_OK;
}
static E8_DECLARE_SUB(Exp)
{
    E8_ARG_DOUBLE(n);
    e8_var_double(result, exp(n));
    E8_RETURN_OK;
}
static E8_DECLARE_SUB(Pow)
{
    E8_ARG_DOUBLE(n);
    E8_ARG_DOUBLE(p);
    e8_var_double(result, pow(n, p));
    E8_RETURN_OK;
}
static E8_DECLARE_SUB(Sqrt)
{
    E8_ARG_DOUBLE(n);
    e8_var_double(result, sqrt(n));
    E8_RETURN_OK;
}

static E8_DECLARE_SUB(CastNumber)
{
    E8_ARGUMENT(value);

    if (value.type == varBool) {
        e8_var_long(result, 1);
        E8_RETURN_OK;
    }

    if (value.type == varString) {

        char *buf = 0;
        e8_utf_to_8bit(value.str->s, &buf, "utf-8");

        double v;
        sscanf(buf, "%lf", &v);
        free(buf);

        e8_var_double(result, v);

        e8_var_destroy(&value);
        E8_RETURN_OK;
    }

    E8_THROW_DETAIL(E8_RT_CANNOT_CAST_TO_NUMERIC, e8_usprintf(NULL, "%v", value));
}


#define REGISTER(name, fn) \
{\
    e8_env_add_function_utf(env, name, fn); \
}


void e8_register_numerics(e8_environment env)
{
    e8_simple_enum_init_vtable(&vmt_round_modes);
    e8_simple_enum_new(&v_round_mode, &vmt_round_modes);
    e8_simple_enum_attach_list(&v_round_mode, round_modes);

    e8_env_add_global_value_utf(env, "РежимОкругления", &v_round_mode);
    e8_env_add_global_value_utf(env, "RoundMode", &v_round_mode);

    e8_var_destroy(&v_round_mode);

    REGISTER("Цел", Int);
    REGISTER("Окр", Round);

    REGISTER("Int", Int);
    REGISTER("Round", Round);
    REGISTER("Log", Log);
    REGISTER("Log10", Log10);

    REGISTER("Sin", Sin);
    REGISTER("Cos", Cos);
    REGISTER("Tan", Tan);

    REGISTER("ASin", ASin);
    REGISTER("ACos", ACos);
    REGISTER("ATan", ATan);

    REGISTER("Exp", Exp);
    REGISTER("Pow", Pow);
    REGISTER("Sqrt",Sqrt);

    REGISTER("Число", CastNumber);
    REGISTER("Number", CastNumber);
}

#undef REGISTER
