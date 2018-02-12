#include "builtins.h"
#include "stack.h"
#include "shelpers.h"

#include <stdlib.h>

extern char *str_func;
extern char *str_undef;

int ifunc_set(interpreter *it);
int ifunc_label(interpreter *it);
int ifunc_end(interpreter *it);
int ifunc_if(interpreter *it);
int ifunc_do(interpreter *it);
int ifunc_while(interpreter *it);
int ifunc_ret(interpreter *it);

const func_t indent_up[IUP_COUNT] = {
	ifunc_if, ifunc_do, ifunc_label
};

const func_t indent_down[IDOWN_COUNT] = {
	ifunc_end, ifunc_while
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
}

int ifunc_set(interpreter *it)
{
	variable *n = igetarg(it, 0);
	variable *v = igetarg(it, 1);

	if (n == 0)
		return -1;

	n->valtype = v->valtype;
	n->value = v->value;
	if (n->svalue != 0 && n->svalue != str_func && n->svalue != str_undef)
		free(n->svalue);
	n->svalue = strclone(v->svalue);
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
	iskip(it);
	return 0;
}

int ifunc_if(interpreter *it)
{
	int v = igetarg_integer(it, 0);
	if (v == 0)
		iskip(it);
	void *tmp = ipop(it);
	ipush(it, (void *)-1);
	ipush(it, tmp);
	return 0;
}

int ifunc_end(interpreter *it)
{
	if (it->stidx == 0)
		return 0;

	uint32_t lnidx = (uint32_t)ipop(it) + 1;
	if (lnidx != 0)
		it->lnidx = lnidx;
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
		//ipush(it, (void *)nidx);
		it->lnidx = nidx - 1;
	}
	ipush(it, 0);
	return 0;
}

int ifunc_ret(interpreter *it)
{
	variable *v = igetarg(it, 0);
	switch (v->valtype) {
	case INTEGER:
		inew_integer(it, "RET", INT(v));
		break;
	case FLOAT:
		inew_float(it, "RET", FLOAT(v));
		break;
	case STRING:
		inew_string(it, "RET", v->svalue);
		break;
	default:
		return -1;
		break;
	}
	if (it->ret != 0) {
		it->ret->valtype = v->valtype;
		it->ret->value = v->value;
		it->ret->svalue = v->svalue;
		it->ret = 0;
	}
	return 0;
}

