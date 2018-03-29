/**
 * @file error.c
 * Provides a simple error-logging mechanism.
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

#include "error.h"

static int lastError = ENONE;

static const char *errorTable[] = {
	"no error",
	"undefined value",
	"bad parameter"
};

int seterror(int error)
{
	if (error < EMAX)
		lastError = error;
	return -lastError;
}

const char *geterror(void)
{
	return errorTable[lastError];
}
