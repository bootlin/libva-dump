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
#include "context.h"
#include "config.h"

VAStatus DumpCreateContext(VADriverContextP context, VAConfigID config_id, int picture_width, int picture_height, int flag, VASurfaceID *render_targets, int render_targets_count, VAContextID *context_id)
{
	struct dump_driver_data *driver_data = (struct dump_driver_data *) context->pDriverData;
	struct object_config *config_object;
	struct object_surface *surface_object;
	struct object_context *context_object = NULL;
	VASurfaceID *targets = NULL;
	VAContextID id;
	VAStatus status;
	unsigned int i;

	config_object = (struct object_config *) object_heap_lookup(&driver_data->config_heap, config_id);
	if (config_object == NULL) {
		status = VA_STATUS_ERROR_INVALID_CONFIG;
		goto error;
	}

	targets = malloc(render_targets_count * sizeof(VASurfaceID));
	if (targets == NULL) {
		status = VA_STATUS_ERROR_ALLOCATION_FAILED;
		goto error;
	}

	id = object_heap_allocate(&driver_data->context_heap);
	context_object = (struct object_context *) object_heap_lookup(&driver_data->context_heap, id);
	if (context_object == NULL) {
		status = VA_STATUS_ERROR_ALLOCATION_FAILED;
		goto error;
	}

	for (i = 0; i < render_targets_count; i++) {
		surface_object = (struct object_surface *) object_heap_lookup(&driver_data->surface_heap, render_targets[i]);
		if (surface_object == NULL) {
			status = VA_STATUS_ERROR_INVALID_SURFACE;
			goto error;
		}
	}

	context_object->config_id = config_id;

	*context_id = id;

	status = VA_STATUS_SUCCESS;
	goto complete;

error:
	if (targets != NULL)
		free(targets);

	if (context_object != NULL)
		object_heap_free(&driver_data->context_heap, (struct object_base *) context_object);

complete:
	return status;
}

VAStatus DumpDestroyContext(VADriverContextP context, VAContextID context_id)
{
	struct dump_driver_data *driver_data = (struct dump_driver_data *) context->pDriverData;
	struct object_context *context_object;

	context_object = (struct object_context *) object_heap_lookup(&driver_data->context_heap, context_id);
	if (context_object == NULL)
		return VA_STATUS_ERROR_INVALID_CONTEXT;

	object_heap_free(&driver_data->context_heap, (struct object_base *) context_object);

	return VA_STATUS_SUCCESS;
}
