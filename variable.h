/**
 * @file variable.h
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

#ifndef VARIABLE_H_
#define VARIABLE_H_

#include <stdint.h>
#include <stddef.h>

/**
 * Data for a script variable.
 */
typedef struct {
	uint8_t tmp    :1; /**< If zero, variable cannot be free'd */
	uint8_t type   :3; /**< The variable's type */
	uint8_t unused :4; /**< Unused... */
	uint8_t array;     /**< Variable is array if >0, this defines array size */
	union {
		float f;
		size_t p;
	} value; /**< The variable's value, either float or a pointer */
} variable;

/**
 * Defines possible variable types.
 */
enum VARTYPE {
	NUMBER = 0, /**< Number/float */
	STRING,     /**< String of text */
	OPERATOR,   /**< Arithmetical operation */
	FUNC,       /**< In-script function */
	CFUNC,      /**< C function */
};

/**
 * Makes a number out of the given variable.
 * @param v the variable to use, if zero one is malloc'd
 * @param f the number to assign the variable
 * @return the new float variable
 */
variable *make_varf(variable *v, float f);

/**
 * Makes a string out of the given variable.
 * @param v the variable to use, if zero one is malloc'd
 * @param f the string to assign the variable
 * @return the new string variable
 */
variable *make_vars(variable *v, const char *s);

/**
 * Creates a temporary number variable out of the given text.
 * @param text the string to convert to a number
 * @return a number variable with the converted value
 */
variable *make_num(const char *text);

/**
 * Clones a variable into a new, malloc'd variable.
 * @param n the variable to clone
 * @return the cloned, malloc'd variable
 */
variable *varclone(variable *n);

#endif // VARIABLE_H_

