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

#ifndef _DUMP_H_
#define _DUMP_H_

#include <va/va_backend.h>

#include "object_heap.h"

/*
 * Values
 */

#define DUMP_STR_VENDOR				"Dump Driver"

#define DUMP_MAX_PROFILES					11
#define DUMP_MAX_ENTRYPOINTS					5
#define DUMP_MAX_CONFIG_ATTRIBUTES				10
#define DUMP_MAX_SURFACE_ATTRIBUTES				10
#define DUMP_MAX_IMAGE_FORMATS					10
#define DUMP_MAX_SUBPIC_FORMATS					4
#define DUMP_MAX_DISPLAY_ATTRIBUTES				4

/*
 * Structures
 */

struct dump_driver_data {
	struct object_heap config_heap;
	struct object_heap context_heap;
	struct object_heap surface_heap;
	struct object_heap buffer_heap;
	struct object_heap image_heap;

	char *slices_path;
	char *slices_filename_format;
	unsigned int dump_count;

	int dump_fd;
	unsigned int frame_index;

	VAIQMatrixBufferH264 matrix;
	VAPictureParameterBufferH264 picture;
	VASliceParameterBufferH264 slice;
};

/*
 * Functions
 */

VAStatus VA_DRIVER_INIT_FUNC(VADriverContextP context);
VAStatus DumpTerminate(VADriverContextP context);

#endif
