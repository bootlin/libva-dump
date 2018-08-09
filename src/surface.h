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

#ifndef _SURFACE_H_
#define _SURFACE_H_

#include <va/va_backend.h>

#include "object_heap.h"

/*
 * Values
 */


#define SURFACE_ID_OFFSET		0x04000000

/*
 * Structures
 */

struct object_surface {
	struct object_base base;

	VAStatus status;
	unsigned int width;
	unsigned int height;
	unsigned int index;

	void *slice_data;
	unsigned int slice_size;
};

/*
 * Functions
 */

VAStatus DumpCreateSurfaces2(VADriverContextP context, unsigned int format,
	unsigned int width, unsigned int height, VASurfaceID *surfaces,
	unsigned int surfaces_count, VASurfaceAttrib *attributes,
	unsigned int attributes_count);
VAStatus DumpCreateSurfaces(VADriverContextP context, int width, int height,
	int format, int surfaces_count, VASurfaceID *surfaces);
VAStatus DumpDestroySurfaces(VADriverContextP context, VASurfaceID *surfaces,
	int surfaces_count);
VAStatus DumpSyncSurface(VADriverContextP context, VASurfaceID surface_id);
VAStatus DumpQuerySurfaceStatus(VADriverContextP context,
	VASurfaceID surface_id, VASurfaceStatus *status);
VAStatus DumpQuerySurfaceAttributes(VADriverContextP context,
	VAConfigID config_id, VASurfaceAttrib *attributes,
	unsigned int *attributes_count);
VAStatus DumpGetSurfaceAttributes(VADriverContextP context,
	VAConfigID config_id, VASurfaceAttrib *attributes,
	unsigned int attributes_count);
VAStatus DumpPutSurface(VADriverContextP context, VASurfaceID surface_id,
	void *draw, short src_x, short src_y, unsigned short src_width,
	unsigned short src_height, short dst_x, short dst_y,
	unsigned short dst_width, unsigned short dst_height,
	VARectangle *cliprects, unsigned int cliprects_count,
	unsigned int flags);
VAStatus DumpLockSurface(VADriverContextP context, VASurfaceID surface_id,
	unsigned int *fourcc, unsigned int *luma_stride,
	unsigned int *chroma_u_stride, unsigned int *chroma_v_stride,
	unsigned int *luma_offset, unsigned int *chroma_u_offset,
	unsigned int *chroma_v_offset, unsigned int *buffer_name,
	void **buffer);
VAStatus DumpUnlockSurface(VADriverContextP context, VASurfaceID surface);

#endif
