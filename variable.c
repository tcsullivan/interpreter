#include "variable.h"
#include "parser.h"

#include <ctype.h>
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

int try_variable(char **name, const char *text)
{
	if (name == 0)
		return 0;

	int neg = 1;
	int i = 0;

	if (text[0] == '-') {
		neg = -1;
		i++;
	}
	if (!isalpha(text[i]))
		return 0;

	for (i++; isalnum(text[i]); i++);

	int o = (neg < 0);
	if (neg < 0)
		i--;
	*name = (char *)malloc(i + 1);
	strncpy(*name, text + o, i);
	(*name)[i] = '\0';
	return (neg > 0) ? i : -(i + 1);
}

int try_number(variable *v, const char *text)
{
	if (v == 0)
		return 0;

	int decimal = -1;
	char valid = 0;

	int i = 0;
	if (text[0] == '-')
		i++;
	do {
		if (text[i] == '.') {
			if (decimal >= 0) {
				valid = 0;
				break;
			}
			decimal = i;
		} else if (isdigit(text[i])) {
			valid |= 1;
		} else {
			break;
		}
	} while (text[++i] != '\0');

	if (valid == 0)
		return 0;

	char *buf = (char *)malloc(i + 1);
	strncpy(buf, text, i);
	buf[i] = '\0';

	make_varn(v, strtof(buf, 0));

	free(buf);
	return i;
}
