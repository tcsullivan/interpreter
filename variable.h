#ifndef VARIABLE_H_
#define VARIABLE_H_

#include <stdint.h>

#define INT(v)   (*((int32_t *)&v->value))
#define FLOAT(v) (*((float *)&v->value))

typedef struct {
	uint8_t used :1;
	uint8_t fromc :1;
	uint8_t valtype :4;
	uint32_t value;
	char *svalue;
} variable;

enum valtype {
	STRING = 0,
	INTEGER,
	FLOAT,
	FUNC,
	EXPR
};

variable *vmake(uint8_t fromc, uint8_t valtype, void *value);
variable *vmakef(float value);

void isetstr(variable *i);
void fsetstr(variable *f);

variable *itostring(variable *v);
variable *itoint(variable *v);
variable *itofloat(variable *v);

#endif // VARIABLE_H_
