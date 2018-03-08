#ifndef VARIABLE_H_
#define VARIABLE_H_

#include <stdint.h>

typedef struct {
	uint8_t used :1;
	uint8_t fromc :1;
	uint8_t valtype :4;
	union {
		float f;
		uint32_t p;
	} value;
} variable;

enum valtype {
	STRING = 0,
	NUMBER,
	FUNC,
	EXPR
};

variable *make_varn(variable *v, float value);
variable *make_vars(variable *v, const char *s);
variable *make_varf(variable *v, uint8_t fromc, uint32_t func);
variable *make_vare(variable *v, const char *e);

int try_number(variable *v, const char *text);
int try_variable(char **name, const char *text);

#endif // VARIABLE_H_
