#ifndef PARSER_H_
#define PARSER_H_

#include <variable.h>

typedef variable *stack_t;

typedef struct {
	variable *vars;
	char **vnames;
	stack_t *stack;
	uint32_t stidx;
} interpreter;

typedef void (*func_t)(interpreter *);

void iinit(interpreter *);

void inew_string(interpreter *, const char *, char *);
void inew_integer(interpreter *, const char *, int32_t);
void inew_float(interpreter *, const char *, float);
void inew_cfunc(interpreter *, const char *, func_t);

int idoline(interpreter *, const char *);

char *igetarg_string(interpreter *, uint32_t);
int igetarg_integer(interpreter *, uint32_t); 
float igetarg_float(interpreter *, uint32_t);

char *iget_string(interpreter *, const char *);
int iget_integer(interpreter *, const char *);
float iget_float(interpreter *, const char *);

#endif // PARSER_H_
