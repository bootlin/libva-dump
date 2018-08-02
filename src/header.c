/*
 * Copyright (C) 2018 Paul Kocialkowski <paul.kocialkowski@bootlin.com>
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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <limits.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "dump.h"
#include "header.h"
#include "surface.h"

void print_indent(unsigned indent, const char *fmt, ...)
{
	va_list args;
	int i;

	for (i = 0; i < indent; i++)
		printf("\t");

	va_start(args, fmt);
	vprintf(fmt, args);
	va_end(args);
}

void print_s16_array(unsigned indent, const char *name,
		     short *array, unsigned x)
{
	int i;

	print_indent(indent, ".%s = { ", name);
	for (i = 0; i < x; i++) {
		printf("%d, ", array[i]);
	}
	printf("},\n");
}

void print_u8_matrix(unsigned indent, const char *name,
		     unsigned char *array, unsigned x, unsigned y)
{
	int i;

	print_indent(indent, ".%s = {", name);

	if (x > 1)
		printf("\n");
	else
		printf(" ");

	for (i = 0; i < x; i++) {
		int j;

		if (x > 1)
			print_indent(indent + 1, "{ ");

		for (j = 0; j < y; j++)
			printf("%u, ", *(array + i * y + j));

		if (x > 1)
			printf("},\n");
	}

	if (x > 1)
		print_indent(indent, "},\n", name);
	else
		printf("},\n");
}

void print_s16_matrix(unsigned indent, const char *name,
		      short *array, unsigned x, unsigned y)
{
	int i;

	print_indent(indent, ".%s = {\n", name);
	for (i = 0; i < x; i++) {
		int j;

		print_indent(indent + 1, "{ ", i);
		for (j = 0; j < y; j++)
			printf("%d, ", *(array + i * y + j));
		printf("},\n");
	}
	print_indent(indent, "},\n", name);
}
