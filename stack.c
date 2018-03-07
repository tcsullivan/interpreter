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

const char *igetarg_string(interpreter *interp, uint32_t index)
{
	if (index >= interp->stidx)
		return 0;
	variable *v = igetarg(interp, index);
	return (const char *)v->value.p;
}

float igetarg_number(interpreter *interp, uint32_t index)
{
	if (index >= interp->stidx)
		return 0;
	variable *v = igetarg(interp, index);
	return v->value.f;
}

