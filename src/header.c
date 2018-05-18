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

#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>

#include "dump.h"
#include "header.h"
#include "surface.h"

static void print_indent(unsigned indent, const char *fmt, ...)
{
	va_list args;
	int i;

	for (i = 0; i < indent; i++)
		printf("\t");

	va_start(args, fmt);
	vprintf(fmt, args);
	va_end(args);
}

void mpeg2_start_dump(struct dump_driver_data *driver_data)
{
	unsigned int index = driver_data->frame_index;

	print_indent(1, "{\n");
	print_indent(2, ".index = %d,\n", index);
	print_indent(2, ".header = {\n");
}

void mpeg2_stop_dump(struct dump_driver_data *driver_data)
{
	print_indent(2, "},\n");
	print_indent(1, "},\n");
}

void mpeg2_header_dump(struct dump_driver_data *driver_data, VAPictureParameterBufferMPEG2 *parameters)
{
	struct object_surface *surface_object;
	char *slice_type;
	unsigned int forward_reference_index;
	unsigned int backward_reference_index;
	unsigned int index = driver_data->frame_index;

	if (parameters->picture_coding_type == 1)
		slice_type = "PCT_I";
	else if (parameters->picture_coding_type == 2)
		slice_type = "PCT_P";
	else if (parameters->picture_coding_type == 3)
		slice_type = "PCT_B";
	else
		slice_type = "PCT_INVALID";

	print_indent(3, ".picture_coding_type = %s,\n", slice_type);
	print_indent(3, ".f_code = { %d, %d, %d, %d },\n",
		     (parameters->f_code >> 12) & 0xf,
		     (parameters->f_code >> 8) & 0xf,
		     (parameters->f_code >> 4) & 0xf,
		     (parameters->f_code >> 0) & 0xf);
	print_indent(3, ".intra_dc_precision = %d,\n",
		     parameters->picture_coding_extension.bits.intra_dc_precision);
	print_indent(3, ".picture_structure = %d,\n",
		     parameters->picture_coding_extension.bits.picture_structure);
	print_indent(3, ".top_field_first = %d,\n",
		     parameters->picture_coding_extension.bits.top_field_first);
	print_indent(3, ".frame_pred_frame_dct = %d,\n",
		     parameters->picture_coding_extension.bits.frame_pred_frame_dct);
	print_indent(3, ".concealment_motion_vectors = %d,\n",
		     parameters->picture_coding_extension.bits.concealment_motion_vectors);
	print_indent(3, ".q_scale_type = %d,\n",
		     parameters->picture_coding_extension.bits.q_scale_type);
	print_indent(3, ".intra_vlc_format = %d,\n",
		     parameters->picture_coding_extension.bits.intra_vlc_format);
	print_indent(3, ".alternate_scan = %d,\n",
		     parameters->picture_coding_extension.bits.alternate_scan);

	surface_object = (struct object_surface *) object_heap_lookup(&driver_data->surface_heap, parameters->forward_reference_picture);
	if (surface_object != NULL)
		forward_reference_index = surface_object->index;
	else
		forward_reference_index = index;

	print_indent(3, ".forward_ref_index = %d,\n", forward_reference_index);

	surface_object = (struct object_surface *) object_heap_lookup(&driver_data->surface_heap, parameters->backward_reference_picture);
	if (surface_object != NULL)
		backward_reference_index = surface_object->index;
	else
		backward_reference_index = index;

	print_indent(3, ".backward_ref_index = %d,\n", backward_reference_index);
}
