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

#ifndef _HEADER_H_
#define _HEADER_H_

struct dump_driver_data;
struct object_surface;

void print_indent(unsigned indent, const char *fmt, ...);
void print_u8_array(unsigned indent, const char *name,
		    unsigned char *array, unsigned x);
void print_s8_array(unsigned indent, const char *name,
		    signed char *array, unsigned x);
void print_s16_array(unsigned indent, const char *name,
		     signed short *array, unsigned x);
void print_u8_matrix(unsigned indent, const char *name,
		     unsigned char *array, unsigned x, unsigned y);
void print_s8_matrix(unsigned indent, const char *name,
		     signed char *matrix, unsigned x, unsigned y);
void print_s16_matrix(unsigned indent, const char *name,
		      signed short *matrix, unsigned x, unsigned y);

void mpeg2_dump_prepare(struct dump_driver_data *driver_data);
void mpeg2_dump_header(struct dump_driver_data *driver_data, void *slice_data,
		       unsigned int slice_size);

void h264_dump_prepare(struct dump_driver_data *driver_data);
void h264_dump_header(struct dump_driver_data *driver_data, struct object_surface *surface);

void h265_dump_prepare(struct dump_driver_data *driver_data);
void h265_dump_header(struct dump_driver_data *driver_data, void *slice_data,
		      unsigned int slice_size);

#endif
