#ifndef BUILTINS_H_
#define BUILTINS_H_

#include "parser.h"

#define SKIP_SIG (uint32_t)-5
#define CALL_SIG (uint32_t)-6

void iload_builtins(instance *it);

int bn_end(instance *it);

#endif // BUILTINS_H_
