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

#include "directfb_arib.h"

#include "core/core.h"
#include "core/coretypes.h"

#include "core/gfxcard.h"
#include "core/surfaces.h"
#include "core/fonts.h"
#include "core/state.h"
#include "core/layer_region_arib.h"
#include "core/windows_arib.h"
#include "core/windows_internal.h" /* FIXME */

#include "idirectfbsurface.h"
#include "idirectfbsurface_window_arib.h"

#include <direct/interface.h>
#include <direct/mem.h>
#include "misc/util.h"

#include "gfx/util.h"


static DFBResult
IDirectFBSurface_Window_Flip( IDirectFBSurface    *thiz,
                              const DFBRegion     *region,
                              DFBSurfaceFlipFlags  flags )
{
     DFBRegion reg;
     DIRECT_INTERFACE_GET_DATA(IDirectFBSurface_Window)

     if (region) {
          if ((region->x2 - region->x1 == 0)
          ||  (region->y2 - region->y1 == 0)) {
               return DFB_OK;
          }
     }

     if (!data->base.surface)
          return DFB_DESTROYED;

     if (data->base.locked)
          return DFB_LOCKED;

     if (!data->base.area.current.w || !data->base.area.current.h ||
         (region && (region->x1 > region->x2 || region->y1 > region->y2)))
          return DFB_INVAREA;


     dfb_region_from_rectangle( &reg, &data->base.area.current );

     if (region) {
          DFBRegion clip = DFB_REGION_INIT_TRANSLATED( region,
                                                       data->base.area.wanted.x,
                                                       data->base.area.wanted.y );

          if (!dfb_region_region_intersect( &reg, &clip ))
               return DFB_INVAREA;
     }


     if (flags & DSFLIP_PIPELINE) {
          dfb_gfxcard_wait_serial( &data->window->serial2 );

          data->window->serial2 = data->window->serial1;

          dfb_state_get_serial( &data->base.state, &data->window->serial1 );
     }

     if (data->window->region) {
          dfb_arib_layer_region_flip_update( data->window->region, &reg, flags );
     }
     else {
          if (data->base.surface->caps & DSCAPS_FLIPPING) {
               if (!(flags & DSFLIP_BLIT) && reg.x1 == 0 && reg.y1 == 0 &&
                   reg.x2 == data->window->config.bounds.w - 1 &&
                   reg.y2 == data->window->config.bounds.h - 1)
                    dfb_surface_flip_buffers( data->base.surface, false );
               else
                    dfb_back_to_front_copy( data->base.surface, &reg );
          }

          dfb_arib_window_repaint( data->window, &reg, flags );
     }

     if (!data->window->config.opacity && data->base.caps & DSCAPS_PRIMARY)
          dfb_arib_window_set_opacity( data->window, 0xff );

     return DFB_OK;
}

DFBResult
IDirectFBSurface_ARIBWindow_Construct( IDirectFBSurface       *thiz,
                                       DFBRectangle           *wanted,
                                       DFBRectangle           *granted,
                                       CoreWindow             *window,
                                       DFBSurfaceCapabilities  caps )
{
     DFBResult ret;

     ret = IDirectFBSurface_Window_Construct( thiz, wanted, granted,
                                              window, caps );
     if (ret)
          return ret;

     thiz->Flip = IDirectFBSurface_Window_Flip;

     return DFB_OK;
}

