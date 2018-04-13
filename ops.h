/**
 * @file ops.h
 * Provides arithematic operators to the parser
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

#ifndef OPS_H_
#define OPS_H_

#include "variable.h"

/**
 * Defines the number of available operators.
 */
#define OPS_COUNT 20

#define OP_MAGIC  0xCAFE3900

typedef int (*opfunc_t)(variable **, variable *, variable *);

/**
 * An array of variable objects for each operator, exposed for the parser.
 */
extern variable opvars[];

/**
 * An array of operator names, exposed for the parser.
 */
extern const char *opnames[];

/**
 * Gets the variable for the given operator.
 * @param name the operator (e.g. "+")
 * @param retlen if not null, stores the operator string's length here
 * @return variable for the operator, zero if not found
 */
variable *igetop(const char *name, int *retlen);

#endif // OPS_H_
