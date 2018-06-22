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

#include "subpicture.h"

VAStatus DumpCreateSubpicture(VADriverContextP context, VAImageID image_id,
	VASubpictureID *subpicture_id)
{
	return VA_STATUS_SUCCESS;
}

VAStatus DumpDestroySubpicture(VADriverContextP context,
	VASubpictureID subpicture_id)
{
	return VA_STATUS_SUCCESS;
}

VAStatus DumpQuerySubpictureFormats(VADriverContextP context,
	VAImageFormat *formats, unsigned int *flags,
	unsigned int *formats_count)
{
	return VA_STATUS_SUCCESS;
}

VAStatus DumpSetSubpictureImage(VADriverContextP context,
	VASubpictureID subpicture_id, VAImageID image_id)
{
	return VA_STATUS_SUCCESS;
}

VAStatus DumpSetSubpicturePalette(VADriverContextP context,
	VASubpictureID subpicture_id, unsigned char *palette)
{
	return VA_STATUS_SUCCESS;
}

VAStatus DumpSetSubpictureChromakey(VADriverContextP context,
	VASubpictureID subpicture_id, unsigned int chromakey_min,
	unsigned int chromakey_max, unsigned int chromakey_mask)
{
	return VA_STATUS_SUCCESS;
}

VAStatus DumpSetSubpictureGlobalAlpha(VADriverContextP ctx,
	VASubpictureID subpicture, float global_alpha)
{
	return VA_STATUS_SUCCESS;
}

VAStatus DumpAssociateSubpicture(VADriverContextP context,
	VASubpictureID subpicture_id, VASurfaceID *surfaces_ids,
	int surfaces_count, short src_x, short src_y, unsigned short src_width,
	unsigned short src_height, short dst_x, short dst_y,
	unsigned short dst_width, unsigned short dst_height, unsigned int flags)
{
	return VA_STATUS_SUCCESS;
}

VAStatus DumpDeassociateSubpicture(VADriverContextP context,
	VASubpictureID subpicture_id, VASurfaceID *surfaces_ids,
	int surfaces_count)
{
	return VA_STATUS_SUCCESS;
}
