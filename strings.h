/**
 * @file string.h
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

#ifndef STRING_H_
#define STRING_H_

#include <stddef.h>
#include <string.h>

/**
 * Clones a string of a given size into a malloc'd buffer.
 * @param s the string to clone
 * @param c the number of characters to copy
 * @return the malloc'd copy
 */
char *strnclone(const char *s, size_t c);

/**
 * Clones a string into a malloc'd buffer.
 * @param s the string to clone
 * @return the malloc'd copy
 */
char *strclone(const char *s);

/**
 * 'Fixes' a string, by converting "\n" and others to '\n'.
 * @param s the string to fix
 * @return the fixed string, in a malloc'd buffer
 */
char *fixstring(const char *s);

#endif // STRING_H_
