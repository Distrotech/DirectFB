/*
 * (c) Copyright 2004-2006 Mitsubishi Electric Corp.
 *
 * All rights reserved.
 *
 * Written by Koichi Hiramatsu,
 *            Seishi Takahashi,
 *            Atsushi Hori
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef __IDIRECTFBARIBFONT_H__
#define __IDIRECTFBARIBFONT_H__

#include <directfb.h>

#include <core/coretypes.h>

/*
 * probing context
 */
typedef struct {
     const char *filename;
} IDirectFBARIBFont_ProbeContext;

/*
 * private data struct of IDirectFBARIBFont
 */
typedef struct {
     int            ref;   /* reference counter    */
     CoreARIBFont  *font;  /* pointer to core font */
     int           *palette; /* array of color indexes for LUT8. minus for transparent */
     int            graylevel; /* number of entries in the gray-level palette */
     IDirectFBARIBDrcs* aribdrcs;
} IDirectFBARIBFont_data;

/*
 * common code to construct the interface (internal usage only)
 */
DFBResult IDirectFBARIBFont_Construct( IDirectFBARIBFont *thiz, CoreARIBFont *font );

/*
 * deinitialize font and its surfaces
 */
void IDirectFBARIBFont_Destruct( IDirectFBARIBFont *thiz );

#endif
