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

#ifndef _PICTURE_H_
#define _PICTURE_H_

#include <va/va_backend.h>

#include "object_heap.h"

VAStatus DumpBeginPicture(VADriverContextP context, VAContextID context_id,
	VASurfaceID surface_id);
VAStatus DumpRenderPicture(VADriverContextP context, VAContextID context_id,
	VABufferID *buffers, int buffers_count);
VAStatus DumpEndPicture(VADriverContextP context, VAContextID context_id);

#endif
