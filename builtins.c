#include "builtins.h"
#include "stack.h"
#include "shelpers.h"

#include <memory.h>
#include <string.h>

int ifunc_set(interpreter *it);
int ifunc_label(interpreter *it);
int ifunc_end(interpreter *it);
int ifunc_if(interpreter *it);
int ifunc_do(interpreter *it);
int ifunc_while(interpreter *it);
int ifunc_ret(interpreter *it);
int ifunc_else(interpreter *it);

const func_t indent_up[IUP_COUNT] = {
	ifunc_if, ifunc_do, ifunc_label
};

const func_t indent_down[IDOWN_COUNT] = {
	ifunc_else, ifunc_end, ifunc_while, 
};

void iload_core(interpreter *interp)
{
	inew_cfunc(interp, "set", ifunc_set);
	inew_cfunc(interp, "func", ifunc_label);
	inew_cfunc(interp, "end", ifunc_end);
	inew_cfunc(interp, "if", ifunc_if);
	inew_cfunc(interp, "do", ifunc_do);
	inew_cfunc(interp, "while", ifunc_while);
	inew_cfunc(interp, "ret", ifunc_ret);
	inew_cfunc(interp, "else", ifunc_else);
}

int ifunc_set(interpreter *it)
{
	variable *n = igetarg(it, 0);
	variable *v = igetarg(it, 1);

	if (n == 0)
		return -1;

	if (n->valtype == STRING)
		free((void *)n->value.p);
	n->valtype = v->valtype;
	n->value.p = v->value.p;
	return 0;
}

int ifunc_label(interpreter *it)
{
	variable *n = igetarg(it, 0);
	
	if (n == 0)
		return -1;

	n->valtype = FUNC;
	n->value.p = it->lnidx;
	iskip(it);
	return 0;
}

int ifunc_if(interpreter *it)
{
	int v = igetarg(it, 0)->value.p;
	if (v == 0)
		iskip(it);
	void *arg = ipop(it);
	ipush(it, (void *)v);
	ipush(it, (void *)-1);
	ipush(it, arg);
	return 0;
}

int ifunc_end(interpreter *it)
{
	if (it->stidx == 0)
		return 0;

	uint32_t lnidx = (uint32_t)ipop(it) + 1;
	if (lnidx == 0) { // from an if, have conditional
		ipop(it); // whatever
	} else {
		if (lnidx == (uint32_t)-1) {
			// script-func call
			lnidx = (uint32_t)ipop(it);
			it->indent = (uint32_t)ipop(it);
		}
		it->lnidx = lnidx;
	}
	return 0;
}

int ifunc_else(interpreter *it)
{
	if (it->stidx == 0)
		return 0;

	ipop(it); // the -1
	int cond = (int)ipop(it);
	it->indent++;
	if (cond != 0)
		iskip(it);
	// otherwise it's whatever?
	ipush(it, 0);
	ipush(it, (void *)-1);

	return 0;
}

int ifunc_do(interpreter *it)
{
	ipush(it, (void *)it->lnidx);
	return 0;
}

int ifunc_while(interpreter *it)
{
	int c = igetarg(it, 0)->value.p;
	ipop(it);
	int nidx = (int)ipop(it);
	if (c != 0) {
		//ipush(it, (void *)nidx);
		it->lnidx = nidx - 1;
	}
	ipush(it, 0);
	return 0;
}

void iret(interpreter *it, variable *v)
{
	switch (v->valtype) {
	case NUMBER:
		inew_number(it, "RET", v->value.f);
		break;
	case STRING:
		inew_string(it, "RET", (char *)v->value.p);
		break;
	default:
		return;
		break;
	}
	if (it->ret != 0) {
		if (it->ret->valtype == STRING && it->ret->value.p != 0)
			free((void *)it->ret->value.p);
		it->ret->valtype = v->valtype;
		it->ret->value.p = v->value.p;
		it->ret = 0;
	}
}

int ifunc_ret(interpreter *it)
{
	variable *v = igetarg(it, 0);
	iret(it, v);
	return 0;
}

