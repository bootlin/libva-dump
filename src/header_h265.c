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

static void h265_dump_sps(struct dump_driver_data *driver_data,
			  unsigned int indent)
{
	VAPictureParameterBufferHEVC *picture_params =
		&driver_data->params.h265.picture;

	print_indent(indent++, ".sps = {\n");
	print_indent(indent, ".chroma_format_idc = %d,\n",
		     picture_params->pic_fields.bits.chroma_format_idc);
	print_indent(indent, ".separate_colour_plane_flag = %d,\n",
		     picture_params->pic_fields.bits.separate_colour_plane_flag);
	print_indent(indent, ".pic_width_in_luma_samples = %d,\n",
		     picture_params->pic_width_in_luma_samples);
	print_indent(indent, ".pic_height_in_luma_samples = %d,\n",
		     picture_params->pic_height_in_luma_samples);
	print_indent(indent, ".bit_depth_luma_minus8 = %d,\n",
		     picture_params->bit_depth_luma_minus8);
	print_indent(indent, ".bit_depth_chroma_minus8 = %d,\n",
		     picture_params->bit_depth_chroma_minus8);
	print_indent(indent, ".log2_max_pic_order_cnt_lsb_minus4 = %d,\n",
		     picture_params->log2_max_pic_order_cnt_lsb_minus4);
	print_indent(indent, ".sps_max_dec_pic_buffering_minus1 = %d,\n",
		     picture_params->sps_max_dec_pic_buffering_minus1);
	print_indent(indent, ".sps_max_num_reorder_pics = %d,\n", 0);
	print_indent(indent, ".sps_max_latency_increase_plus1 = %d,\n", 0);
	print_indent(indent, ".log2_min_luma_coding_block_size_minus3 = %d,\n",
		     picture_params->log2_min_luma_coding_block_size_minus3);
	print_indent(indent, ".log2_diff_max_min_luma_coding_block_size = %d,\n",
		     picture_params->log2_diff_max_min_luma_coding_block_size);
	print_indent(indent, ".log2_min_luma_transform_block_size_minus2 = %d,\n",
		     picture_params->log2_min_transform_block_size_minus2);
	print_indent(indent, ".log2_diff_max_min_luma_transform_block_size = %d,\n",
		     picture_params->log2_diff_max_min_transform_block_size);
	print_indent(indent, ".max_transform_hierarchy_depth_inter = %d,\n",
		     picture_params->max_transform_hierarchy_depth_inter);
	print_indent(indent, ".max_transform_hierarchy_depth_intra = %d,\n",
		     picture_params->max_transform_hierarchy_depth_intra);
	print_indent(indent, ".scaling_list_enabled_flag = %d,\n",
		     picture_params->pic_fields.bits.scaling_list_enabled_flag);
	print_indent(indent, ".amp_enabled_flag = %d,\n",
		     picture_params->pic_fields.bits.amp_enabled_flag);
	print_indent(indent, ".sample_adaptive_offset_enabled_flag = %d,\n",
		     picture_params->slice_parsing_fields.bits.sample_adaptive_offset_enabled_flag);
	print_indent(indent, ".pcm_enabled_flag = %d,\n",
		     picture_params->pic_fields.bits.pcm_enabled_flag);
	print_indent(indent, ".pcm_sample_bit_depth_luma_minus1 = %d,\n",
		     picture_params->pcm_sample_bit_depth_luma_minus1);
	print_indent(indent, ".pcm_sample_bit_depth_chroma_minus1 = %d,\n",
		     picture_params->pcm_sample_bit_depth_chroma_minus1);
	print_indent(indent, ".log2_min_pcm_luma_coding_block_size_minus3 = %d,\n",
		     picture_params->log2_min_pcm_luma_coding_block_size_minus3);
	print_indent(indent, ".log2_diff_max_min_pcm_luma_coding_block_size = %d,\n",
		     picture_params->log2_diff_max_min_pcm_luma_coding_block_size);
	print_indent(indent, ".pcm_loop_filter_disabled_flag = %d,\n",
		     picture_params->pic_fields.bits.pcm_loop_filter_disabled_flag);
	print_indent(indent, ".num_short_term_ref_pic_sets = %d,\n",
		     picture_params->num_short_term_ref_pic_sets);
	print_indent(indent, ".long_term_ref_pics_present_flag = %d,\n",
		     picture_params->slice_parsing_fields.bits.long_term_ref_pics_present_flag);
	print_indent(indent, ".num_long_term_ref_pic_sps = %d,\n",
		     picture_params->num_long_term_ref_pic_sps);
	print_indent(indent, ".sps_temporal_mvp_enabled_flag = %d,\n",
		     picture_params->slice_parsing_fields.bits.sps_temporal_mvp_enabled_flag);
	print_indent(indent, ".strong_intra_smoothing_enabled_flag = %d,\n",
		     picture_params->pic_fields.bits.strong_intra_smoothing_enabled_flag);
	print_indent(--indent, "},\n");
}

static void h265_dump_pps(struct dump_driver_data *driver_data,
			  unsigned int indent)
{
	VAPictureParameterBufferHEVC *picture_params =
		&driver_data->params.h265.picture;
	VASliceParameterBufferHEVC *slice_params =
		&driver_data->params.h265.slice;

	print_indent(indent++, ".pps = {\n");
	print_indent(indent, ".dependent_slice_segment_flag = %d,\n",
		     slice_params->LongSliceFlags.fields.dependent_slice_segment_flag);
	print_indent(indent, ".output_flag_present_flag = %d,\n",
		     picture_params->slice_parsing_fields.bits.output_flag_present_flag);
	print_indent(indent, ".num_extra_slice_header_bits = %d,\n",
		     picture_params->num_extra_slice_header_bits);
	print_indent(indent, ".sign_data_hiding_enabled_flag = %d,\n",
		     picture_params->pic_fields.bits.sign_data_hiding_enabled_flag);
	print_indent(indent, ".cabac_init_present_flag = %d,\n",
		     picture_params->slice_parsing_fields.bits.cabac_init_present_flag);
	print_indent(indent, ".init_qp_minus26 = %d,\n",
		     picture_params->init_qp_minus26);
	print_indent(indent, ".constrained_intra_pred_flag = %d,\n",
		     picture_params->pic_fields.bits.constrained_intra_pred_flag);
	print_indent(indent, ".transform_skip_enabled_flag = %d,\n",
		     picture_params->pic_fields.bits.transform_skip_enabled_flag);
	print_indent(indent, ".cu_qp_delta_enabled_flag = %d,\n",
		     picture_params->pic_fields.bits.cu_qp_delta_enabled_flag);
	print_indent(indent, ".diff_cu_qp_delta_depth = %d,\n",
		     picture_params->diff_cu_qp_delta_depth);
	print_indent(indent, ".pps_cb_qp_offset = %d,\n",
		     picture_params->pps_cb_qp_offset);
	print_indent(indent, ".pps_cr_qp_offset = %d,\n",
		     picture_params->pps_cr_qp_offset);
	print_indent(indent, ".pps_slice_chroma_qp_offsets_present_flag = %d,\n",
		     picture_params->slice_parsing_fields.bits.pps_slice_chroma_qp_offsets_present_flag);
	print_indent(indent, ".weighted_pred_flag = %d,\n",
		     picture_params->pic_fields.bits.weighted_pred_flag);
	print_indent(indent, ".weighted_bipred_flag = %d,\n",
		     picture_params->pic_fields.bits.weighted_bipred_flag);
	print_indent(indent, ".transquant_bypass_enabled_flag = %d,\n",
		     picture_params->pic_fields.bits.transquant_bypass_enabled_flag);
	print_indent(indent, ".tiles_enabled_flag = %d,\n",
		     picture_params->pic_fields.bits.tiles_enabled_flag);
	print_indent(indent, ".entropy_coding_sync_enabled_flag = %d,\n",
		     picture_params->pic_fields.bits.entropy_coding_sync_enabled_flag);
	print_indent(indent, ".num_tile_columns_minus1 = %d,\n",
		     picture_params->num_tile_columns_minus1);
	print_indent(indent, ".num_tile_rows_minus1 = %d,\n",
		     picture_params->num_tile_rows_minus1);

	// TODO: column_width_minus1
	// TODO: row_height_minus1

	print_indent(indent, ".loop_filter_across_tiles_enabled_flag = %d,\n",
		     picture_params->pic_fields.bits.loop_filter_across_tiles_enabled_flag);
	print_indent(indent, ".pps_loop_filter_across_slices_enabled_flag = %d,\n",
		     picture_params->pic_fields.bits.pps_loop_filter_across_slices_enabled_flag);
	print_indent(indent, ".deblocking_filter_override_enabled_flag = %d,\n",
		     picture_params->slice_parsing_fields.bits.deblocking_filter_override_enabled_flag);
	print_indent(indent, ".pps_disable_deblocking_filter_flag = %d,\n",
		     picture_params->slice_parsing_fields.bits.pps_disable_deblocking_filter_flag);
	print_indent(indent, ".pps_beta_offset_div2 = %d,\n",
		     picture_params->pps_beta_offset_div2);
	print_indent(indent, ".pps_tc_offset_div2 = %d,\n",
		     picture_params->pps_tc_offset_div2);
	print_indent(indent, ".lists_modification_present_flag = %d,\n",
		     picture_params->slice_parsing_fields.bits.lists_modification_present_flag);
	print_indent(indent, ".log2_parallel_merge_level_minus2 = %d,\n",
		     picture_params->log2_parallel_merge_level_minus2);
	print_indent(--indent, "},\n");
}

static void h265_dump_slice_params(struct dump_driver_data *driver_data,
				   unsigned int indent, void *slice_data,
				   unsigned int sice_size)
{
	VASliceParameterBufferHEVC *slice_params =
		&driver_data->params.h265.slice;
	uint8_t *p = slice_data + slice_params->slice_data_offset;
	uint8_t nal_unit_type;
	uint8_t nuh_temporal_id_plus1;

	print_indent(indent++, ".slice_params = {\n");
	print_indent(indent, ".bit_size = %d,\n",
		     slice_params->slice_data_size * 8);
	print_indent(indent, ".data_bit_offset = %d,\n",
		     (slice_params->slice_data_offset + slice_params->slice_data_byte_offset - 1) * 8);

	nal_unit_type = (*p++ >> 1) & ((1 << 6) - 1);
	nuh_temporal_id_plus1 = *p & ((1 << 3) - 1);

	print_indent(indent, ".nal_unit_type = %d,\n", nal_unit_type);
	print_indent(indent, ".nuh_temporal_id_plus1 = %d,\n", nuh_temporal_id_plus1);

	print_indent(indent, ".slice_type = %d,\n",
		     slice_params->LongSliceFlags.fields.slice_type);
	print_indent(indent, ".colour_plane_id = %d,\n",
		     slice_params->LongSliceFlags.fields.color_plane_id);
	print_indent(indent, ".slice_sao_luma_flag = %d,\n",
		     slice_params->LongSliceFlags.fields.slice_sao_luma_flag);
	print_indent(indent, ".slice_sao_chroma_flag = %d,\n",
		     slice_params->LongSliceFlags.fields.slice_sao_chroma_flag);
	print_indent(indent, ".slice_temporal_mvp_enabled_flag = %d,\n",
		     slice_params->LongSliceFlags.fields.slice_temporal_mvp_enabled_flag);
	print_indent(indent, ".num_ref_idx_l0_active_minus1 = %d,\n",
		     slice_params->num_ref_idx_l0_active_minus1);
	print_indent(indent, ".num_ref_idx_l1_active_minus1 = %d,\n",
		     slice_params->num_ref_idx_l1_active_minus1);
	print_indent(indent, ".mvd_l1_zero_flag = %d,\n",
		     slice_params->LongSliceFlags.fields.mvd_l1_zero_flag);
	print_indent(indent, ".cabac_init_flag = %d,\n",
		     slice_params->LongSliceFlags.fields.cabac_init_flag);
	print_indent(indent, ".collocated_from_l0_flag = %d,\n",
		     slice_params->LongSliceFlags.fields.collocated_from_l0_flag);
	print_indent(indent, ".collocated_ref_idx = %d,\n",
		     slice_params->collocated_ref_idx);
	print_indent(indent, ".five_minus_max_num_merge_cand = %d,\n",
		     slice_params->five_minus_max_num_merge_cand);
	print_indent(indent, ".slice_qp_delta = %d,\n",
		     slice_params->slice_qp_delta);
	print_indent(indent, ".slice_cb_qp_offset = %d,\n",
		     slice_params->slice_cb_qp_offset);
	print_indent(indent, ".slice_cr_qp_offset = %d,\n",
		     slice_params->slice_cr_qp_offset);
	print_indent(indent, ".slice_deblocking_filter_disabled_flag = %d,\n",
		     slice_params->LongSliceFlags.fields.slice_deblocking_filter_disabled_flag);
	print_indent(indent, ".slice_beta_offset_div2 = %d,\n",
		     slice_params->slice_beta_offset_div2);
	print_indent(indent, ".slice_tc_offset_div2 = %d,\n",
		     slice_params->slice_tc_offset_div2);
	print_indent(indent, ".slice_loop_filter_across_slices_enabled_flag = %d,\n",
		     slice_params->LongSliceFlags.fields.slice_loop_filter_across_slices_enabled_flag);
	// FIXME
	print_indent(indent, ".num_entry_point_offsets = %d,\n", 0);
	print_indent(indent, ".entry_point_offset_minus1 = { %d },\n", 0);

	print_indent(indent++, ".pred_weight_table = {\n");
	print_indent(indent, ".luma_log2_weight_denom = %d,\n", slice_params->luma_log2_weight_denom);
	print_indent(indent, ".delta_chroma_log2_weight_denom = %d,\n", slice_params->delta_chroma_log2_weight_denom);

	// TODO: the rest of pred_weight_table

	print_indent(--indent, "},\n");

	// TODO: get from list of pic lists
	print_indent(indent, ".num_rps_poc_st_curr_before = %d,\n", 0);
	print_indent(indent, ".num_rps_poc_st_curr_after = %d,\n", 0);
	print_indent(indent, ".num_rps_poc_lt_curr = %d,\n", 0);

	print_indent(--indent, "},\n");
}

void h265_dump_prepare(struct dump_driver_data *driver_data)
{
}

void h265_dump_header(struct dump_driver_data *driver_data, void *slice_data,
		      unsigned int slice_size)
{
	unsigned int index = driver_data->frame_index;
	unsigned int indent = 1;

	print_indent(indent++, "{\n");
	print_indent(indent, ".index = %d,\n", index);
	print_indent(indent, ".output_buffer = %d,\n", index);
	print_indent(indent++, ".frame.h265 = {\n");

	h265_dump_sps(driver_data, indent);
	h265_dump_pps(driver_data, indent);
	h265_dump_slice_params(driver_data, indent, slice_data, slice_size);

	print_indent(--indent, "},\n");
	print_indent(--indent, "},\n");
}
