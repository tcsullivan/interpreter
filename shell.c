#include <parser.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void s_put(interpreter *it)
{
	char *s = igetarg_string(it, 0);
	if (s == 0)
		s = "(null)";
	printf("%s\n", s);
}

void s_type(interpreter *it)
{
	if (it->stidx != 1)
		return;
	variable *v = (variable *)it->stack[0];
	switch (v->valtype) {
	case STRING:
		puts("string");
		break;
	case INTEGER:
		puts("integer");
		break;
	case FLOAT:
		puts("float");
		break;
	case FUNC:
		puts(v->value == 0 ? "undefined" : "func" );
		break;
	default:
		puts("unknown");
		break;
	}
}

void quit(interpreter *it)
{
	(void)it;
	exit(0);
}

int main()
{
	interpreter interp;

	iinit(&interp);
	inew_cfunc(&interp, "put", s_put);
	inew_cfunc(&interp, "tp", s_type);
	inew_cfunc(&interp, "q", quit);


	char *line = 0;
	unsigned int size;
	int result;
	while (1) {
		getline(&line, &size, stdin);
		*strchr(line, '\n') = '\0';
		result = idoline(&interp, line);
		if (result != 0)
			printf("Error: %d\n", result);
	}

	return 0;
}
