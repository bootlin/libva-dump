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

#ifndef _IMAGE_H_
#define _IMAGE_H_

#include <va/va_backend.h>

#include "object_heap.h"

/*
 * Values
 */

#define IMAGE_ID_OFFSET						0x10000000

/*
 * Structures
 */

struct object_image {
	struct object_base base;
	VABufferID buffer_id;
};

/*
 * Functions
 */

VAStatus DumpQueryImageFormats(VADriverContextP context, VAImageFormat *formats, int *formats_count);
VAStatus DumpCreateImage(VADriverContextP context, VAImageFormat *format, int width, int height, VAImage *image);
VAStatus DumpDeriveImage(VADriverContextP context, VASurfaceID surface_id, VAImage *image);
VAStatus DumpDestroyImage(VADriverContextP context, VAImageID image_id);
VAStatus DumpSetImagePalette(VADriverContextP context, VAImageID image_id, unsigned char *palette);
VAStatus DumpGetImage(VADriverContextP context, VASurfaceID surface_id, int x, int y, unsigned int width, unsigned int height, VAImageID image_id);
VAStatus DumpPutImage(VADriverContextP context, VASurfaceID surface_id, VAImageID image, int src_x, int src_y, unsigned int src_width, unsigned int src_height, int dst_x, int dst_y, unsigned int dst_width, unsigned int dst_height);

#endif
