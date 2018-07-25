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

#include <va/va_backend.h>

#include "dump.h"

void h264_start_dump(struct dump_driver_data *driver_data);
void h264_picture_parameter_dump(struct dump_driver_data *driver_data,
				 VAPictureParameterBufferH264 *parameters);
void h264_quantization_matrix_dump(struct dump_driver_data *driver_data,
				   VAIQMatrixBufferH264 *parameters);
void h264_slice_parameter_dump(struct dump_driver_data *driver_data,
			       VASliceParameterBufferH264 *parameters);
void h264_stop_dump(struct dump_driver_data *driver_data);

void mpeg2_start_dump(struct dump_driver_data *driver_data);
void mpeg2_header_dump(struct dump_driver_data *driver_data,
	VAPictureParameterBufferMPEG2 *parameters);
void mpeg2_quantization_matrix_dump(struct dump_driver_data *driver_data,
				    VAIQMatrixBufferMPEG2 *parameters);
void mpeg2_stop_dump(struct dump_driver_data *driver_data);

#endif
