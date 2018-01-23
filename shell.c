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
	interpreter_define_value(&interp, "answer", "42");
	interpreter_define_cfunc(&interp, "test", test);

	if (argc > 1) {
		for (int i = 1; i < argc; i++) {
			int result = interpreter_doline(&interp, argv[i]);
			if (result != 0)
				printf("%d\n", result);
		}
	}

	return 0;
}
