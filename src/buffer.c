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

#include <va/va_backend.h>

#include "dump.h"
#include "buffer.h"

VAStatus DumpCreateBuffer(VADriverContextP context, VAContextID context_id, VABufferType type, unsigned int size, unsigned int count, void *data, VABufferID *buffer_id)
{
	struct dump_driver_data *driver_data = (struct dump_driver_data *) context->pDriverData;
	struct object_buffer *buffer_object;
	void *buffer_data;
	VABufferID id;

	switch (type) {
		case VAPictureParameterBufferType:
		case VAIQMatrixBufferType:
		case VASliceParameterBufferType:
		case VASliceDataBufferType:
		case VAImageBufferType:
			break;
		default:
			return VA_STATUS_ERROR_UNSUPPORTED_BUFFERTYPE;
	}

	id = object_heap_allocate(&driver_data->buffer_heap);
	buffer_object = (struct object_buffer *) object_heap_lookup(&driver_data->buffer_heap, id);
	if (buffer_object == NULL)
		return VA_STATUS_ERROR_ALLOCATION_FAILED;

	buffer_data = malloc(size * count);
	if (buffer_data == NULL) {
		object_heap_free(&driver_data->buffer_heap, (struct object_base *) buffer_object);
		return VA_STATUS_ERROR_ALLOCATION_FAILED;
	}

	if (data != NULL)
		memcpy(buffer_data, data, size * count);

	buffer_object->type = type;
	buffer_object->initial_count = count;
	buffer_object->count = count;
	buffer_object->data = buffer_data;
	buffer_object->size = size;

	*buffer_id = id;

	return VA_STATUS_SUCCESS;
}

VAStatus DumpBufferSetNumElements(VADriverContextP context, VABufferID buffer_id, unsigned int count)
{
	struct dump_driver_data *driver_data = (struct dump_driver_data *) context->pDriverData;
	struct object_buffer *buffer_object;

	buffer_object = (struct object_buffer *) object_heap_lookup(&driver_data->buffer_heap, buffer_id);
	if (buffer_object == NULL)
		return VA_STATUS_ERROR_INVALID_BUFFER;

	if (count > buffer_object->initial_count)
		return VA_STATUS_ERROR_INVALID_PARAMETER;

	buffer_object->count = count;

	return VA_STATUS_SUCCESS;
}

VAStatus DumpMapBuffer(VADriverContextP context, VABufferID buffer_id, void **data_map)
{
	struct dump_driver_data *driver_data = (struct dump_driver_data *) context->pDriverData;
	struct object_buffer *buffer_object;

	buffer_object = (struct object_buffer *) object_heap_lookup(&driver_data->buffer_heap, buffer_id);
	if (buffer_object == NULL || buffer_object->data == NULL)
		return VA_STATUS_ERROR_INVALID_BUFFER;

	/* Our buffers are always mapped. */
	*data_map = buffer_object->data;

	return VA_STATUS_SUCCESS;
}

VAStatus DumpUnmapBuffer(VADriverContextP context, VABufferID buffer_id)
{
	struct dump_driver_data *driver_data = (struct dump_driver_data *) context->pDriverData;
	struct object_buffer *buffer_object;

	buffer_object = (struct object_buffer *) object_heap_lookup(&driver_data->buffer_heap, buffer_id);
	if (buffer_object == NULL || buffer_object->data == NULL)
		return VA_STATUS_ERROR_INVALID_BUFFER;

	/* Our buffers are always mapped. */

	return VA_STATUS_SUCCESS;
}

VAStatus DumpDestroyBuffer(VADriverContextP context, VABufferID buffer_id)
{
	struct dump_driver_data *driver_data = (struct dump_driver_data *) context->pDriverData;
	struct object_buffer *buffer_object;

	buffer_object = (struct object_buffer *) object_heap_lookup(&driver_data->buffer_heap, buffer_id);
	if (buffer_object == NULL)
		return VA_STATUS_ERROR_INVALID_BUFFER;

	if (buffer_object->data != NULL)
		free(buffer_object->data);

	object_heap_free(&driver_data->buffer_heap, (struct object_base *) buffer_object);

	return VA_STATUS_SUCCESS;
}

VAStatus DumpBufferInfo(VADriverContextP context, VABufferID buffer_id, VABufferType *type, unsigned int *size, unsigned int *count)
{
	struct dump_driver_data *driver_data = (struct dump_driver_data *) context->pDriverData;
	struct object_buffer *buffer_object;

	buffer_object = (struct object_buffer *) object_heap_lookup(&driver_data->buffer_heap, buffer_id);
	if (buffer_object == NULL)
		return VA_STATUS_ERROR_INVALID_BUFFER;

	*type = buffer_object->type;
	*size = buffer_object->size;
	*count = buffer_object->count;

	return VA_STATUS_SUCCESS;
}
