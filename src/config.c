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

#include "config.h"

VAStatus DumpCreateConfig(VADriverContextP context, VAProfile profile,
	VAEntrypoint entrypoint, VAConfigAttrib *attributes,
	int attributes_count, VAConfigID *config_id)
{
	struct dump_driver_data *driver_data = (struct dump_driver_data *) context->pDriverData;
	struct object_config *config_object;
	VAConfigID id;
	int i, index;

	switch (profile) {
		case VAProfileH264Main:
		case VAProfileH264High:
		case VAProfileH264ConstrainedBaseline:
		case VAProfileH264MultiviewHigh:
		case VAProfileH264StereoHigh:
			if (entrypoint != VAEntrypointVLD)
				return VA_STATUS_ERROR_UNSUPPORTED_ENTRYPOINT;
			break;

		case VAProfileMPEG2Simple:
		case VAProfileMPEG2Main:
			if (entrypoint != VAEntrypointVLD)
				return VA_STATUS_ERROR_UNSUPPORTED_ENTRYPOINT;
			break;

		default:
			return VA_STATUS_ERROR_UNSUPPORTED_PROFILE;
	}

	if (attributes_count > DUMP_MAX_CONFIG_ATTRIBUTES)
		attributes_count = DUMP_MAX_CONFIG_ATTRIBUTES;

	id = object_heap_allocate(&driver_data->config_heap);
	config_object = (struct object_config *) object_heap_lookup(&driver_data->config_heap, id);
	if (config_object == NULL)
		return VA_STATUS_ERROR_ALLOCATION_FAILED;

	config_object->profile = profile;
	config_object->entrypoint = entrypoint;
	config_object->attributes[0].type = VAConfigAttribRTFormat;
	config_object->attributes[0].value = VA_RT_FORMAT_YUV420;
	config_object->attributes_count = 1;

	for (i = 1; i < attributes_count; i++) {
		index = config_object->attributes_count++;
		config_object->attributes[index].type = attributes[index].type;
		config_object->attributes[index].value = attributes[index].value;
	}

	*config_id = id;

	return VA_STATUS_SUCCESS;
}

VAStatus DumpDestroyConfig(VADriverContextP context, VAConfigID config_id)
{
	struct dump_driver_data *driver_data = (struct dump_driver_data *) context->pDriverData;
	struct object_config *config_object;

	config_object = (struct object_config *) object_heap_lookup(&driver_data->config_heap, config_id);
	if (config_object == NULL)
		return VA_STATUS_ERROR_INVALID_CONFIG;

	object_heap_free(&driver_data->config_heap, (struct object_base *) config_object);

	return VA_STATUS_SUCCESS;
}

VAStatus DumpQueryConfigAttributes(VADriverContextP context,
	VAConfigID config_id, VAProfile *profile, VAEntrypoint *entrypoint,
	VAConfigAttrib *attributes, int *attributes_count)
{
	struct dump_driver_data *driver_data = (struct dump_driver_data *) context->pDriverData;
	struct object_config *config_object;
	int i;

	config_object = (struct object_config *) object_heap_lookup(&driver_data->config_heap, config_id);
	if (config_object == NULL)
		return VA_STATUS_ERROR_INVALID_CONFIG;

	if (profile != NULL)
		*profile = config_object->profile;

	if (entrypoint != NULL)
		*entrypoint = config_object->entrypoint;

	if (attributes_count != NULL)
		*attributes_count =  config_object->attributes_count;

	/* Attributes might be NULL to retrieve the associated count. */
	if (attributes != NULL)
		for (i = 0; i < config_object->attributes_count; i++)
			attributes[i] = config_object->attributes[i];

	return VA_STATUS_SUCCESS;
}

VAStatus DumpGetConfigAttributes(VADriverContextP context, VAProfile profile,
	VAEntrypoint entrypoint, VAConfigAttrib *attributes,
	int attributes_count)
{
	unsigned int i;

	for (i = 0; i < attributes_count; i++) {
		switch (attributes[i].type) {
			case VAConfigAttribRTFormat:
				attributes[i].value = VA_RT_FORMAT_YUV420;
				break;
			default:
				attributes[i].value = VA_ATTRIB_NOT_SUPPORTED;
				break;
		}
	}

	return VA_STATUS_SUCCESS;
}

VAStatus DumpQueryConfigProfiles(VADriverContextP context, VAProfile *profiles,
	int *profiles_count)
{
	VAProfile supported_profiles[] = {
		VAProfileH264ConstrainedBaseline,
		VAProfileH264High,
		VAProfileMPEG2Main,
		VAProfileMPEG2Simple,
	};
	unsigned int supported_profiles_count = sizeof(supported_profiles) / sizeof(supported_profiles[0]);
	unsigned int i;

	if (supported_profiles_count > DUMP_MAX_PROFILES)
		supported_profiles_count = DUMP_MAX_PROFILES;

	for (i = 0; i < supported_profiles_count; i++)
		profiles[i] = supported_profiles[i];

	*profiles_count = supported_profiles_count;

	return VA_STATUS_SUCCESS;
}

VAStatus DumpQueryConfigEntrypoints(VADriverContextP context, VAProfile profile,
	VAEntrypoint *entrypoints, int *entrypoints_count)
{
	switch (profile) {
		case VAProfileH264Main:
		case VAProfileH264High:
		case VAProfileH264ConstrainedBaseline:
		case VAProfileH264MultiviewHigh:
		case VAProfileH264StereoHigh:
			entrypoints[0] = VAEntrypointVLD;
			*entrypoints_count = 1;
			break;

		case VAProfileMPEG2Simple:
		case VAProfileMPEG2Main:
			entrypoints[0] = VAEntrypointVLD;
			*entrypoints_count = 1;
			break;

		default:
			*entrypoints_count = 0;
			break;
	}

	return VA_STATUS_SUCCESS;
}

VAStatus DumpQueryDisplayAttributes(VADriverContextP context,
	VADisplayAttribute *attributes, int *attributes_count)
{
	return VA_STATUS_ERROR_UNIMPLEMENTED;
}

VAStatus DumpGetDisplayAttributes(VADriverContextP context,
	VADisplayAttribute *attributes, int attributes_count)
{
	return VA_STATUS_ERROR_UNIMPLEMENTED;
}

VAStatus DumpSetDisplayAttributes(VADriverContextP context,
	VADisplayAttribute *attributes, int attributes_count)
{
	return VA_STATUS_ERROR_UNIMPLEMENTED;
}
