#ifndef TOKEN_H_
#define TOKEN_H_

#include <stdint.h>

typedef struct {
	uint16_t nameidx;
	uint8_t type;
	uint8_t info;
	uint32_t value;
} variable;

#define INFO_ARGS(x)	((x) & 0x07)
#define INFO_RET	(1 << 3)

enum vartype {
	VALUE = 0,
	VARIABLE,
	OPERATOR,
	FUNCTION,
	CFUNCTION
};

#endif // TOKEN_H_
