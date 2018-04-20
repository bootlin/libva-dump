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

#include <stdio.h>

#include <stdlib.h>
#include <string.h>

#include "dump.h"
#include "surface.h"

VAStatus DumpCreateSurfaces2(VADriverContextP context, unsigned int format, unsigned int width, unsigned int height, VASurfaceID *surfaces, unsigned int surfaces_count, VASurfaceAttrib *attributes, unsigned int attributes_count)
{
	struct dump_driver_data *driver_data = (struct dump_driver_data *) context->pDriverData;
	struct object_surface *surface_object;
	VASurfaceID id;
	int i;

	if (format != VA_RT_FORMAT_YUV420)
		return VA_STATUS_ERROR_UNSUPPORTED_RT_FORMAT;

	for (i = 0; i < surfaces_count; i++) {
		id = object_heap_allocate(&driver_data->surface_heap);

		surface_object = (struct object_surface *) object_heap_lookup(&driver_data->surface_heap, id);
		if (surface_object == NULL)
			return VA_STATUS_ERROR_ALLOCATION_FAILED;

		surface_object->status = VASurfaceReady;
		surface_object->width = width;
		surface_object->height = height;
		surface_object->index = i;

		surfaces[i] = id;
	}

	return VA_STATUS_SUCCESS;
}

VAStatus DumpCreateSurfaces(VADriverContextP context, int width, int height, int format, int surfaces_count, VASurfaceID *surfaces)
{
	return DumpCreateSurfaces2(context, format, width, height, surfaces, surfaces_count, NULL, 0);
}

VAStatus DumpDestroySurfaces(VADriverContextP context, VASurfaceID *surfaces, int surfaces_count)
{
	struct dump_driver_data *driver_data = (struct dump_driver_data *) context->pDriverData;
	struct object_surface *surface_object;
	int i;

	for (i = 0; i < surfaces_count; i++) {
		surface_object = (struct object_surface *) object_heap_lookup(&driver_data->surface_heap, surfaces[i]);
		if (surface_object == NULL)
			return VA_STATUS_ERROR_INVALID_SURFACE;

		object_heap_free(&driver_data->surface_heap, (struct object_base *) surface_object);
	}

	return VA_STATUS_SUCCESS;
}

VAStatus DumpSyncSurface(VADriverContextP context, VASurfaceID render_target)
{
	struct dump_driver_data *driver_data = (struct dump_driver_data *) context->pDriverData;
	struct object_surface *surface_object;

	surface_object = (struct object_surface *) object_heap_lookup(&driver_data->surface_heap, render_target);
	if (surface_object == NULL)
		return VA_STATUS_ERROR_INVALID_SURFACE;

	surface_object->status = VASurfaceReady;

	return VA_STATUS_SUCCESS;
}

VAStatus DumpQuerySurfaceStatus(VADriverContextP context, VASurfaceID render_target, VASurfaceStatus *status)
{
	struct dump_driver_data *driver_data = (struct dump_driver_data *) context->pDriverData;
	struct object_surface *surface_object;

	surface_object = (struct object_surface *) object_heap_lookup(&driver_data->surface_heap, render_target);
	if (surface_object == NULL)
		return VA_STATUS_ERROR_INVALID_SURFACE;

	*status = surface_object->status;

	return VA_STATUS_SUCCESS;
}

VAStatus DumpQuerySurfaceAttributes(VADriverContextP context, VAConfigID config_id, VASurfaceAttrib *attributes, unsigned int *attributes_count)
{
	struct dump_driver_data *driver_data = (struct dump_driver_data *) context->pDriverData;
	struct object_config *config_object;

	config_object = (struct object_config *) object_heap_lookup(&driver_data->config_heap, config_id);
	if (config_object == NULL)
		return VA_STATUS_ERROR_INVALID_CONFIG;

	if (attributes_count != NULL)
		*attributes_count = 1;

	/* Attributes might be NULL to retrieve the associated count. */
	if (attributes != NULL) {
		attributes[0].type = VASurfaceAttribPixelFormat;
		attributes[0].value.type = VAGenericValueTypeInteger;
		attributes[0].value.value.i = VA_RT_FORMAT_YUV420;
	}

	return VA_STATUS_SUCCESS;
}

VAStatus DumpGetSurfaceAttributes(VADriverContextP context, VAConfigID config_id, VASurfaceAttrib *attributes, unsigned int attributes_count)
{
	unsigned int i;

	for (i = 0; i < attributes_count; i++) {
		switch (attributes[i].type) {
			case VASurfaceAttribPixelFormat:
				attributes[i].value.type = VAGenericValueTypeInteger;
				attributes[i].value.value.i = VA_RT_FORMAT_YUV420;
				break;
			default:
				attributes[i].value.type = VAGenericValueTypeInteger;
				attributes[i].value.value.i = VA_ATTRIB_NOT_SUPPORTED;
				break;
		}
	}

	return VA_STATUS_SUCCESS;
}

VAStatus DumpPutSurface(VADriverContextP context, VASurfaceID surface_id, void *draw, short src_x, short src_y, unsigned short src_width, unsigned short src_height, short dst_x, short dst_y, unsigned short dst_width, unsigned short dst_height, VARectangle *cliprects, unsigned int cliprects_count, unsigned int flags)
{
	struct dump_driver_data *driver_data = (struct dump_driver_data *) context->pDriverData;
	struct object_surface *surface_object;

	surface_object = (struct object_surface *) object_heap_lookup(&driver_data->surface_heap, surface_id);
	if (surface_object == NULL)
		return VA_STATUS_ERROR_INVALID_SURFACE;

	surface_object->status = VASurfaceReady;

	return VA_STATUS_SUCCESS;
}

VAStatus DumpLockSurface(VADriverContextP context, VASurfaceID surface, unsigned int *fourcc, unsigned int *luma_stride, unsigned int *chroma_u_stride, unsigned int *chroma_v_stride, unsigned int *luma_offset, unsigned int *chroma_u_offset, unsigned int *chroma_v_offset, unsigned int *buffer_name, void **buffer)
{
	return VA_STATUS_ERROR_UNIMPLEMENTED;
}

VAStatus DumpUnlockSurface(VADriverContextP context, VASurfaceID surface)
{
	return VA_STATUS_ERROR_UNIMPLEMENTED;
}
