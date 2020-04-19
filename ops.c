/**
 * @file ops.c
 * Provides arithematic operators to the parser
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

/**
 * Operators are special functions (though different from those that are built-
 * in). Operators have no access to the interpreter instance; instead only the
 * two 'argument' variables and a variable to contain the result are passed in.
 * The operator function returns an integer, zero for success.
 *
 * Argument 'a' is the variable on the left side of the operator, and 'b' is
 * the variable on the right. 'r' is a non-null variable that the result of the
 * operation should be placed in.
 */

#include "error.h"
#include "ops.h"
#include "string.h"

#include <stdlib.h>

#define OP_DEF(o) int op_##o(variable **r, variable *a, variable *b)
#define OP_VAR(o) {0, OPERATOR, 0, 0, {.p = (size_t)op_##o}}
#define OP_NONE   {0, OPERATOR, 0, 0, {.p = 0x0BADCAFE}}

OP_DEF(idx);
OP_DEF(mul);
OP_DEF(div);
OP_DEF(mod);
OP_DEF(add);
OP_DEF(sub);
OP_DEF(shl);
OP_DEF(shr);
OP_DEF(lte);
OP_DEF(lt);
OP_DEF(gte);
OP_DEF(gt);
OP_DEF(eq);
OP_DEF(ne);
OP_DEF(and);
OP_DEF(xor);
OP_DEF(or);
OP_DEF(set);

/**
 * Operators are stored here in order of significance, meaning those towards
 * the beginning of the array are completed before those after them. This
 * priority listing is done in pairs of two, so that mathematical order of
 * operations can be respected. For example, the first two operators
 * multiplication and division) have the same priority. Should an operator not
 * have a 'pair', OP_NONE can be used. Should adjacent operators have the same
 * priority, they will be evaluated from left-to-right by the parser.
 */
variable opvars[] = {
	OP_VAR(idx), OP_NONE,
	OP_VAR(mul), OP_VAR(div), OP_VAR(mod), OP_NONE,
	OP_VAR(add), OP_VAR(sub), OP_VAR(shl), OP_VAR(shr),
	OP_VAR(lte), OP_VAR(lt), OP_VAR(gte), OP_VAR(gt),
	OP_VAR(eq), OP_VAR(ne), OP_VAR(and), OP_VAR(xor),
	OP_VAR(or), OP_VAR(set)
};

const char *opnames[] = {
	".", 0,
	"*", "/", "%", 0,
	"+", "-", "<<", ">>",
	"<=", "<", ">=", ">",
	"==", "!=", "&", "^",
	"|", "="
};

variable *igetop(const char *name, int *retlen)
{
	for (size_t i = 0; i < OPS_COUNT; i++) {
		if (opnames[i] == 0)
			continue;
		int len = strlen(opnames[i]);
		if (opnames[i] != 0 && !strncmp(name, opnames[i], len)) {
			if (retlen != 0)
				*retlen = len;
			return &opvars[i];
		}
	}
	return 0;
}

OP_DEF(idx)
{
	if (a->array == 0) {
		if (a->type != STRING)
			return seterror(EBADPARAM);

		// string index
		int idx = b->value.f;
		for (int i = 0; i <= idx; i++) {
			if (((char *)a->value.p)[i] == '\0') {
				return seterror(EBADPARAM);
				/*// new buf
				char *newbuf = malloc(idx + 2);
				newbuf[idx + 1] = '\0';
				memcpy(newbuf, (char *)a->value.p, i);
				free((void *)a->value.p);
				break;*/
			}
		}

		(*r)->type = NUMBER;
		(*r)->value.f = ((char *)a->value.p)[idx];
		return 0;
	}

	extern void itryfree(variable *);
	itryfree(*r);

	int idx = b->value.f;
	if (idx >= a->array) {
		variable *newarray = (variable *)calloc(idx + 1, sizeof(variable));
		void *old = (void *)a->value.p;
		a->value.p = (size_t)memcpy(newarray, (variable *)a->value.p,
			a->array * sizeof(variable));
		free(old);
		a->array = idx + 1;
	}

	variable *array = (variable *)a->value.p;
	*r = &array[idx];
	return 0;
}
OP_DEF(mul)
{
	if (a->type != NUMBER || b->type != NUMBER)
		return seterror(EBADPARAM);
	(*r)->type = NUMBER;
	(*r)->value.f = a->value.f * b->value.f;
	return 0;
}
OP_DEF(div)
{
	if (a->type != NUMBER || b->type != NUMBER)
		return seterror(EBADPARAM);
	(*r)->type = NUMBER;
	(*r)->value.f = a->value.f / b->value.f;
	return 0;
}
OP_DEF(mod)
{
	if (a->type != NUMBER || b->type != NUMBER)
		return seterror(EBADPARAM);
	(*r)->type = NUMBER;
	(*r)->value.f = (int)a->value.f % (int)b->value.f;
	return 0;
}
OP_DEF(add)
{
	if (a->type != NUMBER || b->type != NUMBER)
		return seterror(EBADPARAM);
	(*r)->type = NUMBER;
	(*r)->value.f = a->value.f + b->value.f;
	return 0;
}
OP_DEF(sub)
{
	if (a->type != NUMBER || b->type != NUMBER)
		return seterror(EBADPARAM);
	(*r)->type = NUMBER;
	(*r)->value.f = a->value.f - b->value.f;
	return 0;
}
OP_DEF(shl)
{
	if (a->type != NUMBER || b->type != NUMBER)
		return seterror(EBADPARAM);
	(*r)->type = NUMBER;
	(*r)->value.f = (int)a->value.f << (int)b->value.f;
	return 0;
}
OP_DEF(shr)
{
	if (a->type != NUMBER || b->type != NUMBER)
		return seterror(EBADPARAM);
	(*r)->type = NUMBER;
	(*r)->value.f = (int)a->value.f >> (int)b->value.f;
	return 0;
}
OP_DEF(lte)
{
	if (a->type != NUMBER || b->type != NUMBER)
		return seterror(EBADPARAM);
	(*r)->type = NUMBER;
	(*r)->value.f = a->value.f <= b->value.f;
	return 0;
}
OP_DEF(lt)
{
	if (a->type != NUMBER || b->type != NUMBER)
		return seterror(EBADPARAM);
	(*r)->type = NUMBER;
	(*r)->value.f = a->value.f < b->value.f;
	return 0;
}
OP_DEF(gte)
{
	if (a->type != NUMBER || b->type != NUMBER)
		return seterror(EBADPARAM);
	(*r)->type = NUMBER;
	(*r)->value.f = a->value.f >= b->value.f;
	return 0;
}
OP_DEF(gt)
{
	if (a->type != NUMBER || b->type != NUMBER)
		return seterror(EBADPARAM);
	(*r)->type = NUMBER;
	(*r)->value.f = a->value.f > b->value.f;
	return 0;
}
OP_DEF(eq)
{
	(*r)->type = NUMBER;
	if (a->type == NUMBER && b->type == NUMBER)
		(*r)->value.f = a->value.f == b->value.f;
	else if (a->type == STRING && b->type == STRING)
		(*r)->value.f = !strcmp((const char *)a->value.p, (const char *)b->value.p);
	else
		(*r)->value.f = 0.0f; // *sshhh*
		//return seterror(EBADPARAM);

	return 0;
}
OP_DEF(ne)
{
	if (a->type != NUMBER || b->type != NUMBER)
		return seterror(EBADPARAM);
	(*r)->type = NUMBER;
	(*r)->value.f = a->value.f != b->value.f;
	return 0;
}
OP_DEF(and)
{
	if (a->type != NUMBER || b->type != NUMBER)
		return seterror(EBADPARAM);
	(*r)->type = NUMBER;
	(*r)->value.f = (int)a->value.f & (int)b->value.f;
	return 0;
}
OP_DEF(xor)
{
	if (a->type != NUMBER || b->type != NUMBER)
		return seterror(EBADPARAM);
	(*r)->type = NUMBER;
	(*r)->value.f = (int)a->value.f ^ (int)b->value.f;
	return 0;
}
OP_DEF(or)
{
	if (a->type != NUMBER || b->type != NUMBER)
		return seterror(EBADPARAM);
	(*r)->type = NUMBER;
	(*r)->value.f = (int)a->value.f | (int)b->value.f;
	return 0;
}
OP_DEF(set)
{
	if (b->type == NUMBER) {
		a->type = NUMBER;
		a->value.f = b->value.f;
		(*r)->type = NUMBER;
		(*r)->value.f = a->value.f;
	} else if (b->type == STRING) {
		a->type = STRING;
		free((void *)a->value.p);
		a->value.p = (size_t)strclone((char *)b->value.p);
		(*r)->type = STRING;
		(*r)->value.p = (size_t)strclone((char *)a->value.p);
	} else {
		return seterror(EBADPARAM);
	}
	return 0;
}

