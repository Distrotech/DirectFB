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

#ifndef __IDIRECTFBWINDOW_ARIB_H__
#define __IDIRECTFBWINDOW_ARIB_H__

#include <directfb_arib.h>

#include <core/coretypes.h>
#include <windows/idirectfbwindow.h>

typedef struct {
     IDirectFBWindow_data  parent;
} IDirectFBARIBWindow_data;

/*
 * initializes a new window and constructs interface
 */
DFBResult
IDirectFBARIBWindow_Construct(
						IDirectFBARIBWindow *thiz,
						CoreWindow          *window,
						CoreLayer           *layer );

#endif
