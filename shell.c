#include <parser.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void test(interpreter *it)
{
	char *s = igetarg_string(it, 0);
	if (s == 0)
		s = "(null)";
	printf("%s\n", s);
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
	inew_integer(&interp, "answer", 42);
	inew_cfunc(&interp, "put", test);
	inew_cfunc(&interp, "exit", quit);


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
