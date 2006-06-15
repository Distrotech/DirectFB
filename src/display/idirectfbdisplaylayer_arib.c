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

#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <string.h>

#include <directfb_arib.h>

#include <core/core.h>
#include <core/coredefs.h>
#include <core/coretypes.h>

#include <core/surfaces.h>
#include <core/gfxcard.h>
#include <core/layers.h>
#include <core/layer_context.h>
#include <core/layer_control.h>
#include <core/layer_region.h>
#include <core/layer_context_arib.h>
#include <core/layer_region_arib.h>
#include <core/state.h>
#include <core/windows.h>
#include <core/windowstack.h>

#include <core/windows_arib.h>
#include <core/windowstack_arib.h>

#include <windows/idirectfbwindow.h>
#include <windows/idirectfbwindow_arib.h>

#include <gfx/convert.h>

#include <direct/interface.h>
#include <direct/interface_arib.h>

#include <direct/mem.h>
#include <direct/messages.h>

#include "idirectfbdisplaylayer.h"
#include "idirectfbdisplaylayer_arib.h"

#include "idirectfbscreen.h"
#include "idirectfbsurface.h"
#include "idirectfbsurface_layer.h"

#include <media/idirectfbfont.h>
#include <media/idirectfbimageprovider.h>
#include <media/idirectfbvideoprovider.h>
#include <media/idirectfbdatabuffer.h>


static DFBResult
IDirectFBDisplayLayer_SetBackgroundMode( IDirectFBDisplayLayer         *thiz,
                                         DFBDisplayLayerBackgroundMode background_mode )
{
	return DFB_UNSUPPORTED;
}

static DFBResult
IDirectFBDisplayLayer_SetBackgroundImage( IDirectFBDisplayLayer *thiz,
                                          IDirectFBSurface      *surface )
{
	return DFB_UNSUPPORTED;
}

static DFBResult
IDirectFBDisplayLayer_SetBackgroundColor( IDirectFBDisplayLayer *thiz,
                                          __u8 r, __u8 g, __u8 b, __u8 a )
{
	return DFB_UNSUPPORTED;
}

static DFBResult
IDirectFBDisplayLayer_SetScreenLocation( IDirectFBDisplayLayer *thiz,
                                         float                 x,
                                         float                 y,
                                         float                 width,
                                         float                 height )
{
	DFBLocation location = { x, y, width, height };

	DIRECT_INTERFACE_GET_DATA(IDirectFBDisplayLayer)

	if (width <= 0 || height <= 0)
		return DFB_INVARG;

	return dfb_arib_layer_context_set_screenlocation( data->context, &location );
}

static DFBResult
IDirectFBDisplayLayer_EnableCursor( IDirectFBDisplayLayer *thiz,
                                    int                   enable )
{
	return DFB_UNSUPPORTED;
}

static DFBResult
IDirectFBDisplayLayer_GetCursorPosition( IDirectFBDisplayLayer *thiz,
                                         int                   *x,
                                         int                   *y )
{
	return DFB_UNSUPPORTED;
}

static DFBResult
IDirectFBDisplayLayer_WarpCursor( IDirectFBDisplayLayer *thiz,
                                  int                   x,
                                  int                   y )
{
	return DFB_UNSUPPORTED;
}

static DFBResult
IDirectFBDisplayLayer_SetCursorAcceleration( IDirectFBDisplayLayer *thiz,
                                             int                   numerator,
                                             int                   denominator,
                                             int                   threshold )
{
	return DFB_UNSUPPORTED;
}

static DFBResult
IDirectFBDisplayLayer_SetCursorShape( IDirectFBDisplayLayer *thiz,
                                      IDirectFBSurface      *shape,
                                      int                   hot_x,
                                      int                   hot_y )
{
	return DFB_UNSUPPORTED;
}

static DFBResult
IDirectFBDisplayLayer_SetCursorOpacity( IDirectFBDisplayLayer *thiz,
                                        __u8                  opacity )
{
	return DFB_UNSUPPORTED;
}

static void
IDirectFBARIBDisplayLayer_Destruct( IDirectFBARIBDisplayLayer *thiz )
{
	IDirectFBDisplayLayer_data *parent_data = (IDirectFBDisplayLayer_data*)thiz->parent.priv;

	dfb_layer_region_unref( parent_data->region );
	dfb_layer_context_unref(parent_data->context );

	DIRECT_DEALLOCATE_CHILD_INTERFACE( thiz );
}

static DFBResult
IDirectFBARIBDisplayLayer_AddRef( IDirectFBARIBDisplayLayer *thiz )
{
	IDirectFBDisplayLayer_data *parent_data;

	DIRECT_CHILD_INTERFACE_GET_DATA(IDirectFBARIBDisplayLayer)

	parent_data = (IDirectFBDisplayLayer_data *)data;

	parent_data->ref++;

	return DFB_OK;
}

static DFBResult
IDirectFBARIBDisplayLayer_Release( IDirectFBARIBDisplayLayer *thiz )
{
	IDirectFBDisplayLayer_data *parent_data;

	DIRECT_CHILD_INTERFACE_GET_DATA(IDirectFBARIBDisplayLayer)

	parent_data = (IDirectFBDisplayLayer_data *)data;

	if (--parent_data->ref == 0)
		IDirectFBARIBDisplayLayer_Destruct( thiz );

	return DFB_OK;
}

static DFBResult
IDirectFBARIBDisplayLayer_CreateAribWindow( IDirectFBARIBDisplayLayer  *thiz,
                                            DFBWindowID                arib_id,
                                            const DFBWindowDescription *desc,
                                            IDirectFBARIBWindow        **window )
{
	CoreWindow                 *w;
	DFBResult                  ret;
	unsigned int               width        = DFB_ARIB_HD_DIMENSION_W;
	unsigned int               height       = DFB_ARIB_HD_DIMENSION_H;
	int                        posx         = 0;
	int                        posy         = 0;
	DFBWindowCapabilities      caps         = 0;
	DFBSurfaceCapabilities     surface_caps = 0;
	DFBSurfacePixelFormat      format       = DSPF_UNKNOWN;
	IDirectFBDisplayLayer_data *parent_data;

	DIRECT_CHILD_INTERFACE_GET_DATA(IDirectFBARIBDisplayLayer);

	parent_data = (IDirectFBDisplayLayer_data *)data;

	if (desc->flags & DWDESC_WIDTH)
		width = desc->width;
	if (desc->flags & DWDESC_HEIGHT)
		height = desc->height;
	if (desc->flags & DWDESC_PIXELFORMAT)
		format = desc->pixelformat;
	if (desc->flags & DWDESC_POSX)
		posx = desc->posx;
	if (desc->flags & DWDESC_POSY)
		posy = desc->posy;
	if (desc->flags & DWDESC_CAPS)
		caps = desc->caps;
	if (desc->flags & DWDESC_SURFACE_CAPS)
		surface_caps = desc->surface_caps;

	if ((caps & ~DWCAPS_ALL) || !window)
		return DFB_INVARG;

	if (width < 1 || width > 4096 || height < 1 || height > 4096)
		return DFB_INVARG;

	ret = dfb_arib_layer_context_create_window( parent_data->context, arib_id,
	                                            posx, posy, width, height, caps,
	                                            surface_caps, format, &w );

	if (ret == DFB_OK) {
		DIRECT_ALLOCATE_INTERFACE( *window, IDirectFBARIBWindow );
		return IDirectFBARIBWindow_Construct( *window, w, parent_data->layer );
	}
	else if (ret == DFB_BUSY) {
		ret = dfb_arib_layer_context_get_window( parent_data->context, arib_id, &w );
		if (ret == DFB_OK) {
			DIRECT_ALLOCATE_INTERFACE( *window, IDirectFBARIBWindow );
			return IDirectFBARIBWindow_Construct( *window, w, parent_data->layer );
		}
	}
	return ret;
}

static DFBResult
IDirectFBARIBDisplayLayer_GetAribWindow( IDirectFBARIBDisplayLayer *thiz,
                                         DFBWindowID               arib_id,
                                         IDirectFBARIBWindow       **window )
{
	CoreWindow                 *w;
	DFBResult                  ret;
	IDirectFBDisplayLayer_data *parent_data;

	DIRECT_CHILD_INTERFACE_GET_DATA(IDirectFBARIBDisplayLayer);

	parent_data = (IDirectFBDisplayLayer_data *)data;

	ret = dfb_arib_layer_context_get_window( parent_data->context, arib_id, &w );
	if (ret == DFB_OK && w != NULL) {
		DIRECT_ALLOCATE_INTERFACE( *window, IDirectFBARIBWindow );
		return IDirectFBARIBWindow_Construct( *window, w, parent_data->layer );
	}
	else {
		return ret;
	}
}

static DFBResult
IDirectFBARIBDisplayLayer_SetMpegResolution( IDirectFBARIBDisplayLayer *thiz,
                                             DFBDimension              *seqhead_resolution,
                                             DFBDimension              *seqdisp_resolution,
                                             DFBBoolean                seqhead_aspect_ratio,
                                             DFBBoolean                progressive )
{
	IDirectFBDisplayLayer_data *parent_data;

	DIRECT_CHILD_INTERFACE_GET_DATA(IDirectFBARIBDisplayLayer);

	parent_data = (IDirectFBDisplayLayer_data *)data;

	return dfb_arib_layer_context_set_mpeg_resolution( parent_data->context,
	                                                   seqhead_resolution,
	                                                   seqdisp_resolution,
	                                                   seqhead_aspect_ratio,
	                                                   progressive );
}

static DFBResult
IDirectFBARIBDisplayLayer_SetDBcastResolution( IDirectFBARIBDisplayLayer *thiz,
                                               DFBDimension              *resolution,
                                               DFBBoolean                aspect_ratio )
{
	IDirectFBDisplayLayer_data *parent_data;

	DIRECT_CHILD_INTERFACE_GET_DATA(IDirectFBARIBDisplayLayer);

	parent_data = (IDirectFBDisplayLayer_data *)data;

	return dfb_arib_layer_context_set_dbcast_resolution( parent_data->context,
	                                                     resolution,
	                                                     aspect_ratio );
}

static DFBResult
IDirectFBARIBDisplayLayer_SetBmlVisibility( IDirectFBARIBDisplayLayer *thiz,
                                            DFBBoolean                bml_valid,
                                            DFBBoolean                bml_invisible,
                                            DFBBoolean                bml_has_video )
{
	IDirectFBDisplayLayer_data *parent_data;

	DIRECT_CHILD_INTERFACE_GET_DATA(IDirectFBARIBDisplayLayer);

	parent_data = (IDirectFBDisplayLayer_data *)data;

	return dfb_arib_layer_context_set_bml_visibility( parent_data->context,
	                                                  bml_valid,
	                                                  bml_invisible,
	                                                  bml_has_video );
}

static DFBResult
IDirectFBARIBDisplayLayer_SetResolutionListener( IDirectFBARIBDisplayLayer *thiz,
                                                 FusionCallHandler         callback,
                                                 void                      *ctx )
{
	IDirectFBDisplayLayer_data *parent_data;

	DIRECT_CHILD_INTERFACE_GET_DATA(IDirectFBARIBDisplayLayer);

	parent_data = (IDirectFBDisplayLayer_data *)data;

	return dfb_arib_layer_context_set_listener( parent_data->context,
	                                            CB_RESOLUTION_CHANGE,
	                                            callback,
	                                            ctx );
}

static DFBResult
IDirectFBARIBDisplayLayer_SetLocationListener( IDirectFBARIBDisplayLayer *thiz,
                                               FusionCallHandler         callback,
                                               void                      *ctx )
{
	IDirectFBDisplayLayer_data *parent_data;

	DIRECT_CHILD_INTERFACE_GET_DATA(IDirectFBARIBDisplayLayer);

	parent_data = (IDirectFBDisplayLayer_data *)data;

	return dfb_arib_layer_context_set_listener( parent_data->context,
	                                            CB_LOCATION_CHANGE,
	                                            callback,
	                                            ctx );
}

static DFBResult
IDirectFBARIBDisplayLayer_GetMonitorInfo( IDirectFBARIBDisplayLayer *thiz,
                                          DFBMonitorType            *monitor_type,
                                          DFBMonitorAspectRatio     *monitor_aspect )
{
	IDirectFBDisplayLayer_data *parent_data;

	DIRECT_CHILD_INTERFACE_GET_DATA(IDirectFBARIBDisplayLayer);

	parent_data = (IDirectFBDisplayLayer_data *)data;

	return dfb_arib_layer_context_get_monitor_info( parent_data->context,
	                                                monitor_type,
	                                                monitor_aspect );
}

static DFBResult
IDirectFBARIBDisplayLayer_SetResolutionTable( IDirectFBARIBDisplayLayer *thiz,
                                              DFB_RESOLUTION_TABLE *resolution_table )
{
	IDirectFBDisplayLayer_data *parent_data;

	DIRECT_CHILD_INTERFACE_GET_DATA(IDirectFBARIBDisplayLayer);

	parent_data = (IDirectFBDisplayLayer_data *)data;

	return dfb_arib_layer_context_set_resolution_table( parent_data->context,
	                                                    resolution_table );
}

DFBResult
IDirectFBARIBDisplayLayer_Construct( IDirectFBARIBDisplayLayer *thiz,
                                     CoreLayer                 *layer )
{
	DFBResult             ret;
	IDirectFBDisplayLayer *parent_thiz;

	DIRECT_ALLOCATE_CHILD_INTERFACE_DATA(thiz, IDirectFBARIBDisplayLayer);

	ret = IDirectFBDisplayLayer_Construct((IDirectFBDisplayLayer *)thiz, layer);
	if (ret)
		return ret;

	parent_thiz = (IDirectFBDisplayLayer *)thiz;

	parent_thiz->SetBackgroundMode     = IDirectFBDisplayLayer_SetBackgroundMode;
	parent_thiz->SetBackgroundColor    = IDirectFBDisplayLayer_SetBackgroundColor;
	parent_thiz->SetBackgroundImage    = IDirectFBDisplayLayer_SetBackgroundImage;
	parent_thiz->SetScreenLocation     = IDirectFBDisplayLayer_SetScreenLocation;
	parent_thiz->EnableCursor          = IDirectFBDisplayLayer_EnableCursor;
	parent_thiz->GetCursorPosition     = IDirectFBDisplayLayer_GetCursorPosition;
	parent_thiz->WarpCursor            = IDirectFBDisplayLayer_WarpCursor;
	parent_thiz->SetCursorAcceleration = IDirectFBDisplayLayer_SetCursorAcceleration;
	parent_thiz->SetCursorShape        = IDirectFBDisplayLayer_SetCursorShape;
	parent_thiz->SetCursorOpacity      = IDirectFBDisplayLayer_SetCursorOpacity;

	thiz->AddRef                 = IDirectFBARIBDisplayLayer_AddRef;
	thiz->Release                = IDirectFBARIBDisplayLayer_Release;
	thiz->CreateAribWindow       = IDirectFBARIBDisplayLayer_CreateAribWindow;
	thiz->GetAribWindow          = IDirectFBARIBDisplayLayer_GetAribWindow;
	thiz->SetMpegResolution      = IDirectFBARIBDisplayLayer_SetMpegResolution;
	thiz->SetDBcastResolution    = IDirectFBARIBDisplayLayer_SetDBcastResolution;
	thiz->SetBmlVisibility       = IDirectFBARIBDisplayLayer_SetBmlVisibility;
	thiz->SetResolutionListener  = IDirectFBARIBDisplayLayer_SetResolutionListener;
	thiz->SetLocationListener    = IDirectFBARIBDisplayLayer_SetLocationListener;
	thiz->GetMonitorInfo         = IDirectFBARIBDisplayLayer_GetMonitorInfo;
	thiz->SetResolutionTable     = IDirectFBARIBDisplayLayer_SetResolutionTable;

	return DFB_OK;
}

