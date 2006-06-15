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

#include <sched.h>

#include <sys/time.h>
#include <errno.h>

#include <pthread.h>

#include <directfb.h>

#include <idirectfb.h>

#include <core/core.h>
#include <core/coredefs.h>
#include <core/coretypes.h>

#include <core/layers.h>
#include <core/palette.h>
#include <core/state.h>
#include <core/surfaces.h>
#include <core/windows.h>
#include <core/windows_arib.h>

#include <core/windowstack.h>
#include <core/windows_internal.h> /* FIXME */
#include <core/layers_internal.h>  /* FIXME */

#include <display/idirectfbsurface.h>
#include <display/idirectfbsurface_window.h>
#include <display/idirectfbsurface_window_arib.h>

#include <input/idirectfbinputbuffer.h>

#include <misc/util.h>
#include <direct/interface.h>
#include <direct/interface_arib.h>

#include <direct/mem.h>

#include <gfx/convert.h>

#include <windows/idirectfbwindow.h>
#include <windows/idirectfbwindow_arib.h>

D_DEBUG_DOMAIN( IDirectFBARIB_Window, "IDirectFBARIBWindow", "DirectFBARIB Window Interface" );

static DFBResult
IDirectFBWindow_EnableEvents( IDirectFBWindow    *thiz,
                              DFBWindowEventType mask )
{
	DIRECT_INTERFACE_GET_DATA(IDirectFBWindow)

	if (data->destroyed)
		return DFB_DESTROYED;

	if (mask & ~DWET_ALL)
		return DFB_INVARG;

	return dfb_arib_window_change_events( data->window, DWET_NONE, mask );
}

static DFBResult
IDirectFBWindow_DisableEvents( IDirectFBWindow    *thiz,
                               DFBWindowEventType mask )
{
	DIRECT_INTERFACE_GET_DATA(IDirectFBWindow)

	if (data->destroyed)
		return DFB_DESTROYED;

	if (mask & ~DWET_ALL)
		return DFB_INVARG;

	return dfb_arib_window_change_events( data->window, mask, DWET_NONE );
}

static DFBResult
IDirectFBWindow_GetID( IDirectFBWindow *thiz,
                       DFBWindowID     *id )
{
	DIRECT_INTERFACE_GET_DATA(IDirectFBWindow)

	if (data->destroyed)
		return DFB_DESTROYED;

	if (!id)
		return DFB_INVARG;

	*id = data->window->arib_id;

	return DFB_OK;
}

static DFBResult
IDirectFBWindow_GetSurface( IDirectFBWindow  *thiz,
                            IDirectFBSurface **surface )
{
	DIRECT_INTERFACE_GET_DATA(IDirectFBWindow)

	if (data->destroyed)
		return DFB_DESTROYED;

	if (!surface)
		return DFB_INVARG;

	if (data->window->caps & DWCAPS_INPUTONLY)
		return DFB_UNSUPPORTED;

	if (!data->surface) {
		DFBResult ret;

		DIRECT_ALLOCATE_INTERFACE( *surface, IDirectFBSurface );

		ret = IDirectFBSurface_ARIBWindow_Construct( *surface,
		                                       NULL, NULL, data->window,
		                                       DSCAPS_VIDEOONLY | DSCAPS_FLIPPING);
		if (ret)
			return ret;

		data->surface = *surface;
	}
	else
		*surface = data->surface;

	data->surface->AddRef( data->surface );

	return DFB_OK;
}

static DFBResult
IDirectFBWindow_SetOptions( IDirectFBWindow  *thiz,
                            DFBWindowOptions options )
{
	DIRECT_INTERFACE_GET_DATA(IDirectFBWindow)

	/* Check arguments */
	if (data->destroyed)
		return DFB_DESTROYED;

	if (options & ~DWOP_ALL)
		return DFB_INVARG;

	if (!(data->window->caps & DWCAPS_ALPHACHANNEL))
		options &= ~DWOP_ALPHACHANNEL;

	/* Set new options */
	return dfb_arib_window_change_options( data->window, DWET_ALL, options );
}

static DFBResult
IDirectFBWindow_SetCursorShape( IDirectFBWindow  *thiz,
                                IDirectFBSurface *shape,
                                int              hot_x,
                                int              hot_y )
{
	return DFB_UNSUPPORTED;
}

static DFBResult
IDirectFBWindow_Move( IDirectFBWindow *thiz,
                      int             dx,
                      int             dy )
{
	return DFB_UNSUPPORTED;
}

static DFBResult
IDirectFBWindow_MoveTo( IDirectFBWindow *thiz,
                        int             x,
                        int             y )
{
	return DFB_UNSUPPORTED;
}

static DFBResult
IDirectFBWindow_Resize( IDirectFBWindow *thiz,
                        int             width,
                        int             height )
{
	return DFB_UNSUPPORTED;
}

static DFBResult
IDirectFBWindow_Raise( IDirectFBWindow *thiz )
{
	return DFB_UNSUPPORTED;
}

static DFBResult
IDirectFBWindow_SetStackingClass( IDirectFBWindow        *thiz,
                                  DFBWindowStackingClass stacking_class )
{
	return DFB_UNSUPPORTED;
}

static DFBResult
IDirectFBWindow_Lower( IDirectFBWindow *thiz )
{
	return DFB_UNSUPPORTED;
}

static DFBResult
IDirectFBWindow_RaiseToTop( IDirectFBWindow *thiz )
{
	return DFB_UNSUPPORTED;
}

static DFBResult
IDirectFBWindow_LowerToBottom( IDirectFBWindow *thiz )
{
	return DFB_UNSUPPORTED;
}

static DFBResult
IDirectFBWindow_PutAtop( IDirectFBWindow *thiz,
                         IDirectFBWindow *lower )
{
	return DFB_UNSUPPORTED;
}

static DFBResult
IDirectFBWindow_PutBelow( IDirectFBWindow *thiz,
                          IDirectFBWindow *upper )
{
	return DFB_UNSUPPORTED;
}

static DFBResult
IDirectFBWindow_SetColorKey( IDirectFBWindow *thiz,
                             __u8            r,
                             __u8            g,
                             __u8            b )
{
	__u32       key;
	CoreSurface *surface;

	DIRECT_INTERFACE_GET_DATA(IDirectFBWindow)

	if (data->destroyed)
		return DFB_DESTROYED;

	if (data->window->caps & DWCAPS_INPUTONLY)
		return DFB_UNSUPPORTED;

	surface = data->window->surface;

	if (DFB_PIXELFORMAT_IS_INDEXED( surface->format ))
		key = dfb_palette_search( surface->palette, r, g, b, 0x80 );
	else
		key = dfb_color_to_pixel( surface->format, r, g, b );

	return dfb_arib_window_set_colorkey( data->window, key );
}

static DFBResult
IDirectFBWindow_SetColorKeyIndex( IDirectFBWindow *thiz,
                                  unsigned int    index )
{
	__u32 key = index;

	DIRECT_INTERFACE_GET_DATA(IDirectFBWindow)

	if (data->destroyed)
		return DFB_DESTROYED;

	if (data->window->caps & DWCAPS_INPUTONLY)
		return DFB_UNSUPPORTED;

	return dfb_arib_window_set_colorkey( data->window, key );
}

static DFBResult
IDirectFBWindow_SetOpaqueRegion( IDirectFBWindow *thiz,
                                 int             x1,
                                 int             y1,
                                 int             x2,
                                 int             y2 )
{
	DFBRegion region;

	DIRECT_INTERFACE_GET_DATA(IDirectFBWindow)

	if (data->destroyed)
		return DFB_DESTROYED;

	if (x1 > x2 || y1 > y2)
		return DFB_INVAREA;

	region = (DFBRegion) { x1, y1, x2, y2 };

	return dfb_arib_window_set_opaque( data->window, &region );
}

static DFBResult
IDirectFBWindow_SetOpacity( IDirectFBWindow *thiz,
                            __u8            opacity )
{
	DIRECT_INTERFACE_GET_DATA(IDirectFBWindow)

	if (data->destroyed)
		return DFB_DESTROYED;

	dfb_arib_window_set_opacity( data->window, opacity );

	return DFB_OK;
}

static void
IDirectFBARIBWindow_Destruct( IDirectFBARIBWindow *thiz )
{
	IDirectFBWindow_data *parent_data = (IDirectFBWindow_data*)thiz->parent.priv;;

	D_DEBUG_AT( IDirectFBARIB_Window, "IDirectFBARIBWindow_Destruct()\n" );

	if (!parent_data->detached) {
		D_DEBUG_AT( IDirectFBARIB_Window, "  -> detaching...\n" );

		dfb_window_detach( parent_data->window, &parent_data->reaction );

		D_DEBUG("IDirectFBARIBWindow_Destruct - detached.\n");
	}

	if (!parent_data->destroyed) {
		D_DEBUG("IDirectFBARIBWindow_Destruct - unrefing...\n");

		dfb_window_unref( parent_data->window );
	}
	D_DEBUG_AT( IDirectFBARIB_Window, "  -> releasing surface...\n" );

	if (parent_data->surface)
		parent_data->surface->Release( parent_data->surface );

	D_DEBUG_AT( IDirectFBARIB_Window, "  -> releasing cursor shape...\n" );

	if (parent_data->cursor.shape)
		parent_data->cursor.shape->Release( parent_data->cursor.shape );

	D_DEBUG_AT( IDirectFBARIB_Window, "  -> done.\n" );

	DIRECT_DEALLOCATE_CHILD_INTERFACE( thiz );
}

static DFBResult
IDirectFBARIBWindow_AddRef( IDirectFBARIBWindow *thiz )
{
	IDirectFBWindow_data *parent_data;

	DIRECT_CHILD_INTERFACE_GET_DATA(IDirectFBARIBWindow);

	parent_data = (IDirectFBWindow_data *)data;

	parent_data->ref++;

	return DFB_OK;
}

static DFBResult
IDirectFBARIBWindow_Release( IDirectFBARIBWindow *thiz )
{
	IDirectFBWindow_data *parent_data;

	DIRECT_CHILD_INTERFACE_GET_DATA(IDirectFBARIBWindow);

	parent_data = (IDirectFBWindow_data *)data;

	if (--parent_data->ref == 0)
		IDirectFBARIBWindow_Destruct( thiz );

	return DFB_OK;
}

static DFBResult
IDirectFBARIBWindow_SetSwitchingRegion( IDirectFBARIBWindow *thiz,
                                        DFBRectangle        *rect1,
                                        DFBRectangle        *rect2,
                                        DFBRectangle        *rect3,
                                        DFBRectangle        *rect4,
                                        DFBBoolean          attribute )
{
	IDirectFBWindow_data *parent_data;


	DIRECT_CHILD_INTERFACE_GET_DATA(IDirectFBARIBWindow);

	parent_data = (IDirectFBWindow_data *)data;

	dfb_arib_window_set_switching_region( parent_data->window, rect1, rect2, rect3, rect4, attribute );

	return DFB_OK;
}

static DFBResult
IDirectFBARIBWindow_BatchStart( IDirectFBARIBWindow *thiz )
{
	IDirectFBWindow_data *parent_data;


	DIRECT_CHILD_INTERFACE_GET_DATA(IDirectFBARIBWindow);

	parent_data = (IDirectFBWindow_data *)data;

	return dfb_arib_window_batch_start( parent_data->window );
}

static DFBResult
IDirectFBARIBWindow_BatchEnd( IDirectFBARIBWindow *thiz )
{
	IDirectFBWindow_data *parent_data;


	DIRECT_CHILD_INTERFACE_GET_DATA(IDirectFBARIBWindow);

	parent_data = (IDirectFBWindow_data *)data;

	return dfb_arib_window_batch_end( parent_data->window );
}

DFBResult
IDirectFBARIBWindow_Construct( IDirectFBARIBWindow *thiz,
                               CoreWindow          *window,
                               CoreLayer           *layer )
{
	IDirectFBWindow_data *parent_data;
	IDirectFBWindow      *parent_thiz = (IDirectFBWindow*)thiz;

	DIRECT_ALLOCATE_CHILD_INTERFACE_DATA(thiz, IDirectFBARIBWindow);

	D_DEBUG_AT( IDirectFBARIB_Window, "IDirectFBARIBWindow_Construct() <- %d, %d - %dx%d\n",
	         DFB_RECTANGLE_VALS( &window->config.bounds ) );

	IDirectFBWindow_Construct( (IDirectFBWindow *)thiz, window, layer );

	parent_data = (IDirectFBWindow_data *)data;

	parent_thiz->SetOptions       = IDirectFBWindow_SetOptions;
	parent_thiz->SetCursorShape   = IDirectFBWindow_SetCursorShape;
	parent_thiz->Move             = IDirectFBWindow_Move;
	parent_thiz->MoveTo           = IDirectFBWindow_MoveTo;
	parent_thiz->Resize           = IDirectFBWindow_Resize;
	parent_thiz->Raise            = IDirectFBWindow_Raise;
	parent_thiz->SetStackingClass = IDirectFBWindow_SetStackingClass;
	parent_thiz->Lower            = IDirectFBWindow_Lower;
	parent_thiz->RaiseToTop       = IDirectFBWindow_RaiseToTop;
	parent_thiz->LowerToBottom    = IDirectFBWindow_LowerToBottom;
	parent_thiz->PutAtop          = IDirectFBWindow_PutAtop;
	parent_thiz->PutBelow         = IDirectFBWindow_PutBelow;
	parent_thiz->SetColorKey      = IDirectFBWindow_SetColorKey;
	parent_thiz->SetColorKeyIndex = IDirectFBWindow_SetColorKeyIndex;
	parent_thiz->SetOpaqueRegion  = IDirectFBWindow_SetOpaqueRegion;
	parent_thiz->EnableEvents     = IDirectFBWindow_EnableEvents;
	parent_thiz->DisableEvents    = IDirectFBWindow_DisableEvents;
	parent_thiz->GetID            = IDirectFBWindow_GetID;
	parent_thiz->GetSurface       = IDirectFBWindow_GetSurface;
	parent_thiz->SetOpacity       = IDirectFBWindow_SetOpacity;

	thiz->AddRef                  = IDirectFBARIBWindow_AddRef;
	thiz->Release                 = IDirectFBARIBWindow_Release;
	thiz->SetSwitchingRegion      = IDirectFBARIBWindow_SetSwitchingRegion;
	thiz->BatchStart              = IDirectFBARIBWindow_BatchStart;
	thiz->BatchEnd                = IDirectFBARIBWindow_BatchEnd;


	return DFB_OK;
}

