/**
 * @file builtins.c
 * Provides built-in functions for the interpreter
 * These must be loaded for an instance to do anything meaningful.
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
 * Built-ins are functions that are meant to be included in every interpreter
 * instance. That is, the interpreter would be near worthless as a scripting
 * language without these functions.
 *
 * A built-in function takes an instance, and returns an error code (zero if
 * successful). Arguments to the function can be obtained through igetarg().
 * Built-in functions have full access to the instance, its variables and
 * stack. To return a variable, push that variable to the stack at the end of
 * the function. If stack values are inserted by the function, push a zero so
 * error doesn't occur.
 */

#include "builtins.h"

#include <stdlib.h>

#define IF_SIG    (uint32_t)-1
#define WHILE_SIG (uint32_t)-2
#define ELSE_SIG  (uint32_t)-3
#define FUNC_SIG  (uint32_t)-4

variable bopen = {
	0, CFUNC, 0, {.p = (uint32_t)bracket_open}
};

variable bclose = {
	0, CFUNC, 0, {.p = (uint32_t)bracket_close}
};

int bn_if(instance *it);
int bn_else(instance *it);
int bn_end(instance *it);
int bn_while(instance *it);
int bn_func(instance *it);
int bn_solve(instance *it);

void iload_builtins(instance *it)
{
	inew_cfunc(it, "if", bn_if);
	inew_cfunc(it, "else", bn_else);
	inew_cfunc(it, "while", bn_while);
	inew_cfunc(it, "func", bn_func);
	inew_cfunc(it, "solve", bn_solve);
}

/**
 * Code for an opening bracket ('{', new scope).
 */
int bracket_open(instance *it)
{
	it->indent++;
	if (it->sindent & SKIP) {
		// make sure this indent is caught by its closing '}'.
		ipush(it, SKIP_SIG);
		ipush(it, 0);
	}
	return 0;
}

/**
 * Code for a closing bracket ('}', end of scope)
 */
int bracket_close(instance *it)
{
	it->indent--;
	// stop skipping if this is the end of the skipped scope
	if (it->indent < (it->sindent & ~(SKIP)))
		it->sindent = 0;
	bn_end(it);
	return 0;
}

int bn_if(instance *it)
{
	variable *cond = (variable *)ipop(it);
	uint32_t result = cond->value.p;

	ipush(it, result);
	ipush(it, IF_SIG);
	if (result == 0)
		it->sindent = SKIP | it->indent;
	ipush(it, 0);
	ipush(it, 0); // need to return because stack modify

	return 0;
}

static uint32_t if_cond = 0;
int bn_else(instance *it)
{
	uint32_t cond = if_cond;
	if (cond != 0)
		it->sindent = SKIP | it->indent;
	ipush(it, ELSE_SIG);
	ipush(it, 0); // for ret
	return 0;
}

/**
 * bn_end is a special function. The parser is hard-coded to interpret '}'
 * characters as calls to this function, which handles closing loops or
 * conditionals.
 *
 * The most recent value on the stack should determine what loop is being
 * closed, so that action can be taken accordingly.
 */
int bn_end(instance *it)
{
	uint32_t sig = ipop(it);
	if (sig == IF_SIG) {
		if_cond = ipop(it);
	} else if (sig == WHILE_SIG) {
		uint32_t lnidx = ipop(it);
		if (lnidx != (uint32_t)-1)
			it->lnidx = lnidx - 1;
	} else if (sig == CALL_SIG) {
		it->lnidx = ipop(it);
		it->indent++;
	} // else, just have *_SIG popped
 	return 0;
}

int bn_while(instance *it)
{
	variable *cond = (variable *)ipop(it);
	uint32_t result = cond->value.p;

	if (result == 0) {
		it->sindent = SKIP | it->indent;
		ipush(it, (uint32_t)-1);
	} else {
		ipush(it, it->lnidx);
	}
	ipush(it, WHILE_SIG);
	ipush(it, 0);
	ipush(it, 0); // need to ret
	return 0;
}

int bn_func(instance *it)
{
	variable *f = igetarg(it, 0);
	f->type = FUNC;
	f->value.p = it->lnidx;
	it->sindent = SKIP | it->indent;
	ipush(it, FUNC_SIG);
	ipush(it, 0); // for ret
	return 0;
}

int bn_solve(instance *it)
{
	variable *s = igetarg(it, 0);
	variable **ops = iparse(it, (const char *)s->value.p);
	if (ops == 0) {
		ipush(it, (uint32_t)make_varf(0, 0.0f));
		// return zero, don't let bad solves break the script
		return 0;
	}

	variable *a = isolve(it, ops, 0);
	free(ops);

	ipush(it, (uint32_t)a);
	return 0;
}

