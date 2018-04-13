/**
 * @file parser.c
 * Main library component; parses, manages and runs script
 *
 * Copyright (C) 2018 Clyne Sullivan
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "parser.h"

#include "builtins.h"
#include "ops.h"
#include "string.h"
#include "variable.h"

#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>

/**
 * Limitations for an instance. TODO make dynamic (no limits).
 */

#define MAX_VARS  256
#define MAX_STACK 64
#define MAX_LINES 1000

/**
 * Attempts to free memory used by a variable, if the variable is temporary.
 * Nothing is done if the variable can't be free'd.
 * @param v the variable to free
 */
void itryfree(variable *v)
{
	if (v == 0 || v->tmp == 0)
		return;
	if (v->type == STRING)// || v->array > 0)
		free((void *)v->value.p);

	free(v);
}

//
// instance construction/deconstruction
//

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

void idelline(variable **ops);
void idelinstance(instance *it)
{
	itryfree(it->ret);

	for (uint32_t i = 0; i < MAX_LINES; i++) {
		if (it->lines[i] == 0)
			continue;

		idelline(it->lines[i]);
		free(it->lines[i]);
	}
	free(it->lines);

	for (uint32_t i = 0; i < MAX_VARS; i++) {
		if (it->vars[i].type == STRING || it->vars[i].array > 0)
			free((void *)it->vars[i].value.p);
		free(it->names[i]);
	}
	free(it->vars);
	free(it->names);

	free(it->stack);
	free(it);
}

void idelline(variable **ops)
{
	for (int j = 0; j < 32; j++) {
		variable *v = ops[j];
		if (v != 0) {
			if (((uint32_t)v & OP_MAGIC) == OP_MAGIC)
				continue;

			if (v->type == FUNC || v->type == CFUNC)
				j++; // argcount

			if (v->tmp == 1)
				itryfree(v);
		}
	}
}

//
// stack operations
//

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

//
// variable creation
//

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

	return igetop(name, 0);
}

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

int iaddline(instance *it, const char *s)
{
	variable **ops = iparse(it, s);
	if (ops == 0)
		return 0;
	it->lines[it->lnidx] = ops;
	it->lnidx++;
	return 0;
}

int irun(instance *it)
{
	variable **copy;
	it->lnidx = 0;
loop:
	//if (it->ret != 0)
	//	itryfree(it->ret);
	//it->ret = 0;

	// clone the 'ops' array carefully, as isolve() frees/deletes used
	// variables as it goes
	copy = (variable **)malloc(32 * sizeof(variable *));
	for (int i = 0; i < 32; i++) {
		variable *v = it->lines[it->lnidx][i];
		if (v != 0) {
			if (((uint32_t)v & OP_MAGIC) == OP_MAGIC) {
				copy[i] = v;
				continue;
			}

			if (v->tmp == 1)
				copy[i] = varclone(v);
			else
				copy[i] = v;
			if (v->type == FUNC || v->type == CFUNC) {
				i++;
				copy[i] = it->lines[it->lnidx][i]; // argcount
			}
		} else {
			copy[i] = 0;
		}
	}
	it->ret = isolve(it, copy, 0);

	if (it->ret == 0) {
		idelline(copy);
	} else {
		// move result global variable "ANS"
		variable *ret = igetvar(it, "ANS");
		if (ret->type == STRING)
			free((void *)ret->value.p);
		ret->type = it->ret->type;
		if (ret->type == STRING)
			ret->value.p = (uint32_t)strclone((char *)it->ret->value.p);
		else
			ret->value.p = it->ret->value.p;
		itryfree(it->ret);
		it->ret = ret;
	}

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
				itryfree(ops[j]);
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
				if (bidx >= count)
					return 0;

				if (!(it->sindent & SKIP)) {
					variable *v = !ops[aidx]->tmp ? make_varf(0, 0.0f) : ops[aidx];
					if (func(&v, ops[aidx], ops[bidx]) != 0)
						return 0;
					ops[aidx] = v;
				} else {
					itryfree(ops[aidx]);
					ops[aidx] = 0;
				}
				itryfree(ops[bidx]);
				ops[bidx] = 0;
				ops[i] = 0;
			}
		}
	}

	// implicit multiply
	/*if (ops[0] != 0 && ops[0]->type == NUMBER) {
		for (uint32_t i = 1; i < count; i++) {
			if (ops[i] != 0 && ops[i]->type == NUMBER)
				ops[0]->value.f *= ops[i]->value.f;
		}
	}*/

	return ops[0];
}

variable **iparse(instance *it, const char *s)
{
	variable **ops = 0;
	uint32_t ooffset = 0;
	int32_t boffset = 1;
	size_t offset = 0;
	uint8_t prevNum = 0;

	// advance to first character
	// and insure there's runnable script on the line
	while (isblank(s[offset]))
		offset++;
	if (s[offset] == '#' || s[offset] == '\0' || s[offset] == '\n')
		goto fail;

	// iterate through script to assemble line of 'ops'
	// that isolve() can run through
	ops = (variable **)calloc(32, sizeof(variable *));
	while (s[offset] != '\0' && s[offset] != '\n') {
		// variable or function
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
					if (c == 0 && last != end && (s[end] == ',' || s[end] == ')' || s[end] == '\0')) {
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
					if (s[end] == ')' || s[end] == '\0')
						c--;
				}
				if (s[end] != '\0')
					end++;
				ops[argidx] = (variable *)argcount;
			} else if (ops[ooffset - 1]->type == FUNC || ops[ooffset - 1]->type == CFUNC) {
				ops[ooffset++] = (variable *)1;
			} else {
				prevNum += 2;
			}
			offset = end;
		} else if (isdigit(s[offset])) {// || (s[offset] == '-' && isdigit(s[offset + 1]))) {
			size_t end = offset + 1;
			while (isdigit(s[end]) || s[end] == '.')
				end++;
			char *word = strnclone(s + offset, end - offset);
			ops[ooffset++] = make_num(word);
			free(word);
			offset = end;
			prevNum += 2;
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
			prevNum += 2;
		} else if (s[offset] == '[') {
			/*uint32_t i = offset + 1;
			uint32_t j = i;
			while (s[offset] != ']') {
				if (s[offset] == ';') {

				}
			}*/
		} else if (!isblank(s[offset])) {
			if (s[offset] == '{' || s[offset] == '}') {
				for (int32_t i = ooffset - 1; i >= boffset - 1; i--)
					ops[i + 2] = ops[i];
				if (s[offset] == '{')
					ops[boffset - 1] = &bopen;
				else
					ops[boffset - 1] = &bclose;
				ops[boffset] = (variable *)1; // arg count + 1
				boffset += 2;
				ooffset += 2;
				offset++;
			} else {
				int len = 0;
				variable *v = igetop(s + offset, &len);
				if (v == 0)
					goto fail;
				if (ooffset == 0) {
					variable *a = (it->ret != 0) ?
						a = it->ret : make_varf(0, 0.0f);
					ops[ooffset++] = a;
				} /*else if (ops[ooffset - 1]->type == OPERATOR) {
					goto fail;
				}*/
				ops[ooffset++] = v;
				offset += len;
			}
		} else {
			offset++;
		}

		if (prevNum > 0) {
			if (--prevNum == 2) {
				--prevNum;
				variable *mul = igetop("*", 0);
				ops[ooffset] = ops[ooffset - 1];
				ops[ooffset - 1] = mul;
				ooffset++;
			}
		}
	}

	// mark end
	ops[ooffset] = 0;
	return ops;

fail:
	if (ops != 0) {
		idelline(ops);
		free(ops);
	}
	return 0;
}

