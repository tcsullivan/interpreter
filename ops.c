#include "ops.h"

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
	"+", "-", "*", "/", "&", "|", "^", ">>", "<<",
	"==", "<", ">", "<=", ">=", "!=", "%"
};

operation_t iopfuncs[IOPS_COUNT] = {
	iop_add, iop_sub, iop_mult, iop_div, iop_and,
	iop_or, iop_xor, iop_shr, iop_shl,
	iop_eq, iop_lt, iop_gt, iop_lte, iop_gte, iop_ne,
	iop_mod
};


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

void iop_mod(variable *r, variable *a, variable *b)
{
	if (a->valtype == INTEGER && b->valtype == INTEGER)
		INT(r) = INT(a) % INT(b);
	else
		INT(r) = 0;
}

