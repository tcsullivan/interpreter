#ifndef STACK_H_
#define STACK_H_

#include "parser.h"

void ipush(interpreter *it, void *v);
void *ipop(interpreter *it);
void ipopm(interpreter *it, uint32_t count);
variable *igetarg(interpreter *interp, uint32_t index);
char *igetarg_string(interpreter *interp, uint32_t index);
int igetarg_integer(interpreter *interp, uint32_t index);
float igetarg_float(interpreter *interp, uint32_t index);


#endif // STACK_H_
