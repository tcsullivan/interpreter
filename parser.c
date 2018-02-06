#include <parser.h>

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_VARS  32
#define MAX_STACK 32

#define INT(v)   (*((int32_t *)&v->value))
#define FLOAT(v) (*((float *)&v->value))

static char *str_func = "(func)";
static char *str_undef = "(undefined)";

char *strclone(const char *s)
{
	char *clone = (char *)malloc(strlen(s) + 1);
	strcpy(clone, s);
	return clone;
}

char *strnclone(const char *s, uint32_t n)
{
	char *clone = (char *)malloc(n + 1);
	strncpy(clone, s, n);
	return clone;
}

void fsetstr(variable *f)
{
	if (f->svalue == 0)
		f->svalue = (char *)malloc(16);
	snprintf(f->svalue, 16, "%f", FLOAT(f));
}

void isetstr(variable *i)
{
	if (i->svalue == 0)
		i->svalue = (char *)malloc(12);
	snprintf(i->svalue, 12, "%d", INT(i));
}

uint8_t eol(int c)
{
	return c == '\n' || c == '\0';
}

uint8_t eot(int c)
{
	return eol(c) || c == ' ';
}

uint8_t eoe(int c)
{
	return eol(c) || c == ')';
}

uint32_t findend(const char *s, char o, char c)
{
	uint8_t indent = 0;
	uint32_t i;
	for (i = 1; !eol(s[i]); i++) {
		if (s[i] == o) {
			indent++;
		} else if (s[i] == c) {
			if (indent == 0)
				break;
			else
				indent--;
		}
	}

	return i;
}

void skipblank(const char *s, uint8_t (*cmp)(int), uint32_t *offset)
{
	uint32_t i = *offset;
	while (!cmp(s[i])) {
		if (s[i] != ' ' && s[i] != '\t')
			break;
		i++;
	}
	*offset = i;
}

int ifunc_set(interpreter *it);
int ifunc_jmp(interpreter *it);
int ifunc_label(interpreter *it);
int ifunc_end(interpreter *it);
int ifunc_if(interpreter *it);
int ifunc_do(interpreter *it);
int ifunc_while(interpreter *it);

variable *idoexpr(interpreter *interp, const char *line);

variable *itostring(variable *v);
variable *itoint(variable *v);
variable *itofloat(variable *v);

void iinit(interpreter *interp)
{
	interp->vars = (variable *)calloc(MAX_VARS, sizeof(variable));
	interp->vnames = (char **)calloc(MAX_VARS, sizeof(char *));
	interp->stack = (stack_t *)calloc(MAX_STACK, sizeof(stack_t));
	interp->stidx = 0;
	interp->lines = (char **)calloc(20, sizeof(char *));
	interp->lnidx = 0;
	interp->indent = 0;

	inew_cfunc(interp, "set", ifunc_set);
	inew_cfunc(interp, "jmp", ifunc_jmp);
	inew_cfunc(interp, "func", ifunc_label);
	inew_cfunc(interp, "end", ifunc_end);
	inew_cfunc(interp, "if", ifunc_if);
	inew_cfunc(interp, "do", ifunc_do);
	inew_cfunc(interp, "while", ifunc_while);
}

void ipush(interpreter *it, void *v)
{
	it->stack[it->stidx++] = v;
}

void *ipop(interpreter *it)
{
	return it->stack[--it->stidx];
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

	interp->lines[interp->lnidx] = strclone(line);
loop:
	if (line[0] == '#') {
		goto norun;
	} else if (interp->indent > 0) {
		if (!strcmp(line, "end"))
			interp->indent--;
		goto norun;
	}

	ooffset = 0;
	offset = 0;

	// step 1 - convert to tokens
	skipblank(line, eol, &offset);
	while (!eol(line[offset])) {
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

	if (ops[0]->value == 0)
		return -3;

	for (uint32_t i = ooffset; --i > 0;)
		ipush(interp, ops[i]);

	if (ops[0]->fromc) {
		int ret = ((func_t)ops[0]->value)(interp);
		if (ret != 0)
			return ret;
	} else {
		ipush(interp, (void *)(interp->lnidx + 1));
		interp->lnidx = ops[0]->value;
	}

	interp->stidx -= ooffset - 1;

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

typedef void (*operation_t)(variable *, variable *, variable *);

#define IOPS_COUNT 15
static char *iops[IOPS_COUNT] = {
	"+", "-", "*", "/", "&", "|", "^", ">>", "<<",
	"==", "<", ">", "<=", ">=", "!="
};

void iop_add(variable *, variable *, variable *);
void iop_sub(variable *, variable *, variable *);
void iop_mult(variable *, variable *, variable *);
void iop_div(variable *, variable *, variable *);
void iop_and(variable *, variable *, variable *);
void iop_or(variable *, variable *, variable *);
void iop_xor(variable *, variable *, variable *);
void iop_shr(variable *, variable *, variable *);
void iop_shl(variable *, variable *, variable *);
void iop_eq(variable *, variable *, variable *);
void iop_lt(variable *, variable *, variable *);
void iop_gt(variable *, variable *, variable *);
void iop_lte(variable *, variable *, variable *);
void iop_gte(variable *, variable *, variable *);
void iop_ne(variable *, variable *, variable *);

static operation_t iopfuncs[IOPS_COUNT] = {
	iop_add, iop_sub, iop_mult, iop_div, iop_and,
	iop_or, iop_xor, iop_shr, iop_shl,
	iop_eq, iop_lt, iop_gt, iop_lte, iop_gte, iop_ne
};

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
		}
		if (ops[ooffset] == 0)
			return 0;
		
		ooffset++;
		offset += next;

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

variable *igetarg(interpreter *interp, uint32_t index)
{
	return interp->stack[interp->stidx - index - 1];
}

char *igetarg_string(interpreter *interp, uint32_t index)
{
	if (index >= interp->stidx)
		return 0;
	variable *v = igetarg(interp, index);
	return v->svalue;
}

int igetarg_integer(interpreter *interp, uint32_t index)
{
	if (index >= interp->stidx)
		return 0;
	variable *v = igetarg(interp, index);
	return INT(itoint(v));
}

float igetarg_float(interpreter *interp, uint32_t index)
{
	if (index >= interp->stidx)
		return 0;
	variable *v = igetarg(interp, index);
	return FLOAT(itofloat(v));
}

variable *itostring(variable *v)
{
	switch (v->valtype) {
	case INTEGER:
		v->valtype = STRING;
		isetstr(v);
		break;
	case FLOAT:
		v->valtype = STRING;
		fsetstr(v);
		break;
	}
	return v;
}

variable *itoint(variable *v)
{
	switch (v->valtype) {
	case STRING:
		v->valtype = INTEGER;
		INT(v) = atoi(v->svalue);
		isetstr(v);
		break;
	case FLOAT:
		v->valtype = INTEGER;
		INT(v) = (int32_t)FLOAT(v);
		isetstr(v);
		break;
	}
	return v;
}

variable *itofloat(variable *v)
{
	switch (v->valtype) {
	case STRING:
		v->valtype = FLOAT;
		FLOAT(v) = strtof(v->svalue, 0);
		fsetstr(v);
		break;
	case INTEGER:
		v->valtype = FLOAT;
		FLOAT(v) = (float)INT(v);
		fsetstr(v);
		break;
	}
	return v;
}

/**
 * Builtin functions
 */

int ifunc_set(interpreter *it)
{
	variable *n = igetarg(it, 0);
	variable *v = igetarg(it, 1);

	if (n == 0)
		return -1;

	n->valtype = v->valtype;
	n->value = v->value;
	n->svalue = v->svalue;
	return 0;
}

int ifunc_label(interpreter *it)
{
	variable *n = igetarg(it, 0);
	
	if (n == 0)
		return -1;

	n->valtype = FUNC;
	n->value = it->lnidx;
	n->svalue = str_func;
	it->indent++;
	return 0;
}

int ifunc_if(interpreter *it)
{
	int v = igetarg_integer(it, 0);
	if (v == 0)
		it->indent++;
	return 0;
}

int ifunc_end(interpreter *it)
{
	if (it->stidx > 0) {
		uint32_t line = (uint32_t)ipop(it);
		it->lnidx = line - 1;
	}
	return 0;
}

int ifunc_jmp(interpreter *it)
{
	int newidx = igetarg_integer(it, 0);
	ipop(it);
	ipush(it, (void *)(it->lnidx + 1));
	ipush(it, 0);
	it->lnidx = newidx - 1;
	return 0;
}

int ifunc_do(interpreter *it)
{
	ipush(it, (void *)it->lnidx);
	return 0;
}

int ifunc_while(interpreter *it)
{
	int c = igetarg_integer(it, 0);
	ipop(it);
	int nidx = (int)ipop(it);
	if (c != 0) {
		ipush(it, (void *)nidx);
		it->lnidx = nidx;
	}
	ipush(it, 0);
	return 0;
}

/**
 * Builtin operations
 */

void iop_add(variable *r, variable *a, variable *b)
{
	if (a->valtype == INTEGER && b->valtype == INTEGER) {
		INT(r) = INT(a) + INT(b);
	} else {
		itofloat(a);
		itofloat(b);
		FLOAT(r) = FLOAT(a) + FLOAT(b);
	}
}

void iop_sub(variable *r, variable *a, variable *b)
{
	if (a->valtype == INTEGER && b->valtype == INTEGER) {
		INT(r) = INT(a) - INT(b);
	} else {
		itofloat(a);
		itofloat(b);
		FLOAT(r) = FLOAT(a) - FLOAT(b);
	}
}

void iop_mult(variable *r, variable *a, variable *b)
{
	if (a->valtype == INTEGER && b->valtype == INTEGER) {
		INT(r) = INT(a) * INT(b);
	} else {
		itofloat(a);
		itofloat(b);
		FLOAT(r) = FLOAT(a) * FLOAT(b);
	}
}

void iop_div(variable *r, variable *a, variable *b)
{
	if (a->valtype == INTEGER && b->valtype == INTEGER) {
		INT(r) = INT(a) / INT(b);
	} else {
		itofloat(a);
		itofloat(b);
		FLOAT(r) = FLOAT(a) / FLOAT(b);
	}
}

void iop_and(variable *r, variable *a, variable *b)
{
	if (a->valtype == INTEGER && b->valtype == INTEGER) {
		INT(r) = INT(a) & INT(b);
	}
}

void iop_or(variable *r, variable *a, variable *b)
{
	if (a->valtype == INTEGER && b->valtype == INTEGER) {
		INT(r) = INT(a) | INT(b);
	}
}

void iop_xor(variable *r, variable *a, variable *b)
{
	if (a->valtype == INTEGER && b->valtype == INTEGER) {
		INT(r) = INT(a) ^ INT(b);
	}
}

void iop_shr(variable *r, variable *a, variable *b)
{
	if (a->valtype == INTEGER && b->valtype == INTEGER) {
		INT(r) = INT(a) >> INT(b);
	}
}

void iop_shl(variable *r, variable *a, variable *b)
{
	if (a->valtype == INTEGER && b->valtype == INTEGER) {
		INT(r) = INT(a) << INT(b);
	}
}

void iop_eq(variable *r, variable *a, variable *b)
{
	if (a->valtype == INTEGER && b->valtype == INTEGER)
		INT(r) = INT(a) == INT(b);
	else
		INT(r) = FLOAT(a) == FLOAT(b);
}

void iop_lt(variable *r, variable *a, variable *b)
{
	if (a->valtype == INTEGER && b->valtype == INTEGER)
		INT(r) = INT(a) < INT(b);
	else
		INT(r) = FLOAT(a) < FLOAT(b);
}

void iop_gt(variable *r, variable *a, variable *b)
{
	if (a->valtype == INTEGER && b->valtype == INTEGER)
		INT(r) = INT(a) > INT(b);
	else
		INT(r) = FLOAT(a) > FLOAT(b);
}

void iop_lte(variable *r, variable *a, variable *b)
{
	if (a->valtype == INTEGER && b->valtype == INTEGER)
		INT(r) = INT(a) <= INT(b);
	else
		INT(r) = FLOAT(a) <= FLOAT(b);
}

void iop_gte(variable *r, variable *a, variable *b)
{
	if (a->valtype == INTEGER && b->valtype == INTEGER)
		INT(r) = INT(a) >= INT(b);
	else
		INT(r) = FLOAT(a) >= FLOAT(b);
}

void iop_ne(variable *r, variable *a, variable *b)
{
	if (a->valtype == INTEGER && b->valtype == INTEGER)
		INT(r) = INT(a) != INT(b);
	else
		INT(r) = FLOAT(a) != FLOAT(b);
}

