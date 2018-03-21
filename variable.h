#ifndef VARIABLE_H_
#define VARIABLE_H_

#include <stdint.h>

typedef struct {
	uint8_t tmp    :1;
	uint8_t type   :3;
	uint8_t unused :4;
	union {
		float f;
		uint32_t p;
	} value;
} variable;

enum VARTYPE {
	NUMBER = 0,
	STRING,
	OPERATOR,
	FUNC,
	CFUNC,
};

#endif // VARIABLE_H_
