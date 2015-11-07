#ifndef E8_CORE_ENV_SHARED_LIBS
#define E8_CORE_ENV_SHARED_LIBS

#include "env.h"

e8_runtime_error *
e8_env_unit_from_shared_library(e8_environment env, const char *path, e8_unit *unit);

extern e8_vtable vmt_shared;

#endif // E8_CORE_ENV_SHARED_LIBS
