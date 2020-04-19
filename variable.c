/**
 * @file variable.c
 * Defines variable data structure, and provides functions for variable
 * creation.
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

#include "variable.h"
#include "string.h"

#include <ctype.h>
#include <stdlib.h>

variable *make_varf(variable *v, float f)
{
	if (v == 0) {
		v = (variable *)malloc(sizeof(variable));
		v->tmp = 1;
	}
	v->type = NUMBER;
	v->value.f = f;
	return v;
}

variable *make_vars(variable *v, const char *s)
{
	if (v == 0) {
		v = (variable *)malloc(sizeof(variable));
		v->tmp = 1;
	}
	v->type = STRING;
	v->value.p = (size_t)strclone(s);
	return v;
}

variable *varclone(variable *n)
{
	variable *v = (variable *)malloc(sizeof(variable));
	v->tmp = 1;
	v->type = n->type;
	if (n->type == STRING)
		v->value.p = (size_t)strclone((char *)n->value.p);
	else
		v->value.p = n->value.p;
	return v;
}

variable *make_num(const char *text)
{
	int decimal = -1;
	char valid = 0;

	int i = 0;
	if (text[0] == '-')
		i++;
	do {
		if (text[i] == '.') {
			if (decimal >= 0) {
				valid = 0;
				break;
			}
			decimal = i;
		} else if (isdigit(text[i])) {
			valid |= 1;
		} else {
			break;
		}
	} while (text[++i] != '\0');

	if (valid == 0)
		return 0;

	char *buf = (char *)malloc(i + 1);
	strncpy(buf, text, i);
	buf[i] = '\0';

	variable *v = make_varf(0, strtof(buf, 0));
	free(buf);
	return v;
}

