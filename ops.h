#ifndef OPS_H_
#define OPS_H_

#include "variable.h"

#define OPS_COUNT 18
#define OP_MAGIC  0xCAFE3900

typedef int (*opfunc_t)(variable *, variable *, variable *);

extern variable opvars[];
extern const char *opnames[];

#endif // OPS_H_
