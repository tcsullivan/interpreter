#include "ops.h"

#include <string.h>

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
void iop_mod(variable *, variable *, variable *);

char *iops[IOPS_COUNT] = {
	"*", "/", "%", "+", "-", "<<", ">>", "<=",
	"<", ">=", ">", "==", "!=", "&", "^", "|"
};

operation_t iopfuncs[IOPS_COUNT] = {
	iop_mult, iop_div, iop_mod, iop_add, iop_sub,
	iop_shl, iop_shr, iop_lte, iop_lt, iop_gte,
	iop_gt, iop_eq, iop_ne, iop_and, iop_xor,
	iop_or
};


void iop_add(variable *r, variable *a, variable *b)
{
	r->value.f = a->value.f + b->value.f;
}

void iop_sub(variable *r, variable *a, variable *b)
{
	r->value.f = a->value.f - b->value.f;
}

void iop_mult(variable *r, variable *a, variable *b)
{
	r->value.f = a->value.f * b->value.f;
}

void iop_div(variable *r, variable *a, variable *b)
{
	r->value.f = a->value.f / b->value.f;
}

void iop_and(variable *r, variable *a, variable *b)
{
	r->value.f = (float)((int)a->value.f & (int)b->value.f);
}

void iop_or(variable *r, variable *a, variable *b)
{
	r->value.f = (float)((int)a->value.f | (int)b->value.f);
}

void iop_xor(variable *r, variable *a, variable *b)
{
	r->value.f = (float)((int)a->value.f ^ (int)b->value.f);
}

void iop_shr(variable *r, variable *a, variable *b)
{
	r->value.f = (float)((int)a->value.f >> (int)b->value.f);
}

void iop_shl(variable *r, variable *a, variable *b)
{
	r->value.f = (float)((int)a->value.f << (int)b->value.f);
}

void iop_eq(variable *r, variable *a, variable *b)
{
	if (a->valtype == STRING && b->valtype == STRING)
		r->value.f = (float)!strcmp((char *)a->value.p, (char *)b->value.p);
	else
		r->value.f = a->value.f == b->value.f;
}

void iop_lt(variable *r, variable *a, variable *b)
{
	r->value.f = a->value.f < b->value.f;
}

void iop_gt(variable *r, variable *a, variable *b)
{
	r->value.f = a->value.f > b->value.f;
}

void iop_lte(variable *r, variable *a, variable *b)
{
	r->value.f = a->value.f <= b->value.f;
}

void iop_gte(variable *r, variable *a, variable *b)
{
	r->value.f = a->value.f >= b->value.f;
}

void iop_ne(variable *r, variable *a, variable *b)
{
	r->value.f = a->value.f != b->value.f;
}

void iop_mod(variable *r, variable *a, variable *b)
{
	r->value.f = (float)((int)a->value.f % (int)b->value.f);
}

