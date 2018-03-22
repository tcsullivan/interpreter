#ifndef PARSER_H_
#define PARSER_H_

#include "variable.h"

#include <stdint.h>

typedef struct {
	variable *vars;
	char **names;
	uint32_t *stack;
	uint32_t stidx;
	variable ***lines;
	uint32_t lnidx;
	variable *ret;
	uint8_t indent;
	uint8_t sindent;
} instance;

#define SKIP (1 << 7)

typedef int (*func_t)(instance *);

instance *inewinstance(void);
void idelinstance(instance *it);

int idoline(instance *it, const char *s);
variable **iparse(instance *it, const char *s);
variable *isolve(instance *it, variable **ops, uint32_t count);

void inew_cfunc(instance *it, const char *name, func_t func);

variable *make_varf(variable *v, float f);
variable *make_vars(variable *v, const char *s);

uint32_t ipop(instance *it);
void ipush(instance *it, uint32_t v);
variable *igetarg(instance *it, uint32_t n);

#endif // PARSER_H_
