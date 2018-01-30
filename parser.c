#include <parser.h>

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#define MAX_VARS  16
#define MAX_STACK 32

char *strclone(const char *s)
{
	char *clone = (char *)malloc(strlen(s));
	strcpy(clone, s);
	return clone;
}

char *strnclone(const char *s, uint32_t n)
{
	char *clone = (char *)malloc(n);
	strncpy(clone, s, n);
	return clone;
}

void iinit(interpreter *interp)
{
	interp->vars = (variable *)calloc(MAX_VARS, sizeof(variable));
	interp->vnames = (char **)calloc(MAX_VARS, sizeof(char *));
	interp->stack = (stack_t *)calloc(MAX_STACK, sizeof(stack_t));
	interp->stidx = 0;
}

variable *interpreter_get_variable(interpreter *interp, const char *name)
{
	for (uint32_t i = 0; i < MAX_VARS; i++) {
		if (!interp->vars[i].used) {
			interp->vars[i].used = 1;
			interp->vars[i].valtype = FUNC;
			interp->vars[i].value = 0;
			interp->vnames[i] = strclone(name);
			return &interp->vars[i];
		} else if (interp->vnames[i] != 0 && !strcmp(interp->vnames[i], name)) {
			return &interp->vars[i];
		}
	}
	return 0;
}

void inew_string(interpreter *interp, const char *name, char *value)
{
	variable *v = interpreter_get_variable(interp, name);
	if (v != 0) {
		v->valtype = STRING;
		v->value = (uint32_t)value;
	}
}

void inew_integer(interpreter *interp, const char *name, int32_t value)
{
	variable *v = interpreter_get_variable(interp, name);
	if (v != 0) {
		v->valtype = INTEGER;
		v->value = (uint32_t)value;
	}
}

void inew_float(interpreter *interp, const char *name, float value)
{
	variable *v = interpreter_get_variable(interp, name);
	if (v != 0) {
		v->valtype = FLOAT;
		v->value = (uint32_t)value;
	}
}

void inew_cfunc(interpreter *interp, const char *name, func_t func)
{
	variable *v = interpreter_get_variable(interp, name);
	if (v != 0) {
		v->fromc = 1;
		v->valtype = FUNC;
		v->value = (uint32_t)func;
	}
}

uint8_t eol(int c)
{
	return c == '\n' || c == '\0';
}

uint8_t eot(int c)
{
	return eol(c) || c == ' ';
}

variable *make_var(interpreter *interp, const char *line, uint32_t *next)
{
	if (line[0] == '\"') { // string literal
		uint32_t end = 1;
		while (!eol(line[end])) {
			if (line[end] == '\"'/* && line[end - 1] != '\\'*/) {
				if (!eot(line[end + 1]))
					return 0;
				// TODO string breakdown
				variable *v = (variable *)malloc(sizeof(variable));
				v->valtype = STRING;
				v->value = (uint32_t)strnclone(line + 1, end - 1);
				*next = end + 1;
				return v;
			}
			end++;
		}
		return 0;
	} else if (line[0] == '(') { // equation literal
		uint32_t end = 1;
		while (!eol(line[end])) {
			if (line[end] == ')' && line[end - 1] != '\\') {
				if (!eot(line[end + 1]))
					return 0;
				// TODO string breakdown
				// TODO make var
				return 0;
			}
			end++;
		}
	} else if (isalpha(line[0])) { // variable/func
		uint32_t end = 1;
		for (; isalnum(line[end]); end++);
		if (!eot(line[end]))
			return 0;
		char *name = (char *)malloc(end + 1);
		strncpy(name, line, end);
		name[end] = '\0';
		*next = end;
		return interpreter_get_variable(interp, name);
	} else if (isdigit(line[0])) { // number
		uint32_t end = 1;
		uint8_t dec = 0;
		for (; !eot(line[end]); end++) {
			if (!isdigit(line[end])) {
				if (line[end] == '.') {
					if (!dec)
						dec = 1;
					else
						return 0;
				}
				return 0;
			}
		}
		variable *v = (variable *)malloc(sizeof(variable));
		v->valtype = INTEGER;
		v->value = atoi(line);
		*next = end;
		return v;
	}
	return 0;
}

int idoline(interpreter *interp, const char *line)
{
	variable *ops[8];
	uint32_t ooffset = 0;
	uint32_t offset = 0;
	uint32_t next;

	// step 1 - convert to tokens
	while (!eol(line[offset])) {
		ops[ooffset] = make_var(interp, line + offset, &next);
		if (ops[ooffset] == 0) {
			return -4;
		} else {
			ooffset++;
			offset += next;
		}

		// skip whitespace
		for (; line[offset] == ' ' && !eol(line[offset]); offset++);
	}

	// step 2 - execute
	if (ooffset == 0)
		return -1;

	if (ops[0]->valtype != FUNC)
		return -2;

	if (ops[0]->value == 0)
		return -3;

	for (uint32_t i = 0; i < ooffset - 1; i++, interp->stidx++)
		interp->stack[i] = ops[i + 1];

	((func_t)ops[0]->value)(interp);

	return 0;
}

char *igetarg_string(interpreter *interp, uint32_t index)
{
	if (index >= interp->stidx)
		return 0;
	if (interp->stack[index]->valtype != STRING)
		return 0;
	return (char *)interp->stack[index]->value;
}

int igetarg_integer(interpreter *interp, uint32_t index)
{
	if (index >= interp->stidx)
		return 0;
	if (interp->stack[index]->valtype != INTEGER)
		return 0;
	return (int)interp->stack[index]->value;
}

float igetarg_float(interpreter *interp, uint32_t index)
{
	if (index >= interp->stidx)
		return 0;
	if (interp->stack[index]->valtype != FLOAT)
		return 0;
	return (float)interp->stack[index]->value;
}

/*char *iget_string(interpreter *, const char *)
{

}

int iget_integer(interpreter *, const char *)
{

}

float iget_float(interpreter *, const char *)
{

}*/

