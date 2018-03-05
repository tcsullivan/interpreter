#include <parser.h>

#include "shelpers.h"
#include "builtins.h"
#include "stack.h"
#include "ops.h"

#include <ctype.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>

#define MAX_VARS  64
#define MAX_STACK 32
#define MAX_LINES 1000

extern int atoi(const char *);
extern float strtof(const char *, char **);

char *str_func = "(func)";
char *str_undef = "(undefined)";

void iinit(interpreter *interp)
{
	interp->vars = (variable *)calloc(MAX_VARS, sizeof(variable));
	interp->vnames = (char **)calloc(MAX_VARS, sizeof(char *));
	interp->stack = (stack_t *)calloc(MAX_STACK, sizeof(stack_t));
	interp->stidx = 0;
	interp->lines = (variable ***)calloc(MAX_LINES, sizeof(variable **));
	interp->lnidx = 0;
	interp->indent = 0;
	interp->sindent = 0;
	interp->ret = 0;

	iload_core(interp);
}

void iend(interpreter *it)
{
	for (unsigned int i = 0; i < MAX_VARS; i++) {
		if (it->vars[i].used == 1) {
			//char *s = it->vars[i].svalue;
			//if (s != 0 && s != str_undef && s != str_func)
			//	free(s);
			free(it->vnames[i]);
		}
	}
	for (unsigned int i = 0; i < MAX_LINES; i++)
		free(it->lines[i]);
	free(it->vars);
	free(it->vnames);
	free(it->stack);
	free(it->lines);
}

void iskip(interpreter *it)
{
	if (!(it->sindent & SKIP))
		it->sindent = it->indent | SKIP;
}

variable *interpreter_get_variable(interpreter *interp, const char *name)
{
	for (uint32_t i = 0; i < MAX_VARS; i++) {
		if (!interp->vars[i].used) {
			variable *v = &interp->vars[i];
			v->used = 1;
			v->fromc = 0;
			v->valtype = STRING;
			v->value = 0;
			v->svalue = str_undef;
			char *s = strclone(name);
			interp->vnames[i] = s;
			return v;
		} else if (interp->vnames[i] != 0 && !strcmp(interp->vnames[i], name)) {
			return &interp->vars[i];
		}
	}
	return 0;
}

char *interpreter_get_name(interpreter *interp, variable *v)
{
	for (uint32_t i = 0; i < MAX_VARS; i++) {
		if (v == &interp->vars[i])
			return interp->vnames[i];
	}
	return str_undef;
}

variable *inew_string(interpreter *interp, const char *name, char *value)
{
	variable *v = interpreter_get_variable(interp, name);
	if (v != 0) {
		v->valtype = STRING;
		INT(v) = 0;
		v->svalue = strclone(value);
	}
	return v;
}

variable *inew_integer(interpreter *interp, const char *name, int32_t value)
{
	variable *v = interpreter_get_variable(interp, name);
	if (v != 0) {
		v->valtype = INTEGER;
		INT(v) = value;
		isetstr(v);
	}
	return v;
}

variable *inew_float(interpreter *interp, const char *name, float value)
{
	variable *v = interpreter_get_variable(interp, name);
	if (v != 0) {
		v->valtype = FLOAT;
		FLOAT(v) = value;
		fsetstr(v);
	}
	return v;
}

void inew_cfunc(interpreter *interp, const char *name, func_t func)
{
	variable *v = interpreter_get_variable(interp, name);
	if (v != 0) {
		v->fromc = 1;
		v->valtype = FUNC;
		v->value = (uint32_t)func;
		v->svalue = str_func;
	}
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
				*next = end + 1;
				return vmake(0, STRING, strnclone(line + 1, end - 1));
			}
			end++;
		}
		return 0;
	} else if (line[0] == '(') { // equation literal
		uint32_t end = findend(line, '(', ')');
		if (eot(line[end]))
			return 0;
		*next = end + 1;
		return vmake(0, EXPR, strnclone(line + 1, end));
		//return idoexpr(interp, line + 1);
	} else if (isalpha(line[0])) { // variable/func
		uint32_t end = 1;
		for (; isalnum(line[end]); end++);
		if (!eot(line[end]))
			return 0;
		char *name = (char *)malloc(end + 1);
		strncpy(name, line, end);
		name[end] = '\0';
		*next = end;
		variable *v = interpreter_get_variable(interp, name);
		free(name);
		return v;
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
				} else {
					return 0;
				}
			}
		}
		char *copy = (char *)malloc(end + 1);
		strncpy(copy, line, end);
		copy[end] = '\0';
		variable *v;
		if (dec)
			v = vmakef(strtof(copy, 0));
		else
			v = vmake(0, INTEGER, (void *)atoi(copy));
		free(copy);
		*next = end;
		return v;
	}
	return 0;
}

int idoline(interpreter *interp, const char *line)
{
	uint32_t ooffset = 0, offset = 0, next;
	int fret = 0;

	if (line[0] == '\0')
		return 0;
	skipblank(line, eol, &offset);
	if (line[offset] == '#' || eol(line[offset]))
		return 0;

	variable **linebuf = (variable **)calloc(8, sizeof(variable *));
	interp->lines[interp->lnidx] = linebuf;
	variable **ops = interp->lines[interp->lnidx];

	// step 1 - convert to tokens
	while (!eol(line[offset])) {
		if (offset > 0 && line[offset] == '>') {
			offset++;
			skipblank(line, eol, &offset);
			variable *r = make_var(interp, line + offset, &next);
			ops[ooffset] = (void *)-1;
			ops[ooffset + 1] = r;
			offset += next;
			skipblank(line, eol, &offset);
			continue;
		}
		variable *v = make_var(interp, line + offset, &next);
		ops[ooffset] = v;
		if (ops[ooffset] == 0) {
			fret = -4;
			goto fail;
		} else {
			ooffset++;
			offset += next;
		}
		skipblank(line, eol, &offset);
	}

	// step 2 - execute
	if (ooffset == 0) {
		fret = -1;
		goto fail;
	}

	if (ops[0]->valtype != FUNC) {
		fret = -2;
		goto fail;
	}

	if (ops[0]->fromc && ops[0]->value == 0) {
		fret = -3;
		goto fail;
	}

	if (ops[ooffset] != (void *)-1)
		ops[ooffset] = 0;

loop:
	for (uint8_t i = 0; i < IUP_COUNT; i++) {
		if (interp->lines[interp->lnidx][0]->value
			== (uint32_t)indent_up[i]) {
			interp->indent++;
			goto cont;
		}
	}
	for (uint8_t i = 0; i < IDOWN_COUNT; i++) {
		if (interp->lines[interp->lnidx][0]->value
			== (uint32_t)indent_down[i]) {
			if (--interp->indent < 0) {
				fret = -6;
				goto fail;
			}
			if (interp->indent < (interp->sindent & ~(SKIP)))
				interp->sindent &= ~(SKIP);
			else
				goto cont;
			break;
		}
	}

cont:
	if (interp->indent > 0 && interp->sindent & SKIP)
		goto norun;

	ops = (variable **)malloc(8 * sizeof(variable *));
	for (uint8_t i = 0; i < 8; i++)
		ops[i] = interp->lines[interp->lnidx][i];
	uint32_t oldLnidx = interp->lnidx;
	
	// eval expressions
	ooffset = 1;
	for (; ops[ooffset] != 0 && ops[ooffset] != (void *)-1; ooffset++) {
		if (ops[ooffset]->valtype == EXPR) {
			char *expr = strclone(ops[ooffset]->svalue);
			variable *r = idoexpr(interp, expr);
			ops[ooffset] = r;
			free(expr);
		}
	}

	if (ops[ooffset] == (void *)-1)
		interp->ret = ops[ooffset + 1];

	if (ops[0]->fromc) {
		for (uint32_t i = ooffset; --i > 0;)
			ipush(interp, ops[i]);

		int ret = ((func_t)ops[0]->value)(interp);
		if (ret != 0)
			return ret;
		ipopm(interp, ooffset - 1);
	} else {
		char an[6];
		for (uint32_t i = 1; i < ooffset; i++) {
			snprintf(an, 6, "arg%d", (int)(i - 1));
			switch (ops[i]->valtype) {
			case STRING:
				inew_string(interp, an, ops[i]->svalue);
				break;
			case INTEGER:
				inew_integer(interp, an, INT(ops[i]));
				break;
			case FLOAT:
				inew_float(interp, an, FLOAT(ops[i]));
				break;
			default:
				break;
			}
		}

		ipush(interp, (void *)(interp->lnidx));
		interp->lnidx = ops[0]->value;
		interp->indent++;
	}

	if ((int32_t)interp->stidx < 0) {
		interp->stidx = 0;
		return -5;
	}

	for (uint32_t i = 1; i < ooffset; i++) {
		if (ops[i] != interp->lines[oldLnidx][i]) {
			free(ops[i]->svalue);
			free(ops[i]);
		}
	}
	free(ops);
norun:
	interp->lnidx++;
	if (interp->lines[interp->lnidx] != 0)
		goto loop;

	return 0;

fail:
	free(interp->lines[interp->lnidx]);
	interp->lines[interp->lnidx] = 0;
	return fret;
}

variable *idoexpr(interpreter *interp, const char *line)
{
	void *ops[16];
	uint32_t ooffset = 0;
	uint32_t offset = 0;
	uint32_t next = 0;

	// step 1 - break apart line

	// skip whitespace
	skipblank(line, eol, &offset);
	while (!eoe(line[offset])) {
		if (line[offset] == '(') {
			uint8_t indent = 0;
			uint32_t i;
			for (i = offset + 1; !eol(line[i]); i++) {
				if (line[i] == '(') {
					indent++;
				} else if (line[i] == ')') {
					if (indent == 0) {
						break;
					} else {
						indent--;
					}
				}
			}
			if (eol(line[i]))
				return 0;
			ops[ooffset] = idoexpr(interp, line + offset + 1);
			offset = i + 1;
		} else {
			uint32_t len = offset;
			for (; isalnum(line[len]) || line[len] == '.'; len++);
			len -= offset;
			char *copy = (char *)malloc(len + 1);
			strncpy(copy, line + offset, len);
			copy[len] = '\0';
			variable *v = make_var(interp, copy, &next);
			free(copy);
			ops[ooffset] = v;
			offset += next;
		}
		if (ops[ooffset] == 0)
			return 0;
		
		ooffset++;

		// skip whitespace
		skipblank(line, eoe, &offset);
		if (eoe(line[offset]))
			break;

		for (uint32_t i = 0; i < IOPS_COUNT; i++) {
			int len = strlen(iops[i]);
			if (!strncmp(iops[i], line + offset, len)) {
				ops[ooffset] = (void *)(i + 1);
				offset += len;
				break;
			}
		}
		if (ops[ooffset] == 0)
			return 0;
		ooffset++;

		// skip whitespace
		skipblank(line, eol, &offset);
	}

	if (ooffset % 2 == 0)
		return 0;

	// step 2 - do operations
	variable *result = (variable *)calloc(1, sizeof(variable));
	result->valtype = ((variable *)ops[0])->valtype;
	result->value = ((variable *)ops[0])->value;
	for (uint32_t i = 1; i < ooffset; i += 2)
		iopfuncs[(uint32_t)ops[i] - 1](result, result, ops[i + 1]);
	
	if (result->valtype == INTEGER)
		isetstr(result);
	else
		fsetstr(result);

	for (uint32_t i = 0; i < ooffset; i += 2) {
		if (!((variable *)ops[i])->used) {
			char *s = ((variable *)ops[i])->svalue;
			if (s != 0 && s != str_undef)
				free(s);
			free(ops[i]);
		}
	}

	return result;
}

