/**
 * @file builtins.h
 * Provides built-in functions for the interpreter
 * These must be loaded for an instance to do anything meaningful.
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

#ifndef BUILTINS_H_
#define BUILTINS_H_

#include "parser.h"

#define SKIP_SIG (size_t)-5
#define CALL_SIG (size_t)-6

// open bracket 'operator', for use in a compiled line
extern variable bopen;

// close bracket 'operator', for use in a compiled line
extern variable bclose;

int bracket_open(instance *it);
int bracket_close(instance *it);

/**
 * Loads the built-in functions into the given instance.
 * @param it the instance to use
 */
void iload_builtins(instance *it);

/**
 * The built-in 'end' function, called at closed scopes (i.e. '}').
 * Exposed for the parser so that closing brackets can be tied to this call.
 * @param it the current instance
 */
int bn_end(instance *it);

#endif // BUILTINS_H_
