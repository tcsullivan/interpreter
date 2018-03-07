#ifndef STACK_H_
#define STACK_H_

#include "parser.h"

void ipush(interpreter *it, void *v);
void *ipop(interpreter *it);
void ipopm(interpreter *it, uint32_t count);

variable *igetarg(interpreter *interp, uint32_t index);
const char *igetarg_string(interpreter *interp, uint32_t index);
float igetarg_number(interpreter *interp, uint32_t index);

#define igetarg_integer(i, x) (int)igetarg_number(i, x)

#endif // STACK_H_
