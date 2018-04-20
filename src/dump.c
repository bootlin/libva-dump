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

#include <unistd.h>
#include <stdarg.h>
#include <fcntl.h>

#include <va/va_backend.h>

#include "dump.h"
#include "buffer.h"
#include "context.h"
#include "image.h"
#include "picture.h"
#include "subpicture.h"
#include "surface.h"
#include "config.h"

#include "autoconfig.h"

VAStatus DumpTerminate(VADriverContextP context)
{
	struct dump_driver_data *driver_data = (struct dump_driver_data *) context->pDriverData;
	struct object_buffer *buffer_object;
	struct object_image *image_object;
	struct object_surface *surface_object;
	struct object_context *context_object;
	struct object_config *config_object;
	object_heap_iterator iterator;

	image_object = (struct object_image *) object_heap_first(&driver_data->image_heap, &iterator);
	while (image_object != NULL) {
		DumpDestroyImage(context, (VAImageID) image_object->base.id);
		image_object = (struct object_image *) object_heap_next(&driver_data->image_heap, &iterator);
	}

	object_heap_destroy(&driver_data->image_heap);

	buffer_object = (struct object_buffer *) object_heap_first(&driver_data->buffer_heap, &iterator);
	while (buffer_object != NULL) {
		DumpDestroyBuffer(context, (VABufferID) buffer_object->base.id);
		buffer_object = (struct object_buffer *) object_heap_next(&driver_data->buffer_heap, &iterator);
	}

	object_heap_destroy(&driver_data->buffer_heap);

	surface_object = (struct object_surface *) object_heap_first(&driver_data->surface_heap, &iterator);
	while (surface_object != NULL) {
		DumpDestroySurfaces(context, (VASurfaceID *) &surface_object->base.id, 1);
		surface_object = (struct object_surface *) object_heap_next(&driver_data->surface_heap, &iterator);
	}

	object_heap_destroy(&driver_data->surface_heap);

	context_object = (struct object_context *) object_heap_first(&driver_data->context_heap, &iterator);
	while (context_object != NULL) {
		DumpDestroyContext(context, (VAContextID) context_object->base.id);
		context_object = (struct object_context *) object_heap_next(&driver_data->context_heap, &iterator);
	}

	object_heap_destroy(&driver_data->context_heap);

	config_object = (struct object_config *) object_heap_first(&driver_data->config_heap, &iterator);
	while (config_object != NULL) {
		DumpDestroyConfig(context, (VAConfigID) config_object->base.id);
		config_object = (struct object_config *) object_heap_next(&driver_data->config_heap, &iterator);
	}

	object_heap_destroy(&driver_data->config_heap);

	free(context->pDriverData);
	context->pDriverData = NULL;

	return VA_STATUS_SUCCESS;
}

/* Only expose the init function. */
VAStatus __attribute__((visibility("default"))) VA_DRIVER_INIT_FUNC(VADriverContextP context) // FIXME: might bug here
{
	struct dump_driver_data *driver_data;
	struct VADriverVTable *vtable = context->vtable;
	char *env;

	context->version_major = VA_MAJOR_VERSION;
	context->version_minor = VA_MINOR_VERSION;
	context->max_profiles = DUMP_MAX_PROFILES;
	context->max_entrypoints = DUMP_MAX_ENTRYPOINTS;
	context->max_attributes = DUMP_MAX_CONFIG_ATTRIBUTES;
	context->max_image_formats = DUMP_MAX_IMAGE_FORMATS;
	context->max_subpic_formats = DUMP_MAX_SUBPIC_FORMATS;
	context->max_display_attributes = DUMP_MAX_DISPLAY_ATTRIBUTES;
	context->str_vendor = DUMP_STR_VENDOR;

	vtable->vaTerminate = DumpTerminate;
	vtable->vaQueryConfigEntrypoints = DumpQueryConfigEntrypoints;
	vtable->vaQueryConfigProfiles = DumpQueryConfigProfiles;
	vtable->vaQueryConfigEntrypoints = DumpQueryConfigEntrypoints;
	vtable->vaQueryConfigAttributes = DumpQueryConfigAttributes;
	vtable->vaCreateConfig = DumpCreateConfig;
	vtable->vaDestroyConfig = DumpDestroyConfig;
	vtable->vaGetConfigAttributes = DumpGetConfigAttributes;
	vtable->vaCreateSurfaces = DumpCreateSurfaces;
	vtable->vaCreateSurfaces2 = DumpCreateSurfaces2;
	vtable->vaDestroySurfaces = DumpDestroySurfaces;
	vtable->vaCreateContext = DumpCreateContext;
	vtable->vaDestroyContext = DumpDestroyContext;
	vtable->vaCreateBuffer = DumpCreateBuffer;
	vtable->vaBufferSetNumElements = DumpBufferSetNumElements;
	vtable->vaMapBuffer = DumpMapBuffer;
	vtable->vaUnmapBuffer = DumpUnmapBuffer;
	vtable->vaDestroyBuffer = DumpDestroyBuffer;
	vtable->vaBeginPicture = DumpBeginPicture;
	vtable->vaRenderPicture = DumpRenderPicture;
	vtable->vaEndPicture = DumpEndPicture;
	vtable->vaSyncSurface = DumpSyncSurface;
	vtable->vaQuerySurfaceStatus = DumpQuerySurfaceStatus;
	vtable->vaPutSurface = DumpPutSurface;
	vtable->vaQueryImageFormats = DumpQueryImageFormats;
	vtable->vaCreateImage = DumpCreateImage;
	vtable->vaDeriveImage = DumpDeriveImage;
	vtable->vaDestroyImage = DumpDestroyImage;
	vtable->vaSetImagePalette = DumpSetImagePalette;
	vtable->vaGetImage = DumpGetImage;
	vtable->vaPutImage = DumpPutImage;
	vtable->vaQuerySubpictureFormats = DumpQuerySubpictureFormats;
	vtable->vaCreateSubpicture = DumpCreateSubpicture;
	vtable->vaDestroySubpicture = DumpDestroySubpicture;
	vtable->vaSetSubpictureImage = DumpSetSubpictureImage;
	vtable->vaSetSubpictureChromakey = DumpSetSubpictureChromakey;
	vtable->vaSetSubpictureGlobalAlpha = DumpSetSubpictureGlobalAlpha;
	vtable->vaAssociateSubpicture = DumpAssociateSubpicture;
	vtable->vaDeassociateSubpicture = DumpDeassociateSubpicture;
	vtable->vaQueryDisplayAttributes = DumpQueryDisplayAttributes;
	vtable->vaGetDisplayAttributes = DumpGetDisplayAttributes;
	vtable->vaSetDisplayAttributes = DumpSetDisplayAttributes;
	vtable->vaLockSurface = DumpLockSurface;
	vtable->vaUnlockSurface = DumpUnlockSurface;
	vtable->vaGetSurfaceAttributes = DumpGetSurfaceAttributes;
	vtable->vaQuerySurfaceAttributes = DumpQuerySurfaceAttributes;

	vtable->vaBufferInfo = DumpBufferInfo;

	driver_data = (struct dump_driver_data *) malloc(sizeof(*driver_data));
	memset(driver_data, 0, sizeof(*driver_data));

	context->pDriverData = (void *) driver_data;

	object_heap_init(&driver_data->config_heap, sizeof(struct object_config), CONFIG_ID_OFFSET);
	object_heap_init(&driver_data->context_heap, sizeof(struct object_context), CONTEXT_ID_OFFSET);
	object_heap_init(&driver_data->surface_heap, sizeof(struct object_surface), SURFACE_ID_OFFSET);
	object_heap_init(&driver_data->buffer_heap, sizeof(struct object_buffer), BUFFER_ID_OFFSET);
	object_heap_init(&driver_data->image_heap, sizeof(struct object_image), IMAGE_ID_OFFSET);

	driver_data->slices_path = ".";
	driver_data->slices_filename_format = "slice-%d.dump";
	driver_data->dump_count = 3;

	driver_data->dump_fd = -1;

	env = getenv("DUMP_COUNT");
	if (env != NULL)
		driver_data->dump_count = atoi(env);

	return VA_STATUS_SUCCESS;
}
