#ifndef E8_ENV_INSTRUCTIONS
#define E8_ENV_INSTRUCTIONS

#include "e8core/variant/variant.h"
#include "e8core/utf/utf.h"
#include "unit.h"
#include "_strings.h"


typedef e8_runtime_error* (*__e8_instruction_command)(e8_unit_runtime_info_t *rti);
#define DECLARE_OPERATOR(name) e8_runtime_error* name(e8_unit_runtime_info_t *rti)

__e8_instruction_command ops[E8_COMMAND_OperandMax];

DECLARE_OPERATOR(__e8_operator_nop);
DECLARE_OPERATOR(__e8_operator_Raise);
DECLARE_OPERATOR(__e8_operator_Critical);
DECLARE_OPERATOR(__e8_operator_EndCritical);
DECLARE_OPERATOR(__e8_operator_Assert);
DECLARE_OPERATOR(__e8_operator_Table);
DECLARE_OPERATOR(__e8_operator_Pop);
DECLARE_OPERATOR(__e8_operator_Iterator);
DECLARE_OPERATOR(__e8_operator_Jump);
DECLARE_OPERATOR(__e8_operator_JumpTrue);
DECLARE_OPERATOR(__e8_operator_JumpFalse);
DECLARE_OPERATOR(__e8_operator_Const);
DECLARE_OPERATOR(__e8_operator_LoadRef);
DECLARE_OPERATOR(__e8_operator_DeclareVar);
DECLARE_OPERATOR(__e8_operator_LoadRefIndex);
DECLARE_OPERATOR(__e8_operator_Call);
DECLARE_OPERATOR(__e8_operator_ObjectCall);
DECLARE_OPERATOR(__e8_operator_StackDup);

DECLARE_OPERATOR(__e8_operator_DeclareParamByRef);
DECLARE_OPERATOR(__e8_operator_DeclareParamByVal);

DECLARE_OPERATOR(__e8_operator_Add);
DECLARE_OPERATOR(__e8_operator_Sub);
DECLARE_OPERATOR(__e8_operator_Mul);
DECLARE_OPERATOR(__e8_operator_Div);
DECLARE_OPERATOR(__e8_operator_Neg);
DECLARE_OPERATOR(__e8_operator_Assign);
DECLARE_OPERATOR(__e8_operator_Mod);
DECLARE_OPERATOR(__e8_operator_Or);
DECLARE_OPERATOR(__e8_operator_And);
DECLARE_OPERATOR(__e8_operator_Not);
DECLARE_OPERATOR(__e8_operator_Equal);
DECLARE_OPERATOR(__e8_operator_NotEqual);
DECLARE_OPERATOR(__e8_operator_Less);
DECLARE_OPERATOR(__e8_operator_Greater);
DECLARE_OPERATOR(__e8_operator_LessEqual);
DECLARE_OPERATOR(__e8_operator_GreaterEqual);
DECLARE_OPERATOR(__e8_operator_Inc);
DECLARE_OPERATOR(__e8_operator_Dec);
DECLARE_OPERATOR(__e8_operator_Index);

#endif // E8_ENV_INSTRUCTIONS
