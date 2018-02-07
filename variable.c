#include "variable.h"
#include "parser.h"

#include <stdio.h>
#include <stdlib.h>

extern char *str_undef;

void fsetstr(variable *f)
{
	if (f->svalue == 0 || f->svalue == str_undef)
		f->svalue = (char *)malloc(16);
	snprintf(f->svalue, 16, "%f", FLOAT(f));
}

void isetstr(variable *i)
{
	if (i->svalue == 0 || i->svalue == str_undef)
		i->svalue = (char *)malloc(12);
	snprintf(i->svalue, 12, "%d", INT(i));
}

variable *itostring(variable *v)
{
	switch (v->valtype) {
	case INTEGER:
		v->valtype = STRING;
		isetstr(v);
		break;
	case FLOAT:
		v->valtype = STRING;
		fsetstr(v);
		break;
	}
	return v;
}

variable *itoint(variable *v)
{
	switch (v->valtype) {
	case STRING:
		v->valtype = INTEGER;
		INT(v) = atoi(v->svalue);
		isetstr(v);
		break;
	case FLOAT:
		v->valtype = INTEGER;
		INT(v) = (int32_t)FLOAT(v);
		isetstr(v);
		break;
	}
	return v;
}

variable *itofloat(variable *v)
{
	switch (v->valtype) {
	case STRING:
		v->valtype = FLOAT;
		FLOAT(v) = strtof(v->svalue, 0);
		fsetstr(v);
		break;
	case INTEGER:
		v->valtype = FLOAT;
		FLOAT(v) = (float)INT(v);
		fsetstr(v);
		break;
	}
	return v;
}

