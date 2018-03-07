#include "variable.h"
#include "parser.h"

#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <shelpers.h>

extern int atoi(const char *);

char *fixstring(char *s)
{
	char *n = malloc(strlen(s) + 1);
	int j = 0;
	for (int i = 0; s[i] != '\0'; i++, j++) {
		if (s[i] == '\\') {
			if (s[i + 1] == 'n')
				n[j] = '\n';
			i++;
		} else {
			n[j] = s[i];
		}
	}
	n[j] = '\0';
	return n;
}

variable *make_varn(variable *v, float value)
{
	if (v == 0)
		v = (variable *)malloc(sizeof(variable));
	v->used = 0;
	v->fromc = 0;
	v->valtype = NUMBER;
	v->value.f = value;
	return v;
}

variable *make_vars(variable *v, const char *value)
{
	if (v == 0)
		v = (variable *)malloc(sizeof(variable));
	v->used = 0;
	v->fromc = 0;
	v->valtype = STRING;
	v->value.p = (value != 0) ? (uint32_t)fixstring(value) : 0;
	return v;
}

variable *make_varf(variable *v, uint8_t fromc, uint32_t func)
{
	if (v == 0)
		v = (variable *)malloc(sizeof(variable));
	v->used = 0;
	v->fromc = fromc;
	v->valtype = FUNC;
	v->value.p = func;
	return v;
}

variable *make_vare(variable *v, const char *expr)
{
	if (v == 0)
		v = (variable *)malloc(sizeof(variable));
	v->used = 0;
	v->fromc = 0;
	v->valtype = EXPR;
	v->value.p = (uint32_t)strclone(expr);
	return v;
}

