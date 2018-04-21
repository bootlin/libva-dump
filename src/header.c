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

static void header_dump_head(unsigned int index)
{
	printf("	{\n");
	printf("		.index = %d,\n", index);
	printf("		.header = {\n");
}

static void header_dump_tail(void)
{
	printf("		},\n");
	printf("	},\n");
}

static void header_dump_format(const char *field, const char *format, ...)
{
	char buffer[1024];
	va_list args;

	va_start(args, format);
	vsnprintf((char *) &buffer, sizeof(buffer), format, args);

	printf("			.%s = %s,\n", field, buffer);

	va_end(args);
}

void mpeg2_header_dump(struct dump_driver_data *driver_data, VAPictureParameterBufferMPEG2 *parameters)
{
	struct object_surface *surface_object;
	char *slice_type;
	unsigned int forward_reference_index;
	unsigned int backward_reference_index;
	unsigned int index = driver_data->frame_index;

	header_dump_head(index);

	if (parameters->picture_coding_type == 1)
		slice_type = "PCT_I";
	else if (parameters->picture_coding_type == 2)
		slice_type = "PCT_P";
	else if (parameters->picture_coding_type == 3)
		slice_type = "PCT_B";
	else
		slice_type = "PCT_INVALID";

	header_dump_format("picture_coding_type", slice_type);
	header_dump_format("f_code[0][0]", "%d", (parameters->f_code >> 12) & 0xf);
	header_dump_format("f_code[0][1]", "%d", (parameters->f_code >> 8) & 0xf);
	header_dump_format("f_code[1][0]", "%d", (parameters->f_code >> 4) & 0xf);
	header_dump_format("f_code[1][1]", "%d", (parameters->f_code >> 0) & 0xf);
	header_dump_format("intra_dc_precision", "%d", parameters->picture_coding_extension.bits.intra_dc_precision);
	header_dump_format("picture_structure", "%d", parameters->picture_coding_extension.bits.picture_structure);
	header_dump_format("top_field_first", "%d", parameters->picture_coding_extension.bits.top_field_first);
	header_dump_format("frame_pred_frame_dct", "%d", parameters->picture_coding_extension.bits.frame_pred_frame_dct);
	header_dump_format("concealment_motion_vectors", "%d", parameters->picture_coding_extension.bits.concealment_motion_vectors);
	header_dump_format("q_scale_type", "%d", parameters->picture_coding_extension.bits.q_scale_type);
	header_dump_format("intra_vlc_format", "%d", parameters->picture_coding_extension.bits.intra_vlc_format);
	header_dump_format("alternate_scan", "%d", parameters->picture_coding_extension.bits.alternate_scan);

	surface_object = (struct object_surface *) object_heap_lookup(&driver_data->surface_heap, parameters->forward_reference_picture);
	if (surface_object != NULL)
		forward_reference_index = surface_object->index;
	else
		forward_reference_index = index;

	header_dump_format("forward_ref_index", "%d", forward_reference_index);

	surface_object = (struct object_surface *) object_heap_lookup(&driver_data->surface_heap, parameters->backward_reference_picture);
	if (surface_object != NULL)
		backward_reference_index = surface_object->index;
	else
		backward_reference_index = index;

	header_dump_format("backward_ref_index", "%d", backward_reference_index);

	header_dump_tail();
}
