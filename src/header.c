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

static void print_s16_array(unsigned indent, const char *name,
			    short *array, unsigned x)
{
	int i;

	print_indent(indent, ".%s = { ", name);
	for (i = 0; i < x; i++) {
		printf("%d, ", array[i]);
	}
	printf("},\n");
}

static void print_u8_matrix(unsigned indent, const char *name,
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

static void print_s16_matrix(unsigned indent, const char *name,
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

#define DPB_SIZE	16

struct dpb_entry {
	VAPictureH264	pic;
	unsigned int	age;
	bool		used;
	bool		valid;
	bool		reserved;
};

struct dpb {
	struct dpb_entry	entries[DPB_SIZE];
	unsigned int		age;
};

static struct dpb local_dpb;

static struct dpb_entry *find_invalid_dpb_entry()
{
	unsigned int i;

	for (i = 0; i < DPB_SIZE; i++) {
		struct dpb_entry *entry = &local_dpb.entries[i];

		if (!entry->valid && !entry->reserved)
			return entry;
	}

	return NULL;
}

static struct dpb_entry *find_oldest_unused_dpb_entry()
{
	unsigned int min_age = UINT_MAX;
	unsigned int i;
	struct dpb_entry *match = NULL;

	for (i = 0; i < DPB_SIZE; i++) {
		struct dpb_entry *entry = &local_dpb.entries[i];

		if (!entry->used && (entry->age < min_age)) {
			min_age = entry->age;
			match = entry;
		}
	}

	return match;
}

static struct dpb_entry *find_dpb_entry()
{
	struct dpb_entry *entry;

	entry = find_invalid_dpb_entry();
	if (!entry)
		entry = find_oldest_unused_dpb_entry();

	return entry;
}

static int dpb_find_entry_index(struct dpb_entry *entry)
{
	return entry - &local_dpb.entries[0];
}

static struct dpb_entry *dpb_lookup(VAPictureH264 *pic, unsigned int *idx)
{
	unsigned int i;

	for (i = 0; i < DPB_SIZE; i++) {
		struct dpb_entry *entry = &local_dpb.entries[i];

		if (!entry->valid)
			continue;

		if (entry->pic.picture_id == pic->picture_id) {
			if (idx)
				*idx = i;

			return entry;
		}
	}

	return NULL;
}

static bool is_pic_null(VAPictureH264 *pic)
{
	return !pic->frame_idx && !pic->TopFieldOrderCnt &&
		!pic->BottomFieldOrderCnt;
}

static void clear_dpb_entry(struct dpb_entry *entry, bool reserved)
{
	memset(entry, 0, sizeof(*entry));

	if (reserved)
		entry->reserved = true;
}

static void insert_in_dpb(VAPictureH264 *pic, struct dpb_entry *entry)
{
	if (is_pic_null(pic))
		return;

	if (dpb_lookup(pic, NULL))
		return;

	if (!entry)
		entry = find_dpb_entry();

	memcpy(&entry->pic, pic, sizeof(entry->pic));
	entry->age = local_dpb.age;
	entry->valid = true;
	entry->reserved = false;

	if (!(pic->flags & VA_PICTURE_H264_INVALID))
		entry->used = true;
}

static void update_dpb(VAPictureParameterBufferH264 *parameters)
{
	unsigned int i;

	local_dpb.age++;

	for (i = 0; i < DPB_SIZE; i++) {
		struct dpb_entry *entry = &local_dpb.entries[i];

		entry->used = false;
	}

	for (i = 0; i < parameters->num_ref_frames; i++) {
		VAPictureH264 *pic = &parameters->ReferenceFrames[i];
		struct dpb_entry *entry;

		if (is_pic_null(pic))
			continue;

		entry = dpb_lookup(pic, NULL);
		if (entry) {
			entry->age = local_dpb.age;
			entry->used = true;
		} else {
			insert_in_dpb(pic, NULL);
		}
	}
}

void h264_start_dump(struct dump_driver_data *driver_data) { }

static void h264_dump_dpb(struct dump_driver_data *driver_data,
			  VAPictureParameterBufferH264 *parameters,
			  unsigned int indent)
{
	int i;

	print_indent(indent++, ".dpb = {\n");

	for (i = 0; i < DPB_SIZE; i++) {
		struct dpb_entry *entry = &local_dpb.entries[i];
		VAPictureH264 *pic = &entry->pic;

		if (!entry->valid)
			continue;

		print_indent(indent++, "[%d] = {\n", i);

		print_indent(indent, ".frame_num = %d,\n", pic->frame_idx);
		print_indent(indent, ".buf_index = %d,\n",
			     dpb_find_entry_index(entry));
		print_indent(indent, ".top_field_order_cnt = %d,\n",
			     pic->TopFieldOrderCnt);
		print_indent(indent, ".bottom_field_order_cnt = %d,\n",
			     pic->BottomFieldOrderCnt);
		print_indent(indent, ".flags = %s | %s | %s,\n",
			     "V4L2_H264_DPB_ENTRY_FLAG_VALID",
			     pic->flags & VA_PICTURE_H264_LONG_TERM_REFERENCE ?
			     "V4L2_H264_DPB_ENTRY_FLAG_LONG_TERM" : "0",
			     entry->used ?
			     "V4L2_H264_DPB_ENTRY_FLAG_ACTIVE" : "0");
		print_indent(--indent, "},\n");
	}
	print_indent(--indent, "},\n");
}

static void h264_emit_picture_parameter(struct dump_driver_data *driver_data,
					unsigned int indent)
{
	VAPictureParameterBufferH264 *parameters = &driver_data->picture;

	print_indent(indent++, ".decode_param = {\n");
	print_indent(indent, ".top_field_order_cnt = %d,\n",
		     parameters->CurrPic.TopFieldOrderCnt);
	print_indent(indent, ".bottom_field_order_cnt = %d,\n",
		     parameters->CurrPic.BottomFieldOrderCnt);
	h264_dump_dpb(driver_data, parameters, indent);
	print_indent(--indent, "},\n");

	print_indent(indent++, ".pps = {\n");
	print_indent(indent, ".weighted_bipred_idc = %d,\n",
		     parameters->pic_fields.bits.weighted_bipred_idc);
	print_indent(indent, ".pic_init_qp_minus26 = %d,\n",
		     parameters->pic_init_qp_minus26);
	print_indent(indent, ".pic_init_qs_minus26 = %d,\n",
		     parameters->pic_init_qs_minus26);
	print_indent(indent, ".chroma_qp_index_offset = %d,\n",
		     parameters->chroma_qp_index_offset);
	print_indent(indent, ".second_chroma_qp_index_offset = %d,\n",
		     parameters->second_chroma_qp_index_offset);
	print_indent(indent, ".flags = %s | %s | %s | %s | %s,\n",
		     parameters->pic_fields.bits.entropy_coding_mode_flag ?
			"V4L2_H264_PPS_FLAG_ENTROPY_CODING_MODE " : " 0 ",
		     parameters->pic_fields.bits.weighted_pred_flag ?
			"V4L2_H264_PPS_FLAG_WEIGHTED_PRED" : "0 ",
		     parameters->pic_fields.bits.transform_8x8_mode_flag ?
			"V4L2_H264_PPS_FLAG_TRANSFORM_8X8_MODE " : "0 ",
		     parameters->pic_fields.bits.constrained_intra_pred_flag ?
			"V4L2_H264_PPS_FLAG_CONSTRAINED_INTRA_PRED" : "0 ",
		     parameters->pic_fields.bits.pic_order_present_flag ?
			"V4L2_H264_PPS_FLAG_BOTTOM_FIELD_PIC_ORDER_IN_FRAME_PRESENT" : "0 ",
		     parameters->pic_fields.bits.deblocking_filter_control_present_flag ?
			"V4L2_H264_PPS_FLAG_DEBLOCKING_FILTER_CONTROL_PRESENT" :  "0 ",
		     parameters->pic_fields.bits.redundant_pic_cnt_present_flag ?
			"V4L2_H264_PPS_FLAG_REDUNDANT_PIC_CNT_PRESENT" : "0 ");
	print_indent(--indent, "},\n");

	print_indent(indent++, ".sps = {\n");
	print_indent(indent, ".chroma_format_idc = %u,\n",
		     parameters->seq_fields.bits.chroma_format_idc);
	print_indent(indent, ".bit_depth_luma_minus8 = %u,\n",
		     parameters->bit_depth_luma_minus8);
	print_indent(indent, ".bit_depth_chroma_minus8 = %u,\n",
		     parameters->bit_depth_chroma_minus8);
	print_indent(indent, ".log2_max_frame_num_minus4 = %u,\n",
		     parameters->seq_fields.bits.log2_max_frame_num_minus4);
	print_indent(indent, ".log2_max_pic_order_cnt_lsb_minus4 = %u,\n",
		     parameters->seq_fields.bits.log2_max_pic_order_cnt_lsb_minus4);
	print_indent(indent, ".pic_order_cnt_type = %u,\n",
		     parameters->seq_fields.bits.pic_order_cnt_type);
	print_indent(indent, ".pic_width_in_mbs_minus1 = %u,\n",
		     parameters->picture_width_in_mbs_minus1);
	print_indent(indent, ".pic_height_in_map_units_minus1 = %u,\n",
		     parameters->picture_height_in_mbs_minus1);

	print_indent(indent, ".flags = %s | %s | %s | %s | %s,\n",
		     parameters->seq_fields.bits.residual_colour_transform_flag ?
		     	"V4L2_H264_SPS_FLAG_SEPARATE_COLOUR_PLANE" : "0",
		     parameters->seq_fields.bits.gaps_in_frame_num_value_allowed_flag ?
		     	"V4L2_H264_SPS_FLAG_GAPS_IN_FRAME_NUM_VALUE_ALLOWED" : "0",
		     parameters->seq_fields.bits.frame_mbs_only_flag ?
		     	"V4L2_H264_SPS_FLAG_FRAME_MBS_ONLY" : "0",
		     parameters->seq_fields.bits.mb_adaptive_frame_field_flag ?
		     	"V4L2_H264_SPS_FLAG_MB_ADAPTIVE_FRAME_FIELD" : "0",
		     parameters->seq_fields.bits.direct_8x8_inference_flag ?
		     	"V4L2_H264_SPS_FLAG_DIRECT_8X8_INFERENCE" : "0",
		     parameters->seq_fields.bits.delta_pic_order_always_zero_flag ?
		     	"V4L2_H264_SPS_FLAG_DELTA_PIC_ORDER_ALWAYS_ZERO" : "0");
	print_indent(--indent, "},\n");
}

static void h264_emit_quantization_matrix(struct dump_driver_data *driver_data,
					  unsigned int indent)
{
	VAIQMatrixBufferH264 *parameters = &driver_data->matrix;

	print_indent(indent++, ".scaling_matrix = {\n");
	print_u8_matrix(indent, "scaling_list_4x4",
			(uint8_t*)&parameters->ScalingList4x4,
			6, 16);
	print_u8_matrix(indent, "scaling_list_8x8",
			(uint8_t*)&parameters->ScalingList8x8,
			6, 64);
	print_indent(--indent, "},\n");
}

#define H264_SLICE_P	0
#define H264_SLICE_B	1

static void h264_emit_slice_parameter(struct dump_driver_data *driver_data,
				      unsigned int indent)
{
	VASliceParameterBufferH264 *parameters = &driver_data->slice;
	int i;

	print_indent(indent++, ".slice_param = {\n");
	print_indent(indent, ".size = %u,\n", parameters->slice_data_size);
	print_indent(indent, ".header_bit_size = %u,\n", parameters->slice_data_bit_offset);
	print_indent(indent, ".first_mb_in_slice = %u,\n", parameters->first_mb_in_slice);
	print_indent(indent, ".slice_type = %u,\n", parameters->slice_type);
	print_indent(indent, ".cabac_init_idc = %u,\n", parameters->cabac_init_idc);
	print_indent(indent, ".slice_qp_delta = %d,\n", parameters->slice_qp_delta);
	print_indent(indent, ".disable_deblocking_filter_idc = %u,\n", parameters->disable_deblocking_filter_idc);
	print_indent(indent, ".slice_alpha_c0_offset_div2 = %d,\n", parameters->slice_alpha_c0_offset_div2);
	print_indent(indent, ".slice_beta_offset_div2 = %d,\n", parameters->slice_beta_offset_div2);

	if (((parameters->slice_type % 5) == H264_SLICE_P) ||
	    ((parameters->slice_type % 5) == H264_SLICE_B)) {
		print_indent(indent, ".num_ref_idx_l0_active_minus1 = %u,\n",
			     parameters->num_ref_idx_l0_active_minus1);
		print_indent(indent, ".ref_pic_list0 = { ");
		for (i = 0; i < parameters->num_ref_idx_l0_active_minus1 + 1; i++) {
			VAPictureH264 *pic = &parameters->RefPicList0[i];
			struct dpb_entry *entry;
			unsigned int idx = 0;

			entry = dpb_lookup(pic, &idx);
			printf(" %u, ", entry ? idx : 0);
		}
		printf("},\n");
	}

	if ((parameters->slice_type % 5) == H264_SLICE_B) {
		print_indent(indent, ".num_ref_idx_l1_active_minus1 = %u,\n",
			     parameters->num_ref_idx_l1_active_minus1);
		print_indent(indent, ".ref_pic_list1 = { ");
		for (i = 0; i < parameters->num_ref_idx_l1_active_minus1 + 1; i++) {
			VAPictureH264 *pic = &parameters->RefPicList1[i];
			struct dpb_entry *entry;
			unsigned int idx = 0;

			entry = dpb_lookup(pic, &idx);
			printf(" %u, ", entry ? idx : 0);
		}
		printf("},\n");
	}

	if (parameters->direct_spatial_mv_pred_flag)
		print_indent(indent, ".flags = V4L2_H264_SLICE_FLAG_DIRECT_SPATIAL_MV_PRED,\n");

	print_indent(indent++, ".pred_weight_table = {\n");
	print_indent(indent, ".chroma_log2_weight_denom = %u,\n",
		     parameters->chroma_log2_weight_denom);
	print_indent(indent, ".luma_log2_weight_denom = %u,\n",
		     parameters->luma_log2_weight_denom);

	print_indent(indent, ".weight_factors = {\n");
	for (i = 0; i < 2; i++) {
		print_indent(5, "{\n");
		print_s16_array(6, "luma_weight",
				i ? parameters->luma_weight_l1 : parameters->luma_weight_l0,
				32);
		print_s16_array(6, "luma_offset",
				i ? parameters->luma_offset_l1 : parameters->luma_offset_l0,
				32);
		print_s16_matrix(6, "chroma_weight",
				 i ? (int16_t*)&parameters->chroma_weight_l1 : (int16_t*)&parameters->chroma_weight_l0,
				 32, 2);
		print_s16_matrix(6, "chroma_offset",
				 i ? (int16_t*)parameters->chroma_offset_l1 : (int16_t*)parameters->chroma_offset_l0,
				 32, 2);
		print_indent(5, "},\n");
	}
	print_indent(indent, "},\n");
	print_indent(--indent, "},\n");

	print_indent(--indent, "},\n");
}

static void h264_emit_frame(struct dump_driver_data *driver_data)
{
	struct dpb_entry *output;
	unsigned int index = driver_data->frame_index;
	unsigned int indent = 1;

	output = dpb_lookup(&driver_data->picture.CurrPic, NULL);
	if (!output)
		output = find_dpb_entry();

	clear_dpb_entry(output, true);

	update_dpb(&driver_data->picture);

	print_indent(indent++, "{\n");
	print_indent(indent, ".index = %d,\n", index);
	print_indent(indent, ".output_buffer = %d,\n",
		     dpb_find_entry_index(output));
	print_indent(indent++, ".frame.h264 = {\n");

	h264_emit_picture_parameter(driver_data, indent);
	h264_emit_quantization_matrix(driver_data, indent);
	h264_emit_slice_parameter(driver_data, indent);

	print_indent(--indent, "},\n");
	print_indent(--indent, "},\n");

	insert_in_dpb(&driver_data->picture.CurrPic, output);
}

void h264_stop_dump(struct dump_driver_data *driver_data)
{
	h264_emit_frame(driver_data);
}

void h264_quantization_matrix_dump(struct dump_driver_data *driver_data,
				   VAIQMatrixBufferH264 *parameters)
{
	memcpy(&driver_data->matrix, parameters, sizeof(*parameters));
}

void h264_picture_parameter_dump(struct dump_driver_data *driver_data,
				 VAPictureParameterBufferH264 *parameters)
{
	memcpy(&driver_data->picture, parameters, sizeof(*parameters));
}

void h264_slice_parameter_dump(struct dump_driver_data *driver_data,
			       VASliceParameterBufferH264 *parameters)
{
	memcpy(&driver_data->slice, parameters, sizeof(*parameters));
}

void mpeg2_start_dump(struct dump_driver_data *driver_data)
{
	unsigned int index = driver_data->frame_index;

	print_indent(1, "{\n");
	print_indent(2, ".index = %d,\n", index);
}

void mpeg2_stop_dump(struct dump_driver_data *driver_data)
{
	print_indent(1, "},\n");
}

void mpeg2_header_dump(struct dump_driver_data *driver_data,
	VAPictureParameterBufferMPEG2 *parameters)
{
	struct object_surface *surface_object;
	char *slice_type;
	unsigned int forward_reference_index;
	unsigned int backward_reference_index;
	unsigned int index = driver_data->frame_index;

	print_indent(2, ".frame.mpeg2.slice_params = {\n");

	if (parameters->picture_coding_type == 1)
		slice_type = "V4L2_MPEG2_SLICE_TYPE_I";
	else if (parameters->picture_coding_type == 2)
		slice_type = "V4L2_MPEG2_SLICE_TYPE_P";
	else if (parameters->picture_coding_type == 3)
		slice_type = "V4L2_MPEG2_SLICE_TYPE_B";
	else
		slice_type = "V4L2_MPEG2_SLICE_TYPE_INVALID";

	print_indent(3, ".slice_type = %s,\n", slice_type);
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

	print_indent(2, "},\n");
}

void mpeg2_quantization_matrix_dump(struct dump_driver_data *driver_data,
				    VAIQMatrixBufferMPEG2 *parameters)
{
	print_indent(2, ".frame.mpeg2.quantization = {\n");

	print_indent(3, ".load_luma_intra_quantiser_matrix = %d,\n",
		     parameters->load_intra_quantiser_matrix);
	print_indent(3, ".load_luma_non_intra_quantiser_matrix = %d,\n",
		     parameters->load_non_intra_quantiser_matrix);
	print_indent(3, ".load_chroma_intra_quantiser_matrix = %d,\n",
		     parameters->load_chroma_intra_quantiser_matrix);
	print_indent(3, ".load_chroma_non_intra_quantiser_matrix = %d,\n",
		     parameters->load_chroma_non_intra_quantiser_matrix);

	print_u8_matrix(3, "luma_intra_quantiser_matrix",
			(uint8_t *) &parameters->intra_quantiser_matrix,
			1, 64);
	print_u8_matrix(3, "luma_non_intra_quantiser_matrix",
			(uint8_t *) &parameters->non_intra_quantiser_matrix,
			1, 64);
	print_u8_matrix(3, "chroma_intra_quantiser_matrix",
			(uint8_t *) &parameters->chroma_intra_quantiser_matrix,
			1, 64);
	print_u8_matrix(3, "chroma_non_intra_quantiser_matrix",
			(uint8_t *) &parameters->chroma_non_intra_quantiser_matrix,
			1, 64);

	print_indent(2, "},\n");
}
