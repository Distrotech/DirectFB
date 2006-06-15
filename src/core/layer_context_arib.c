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

#include <directfb_arib.h>

#include <core/coredefs.h>
#include <core/coretypes.h>

#include <core/core.h>
#include <core/layer_context.h>
#include <core/layer_context_arib.h>

#include <core/layer_control.h>
#include <core/layer_region.h>
#include <core/screen.h>
#include <core/surfaces.h>
#include <core/system.h>
#include <core/windows_arib.h>
#include <core/windowstack.h>
#include <core/windowstack_arib.h>

#include <core/layers_internal.h>
#include <core/windows_internal.h>

#include <direct/messages.h>

#include <direct/util.h>

#include <misc/conf.h>
#include <misc/util.h>

DFBResult
dfb_arib_layer_context_set_screenlocation( CoreLayerContext  *context,
                                           const DFBLocation *location )
{
	CoreLayer         *layer;
	DisplayLayerFuncs *funcs;

	D_ASSERT( context  != NULL );
	D_ASSERT( location != NULL );

	layer = dfb_layer_at( context->layer_id );
	D_ASSERT( layer != NULL );

	funcs = layer->funcs;
	D_ASSERT( funcs != NULL );

	/* Lock the context. */
	if (dfb_layer_context_lock( context ))
		return DFB_FUSION;

	if (funcs->SetAribLocation) {
		funcs->SetAribLocation( layer,
		                        layer->driver_data,
		                        layer->layer_data,
		                        location );
	}
	/* Unlock the context. */
	dfb_layer_context_unlock( context );

	return DFB_OK;
}

DFBResult
dfb_arib_layer_context_create_window( CoreLayerContext       *context,
                                      DFBWindowID            arib_id,
                                      int                    x,
                                      int                    y,
                                      int                    width,
                                      int                    height,
                                      DFBWindowCapabilities  caps,
                                      DFBSurfaceCapabilities surface_caps,
                                      DFBSurfacePixelFormat  pixelformat,
                                      CoreWindow             **ret_window )
{
	DFBResult       ret;
	CoreWindow      *window;
	CoreWindowStack *stack;
	CoreLayer       *layer;

	D_ASSERT( context != NULL );
	D_ASSERT( context->stack != NULL );
	D_ASSERT( ret_window != NULL );

	layer = dfb_layer_at( context->layer_id );
	D_ASSERT( layer != NULL );

	A_TRACE("%s: id(%d) (%d,%d,%d,%d)\n", __FUNCTION__,arib_id, x,y,width,height);

	if (dfb_layer_context_lock( context ))
		return DFB_FUSION;

	stack = context->stack;

	if (!stack->cursor.set) {
		/* DFB_ARIB : true -> false */
		ret = dfb_windowstack_cursor_enable( stack, false );
		if (ret) {
			dfb_layer_context_unlock( context );
			return ret;
		}
	}

	ret = dfb_arib_window_create( stack,
	                              arib_id,
	                              x,
	                              y,
	                              width,
	                              height,
	                              caps,
								  surface_caps,
								  pixelformat,
								  &window );

	*ret_window = window;

	if (ret) {
		dfb_layer_context_unlock( context );
		return ret;
	}


	dfb_layer_context_unlock( context );

	return DFB_OK;
}

DFBResult
dfb_arib_layer_context_get_window( CoreLayerContext *context,
                                   DFBWindowID      arib_id,
                                   CoreWindow       **ret_window )
{
	DFBResult ret;
	CoreLayer *layer;

	D_ASSERT( context != NULL );
	D_ASSERT( context->stack != NULL );
	D_ASSERT( ret_window != NULL );

	layer = dfb_layer_at( context->layer_id );
	D_ASSERT( layer != NULL );

	A_TRACE("%s: id(%d)\n", __FUNCTION__,arib_id);

	if (dfb_layer_context_lock( context ))
		return DFB_FUSION;

	ret = dfb_arib_window_get( context->stack, arib_id, ret_window );
	if (ret == DFB_BUSY) {
		ret = DFB_OK;
	}
	dfb_layer_context_unlock( context );

	return ret;
}

DFBResult
dfb_arib_layer_context_set_mpeg_resolution( CoreLayerContext *context,
                                            DFBDimension     *seqhead_resolution,
                                            DFBDimension     *seqdisp_resolution,
                                            DFBBoolean       seqhead_aspect_ratio,
                                            DFBBoolean       seqhead_progressive )
{
	CoreLayer         *layer;
	DisplayLayerFuncs *funcs;
	DFBDimension      ret_resolution;
	DFBBoolean        change;

	D_ASSERT( context != NULL );
	D_ASSERT( seqhead_resolution != NULL );
	D_ASSERT( seqdisp_resolution != NULL );

	layer = dfb_layer_at( context->layer_id );
	D_ASSERT( layer != NULL );

	funcs = layer->funcs;
	D_ASSERT( funcs != NULL );

	/* Lock the context. */
	if (dfb_layer_context_lock( context ))
		return DFB_FUSION;

	if (funcs->SetMpegResolution) {
		change = funcs->SetMpegResolution( layer,
		                                   layer->driver_data,
		                                   layer->layer_data,
		                                   seqhead_resolution,
		                                   seqdisp_resolution,
		                                   seqhead_aspect_ratio,
		                                   seqhead_progressive,
		                                   &ret_resolution );
	}
	/* Unlock the context. */
	dfb_layer_context_unlock( context );

	return DFB_OK;
}

DFBResult
dfb_arib_layer_context_set_dbcast_resolution( CoreLayerContext *context,
                                              DFBDimension     *resolution,
                                              DFBBoolean       aspect_ratio )
{
	CoreLayer         *layer;
	DisplayLayerFuncs *funcs;
	DFBDimension      ret_resolution;
	DFBBoolean        change;

	D_ASSERT( context != NULL );
	D_ASSERT( resolution != NULL );

	layer = dfb_layer_at( context->layer_id );
	D_ASSERT( layer != NULL );

	funcs = layer->funcs;
	D_ASSERT( funcs != NULL );

	/* Lock the context. */
	if (dfb_layer_context_lock( context ))
		return DFB_FUSION;

	if (funcs->SetDBcastResolution) {
		change = funcs->SetDBcastResolution( layer,
		                                     layer->driver_data,
		                                     layer->layer_data,
		                                     resolution,
		                                     aspect_ratio,
		                                     &ret_resolution );
	}
	/* Unlock the context. */
	dfb_layer_context_unlock( context );

	return DFB_OK;
}

DFBResult
dfb_arib_layer_context_set_bml_visibility( CoreLayerContext *context,
                                           DFBBoolean        bml_valid,
                                           DFBBoolean        bml_invisible,
                                           DFBBoolean        bml_has_video)
{
	DFBAribBmlInfo    bml;
	CoreLayer         *layer;
	DisplayLayerFuncs *funcs;
	DFBDimension      ret_resolution;
	DFBBoolean        change;

	D_ASSERT( context != NULL );

	layer = dfb_layer_at( context->layer_id );
	D_ASSERT( layer != NULL );

	funcs = layer->funcs;
	D_ASSERT( funcs != NULL );

	/* Lock the context. */
	if (dfb_layer_context_lock( context ))
		return DFB_FUSION;

	if (funcs->SetBmlVisibility) {
		bml.bml_valid     = bml_valid;
		bml.bml_invisible = bml_invisible;
		bml.bml_has_video = bml_has_video;

		change = funcs->SetBmlVisibility( layer,
		                                  layer->driver_data,
		                                  layer->layer_data,
		                                  &bml,
		                                  &ret_resolution );
	}
	/* Unlock the context. */
	dfb_layer_context_unlock( context );

	return DFB_OK;
}

DFBResult
dfb_arib_layer_context_set_listener( CoreLayerContext  *context,
                                     DFBCallBackId     call_id,
                                     FusionCallHandler callback,
                                     void              *ctx )
{
	DFBResult         ret = DFB_OK;
	CoreLayer         *layer;
	DisplayLayerFuncs *funcs;

	D_ASSERT( context != NULL );

	layer = dfb_layer_at( context->layer_id );
	D_ASSERT( layer != NULL );

	funcs = layer->funcs;
	D_ASSERT( funcs != NULL );

	/* Lock the context. */
	if (dfb_layer_context_lock( context ))
		return DFB_FUSION;

	if (funcs->SetCallBackFunction) {
		ret = funcs->SetCallBackFunction( layer,
		                                  layer->driver_data,
		                                  layer->layer_data,
		                                  call_id,
		                                  callback,
		                                  ctx );
	}
	/* Unlock the context. */
	dfb_layer_context_unlock( context );

	return ret;
}

DFBResult
dfb_arib_layer_context_get_monitor_info( CoreLayerContext  *context,
                                         DFBMonitorType    *monitor_type,
                                         DFBMonitorAspectRatio *monitor_aspect )
{
	DFBResult ret = DFB_OK;

	D_ASSERT( context != NULL );

	/* Lock the context. */
	if (dfb_layer_context_lock( context ))
		return DFB_FUSION;

	*monitor_type   = dfb_config->monitor_type;
	*monitor_aspect = dfb_config->monitor_aspect;

	/* Unlock the context. */
	dfb_layer_context_unlock( context );

	return ret;
}

DFBResult
dfb_arib_layer_context_set_resolution_table( CoreLayerContext *context,
                                             DFB_RESOLUTION_TABLE *resolution_table )
{
	DFBResult         ret = DFB_OK;
	CoreLayer         *layer;
	DisplayLayerFuncs *funcs;

	D_ASSERT( context != NULL );

	layer = dfb_layer_at( context->layer_id );
	D_ASSERT( layer != NULL );

	funcs = layer->funcs;
	D_ASSERT( funcs != NULL );

	/* Lock the context. */
	if (dfb_layer_context_lock( context ))
		return DFB_FUSION;

	if (funcs->SetResolutionTable) {
		ret = funcs->SetResolutionTable( layer,
		                                 layer->driver_data,
		                                 layer->layer_data,
		                                 resolution_table );
	}
	/* Unlock the context. */
	dfb_layer_context_unlock( context );

	return ret;
}

