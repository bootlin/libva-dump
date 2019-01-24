/*
 * Copyright (C) 2018 Bootlin
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


#define DPB_SIZE	16

struct dpb_entry {
	VAPictureH264	pic;
	unsigned int	age;
	unsigned int	tag;
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
	return pic->flags & VA_PICTURE_H264_INVALID;
}

static void clear_dpb_entry(struct dpb_entry *entry, bool reserved)
{
	memset(entry, 0, sizeof(*entry));

	if (reserved)
		entry->reserved = true;
}

static void insert_in_dpb(VAPictureH264 *pic, struct dpb_entry *entry, unsigned int tag)
{
	if (is_pic_null(pic))
		return;

	if (dpb_lookup(pic, NULL))
		return;

	if (!entry)
		entry = find_dpb_entry();

	memcpy(&entry->pic, pic, sizeof(entry->pic));
	entry->tag = tag;
	entry->age = local_dpb.age;
	entry->valid = true;
	entry->reserved = false;

	if (!(pic->flags & VA_PICTURE_H264_INVALID))
		entry->used = true;
}

static void update_dpb(struct dump_driver_data *driver)
{
	VAPictureParameterBufferH264 *picture_params = &driver->params.h264.picture;
	unsigned int i;

	local_dpb.age++;

	for (i = 0; i < DPB_SIZE; i++) {
		struct dpb_entry *entry = &local_dpb.entries[i];

		entry->used = false;
	}

	for (i = 0; i < picture_params->num_ref_frames; i++) {
		VAPictureH264 *pic = &picture_params->ReferenceFrames[i];
		struct dpb_entry *entry;

		if (is_pic_null(pic))
			continue;

		entry = dpb_lookup(pic, NULL);
		if (entry) {
			entry->age = local_dpb.age;
			entry->used = true;
		} else {
			struct object_surface *surface;

			surface = (struct object_surface *)object_heap_lookup(&driver->surface_heap,
									      pic->picture_id);
			insert_in_dpb(pic, NULL, surface->index);
		}
	}
}

static void h264_dump_dpb(struct dump_driver_data *driver_data,
			  VAPictureParameterBufferH264 *picture_params,
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
		print_indent(indent, ".timestamp = TS_REF_INDEX(%d),\n", entry->tag);
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
	VAPictureParameterBufferH264 *picture_params =
		&driver_data->params.h264.picture;

	print_indent(indent++, ".decode_param = {\n");
	print_indent(indent, ".top_field_order_cnt = %d,\n",
		     picture_params->CurrPic.TopFieldOrderCnt);
	print_indent(indent, ".bottom_field_order_cnt = %d,\n",
		     picture_params->CurrPic.BottomFieldOrderCnt);
	h264_dump_dpb(driver_data, picture_params, indent);
	print_indent(--indent, "},\n");

	print_indent(indent++, ".pps = {\n");
	print_indent(indent, ".weighted_bipred_idc = %d,\n",
		     picture_params->pic_fields.bits.weighted_bipred_idc);
	print_indent(indent, ".pic_init_qp_minus26 = %d,\n",
		     picture_params->pic_init_qp_minus26);
	print_indent(indent, ".pic_init_qs_minus26 = %d,\n",
		     picture_params->pic_init_qs_minus26);
	print_indent(indent, ".chroma_qp_index_offset = %d,\n",
		     picture_params->chroma_qp_index_offset);
	print_indent(indent, ".second_chroma_qp_index_offset = %d,\n",
		     picture_params->second_chroma_qp_index_offset);
	print_indent(indent, ".flags = %s | %s | %s | %s | %s,\n",
		     picture_params->pic_fields.bits.entropy_coding_mode_flag ?
			"V4L2_H264_PPS_FLAG_ENTROPY_CODING_MODE " : " 0 ",
		     picture_params->pic_fields.bits.weighted_pred_flag ?
			"V4L2_H264_PPS_FLAG_WEIGHTED_PRED" : "0 ",
		     picture_params->pic_fields.bits.transform_8x8_mode_flag ?
			"V4L2_H264_PPS_FLAG_TRANSFORM_8X8_MODE " : "0 ",
		     picture_params->pic_fields.bits.constrained_intra_pred_flag ?
			"V4L2_H264_PPS_FLAG_CONSTRAINED_INTRA_PRED" : "0 ",
		     picture_params->pic_fields.bits.pic_order_present_flag ?
			"V4L2_H264_PPS_FLAG_BOTTOM_FIELD_PIC_ORDER_IN_FRAME_PRESENT" : "0 ",
		     picture_params->pic_fields.bits.deblocking_filter_control_present_flag ?
			"V4L2_H264_PPS_FLAG_DEBLOCKING_FILTER_CONTROL_PRESENT" :  "0 ",
		     picture_params->pic_fields.bits.redundant_pic_cnt_present_flag ?
			"V4L2_H264_PPS_FLAG_REDUNDANT_PIC_CNT_PRESENT" : "0 ");
	print_indent(--indent, "},\n");

	print_indent(indent++, ".sps = {\n");
	print_indent(indent, ".chroma_format_idc = %u,\n",
		     picture_params->seq_fields.bits.chroma_format_idc);
	print_indent(indent, ".bit_depth_luma_minus8 = %u,\n",
		     picture_params->bit_depth_luma_minus8);
	print_indent(indent, ".bit_depth_chroma_minus8 = %u,\n",
		     picture_params->bit_depth_chroma_minus8);
	print_indent(indent, ".log2_max_frame_num_minus4 = %u,\n",
		     picture_params->seq_fields.bits.log2_max_frame_num_minus4);
	print_indent(indent, ".log2_max_pic_order_cnt_lsb_minus4 = %u,\n",
		     picture_params->seq_fields.bits.log2_max_pic_order_cnt_lsb_minus4);
	print_indent(indent, ".pic_order_cnt_type = %u,\n",
		     picture_params->seq_fields.bits.pic_order_cnt_type);
	print_indent(indent, ".pic_width_in_mbs_minus1 = %u,\n",
		     picture_params->picture_width_in_mbs_minus1);
	print_indent(indent, ".pic_height_in_map_units_minus1 = %u,\n",
		     picture_params->picture_height_in_mbs_minus1);

	print_indent(indent, ".flags = %s | %s | %s | %s | %s,\n",
		     picture_params->seq_fields.bits.residual_colour_transform_flag ?
			"V4L2_H264_SPS_FLAG_SEPARATE_COLOUR_PLANE" : "0",
		     picture_params->seq_fields.bits.gaps_in_frame_num_value_allowed_flag ?
			"V4L2_H264_SPS_FLAG_GAPS_IN_FRAME_NUM_VALUE_ALLOWED" : "0",
		     picture_params->seq_fields.bits.frame_mbs_only_flag ?
			"V4L2_H264_SPS_FLAG_FRAME_MBS_ONLY" : "0",
		     picture_params->seq_fields.bits.mb_adaptive_frame_field_flag ?
			"V4L2_H264_SPS_FLAG_MB_ADAPTIVE_FRAME_FIELD" : "0",
		     picture_params->seq_fields.bits.direct_8x8_inference_flag ?
			"V4L2_H264_SPS_FLAG_DIRECT_8X8_INFERENCE" : "0",
		     picture_params->seq_fields.bits.delta_pic_order_always_zero_flag ?
			"V4L2_H264_SPS_FLAG_DELTA_PIC_ORDER_ALWAYS_ZERO" : "0");
	print_indent(--indent, "},\n");
}

static void h264_emit_quantization_matrix(struct dump_driver_data *driver_data,
					  unsigned int indent)
{
	VAIQMatrixBufferH264 *quantization_params =
		&driver_data->params.h264.quantization;

	print_indent(indent++, ".scaling_matrix = {\n");
	print_u8_matrix(indent, "scaling_list_4x4",
			(uint8_t*)&quantization_params->ScalingList4x4,
			6, 16);
	print_u8_matrix(indent, "scaling_list_8x8",
			(uint8_t*)&quantization_params->ScalingList8x8,
			6, 64);
	print_indent(--indent, "},\n");
}

#define H264_SLICE_P	0
#define H264_SLICE_B	1

static void h264_emit_slice_parameter(struct dump_driver_data *driver_data,
				      unsigned int indent)
{
	VASliceParameterBufferH264 *slice_params =
		&driver_data->params.h264.slice;
	int i;

	print_indent(indent++, ".slice_param = {\n");
	print_indent(indent, ".size = %u,\n", slice_params->slice_data_size);
	print_indent(indent, ".header_bit_size = %u,\n", slice_params->slice_data_bit_offset);
	print_indent(indent, ".first_mb_in_slice = %u,\n", slice_params->first_mb_in_slice);
	print_indent(indent, ".slice_type = %u,\n", slice_params->slice_type);
	print_indent(indent, ".cabac_init_idc = %u,\n", slice_params->cabac_init_idc);
	print_indent(indent, ".slice_qp_delta = %d,\n", slice_params->slice_qp_delta);
	print_indent(indent, ".disable_deblocking_filter_idc = %u,\n", slice_params->disable_deblocking_filter_idc);
	print_indent(indent, ".slice_alpha_c0_offset_div2 = %d,\n", slice_params->slice_alpha_c0_offset_div2);
	print_indent(indent, ".slice_beta_offset_div2 = %d,\n", slice_params->slice_beta_offset_div2);

	if (((slice_params->slice_type % 5) == H264_SLICE_P) ||
	    ((slice_params->slice_type % 5) == H264_SLICE_B)) {
		print_indent(indent, ".num_ref_idx_l0_active_minus1 = %u,\n",
			     slice_params->num_ref_idx_l0_active_minus1);
		print_indent(indent, ".ref_pic_list0 = { ");
		for (i = 0; i < slice_params->num_ref_idx_l0_active_minus1 + 1; i++) {
			VAPictureH264 *pic = &slice_params->RefPicList0[i];
			struct dpb_entry *entry;
			unsigned int idx = 0;

			entry = dpb_lookup(pic, &idx);
			printf(" %u, ", entry ? idx : 0);
		}
		printf("},\n");
	}

	if ((slice_params->slice_type % 5) == H264_SLICE_B) {
		print_indent(indent, ".num_ref_idx_l1_active_minus1 = %u,\n",
			     slice_params->num_ref_idx_l1_active_minus1);
		print_indent(indent, ".ref_pic_list1 = { ");
		for (i = 0; i < slice_params->num_ref_idx_l1_active_minus1 + 1; i++) {
			VAPictureH264 *pic = &slice_params->RefPicList1[i];
			struct dpb_entry *entry;
			unsigned int idx = 0;

			entry = dpb_lookup(pic, &idx);
			printf(" %u, ", entry ? idx : 0);
		}
		printf("},\n");
	}

	if (slice_params->direct_spatial_mv_pred_flag)
		print_indent(indent, ".flags = V4L2_H264_SLICE_FLAG_DIRECT_SPATIAL_MV_PRED,\n");

	print_indent(indent++, ".pred_weight_table = {\n");
	print_indent(indent, ".chroma_log2_weight_denom = %u,\n",
		     slice_params->chroma_log2_weight_denom);
	print_indent(indent, ".luma_log2_weight_denom = %u,\n",
		     slice_params->luma_log2_weight_denom);

	print_indent(indent, ".weight_factors = {\n");
	for (i = 0; i < 2; i++) {
		print_indent(5, "{\n");
		print_s16_array(6, "luma_weight",
				i ? slice_params->luma_weight_l1 : slice_params->luma_weight_l0,
				32);
		print_s16_array(6, "luma_offset",
				i ? slice_params->luma_offset_l1 : slice_params->luma_offset_l0,
				32);
		print_s16_matrix(6, "chroma_weight",
				 i ? (int16_t*)&slice_params->chroma_weight_l1 : (int16_t*)&slice_params->chroma_weight_l0,
				 32, 2);
		print_s16_matrix(6, "chroma_offset",
				 i ? (int16_t*)slice_params->chroma_offset_l1 : (int16_t*)slice_params->chroma_offset_l0,
				 32, 2);
		print_indent(5, "},\n");
	}
	print_indent(indent, "},\n");
	print_indent(--indent, "},\n");

	print_indent(--indent, "},\n");
}

void h264_dump_prepare(struct dump_driver_data *driver_data)
{
}

void h264_dump_header(struct dump_driver_data *driver_data, struct object_surface *surface)
{
	VAPictureH264 *pic = &driver_data->params.h264.picture.CurrPic;
	struct dpb_entry *output;
	unsigned int index = driver_data->frame_index;
	unsigned int indent = 1;

	output = dpb_lookup(pic, NULL);
	if (!output)
		output = find_dpb_entry();

	clear_dpb_entry(output, true);

	update_dpb(driver_data);

	print_indent(indent++, "{\n");
	print_indent(indent, ".index = %d,\n", index);
	print_indent(indent++, ".frame.h264 = {\n");

	h264_emit_picture_parameter(driver_data, indent);
	h264_emit_quantization_matrix(driver_data, indent);
	h264_emit_slice_parameter(driver_data, indent);

	print_indent(--indent, "},\n");
	print_indent(--indent, "},\n");

	insert_in_dpb(pic, output, index);
}
