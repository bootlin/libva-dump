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

static void mpeg2_dump_slice_params(struct dump_driver_data *driver_data,
				    unsigned int indent)
{
	VAPictureParameterBufferMPEG2 *picture_params =
		&driver_data->params.mpeg2.picture;
	struct object_surface *surface_object;
	char *slice_type;
	unsigned int forward_reference_index;
	unsigned int backward_reference_index;
	unsigned int index = driver_data->frame_index;

	print_indent(indent++, ".frame.mpeg2.slice_params = {\n");

	if (picture_params->picture_coding_type == 1)
		slice_type = "V4L2_MPEG2_SLICE_TYPE_I";
	else if (picture_params->picture_coding_type == 2)
		slice_type = "V4L2_MPEG2_SLICE_TYPE_P";
	else if (picture_params->picture_coding_type == 3)
		slice_type = "V4L2_MPEG2_SLICE_TYPE_B";
	else
		slice_type = "V4L2_MPEG2_SLICE_TYPE_INVALID";

	print_indent(indent, ".slice_type = %s,\n", slice_type);
	print_indent(indent, ".f_code = { %d, %d, %d, %d },\n",
		     (picture_params->f_code >> 12) & 0xf,
		     (picture_params->f_code >> 8) & 0xf,
		     (picture_params->f_code >> 4) & 0xf,
		     (picture_params->f_code >> 0) & 0xf);
	print_indent(indent, ".intra_dc_precision = %d,\n",
		     picture_params->picture_coding_extension.bits.intra_dc_precision);
	print_indent(indent, ".picture_structure = %d,\n",
		     picture_params->picture_coding_extension.bits.picture_structure);
	print_indent(indent, ".top_field_first = %d,\n",
		     picture_params->picture_coding_extension.bits.top_field_first);
	print_indent(indent, ".frame_pred_frame_dct = %d,\n",
		     picture_params->picture_coding_extension.bits.frame_pred_frame_dct);
	print_indent(indent, ".concealment_motion_vectors = %d,\n",
		     picture_params->picture_coding_extension.bits.concealment_motion_vectors);
	print_indent(indent, ".q_scale_type = %d,\n",
		     picture_params->picture_coding_extension.bits.q_scale_type);
	print_indent(indent, ".intra_vlc_format = %d,\n",
		     picture_params->picture_coding_extension.bits.intra_vlc_format);
	print_indent(indent, ".alternate_scan = %d,\n",
		     picture_params->picture_coding_extension.bits.alternate_scan);

	surface_object = (struct object_surface *)
		object_heap_lookup(&driver_data->surface_heap,
				   picture_params->forward_reference_picture);
	if (surface_object != NULL)
		forward_reference_index = surface_object->index;
	else
		forward_reference_index = index;

	print_indent(indent, ".forward_ref_index = %d,\n", forward_reference_index);

	surface_object = (struct object_surface *)
		object_heap_lookup(&driver_data->surface_heap,
				   picture_params->backward_reference_picture);
	if (surface_object != NULL)
		backward_reference_index = surface_object->index;
	else
		backward_reference_index = index;

	print_indent(indent, ".backward_ref_index = %d,\n", backward_reference_index);

	print_indent(--indent, "},\n");
}

static void mpeg2_dump_quantization(struct dump_driver_data *driver_data,
				    unsigned int indent)
{
	VAIQMatrixBufferMPEG2 *quantization_params =
		&driver_data->params.mpeg2.quantization;

	print_indent(indent++, ".frame.mpeg2.quantization = {\n");

	print_indent(indent, ".load_intra_quantiser_matrix = %d,\n",
		     quantization_params->load_intra_quantiser_matrix);
	print_indent(indent, ".load_non_intra_quantiser_matrix = %d,\n",
		     quantization_params->load_non_intra_quantiser_matrix);
	print_indent(indent, ".load_chroma_intra_quantiser_matrix = %d,\n",
		     quantization_params->load_chroma_intra_quantiser_matrix);
	print_indent(indent, ".load_chroma_non_intra_quantiser_matrix = %d,\n",
		     quantization_params->load_chroma_non_intra_quantiser_matrix);

	print_u8_matrix(indent, "intra_quantiser_matrix",
			(uint8_t *)&quantization_params->intra_quantiser_matrix,
			1, 64);
	print_u8_matrix(indent, "non_intra_quantiser_matrix",
			(uint8_t *)&quantization_params->non_intra_quantiser_matrix,
			1, 64);
	print_u8_matrix(indent, "chroma_intra_quantiser_matrix",
			(uint8_t *)&quantization_params->chroma_intra_quantiser_matrix,
			1, 64);
	print_u8_matrix(indent, "chroma_non_intra_quantiser_matrix",
			(uint8_t *)&quantization_params->chroma_non_intra_quantiser_matrix,
			1, 64);

	print_indent(--indent, "},\n");
}

void mpeg2_dump_prepare(struct dump_driver_data *driver_data)
{
}

void mpeg2_dump_header(struct dump_driver_data *driver_data)
{
	unsigned int index = driver_data->frame_index;
	unsigned int indent = 1;

	print_indent(indent++, "{\n");
	print_indent(indent, ".index = %d,\n", index);

	mpeg2_dump_slice_params(driver_data, indent);
	mpeg2_dump_quantization(driver_data, indent);

	print_indent(--indent, "},\n");
}
