#include "stack.h"

void ipush(interpreter *it, void *v)
{
	it->stack[it->stidx++] = v;
}

void *ipop(interpreter *it)
{
	return it->stack[--it->stidx];
}

void ipopm(interpreter *it, uint32_t count)
{
	it->stidx -= count;
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

