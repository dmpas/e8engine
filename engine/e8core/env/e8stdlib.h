/*! \file */
#ifndef E8_CORE_ENVIRONMENT_STDLIB
#define E8_CORE_ENVIRONMENT_STDLIB

#include "env.h"

/*! Включить в среду исполнения стандартые типы и методы */
void e8_register_stdlib(e8_environment env);

/*! Включить в среду исполнения стандартные методы ввода-вывода */
void e8_register_stdio(e8_environment env);

#endif // E8_CORE_ENVIRONMENT_STDLIB
