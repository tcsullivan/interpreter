#ifndef BUILTINS_H_
#define BUILTINS_H_

#include "parser.h"

#define IUP_COUNT 3
#define IDOWN_COUNT 2

void iload_core(interpreter *it);

const func_t indent_up[IUP_COUNT];
const func_t indent_down[IDOWN_COUNT];

#endif // BUILTINS_H_
