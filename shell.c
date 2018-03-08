#include <parser.h>
#include <builtins.h>

#include "stack.h"

#include <memory.h>
#include <stdio.h>
//#include <stdlib.h>
#include <string.h>

int s_put(interpreter *it)
{
	variable *v = igetarg(it, 0);
	if (v->valtype == NUMBER)
		printf("%.f", v->value.f);
	else
		printf("%s", (char *)v->value.p);
	return 0;
}

int main(int argc, char **argv)
{
	interpreter interp;

	if (argc != 2) {
		printf("Usage: %s file\n", argv[0]);
		return -1;
	}

	FILE *fp = fopen(argv[1], "r");
	if (fp == 0) {
		printf("Could not open file: %s\n", argv[1]);
		return -1;
	}

	iinit(&interp);
	inew_cfunc(&interp, "print", s_put);

	char *line = 0;
	unsigned int size;
	int result;
	while (getline(&line, &size, fp) != -1) {
		*strchr(line, '\n') = '\0';
		result = idoline(&interp, line);
		if (result != 0)
			printf("Error: %d\n", result);
	}

	fclose(fp);
	iend(&interp);
	return 0;
}
