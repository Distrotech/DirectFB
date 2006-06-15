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

#ifndef __CORE__ARIB_LAYER_REGION_H__
#define __CORE__ARIB_LAYER_REGION_H__

#include <directfb_arib.h>

#include <core/coretypes.h>
#include <core/layers.h>

#include <fusion/object.h>
#include <core/layer_region.h>


/*
 * Generates dfb_layer_region_ref(), dfb_layer_region_attach() etc.
 */
DFBResult
dfb_arib_layer_region_flip_update (
						CoreLayerRegion     *region,
						const DFBRegion     *update,
						DFBSurfaceFlipFlags flags );

DFBResult
dfb_arib_layer_region_temporary_to_layer_surface(
						CoreLayerRegion *region,
						DFBRectangle    *src_full_rect,
						DFBRectangle    *dst_full_rect);

DFBResult
dfb_arib_layer_region_clear_temporary_surface(
						CoreLayerRegion *region,
						DFBRectangle    *clear_rect );

DFBResult
dfb_arib_layer_region_get_temporary_surface(
						CoreLayerRegion *region,
						CoreSurface     **surface );

DFBResult
dfb_arib_layer_region_get_destination_rectangle(
						CoreLayerRegion *region,
						DFBRectangle    *src_rect,
						DFBRectangle    *src_full_rect,
						DFBRectangle    *dst_rect,
						DFBRectangle    *dst_full_rect,
						DFBRectangle    *modify_rect );

#endif

