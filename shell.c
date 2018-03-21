#include <stdio.h>
#include <string.h>

#include "parser.h"

int print(instance *it)
{
	variable *s = igetarg(it, 0);
	if (s->type == NUMBER) {
		if (s->value.f == (int)s->value.f)
			printf("%d\n", (int)s->value.f);
		else
			printf("%.3f\n", s->value.f);
	} else if (s->value.p != 0) {
		printf("%s\n", (char *)s->value.p);
	}
	return 0;
}

int main(int argc, char **argv)
{
	if (argc != 2) {
		printf("Usage: %s file\n", argv[0]);
		return -1;
	}

	FILE *fp = fopen(argv[1], "r");
	if (fp == 0) {
		printf("Could not open file: %s\n", argv[1]);
		return -1;
	}

	instance *it = inewinstance();
	inew_cfunc(it, "print", print);

	char *line = 0;
	size_t size;
	int result;
	while (getline(&line, &size, fp) != -1) {
		*strchr(line, '\n') = '\0';
		result = idoline(it, line);
		if (result != 0)
			printf("Error: %d\n", result);
		//if (it->ret != 0)
		//	printf("%s = %f\n", line, it->ret->value.f);
	}

	fclose(fp);
	idelinstance(it);
	return 0;
}

