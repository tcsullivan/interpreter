#include <stdarg.h>

char *snprintf(char *buf, unsigned int max, const char *format, ...)
{
	(void)max;
	va_list args;
	va_start(args, format);

	buf[0] = '0';
	buf[1] = '\0';

	va_end(args);
	return buf;
}

float strtof(const char *s, char **endptr)
{
	(void)s;
	(void)endptr;
	return 0.0f;
}
