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

#ifndef __CORE__ARIB_LAYER_CONTEXT_H__
#define __CORE__ARIB_LAYER_CONTEXT_H__

#include <directfb.h>

#include <core/coretypes.h>

DFBResult
dfb_arib_layer_context_set_screenlocation(
						CoreLayerContext  *context,
						const DFBLocation *location );

DFBResult
dfb_arib_layer_context_create_window(
						CoreLayerContext       *context,
						DFBWindowID            arib_id,
						int                    x,
						int                    y,
						int                    width,
						int                    height,
						DFBWindowCapabilities  caps,
						DFBSurfaceCapabilities surface_caps,
						DFBSurfacePixelFormat  pixelformat,
						CoreWindow             **window );

DFBResult
dfb_arib_layer_context_get_window(
						CoreLayerContext *context,
						DFBWindowID      arib_id,
						CoreWindow       **ret_window );

DFBResult
dfb_arib_layer_context_set_mpeg_resolution(
						CoreLayerContext *context,
						DFBDimension     *seqhead_resolution,
						DFBDimension     *seqdisp_resolution,
						DFBBoolean       seqhead_aspect_ratio,
						DFBBoolean       seqhead_progressive );

DFBResult
dfb_arib_layer_context_set_dbcast_resolution(
						CoreLayerContext *context,
						DFBDimension     *resolution,
						DFBBoolean       aspect_ratio);

DFBResult
dfb_arib_layer_context_set_bml_visibility(
						CoreLayerContext *context,
						DFBBoolean       bml_valid,
						DFBBoolean       bml_invisible,
						DFBBoolean       bml_has_video);

DFBResult
dfb_arib_layer_context_set_listener(
						CoreLayerContext  *context,
						DFBCallBackId     call_id,
						FusionCallHandler callback,
						void              *ctx );

DFBResult
dfb_arib_layer_context_get_monitor_info(
						CoreLayerContext      *context,
						DFBMonitorType        *monitor_type,
						DFBMonitorAspectRatio *monitor_aspect );

DFBResult
dfb_arib_layer_context_set_resolution_table(
						CoreLayerContext     *context,
						DFB_RESOLUTION_TABLE *resolution_table );

#endif
