#ifndef VARIABLE_H_
#define VARIABLE_H_

#include <stdint.h>

typedef struct {
	uint8_t used :1;
	uint8_t fromc :1;
	uint8_t valtype :2;
	uint32_t value;
	char *svalue;
} variable;

enum valtype {
	STRING = 0,
	INTEGER,
	FLOAT,
	FUNC
};

#endif // VARIABLE_H_
