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

#ifndef __WINDOWS_ARIB_H__
#define __WINDOWS_ARIB_H__

#include <directfb_arib.h>

#include <core/coredefs.h>
#include <core/coretypes.h>

#include <fusion/object.h>



/*
 * creates a window on a given stack
 */
DFBResult
dfb_arib_window_create(
						CoreWindowStack        *stack,
						DFBWindowID            arib_id,
						int                    x,
						int                    y,
						int                    width,
						int                    height,
						DFBWindowCapabilities  caps,
						DFBSurfaceCapabilities surface_caps,
						DFBSurfacePixelFormat  pixelformat,
						CoreWindow             **ret_window );

DFBResult
dfb_arib_window_get( CoreWindowStack *stack,
                     DFBWindowID     arib_id,
                     CoreWindow      **ret_window );

DFBResult
dfb_arib_window_repaint(
						CoreWindow          *window,
						DFBRegion           *region,
						DFBSurfaceFlipFlags flags );

DFBResult
dfb_arib_window_set_switching_region(
						CoreWindow   *window,
						DFBRectangle *rect1,
						DFBRectangle *rect2,
						DFBRectangle *rect3,
						DFBRectangle *rect4,
						DFBBoolean   attribute );

DFBResult
dfb_arib_window_change_events(
						CoreWindow         *window,
						DFBWindowEventType disable,
						DFBWindowEventType enable );

DFBResult
dfb_arib_window_set_colorkey(
						CoreWindow *window,
						__u32      color_key );

DFBResult
dfb_arib_window_change_options(
						CoreWindow       *window,
						DFBWindowOptions disable,
						DFBWindowOptions enable );

DFBResult
dfb_arib_window_set_opaque(
						CoreWindow      *window,
						const DFBRegion *region );

DFBResult
dfb_arib_window_set_opacity(
						CoreWindow *window,
						__u8       opacity );

DFBResult
dfb_arib_window_batch_start(
						CoreWindow *window );

DFBResult
dfb_arib_window_batch_end(
						CoreWindow *window );

#endif
