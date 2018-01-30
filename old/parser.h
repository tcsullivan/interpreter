#ifndef PARSER_H_
#define PARSER_H_

#include <variable.h>

typedef void *stack_t;

typedef struct {
	uint16_t status;
	uint16_t vcount;
	variable *vars;
	char **names;
	stack_t *stack;
} interpreter;

enum status {
	READY = 0
};

typedef void (*func_t)(stack_t *);

void interpreter_init(interpreter *);

void interpreter_define_value(interpreter *, const char *, int32_t);
void interpreter_define_cfunc(interpreter *, const char *, func_t);

int32_t interpreter_get_value(interpreter *, const char *);

int interpreter_doline(interpreter *, const char *);

#endif // PARSER_H_
