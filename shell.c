#include <parser.h>

#include <stdio.h>

void test(stack_t *stack)
{
	printf("%s\n", stack[0]);
}

int main(int argc, char *argv[])
{
	interpreter interp;

	interpreter_init(&interp);
	interpreter_define_value(&interp, "answer", 42);
	interpreter_define_cfunc(&interp, "test", test);

	if (argc == 2) {
		printf("%d\n", interpreter_doline(&interp, argv[1]));
	}

	return 0;
}
