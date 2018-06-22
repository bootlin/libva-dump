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
#include <string.h>

#include "dump.h"
#include "image.h"
#include "surface.h"
#include "buffer.h"

VAStatus DumpCreateImage(VADriverContextP context, VAImageFormat *format,
	int width, int height, VAImage *image)
{
	struct dump_driver_data *driver_data = (struct dump_driver_data *) context->pDriverData;
	struct object_image *image_object;
	unsigned int size;
	VABufferID buffer_id;
	VAImageID id;
	VAStatus status;

	size = width * height * 3 / 2;

	id = object_heap_allocate(&driver_data->image_heap);
	image_object = (struct object_image *) object_heap_lookup(&driver_data->image_heap, id);
	if (image_object == NULL)
		return VA_STATUS_ERROR_ALLOCATION_FAILED;

	status = DumpCreateBuffer(context, 0, VAImageBufferType, size, 1, NULL, &buffer_id);
	if (status != VA_STATUS_SUCCESS) {
		object_heap_free(&driver_data->image_heap, (struct object_base *) image_object);
		return status;
	}

	image_object->buffer_id = buffer_id;

	memset(image, 0, sizeof(*image));

	image->format = *format;
	image->width = width;
	image->height = height;
	image->num_planes = 2;
	image->pitches[0] = image->width;
	image->pitches[1] = image->width;
	image->offsets[0] = 0;
	image->offsets[1] = image->width * image->height;
	image->data_size  = image->width * image->height * 3 / 2;
	image->buf = buffer_id;
	image->image_id = id;

	return VA_STATUS_SUCCESS;
}

VAStatus DumpDestroyImage(VADriverContextP context, VAImageID image_id)
{
	struct dump_driver_data *driver_data = (struct dump_driver_data *) context->pDriverData;
	struct object_image *image_object;
	VAStatus status;

	image_object = (struct object_image *) object_heap_lookup(&driver_data->image_heap, image_id);
	if (image_object == NULL)
		return VA_STATUS_ERROR_INVALID_IMAGE;

	status = DumpDestroyBuffer(context, image_object->buffer_id);
	if (status != VA_STATUS_SUCCESS)
		return status;

	object_heap_free(&driver_data->image_heap, (struct object_base *) image_object);

	return VA_STATUS_SUCCESS;
}

VAStatus DumpDeriveImage(VADriverContextP context, VASurfaceID surface_id,
	VAImage *image)
{
	struct dump_driver_data *driver_data = (struct dump_driver_data *) context->pDriverData;
	struct object_surface *surface_object;
	VAImageFormat format;
	VAStatus status;

	surface_object = (struct object_surface *) object_heap_lookup(&driver_data->surface_heap, surface_id);
	if (surface_object == NULL)
		return VA_STATUS_ERROR_INVALID_SURFACE;

	format.fourcc = VA_FOURCC_NV12;

	status = DumpCreateImage(context, &format, surface_object->width, surface_object->height, image);
	if (status != VA_STATUS_SUCCESS)
		return status;

	surface_object->status = VASurfaceReady;

	return VA_STATUS_SUCCESS;
}

VAStatus DumpQueryImageFormats(VADriverContextP context, VAImageFormat *formats,
	int *formats_count)
{
	formats[0].fourcc = VA_FOURCC_NV12;
	*formats_count = 1;

	return VA_STATUS_SUCCESS;
}

VAStatus DumpSetImagePalette(VADriverContextP context, VAImageID image_id,
	unsigned char *palette)
{
	return VA_STATUS_SUCCESS;
}

VAStatus DumpGetImage(VADriverContextP context, VASurfaceID surface_id, int x,
	int y, unsigned int width, unsigned int height, VAImageID image_id)
{
	return VA_STATUS_SUCCESS;
}

VAStatus DumpPutImage(VADriverContextP context, VASurfaceID surface_id,
	VAImageID image, int src_x, int src_y, unsigned int src_width,
	unsigned int src_height, int dst_x, int dst_y, unsigned int dst_width,
	unsigned int dst_height)
{
	return VA_STATUS_SUCCESS;
}
