#include <parser.h>

#include "shelpers.h"
#include "builtins.h"
#include "stack.h"
#include "ops.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#define MAX_VARS  48
#define MAX_STACK 16
#define MAX_LINES 1000

char *str_func = "(func)";
char *str_undef = "(undefined)";

variable *idoexpr(interpreter *interp, const char *line);

void iinit(interpreter *interp)
{
	interp->vars = (variable *)malloc(MAX_VARS * sizeof(variable));
	interp->vnames = (char **)malloc(MAX_VARS * sizeof(char *));
	interp->stack = (stack_t *)malloc(MAX_STACK * sizeof(stack_t));
	interp->stidx = 0;
	interp->lines = (char **)calloc(MAX_LINES, sizeof(char *));
	interp->lnidx = 0;
	interp->indent = 0;

	for (unsigned int i = 0; i < MAX_VARS; i++) {
		interp->vars[i].used = 0;
		interp->vnames[i] = 0;
	}
	for (unsigned int i = 0; i < MAX_LINES; i++)
		interp->lines[i] = 0;

	iload_core(interp);
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
			interp->vnames[i] = strclone(name);
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

void inew_string(interpreter *interp, const char *name, char *value)
{
	variable *v = interpreter_get_variable(interp, name);
	if (v != 0) {
		v->valtype = STRING;
		INT(v) = 0;
		v->svalue = strclone(value);
	}
}

void inew_integer(interpreter *interp, const char *name, int32_t value)
{
	variable *v = interpreter_get_variable(interp, name);
	if (v != 0) {
		v->valtype = INTEGER;
		INT(v) = value;
		isetstr(v);
	}
}

void inew_float(interpreter *interp, const char *name, float value)
{
	variable *v = interpreter_get_variable(interp, name);
	if (v != 0) {
		v->valtype = FLOAT;
		FLOAT(v) = value;
		fsetstr(v);
	}
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
				variable *v = (variable *)malloc(sizeof(variable));
				v->used = 0;
				v->valtype = STRING;
				v->svalue = strnclone(line + 1, end - 1);
				*next = end + 1;
				return v;
			}
			end++;
		}
		return 0;
	} else if (line[0] == '(') { // equation literal
		uint32_t end = findend(line, '(', ')');
		if (eot(line[end]))
			return 0;
		*next = end + 1;
		return idoexpr(interp, line + 1);
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
				} else {
					return 0;
				}
			}
		}
		variable *v = (variable *)malloc(sizeof(variable));
		v->used = 0;
		if (dec) {
			v->valtype = FLOAT;
			FLOAT(v) = strtof(line, 0);
			fsetstr(v);
		} else {
			v->valtype = INTEGER;
			INT(v) = atoi(line);
			isetstr(v);
		}
		*next = end;
		return v;
	}
	return 0;
}

int idoline(interpreter *interp, const char *line)
{
	variable *ops[8];
	uint32_t ooffset, offset, next;

	if (line[0] == '\0')
		return 0;

	interp->lines[interp->lnidx] = strclone(line);
loop:
	ooffset = 0;
	offset = 0;
	skipblank(line, eol, &offset);

	if (line[offset] == '#') {
		goto norun;
	} else if (interp->indent > 0) {
		if (!strcmp(line + offset, "end"))
			interp->indent--;
		goto norun;
	}

	// step 1 - convert to tokens
	while (!eol(line[offset])) {
		if (offset > 0 && line[offset] == '>') {
			offset++;
			skipblank(line, eol, &offset);
			interp->ret = make_var(interp, line + offset, &next);
			offset += next;
			skipblank(line, eol, &offset);
			continue;
		}
		ops[ooffset] = make_var(interp, line + offset, &next);
		if (ops[ooffset] == 0) {
			return -4;
		} else {
			ooffset++;
			offset += next;
		}
		skipblank(line, eol, &offset);
	}

	// step 2 - execute
	if (ooffset == 0)
		return -1;

	if (ops[0]->valtype != FUNC)
		return -2;

	if (ops[0]->fromc && ops[0]->value == 0)
		return -3;

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
			snprintf(an, 6, "arg%d", i - 1);
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
	}

	if ((int32_t)interp->stidx < 0) {
		interp->stidx = 0;
		return -5;
	}

	for (uint32_t i = 0; i < ooffset; i++) {
		if (!ops[i]->used)
			free(ops[i]);
	}

norun:
	interp->lnidx++;
	if (interp->lines[interp->lnidx] != 0) {
		line = interp->lines[interp->lnidx];
		goto loop;
	}

	return 0;
}

variable *idoexpr(interpreter *interp, const char *line)
{
	variable *result = (variable *)malloc(sizeof(variable));
	char *mline = line;
	void *ops[16];
	uint32_t ooffset = 0;
	uint32_t offset = 0;
	uint32_t next;

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
			uint32_t end = offset;
			char cend;
			if (line[offset] != '\"') {
				for (; isalnum(line[end]) || line[end] == '.'; end++);
				cend = line[end];
				mline[end] = ' ';
			}
			ops[ooffset] = make_var(interp, line + offset, &next);
			if (end != 0)
				mline[end] = cend;
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
	result->valtype = ((variable *)ops[0])->valtype;
	result->value = ((variable *)ops[0])->value;
	for (uint32_t i = 1; i < ooffset; i += 2) {
		iopfuncs[(uint32_t)ops[i] - 1](result, result, ops[i + 1]);
	}

	for (uint32_t i = 0; i < ooffset; i += 2) {
		if (!((variable *)ops[i])->used)
			free(ops[i]);
	}

	if (result->valtype == INTEGER)
		isetstr(result);
	else
		fsetstr(result);

	return result;
}

