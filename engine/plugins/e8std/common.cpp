#include "common.hpp"

bool IsDate(E8_IN Value)
{
    return Value.of_type(varDateTime);
}

bool IsString(E8_IN Value)
{
    return Value.of_type(varString);
}

bool IsBool(E8_IN Value)
{
    return Value.of_type(varBool);
}

bool IsNumber(E8_IN Value)
{
    return Value.of_type(varNumeric);
}


bool IsScalar(E8_IN Value)
{
    return IsDate(Value) || IsString(Value) || IsBool(Value) || IsNumber(Value);
}

bool IsObject(E8_IN Value)
{
    return Value.of_type(varObject);
}


