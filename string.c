/**
 * @file string.c
 * Provides string.h from stdlib, plus some extra functions.
 *
 * Copyright (C) 2018 Clyne Sullivan
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "string.h"

#include <stdlib.h>

char *strnclone(const char *s, size_t c)
{
	char *b = strncpy((char *)malloc(c + 1), s, c);
	b[c] = '\0';
	return b;
}

char *strclone(const char *s)
{
	return strnclone(s, strlen(s)); 
}

char *fixstring(const char *s)
{
	char *n = malloc(strlen(s) + 1 - 2);
	int j = 0;
	for (int i = 1; s[i] != '\"'; i++, j++) {
		if (s[i] == '\\') {
			if (s[i + 1] == 'n')
				n[j] = '\n';
			i++;
		} else {
			n[j] = s[i];
		}
	}
	n[j] = '\0';
	return n;
}

