#ifndef PARSER_H_
#define PARSER_H_

#include <variable.h>

typedef variable *stack_t;

typedef struct {
	variable *vars;
	char **vnames;
	stack_t *stack;
	uint32_t stidx;
	variable ***lines;
	uint32_t lnidx;
	int8_t indent;
	uint8_t sindent;
	variable *ret;
} interpreter;

#define SKIP (1 << 7)

typedef int (*func_t)(interpreter *);

void iinit(interpreter *);
void iend(interpreter *it);

void iskip(interpreter *it);

variable *inew_string(interpreter *, const char *, char *);
variable *inew_integer(interpreter *, const char *, int32_t);
variable *inew_float(interpreter *, const char *, float);
void inew_cfunc(interpreter *, const char *, func_t);

int idoline(interpreter *, const char *);

#endif // PARSER_H_
