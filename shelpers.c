#include "shelpers.h"

#include <stdlib.h>
#include <string.h>

char *strclone(const char *s)
{
	char *clone = (char *)malloc(strlen(s) + 1);
	strcpy(clone, s);
	return clone;
}

char *strnclone(const char *s, uint32_t n)
{
	char *clone = (char *)malloc(n + 1);
	strncpy(clone, s, n);
	clone[n] = '\0';
	return clone;
}

uint8_t eol(int c)
{
	return c == '\n' || c == '\0';
}

uint8_t eot(int c)
{
	return eol(c) || c == ' ';
}

uint8_t eoe(int c)
{
	return eol(c) || c == ')';
}

uint32_t findend(const char *s, char o, char c)
{
	uint8_t indent = 0;
	uint32_t i;
	for (i = 1; !eol(s[i]); i++) {
		if (s[i] == o) {
			indent++;
		} else if (s[i] == c) {
			if (indent == 0)
				break;
			else
				indent--;
		}
	}

	return i;
}

void skipblank(const char *s, uint8_t (*cmp)(int), uint32_t *offset)
{
	uint32_t i = *offset;
	while (!cmp(s[i])) {
		if (s[i] != ' ' && s[i] != '\t')
			break;
		i++;
	}
	*offset = i;
}

