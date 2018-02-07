#ifndef OPS_H_
#define OPS_H_

#include "parser.h"

#define IOPS_COUNT 16

typedef void (*operation_t)(variable *, variable *, variable *);

extern char *iops[IOPS_COUNT];
extern operation_t iopfuncs[IOPS_COUNT];

#endif // OPS_H_
