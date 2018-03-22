#include "parser.h"

#include "builtins.h"
#include "ops.h"

#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAX_VARS  256
#define MAX_STACK 64
#define MAX_LINES 1000

int bracket_open(instance *it)
{
	it->indent++;
	if (it->sindent & SKIP) {
		ipush(it, SKIP_SIG);
		ipush(it, 0);
	}
	return 0;
}
int bracket_close(instance *it)
{
	it->indent--;
	if (it->indent < (it->sindent & ~(SKIP)))
		it->sindent = 0;
	bn_end(it);
	return 0;
}
static variable bopen = {
	0, CFUNC, 0, {.p = (uint32_t)bracket_open}
};
static variable bclose = {
	0, CFUNC, 0, {.p = (uint32_t)bracket_close}
};

char *strnclone(const char *s, size_t c)
{
	char *b = strncpy((char *)malloc(c + 1), s, c);
	b[c] = '\0';
	return b;
}
char *strclone(const char *s)
{
	return strnclone(s, strlen(s)); 
}
char *fixstring(const char *s)
{
	char *n = malloc(strlen(s) + 1 - 2);
	int j = 0;
	for (int i = 1; s[i] != '\"'; i++, j++) {
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

void itryfree(variable *v)
{
	if (v == 0 || v->tmp == 0)
		return;
	if (v->type == STRING)
		free((void *)v->value.p);
	free(v);
}

instance *inewinstance(void)
{
	instance *it = (instance *)malloc(sizeof(instance));
	it->vars = (variable *)calloc(MAX_VARS, sizeof(variable));
	it->names = (char **)calloc(MAX_VARS, sizeof(char *));
	it->stack = (uint32_t *)malloc(MAX_STACK * sizeof(uint32_t));
	it->stidx = 0;
	it->lines = (variable ***)calloc(MAX_LINES, sizeof(variable **));
	it->lnidx = 0;
	it->ret = 0;
	it->indent = 0;
	it->sindent = 0;

	iload_builtins(it);
	return it;
}

void idelinstance(instance *it)
{
	free(it->vars);
	for (uint32_t i = 0; i < MAX_VARS; i++)
		free(it->names[i]);
	free(it->names);
	free(it->stack);
	for (uint32_t i = 0; i < MAX_LINES; i++) // TODO free vars!
		free(it->lines[i]);
	free(it->lines);
	itryfree(it->ret);
	free(it);
}

void ipush(instance *it, uint32_t v)
{
	it->stack[it->stidx++] = v;
}

uint32_t ipop(instance *it)
{
	return it->stack[--it->stidx];
}

void ipopm(instance *it, uint32_t count)
{
	it->stidx -= count;
}

variable *igetarg(instance *it, uint32_t n)
{
	return (variable *)it->stack[it->stidx - n - 1];
}

variable *igetvar(instance *it, const char *name);
void inew_cfunc(instance *it, const char *name, func_t func)
{
	variable *v = igetvar(it, name);
	v->type = CFUNC;
	v->value.p = (uint32_t)func;
}

void inew_number(instance *it, const char *name, float f)
{
	variable *v = igetvar(it, name);
	v->type = NUMBER;
	v->value.f = f;
}

void inew_string(instance *it, const char *name, const char *s)
{
	variable *v = igetvar(it, name);
	v->type = STRING;
	v->value.p = (uint32_t)strclone(s);
}

variable *varclone(variable *n)
{
	variable *v = (variable *)malloc(sizeof(variable));
	v->tmp = 1;
	v->type = n->type;
	if (n->type == STRING)
		v->value.p = (uint32_t)strclone((char *)n->value.p);
	else
		v->value.p = n->value.p;
	return v;
}

variable *make_varf(variable *v, float f)
{
	if (v == 0) {
		v = (variable *)malloc(sizeof(variable));
		v->tmp = 1;
	}
	v->type = NUMBER;
	v->value.f = f;
	return v;
}

variable *make_vars(variable *v, const char *s)
{
	if (v == 0) {
		v = (variable *)malloc(sizeof(variable));
		v->tmp = 1;
	}
	v->type = STRING;
	v->value.p = (uint32_t)strclone(s);
	return v;
}

variable *make_num(const char *text)
{
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

	variable *v = make_varf(0, strtof(buf, 0));
	free(buf);
	return v;
}

variable *igetop(const char *name)
{
	for (uint32_t i = 0; i < OPS_COUNT; i++) {
		if (opnames[i] != 0 && !strcmp(name, opnames[i])) {
			return &opvars[i];
		}
	}
	return 0;
}
variable *igetvar(instance *it, const char *name)
{
	if (isalpha(name[0])) {
		for (uint32_t i = 0; i < MAX_VARS; i++) {
			if (it->names[i] == 0) {
				it->names[i] = strclone(name);
				// default to 0 float
				return make_varf(&it->vars[i], 0.0f);
			} else if (!strcmp(name, it->names[i])) {
				return &it->vars[i];
			}
		}
	}

	return igetop(name);
}

int idoline(instance *it, const char *s)
{
	variable **ops = iparse(it, s);
	if (ops == 0)
		return 0;
	it->lines[it->lnidx] = ops;

loop:
	if (it->ret != 0)
		itryfree(it->ret);
	it->ret = 0;

	variable **copy = malloc(32 * sizeof(variable *));
	for (int i = 0; i < 32; i++)
		copy[i] = it->lines[it->lnidx][i];
	it->ret = isolve(it, copy, 0);
	free(copy);

	it->lnidx++;
	if (it->lines[it->lnidx] != 0)
		goto loop;

	return 0;
}

variable *isolve_(instance *it, variable **ops, uint32_t count);
variable *isolve(instance *it, variable **ops, uint32_t count)
{
	if (count == 0)
		for (count = 0; ops[count] != 0; count++);

	for (uint32_t i = 0; i < count; i++) {
		if (((uint32_t)ops[i] & OP_MAGIC) == OP_MAGIC) {
			uint32_t count_ = (uint32_t)ops[i] & 0xFF;
			ops[i] = isolve(it, ops + i + 1, count_);
			for (uint32_t j = 1; j <= count_; j++)
				ops[i + j] = 0;
		}
	}

	return isolve_(it, ops, count);
}

variable *isolve_(instance *it, variable **ops, uint32_t count)
{
	// first, look for functions
	for (uint32_t i = 0; i < count; i++) {
		if (ops[i] == 0)
			continue;
		if (ops[i]->type == CFUNC || ops[i]->type == FUNC) {
			uint32_t nargs = (uint32_t)ops[i + 1] - 1;
			uint32_t start = i;
			i++;
			if (nargs > 0)
				i++;
			int32_t j;
			for (j = nargs; j > 0 && i < count; i++) {
				if (ops[i] != 0) {
					if (ops[start]->type == CFUNC) {
						it->stack[it->stidx + j - 1] = (uint32_t)ops[i];
					} else {
						char namebuf[6];
						snprintf(namebuf, 6, "arg%u",
							(uint16_t)(nargs - j));
						if (ops[i]->type == NUMBER)
							inew_number(it, namebuf, ops[i]->value.f);
						else
							inew_string(it, namebuf,
								(const char *)ops[i]->value.p);
					}				
					j--;
				}
			}
			if (j != 0)
				return 0;

			if (ops[start]->type == CFUNC) {
				func_t func = (func_t)ops[start]->value.p;
				it->stidx += nargs;

				uint32_t sidx = it->stidx;
				int ret = 0;
				if (!(it->sindent & SKIP) || (func == bracket_open ||
					func == bracket_close))
					ret = func(it);
				if (ret != 0)
					return 0;
				if (it->stidx > sidx)
					ops[start] = (variable *)ipop(it);
				else
					ops[start] = 0;

				it->stidx -= nargs;
			} else {
				ipush(it, it->lnidx);
				ipush(it, CALL_SIG);
				it->lnidx = ops[start]->value.p;
			}

			ops[start + 1] = 0;
			for (uint32_t j = start + 2; j < i; j++) {
				//itryfree(ops[j]);
				ops[j] = 0;
			}
		}
	}

	// next, operators
	for (uint32_t j = 0; j < OPS_COUNT; j += 2) {
		for (uint32_t i = 0; i < count; i++) {
			if (ops[i] == 0)
				continue;
			if (ops[i]->type == OPERATOR) {
				if (ops[i]->value.p != (uint32_t)opvars[j].value.p) {
					if (ops[i]->value.p != (uint32_t)opvars[j + 1].value.p)
						continue;
				}

				opfunc_t func = (opfunc_t)ops[i]->value.p;
				uint32_t aidx = i - 1;
				while (ops[aidx] == 0 && aidx != 0)
					aidx--;
				if (ops[aidx] == 0)
					return 0;
				uint32_t bidx = i + 1;
				while (ops[bidx] == 0 && ++bidx < count);
				if (bidx == count)
					return 0;

				if (it->sindent & SKIP) {
					//itryfree(ops[aidx]);
					//itryfree(ops[bidx]);
					ops[aidx] = 0;
				} else {
					variable *v = varclone(ops[aidx]);
					if (func(v, ops[aidx], ops[bidx]) != 0)
						return 0;
					//itryfree(ops[aidx]);
					ops[aidx] = v;
					//itryfree(ops[bidx]);
				}
				ops[i] = 0;
				ops[bidx] = 0;
			}
		}
	}

	return ops[0];
}

variable **iparse(instance *it, const char *s)
{
	uint32_t ooffset = 0;
	int32_t boffset = 1;
	size_t offset = 0;

	while (isblank(s[offset]))
		offset++;
	if (s[offset] == '#' || s[offset] == '\0' || s[offset] == '\n')
		return 0;

	variable **ops = (variable **)calloc(32, sizeof(variable *));
	while (s[offset] != '\0' && s[offset] != '\n') {
		if (isalpha(s[offset])) {
			size_t end = offset + 1;
			while (isalnum(s[end]))
				end++;
			char *name = strnclone(s + offset, end - offset);
			ops[ooffset++] = igetvar(it, name);
			free(name);
			while (isblank(s[end]))
				end++;
			if (s[end] == '(') {
				uint32_t argidx = ooffset;
				uint32_t argcount = 1;
				ooffset++;
				end++;
				uint32_t last = end;
				for (int c = 0; c >= 0; end++) {
					if (s[end] == '(')
						c++;
					if (c == 0 && last != end && (s[end] == ',' || s[end] == ')')) {
						argcount++;
						char *arg = strnclone(s + last, end - last);
						uint32_t parenidx = ooffset;
						ooffset++;
						variable **moreops = iparse(it, arg);
						uint32_t count = 0;
						if (moreops != 0) {
							for (uint32_t i = 0; moreops[i] != 0; count++, i++)
								ops[ooffset++] = moreops[i];
							free(moreops);
						}
						free(arg);
						ops[parenidx] = (variable *)(OP_MAGIC | count);
						last = end + 1;
					}
					if (s[end] == ')')
						c--;
				}
				if (s[end] != '\0')
					end++;
				ops[argidx] = (variable *)argcount;
			} else if (ops[ooffset - 1]->type == FUNC || ops[ooffset - 1]->type == CFUNC) {
				ops[ooffset++] = (variable *)1;
			}
			offset = end;
		} else if (isdigit(s[offset])) {
			size_t end = offset + 1;
			while (isdigit(s[end]) || s[end] == '.')
				end++;
			char *word = strnclone(s + offset, end - offset);
			ops[ooffset++] = make_num(word);
			free(word);
			offset = end;
		} else if (s[offset] == '\"') {
			size_t end = offset + 1;
			while (s[end] != '\"')// && s[end - 1] == '\\')
				end++;
			end++;
			char *word = strnclone(s + offset, end - offset);
			char *fword = fixstring(word);
			ops[ooffset++] = make_vars(0, fword);
			free(word);
			free(fword);
			offset = end;
		} else if (s[offset] == '(') {
			size_t i = offset + 1;
			for (int c = 0; s[i] != ')' || --c >= 0; i++) {
				if (s[i] == '(')
					c++;
			}
			i++;
			char *word = strnclone(s + offset + 1, i - offset - 2);
			uint32_t parenidx = ooffset;
			ooffset++;
			variable **moreops = iparse(it, word);
			uint32_t count = 0;
			if (moreops != 0) {
				for (uint32_t i = 0; moreops[i] != 0; count++, i++)
					ops[ooffset++] = moreops[i];
				free(moreops);
			}
			free(word);
			ops[parenidx] = (variable *)(OP_MAGIC | count);
			offset = i;
		} else if (!isblank(s[offset])) {
			size_t end = offset + 1;
			while (!isblank(s[end]) && !isalnum(s[end]) && s[end] != '\0')
				end++;
			char *word = strnclone(s + offset, end - offset);

			// bracket?
			if (!strcmp(word, "{") || !strcmp(word, "}")) {
				for (int32_t i = ooffset - 1; i >= boffset - 1; i--)
					ops[i + 2] = ops[i];
				if (word[0] == '{')
					ops[boffset - 1] = &bopen;
				else
					ops[boffset - 1] = &bclose;
				ops[boffset] = (variable *)1; // arg count + 1
				boffset += 2;
				ooffset += 2;
			} else {
				variable *v = igetop(word);
				if (v == 0)
					return 0;
				ops[ooffset++] = v;
			}
			free(word);
			offset = end;
		} else {
			offset++;
		}
	}

	// mark end
	ops[ooffset] = 0;
	return ops;
}
