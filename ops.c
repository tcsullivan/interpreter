#include "ops.h"

#include <stdlib.h>
#include <string.h>

#define OP_DEF(o) int op_##o(variable *r, variable *a, variable *b)
#define OP_VAR(o) {0, OPERATOR, 0, {.p = (uint32_t)op_##o}}
#define OP_NONE   {0, OPERATOR, 0, {.p = 0x0BADCAFE}}

extern char *strclone(const char *s);

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

variable opvars[] = {
	OP_VAR(mul), OP_VAR(div), OP_VAR(mod), OP_NONE,
	OP_VAR(add), OP_VAR(sub), OP_VAR(shl), OP_VAR(shr),
	OP_VAR(lte), OP_VAR(lt), OP_VAR(gte), OP_VAR(gt),
	OP_VAR(eq), OP_VAR(ne), OP_VAR(and), OP_VAR(xor),
	OP_VAR(or), OP_VAR(set)
};

const char *opnames[] = {
	"*", "/", "%", 0,
	"+", "-", "<<", ">>",
	"<=", "<", ">=", ">",
	"==", "!=", "&", "^",
	"|", "=" 
};

OP_DEF(mul)
{
	if (a->type != NUMBER || b->type != NUMBER)
		return -1;
	r->type = NUMBER;
	r->value.f = a->value.f * b->value.f;
	return 0;
}
OP_DEF(div)
{
	if (a->type != NUMBER || b->type != NUMBER)
		return -1;
	r->type = NUMBER;
	r->value.f = a->value.f / b->value.f;
	return 0;
}
OP_DEF(mod)
{
	if (a->type != NUMBER || b->type != NUMBER)
		return -1;
	r->type = NUMBER;
	r->value.f = (int)a->value.f % (int)b->value.f;
	return 0;
}
OP_DEF(add)
{
	if (a->type != NUMBER || b->type != NUMBER)
		return -1;
	r->type = NUMBER;
	r->value.f = a->value.f + b->value.f;
	return 0;
}
OP_DEF(sub)
{
	if (a->type != NUMBER || b->type != NUMBER)
		return -1;
	r->type = NUMBER;
	r->value.f = a->value.f - b->value.f;
	return 0;
}
OP_DEF(shl)
{
	if (a->type != NUMBER || b->type != NUMBER)
		return -1;
	r->type = NUMBER;
	r->value.f = (int)a->value.f << (int)b->value.f;
	return 0;
}
OP_DEF(shr)
{
	if (a->type != NUMBER || b->type != NUMBER)
		return -1;
	r->type = NUMBER;
	r->value.f = (int)a->value.f >> (int)b->value.f;
	return 0;
}
OP_DEF(lte)
{
	if (a->type != NUMBER || b->type != NUMBER)
		return -1;
	r->type = NUMBER;
	r->value.f = a->value.f <= b->value.f;
	return 0;
}
OP_DEF(lt)
{
	if (a->type != NUMBER || b->type != NUMBER)
		return -1;
	r->type = NUMBER;
	r->value.f = a->value.f < b->value.f;
	return 0;
}
OP_DEF(gte)
{
	if (a->type != NUMBER || b->type != NUMBER)
		return -1;
	r->type = NUMBER;
	r->value.f = a->value.f >= b->value.f;
	return 0;
}
OP_DEF(gt)
{
	if (a->type != NUMBER || b->type != NUMBER)
		return -1;
	r->type = NUMBER;
	r->value.f = a->value.f > b->value.f;
	return 0;
}
OP_DEF(eq)
{
	r->type = NUMBER;
	if (a->type == NUMBER && b->type == NUMBER)
		r->value.f = a->value.f == b->value.f;
	else if (a->type == STRING && b->type == STRING)
		r->value.f = !strcmp((const char *)a->value.p, (const char *)b->value.p);
	else
		return -1;

	return 0;
}
OP_DEF(ne)
{
	if (a->type != NUMBER || b->type != NUMBER)
		return -1;
	r->type = NUMBER;
	r->value.f = a->value.f != b->value.f;
	return 0;
}
OP_DEF(and)
{
	if (a->type != NUMBER || b->type != NUMBER)
		return -1;
	r->type = NUMBER;
	r->value.f = (int)a->value.f & (int)b->value.f;
	return 0;
}
OP_DEF(xor)
{
	if (a->type != NUMBER || b->type != NUMBER)
		return -1;
	r->type = NUMBER;
	r->value.f = (int)a->value.f ^ (int)b->value.f;
	return 0;
}
OP_DEF(or)
{
	if (a->type != NUMBER || b->type != NUMBER)
		return -1;
	r->type = NUMBER;
	r->value.f = (int)a->value.f | (int)b->value.f;
	return 0;
}
OP_DEF(set)
{
	if (b->type == NUMBER) {
		a->type = NUMBER;
		a->value.f = b->value.f;
		r->type = NUMBER;
		r->value.f = a->value.f;
	} else if (b->type == STRING) {
		a->type = STRING;
		if (a->value.p != 0)
			free((void *)a->value.p);
		a->value.p = (uint32_t)strclone((char *)b->value.p);
		r->type = STRING;
		r->value.p = (uint32_t)strclone((char *)a->value.p);
	} else {
		return -1;
	}
	return 0;
}

