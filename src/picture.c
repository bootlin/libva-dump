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

#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>

#include "dump.h"
#include "picture.h"
#include "config.h"
#include "context.h"
#include "surface.h"
#include "buffer.h"
#include "header.h"

VAStatus DumpBeginPicture(VADriverContextP context, VAContextID context_id,
	VASurfaceID surface_id)
{
	struct dump_driver_data *driver_data = (struct dump_driver_data *) context->pDriverData;
	struct object_context *context_object;
	struct object_config *config_object;
	struct object_surface *surface_object;
	char *slice_filename;
	char *slice_path;
	unsigned int index;
	int fd;

	context_object = (struct object_context *) object_heap_lookup(&driver_data->context_heap, context_id);
	if (context_object == NULL)
		return VA_STATUS_ERROR_INVALID_CONTEXT;

	config_object = (struct object_config *) object_heap_lookup(&driver_data->config_heap, context_object->config_id);
	if (config_object == NULL)
		return VA_STATUS_ERROR_INVALID_CONFIG;

	surface_object = (struct object_surface *) object_heap_lookup(&driver_data->surface_heap, surface_id);
	if (surface_object == NULL)
		return VA_STATUS_ERROR_INVALID_SURFACE;

	surface_object->status = VASurfaceRendering;
	context_object->render_surface_id = surface_id;

	index = driver_data->frame_index;
	if (index >= driver_data->dump_count)
		return VA_STATUS_SUCCESS;

	asprintf(&slice_filename, driver_data->slices_filename_format, index);
	asprintf(&slice_path, "%s/%s", driver_data->slices_path, slice_filename);

	fd = open(slice_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);

	free(slice_path);
	free(slice_filename);

	if (fd < 0) {
		fprintf(stderr, "Unable to open slice dump path %s: %s\n", slice_path, strerror(errno));
		return VA_STATUS_SUCCESS;
	}

	driver_data->dump_fd = fd;

	switch (config_object->profile) {
		case VAProfileH264Main:
		case VAProfileH264High:
		case VAProfileH264ConstrainedBaseline:
		case VAProfileH264MultiviewHigh:
		case VAProfileH264StereoHigh:
			h264_start_dump(driver_data);
			break;

		case VAProfileMPEG2Simple:
		case VAProfileMPEG2Main:
			mpeg2_start_dump(driver_data);
			break;

		default:
			fprintf(stderr, "Unsupported profile\n");
			return VA_STATUS_SUCCESS;
	}

	return VA_STATUS_SUCCESS;
}

VAStatus DumpRenderPicture(VADriverContextP context, VAContextID context_id,
	VABufferID *buffers, int buffers_count)
{
	struct dump_driver_data *driver_data = (struct dump_driver_data *) context->pDriverData;
	struct object_context *context_object;
	struct object_config *config_object;
	struct object_surface *surface_object;
	struct object_buffer *buffer_object;
	VABufferID buffer_id;
	int i;

	context_object = (struct object_context *) object_heap_lookup(&driver_data->context_heap, context_id);
	if (context_object == NULL)
		return VA_STATUS_ERROR_INVALID_CONTEXT;

	config_object = (struct object_config *) object_heap_lookup(&driver_data->config_heap, context_object->config_id);
	if (config_object == NULL)
		return VA_STATUS_ERROR_INVALID_CONFIG;

	surface_object = (struct object_surface *) object_heap_lookup(&driver_data->surface_heap, context_object->render_surface_id);
	if (surface_object == NULL)
		return VA_STATUS_ERROR_INVALID_SURFACE;

	if (driver_data->dump_fd < 0)
		return VA_STATUS_SUCCESS;

	for (i = 0; i < buffers_count; i++) {
		buffer_id = buffers[i];

		buffer_object = (struct object_buffer *) object_heap_lookup(&driver_data->buffer_heap, buffer_id);
		if (buffer_object == NULL)
			return VA_STATUS_ERROR_INVALID_BUFFER;

		if (buffer_object->type == VASliceDataBufferType) {
			fprintf(stderr, "Dumping %d bytes of slice %d/%d\n", buffer_object->size, driver_data->frame_index + 1, driver_data->dump_count);
			write(driver_data->dump_fd, buffer_object->data, buffer_object->size);
		} else if (buffer_object->type == VASliceParameterBufferType) {
			switch (config_object->profile) {
				case VAProfileH264Main:
				case VAProfileH264High:
				case VAProfileH264ConstrainedBaseline:
				case VAProfileH264MultiviewHigh:
				case VAProfileH264StereoHigh:
					h264_slice_parameter_dump(driver_data,
								  buffer_object->data);
					break;
			default:
				break;
			}
		} else if (buffer_object->type == VAPictureParameterBufferType) {
			switch (config_object->profile) {
				case VAProfileMPEG2Simple:
				case VAProfileMPEG2Main:
					mpeg2_header_dump(driver_data, (VAPictureParameterBufferMPEG2 *) buffer_object->data);
					break;
				case VAProfileH264Main:
				case VAProfileH264High:
				case VAProfileH264ConstrainedBaseline:
				case VAProfileH264MultiviewHigh:
				case VAProfileH264StereoHigh:
					h264_picture_parameter_dump(driver_data,
								    buffer_object->data);
					break;
				default:
					break;
			}
		} else if (buffer_object->type == VAIQMatrixBufferType) {
			switch (config_object->profile) {
				case VAProfileMPEG2Simple:
				case VAProfileMPEG2Main:
					mpeg2_quantization_matrix_dump(driver_data, (VAIQMatrixBufferMPEG2 *) buffer_object->data);
					break;
				case VAProfileH264Main:
				case VAProfileH264High:
				case VAProfileH264ConstrainedBaseline:
				case VAProfileH264MultiviewHigh:
				case VAProfileH264StereoHigh:
					h264_quantization_matrix_dump(driver_data,
								      buffer_object->data);
					break;
			default:
				break;
			}
		} else {
			fprintf(stderr, "Unknown buffer type %d\n",
				buffer_object->type);
		}
	}

	return VA_STATUS_SUCCESS;
}

VAStatus DumpEndPicture(VADriverContextP context, VAContextID context_id)
{
	struct dump_driver_data *driver_data = (struct dump_driver_data *) context->pDriverData;
	struct object_context *context_object;
	struct object_config *config_object;
	struct object_surface *surface_object;

	context_object = (struct object_context *) object_heap_lookup(&driver_data->context_heap, context_id);
	if (context_object == NULL)
		return VA_STATUS_ERROR_INVALID_CONTEXT;

	config_object = (struct object_config *) object_heap_lookup(&driver_data->config_heap, context_object->config_id);
	if (config_object == NULL)
		return VA_STATUS_ERROR_INVALID_CONFIG;

	surface_object = (struct object_surface *) object_heap_lookup(&driver_data->surface_heap, context_object->render_surface_id);
	if (surface_object == NULL)
		return VA_STATUS_ERROR_INVALID_SURFACE;

	if (driver_data->frame_index < driver_data->dump_count) {
		switch (config_object->profile) {
			case VAProfileH264Main:
			case VAProfileH264High:
			case VAProfileH264ConstrainedBaseline:
			case VAProfileH264MultiviewHigh:
			case VAProfileH264StereoHigh:
				h264_stop_dump(driver_data);
				break;

			case VAProfileMPEG2Simple:
			case VAProfileMPEG2Main:
				mpeg2_stop_dump(driver_data);
				break;

			default:
				fprintf(stderr, "Unsupported profile\n");
				return VA_STATUS_SUCCESS;
		}
	}

	/* Update last-seen frame index of the surface to stay in sync with current frame index. */
	surface_object->index = driver_data->frame_index;

	context_object->render_surface_id = VA_INVALID_ID;

	if (driver_data->dump_fd >= 0) {
		close(driver_data->dump_fd);
		driver_data->dump_fd = -1;
	}

	driver_data->frame_index++;

	return VA_STATUS_SUCCESS;
}
