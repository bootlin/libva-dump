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

#ifndef _CONTEXT_H_
#define _CONTEXT_H_

#include <va/va_backend.h>

#include "object_heap.h"

/*
 * Values
 */

#define CONTEXT_ID_OFFSET					0x02000000

/*
 * Structures
 */

struct object_context {
	struct object_base base;

	VAConfigID config_id;
	VASurfaceID render_surface_id;
	VASurfaceID *surfaces_ids;
	int surfaces_count;

	int picture_width;
	int picture_height;
	int flags;
};

/*
 * Functions
 */

VAStatus DumpCreateContext(VADriverContextP context, VAConfigID config_id,
	int picture_width, int picture_height, int flag,
	VASurfaceID *surfaces_ids, int surfaces_count, VAContextID *context_id);
VAStatus DumpDestroyContext(VADriverContextP context, VAContextID context_id);

#endif
