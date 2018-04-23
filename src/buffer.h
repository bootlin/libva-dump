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

#ifndef _BUFFER_H_
#define _BUFFER_H_

#include <va/va_backend.h>

#include "object_heap.h"

/*
 * Values
 */

#define BUFFER_ID_OFFSET					0x08000000

/*
 * Structures
 */

struct object_buffer {
	struct object_base base;

	VABufferType type;
	unsigned int initial_count;
	unsigned int count;

	void *data;
	unsigned int size;
};

/*
 * Functions
 */

VAStatus DumpCreateBuffer(VADriverContextP context, VAContextID context_id, VABufferType type, unsigned int size, unsigned int count, void *data, VABufferID *buffer_id);
VAStatus DumpDestroyBuffer(VADriverContextP context, VABufferID buffer_id);
VAStatus DumpMapBuffer(VADriverContextP context, VABufferID buffer_id, void **data_map);
VAStatus DumpUnmapBuffer(VADriverContextP context, VABufferID buffer_id);
VAStatus DumpBufferSetNumElements(VADriverContextP context, VABufferID buffer_id, unsigned int count);
VAStatus DumpBufferInfo(VADriverContextP context, VABufferID buffer_id, VABufferType *type, unsigned int *size, unsigned int *count);

#endif
