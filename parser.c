#include <parser.h>

#include <stdbool.h>
#include <stdlib.h>

static const char *interpreter_operators = "=(";

bool strcmp(const char *a, const char *b)
{
	int i = 0;
	for (; a[i] == b[i] && a[i] != '\0'; i++);
	return a[i] == b[i];
}

bool strncmp(const char *a, const char *b, int count)
{
	int i = 0;
	for (; a[i] == b[i] && i < count; i++);
	return i == count;
}

uint8_t isalpha(char c)
{
	return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}

uint8_t isnum(char c)
{
	return (c >= '0' && c <= '9');
}

uint8_t isname(char c)
{
	return isalpha(c) || isnum(c);
}

uint8_t isspace(char c)
{
	return (c == ' ' || c == '\t' || c == '\n');
}

uint8_t isoper(char c)
{
	for (uint8_t i = 0; i < sizeof(interpreter_operators); i++) {
		if (c == interpreter_operators[i])
			return 1;
	}

	return 0;
}

void interpreter_init(interpreter *interp)
{
	interp->status = READY;
	interp->vcount = 0;
	interp->vars = (variable *)calloc(32, sizeof(variable));
	interp->names = (char **)calloc(32, sizeof(char *));
	interp->stack = (stack_t *)calloc(64, sizeof(stack_t));
}

void interpreter_define_value(interpreter *interp, const char *name, int32_t value)
{
	interp->names[interp->vcount] = (char *)name; 
	interp->vars[interp->vcount].nameidx = interp->vcount;
	interp->vars[interp->vcount].type = VALUE;
	interp->vars[interp->vcount].value = (uint32_t)value;
	interp->vcount++;
}

void interpreter_define_cfunc(interpreter *interp, const char *name, func_t addr)
{
	interp->names[interp->vcount] = (char *)name;
	interp->vars[interp->vcount].nameidx = interp->vcount;
	interp->vars[interp->vcount].type = CFUNCTION;
	interp->vars[interp->vcount].value = (uint32_t)addr;
	interp->vcount++;
}

int32_t interpreter_get_value(interpreter *interp, const char *name)
{
	for (uint16_t i = 0; i < interp->vcount; i++) {
		if (!strcmp(interp->names[i], name))
			return (int32_t)interp->vars[i].value;
	}

	return 0;
}

/**
 * doline section
 */

bool namencmp(const char *name, const char *s)
{
	uint16_t i;
	for (i = 0; name[i] == s[i] && s[i] != '\0'; i++);
	return (name[i] == '\0' && !isname(s[i]));
}

uint16_t spacecount(const char *s)
{
	uint16_t i;
	for (i = 0; isspace(s[i]); i++);
	return i;
}

char *copystr(const char *s, char end)
{
	uint16_t len = 0;
	while (s[len++] != end);
	char *buf = (char *)malloc(len);
	for (uint16_t i = 0; i < len; i++)
		buf[i] = s[i];
	return buf;
}

char *copysubstr(const char *s, int end)
{
	char *buf = (char *)malloc(end);
	for (uint16_t i = 0; i < end; i++)
		buf[i] = s[i];
	return buf;
}

variable *interpreter_getvar(interpreter *interp, const char *line)
{
	for (uint16_t i = 0; i < interp->vcount; i++) {
		if (namencmp(interp->names[i], line))
			return &interp->vars[i];
	}

	return 0;
}

int interpreter_doline(interpreter *interp, const char *line)
{
	variable *bits[16];
	uint16_t offset = 0, boffset = 0;

	// check for var/func set or usage
	int end;
getvar:
	for (end = 0; isname(line[end]); end++);
	variable *var = interpreter_getvar(interp, line);

	if (var != 0) {
		bits[boffset++] = var;
	} else {
		// defining new variable
		interpreter_define_value(interp, copysubstr(line, end), 0);
		goto getvar; // try again
	}

	// skip whitespace/name
	offset += end;
	offset += spacecount(line + offset); 

	if (boffset == 0 && line[offset] != '=')
		return -1; // variable not found

	// find operator
	if (line[offset] == '\0') {
		// print value
		return -99;
	} else if (line[offset] == '=') {
		// assignment/expression
		offset++;
		offset += spacecount(line + offset);
		bits[0]->value = (uint32_t)copystr(line + offset, '\0');
	} else if (line[offset] == '(') {
		// function call
		offset++;
		if (bits[0]->type != FUNCTION && bits[0]->type != CFUNCTION)
			return -2;
		offset += spacecount(line + offset);
		
		// collect arg offsets
		uint16_t offsets[8];
		uint8_t ooffset = 0;
		while (line[offset] != ')' && line[offset] != '\0') {
			offsets[ooffset] = offset;
			offset += spacecount(line + offset);

			uint8_t isvn = 1;
			do {
				if (line[offset] == ' ' || line[offset] == '\t') {
					offset += spacecount(line + offset);
					isvn = 0;
				}

				if (line[offset] == ',') {
					offset++;
					ooffset++;
					break;
				} else if (line[offset] == ')') {
					ooffset++;
					break;
				} else if (isvn == 0) {
					return -3;
				}
			} while (++offset);
		}

		// populate stack
		for (uint8_t i = 0; i < ooffset; i++) {
			uint16_t j;
			for (j = offsets[i]; line[j] != ' ' && line[j] != '\t' &&
					line[j] != ',' && line[j] != ')'; j++);
			j -= offsets[i];

			variable *var = interpreter_getvar(interp, line + offsets[i]);
			if (var != 0)
				interp->stack[i] = copystr((char *)var->value, '\0');
			else
				interp->stack[i] = copysubstr(line + offsets[i], j);
		}

		((func_t)bits[0]->value)(interp->stack);
	} else {
		return -2; // invalid operation
	}

	return 0;
}

