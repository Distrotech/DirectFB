/*
   (c) Copyright 2000-2002  convergence integrated media GmbH.
   (c) Copyright 2002-2004  convergence GmbH.

   All rights reserved.

   Written by Denis Oliver Kropp <dok@directfb.org>,
              Andreas Hundt <andi@fischlustig.de>,
              Sven Neumann <neo@directfb.org> and
              Ville Syrjälä <syrjala@sci.fi>.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the
   Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <string.h>

#include <directfb.h>

#include <core/core.h>
#include <core/coredefs.h>
#include <core/coretypes.h>

#include <core/surfaces.h>
#include <core/gfxcard.h>
#include <core/layers.h>
#include <core/layer_context.h>
#include <core/layer_control.h>
#include <core/layer_region.h>
#include <core/layers_internal.h>
#include <misc/conf.h>
#include <core/state.h>
#include <gfx/convert.h>
#include <gfx/util.h>

#include <direct/debug.h>
#include <direct/interface.h>
#include <direct/mem.h>
#include <direct/messages.h>

#include "idirectfbdisplaylayer.h"
#include "idirectfbscreen.h"
#include "idirectfbsurface.h"



D_DEBUG_DOMAIN( Layer, "IDirectFBDisplayLayer", "Display Layer Interface" );


#define CURSORFILE  DATADIR"/cursor.dat"

/*
 * private data struct of IDirectFB
 */
typedef struct {
     int                              ref;     /* reference counter */
     DFBDisplayLayerDescription       desc;    /* description of the layer's caps */
     DFBDisplayLayerCooperativeLevel  level;   /* current cooperative level */
     CoreScreen                      *screen;  /* layer's screen */
     CoreLayer                       *layer;   /* core layer data */
     CoreLayerContext                *context; /* shared or exclusive context */
     CoreLayerRegion                 *region;  /* primary region of the context */
     struct {
          int                enabled;     /* is cursor enabled ? */
          int                x, y;        /* cursor position */
          DFBDimension       size;        /* cursor shape size */
          DFBPoint           hot;         /* hot spot */
          CoreSurface       *surface;     /* shape */
          __u8               opacity;     /* cursor opacity */
          DFBRegion          region;      /* cursor is clipped by this region */
          int                numerator;   /* cursor acceleration */
          int                denominator;
          int                threshold;

          bool               set;         /* cursor enable/disable has
                                             been called at least one time */

          CoreSurfacePolicy  policy;
     } cursor;

} IDirectFBDisplayLayer_data;


static DFBResult
create_cursor_surface( IDirectFBDisplayLayer_data *data,
                       int              width,
                       int              height )
{
     DFBResult          ret;
     CoreSurface       *surface;
     CoreLayer         *layer;
     CoreLayerContext  *context;

     D_DEBUG_AT( Layer, "%s( %p, %dx%d )\n", __FUNCTION__, data, width, height );

     D_ASSERT( data != NULL );
     D_ASSERT( data->cursor.surface == NULL );

     context = data->context;

     D_ASSERT( context != NULL );

     layer = dfb_layer_at( context->layer_id );

     D_ASSERT( layer != NULL );

     data->cursor.x   = data->context->config.width  / 2;
     data->cursor.y   = data->context->config.height / 2;
     data->cursor.hot.x   = 0;
     data->cursor.hot.y   = 0;
     data->cursor.size.w  = width;
     data->cursor.size.h  = height;
     data->cursor.opacity = 0xFF;

     if (context->config.buffermode == DLBM_WINDOWS)
          D_WARN( "cursor not yet visible with DLBM_WINDOWS" );

     /* Create the cursor surface. */
     ret = dfb_surface_create( layer->core, width, height, DSPF_ARGB,
                               data->cursor.policy, DSCAPS_NONE, NULL, &surface );
     if (ret) {
          D_ERROR( "Core/Layer: Failed creating a surface for software cursor!\n" );
          return ret;
     }

     dfb_surface_globalize( surface );

     data->cursor.surface = surface;

     return DFB_OK;
}
/*
 * internal function that installs the cursor window
 * and fills it with data from 'cursor.dat'
 */
static DFBResult
load_default_cursor( IDirectFBDisplayLayer_data *data )
{
     DFBResult ret;
     int       i;
     int       pitch;
     void     *cdata;
     FILE     *f;

     D_DEBUG_AT( Layer, "%s( %p )\n", __FUNCTION__, data );

     D_ASSERT( data != NULL );

     if (!data->cursor.surface) {
          ret = create_cursor_surface( data, 40, 40 );
          if (ret)
               return ret;
     }
     else {
          data->cursor.hot.x  = 0;
          data->cursor.hot.y  = 0;
          data->cursor.size.w = 40;
          data->cursor.size.h = 40;
     }

     /* lock the surface of the window */
     ret = dfb_surface_soft_lock( data->cursor.surface, DSLF_WRITE, &cdata, &pitch, false );
     if (ret) {
          D_ERROR( "Core/WindowStack: cannot lock the surface for cursor window data!\n" );
          return ret;
     }

     /* initialize as empty cursor */
     memset( cdata, 0, 40 * pitch );

     /* open the file containing the cursors image data */
     f = fopen( CURSORFILE, "rb" );
     if (!f) {
          ret = errno2result( errno );

          /* ignore a missing cursor file */
          if (ret == DFB_FILENOTFOUND)
               ret = DFB_OK;
          else
               D_PERROR( "Core/Layer: `" CURSORFILE "` could not be opened!\n" );

          goto finish;
     }

     /* read from file directly into the cursor window surface */
     for (i=0; i<40; i++) {
          if (fread( cdata, MIN (40*4, pitch), 1, f ) != 1) {
               ret = errno2result( errno );

               D_ERROR( "Core/WindowStack: unexpected end or read error of cursor data!\n" );

               goto finish;
          }
#ifdef WORDS_BIGENDIAN
          {
               int i = MIN (40, pitch/4);
               __u32 *tmp_data = data;

               while (i--) {
                    *tmp_data = (*tmp_data & 0xFF000000) >> 24 |
                                (*tmp_data & 0x00FF0000) >>  8 |
                                (*tmp_data & 0x0000FF00) <<  8 |
                                (*tmp_data & 0x000000FF) << 24;
                    ++tmp_data;
               }
          }
#endif
          cdata += pitch;
     }

finish:
     if (f)
          fclose( f );

     dfb_surface_unlock( data->cursor.surface, false );

     return ret;
}

static void
IDirectFBDisplayLayer_Destruct( IDirectFBDisplayLayer *thiz )
{
     IDirectFBDisplayLayer_data *data = (IDirectFBDisplayLayer_data*)thiz->priv;

     D_DEBUG_AT( Layer, "IDirectFBDisplayLayer_Destruct()\n" );

     D_DEBUG_AT( Layer, "  -> unref region...\n" );

     dfb_layer_region_unref( data->region );

     D_DEBUG_AT( Layer, "  -> unref context...\n" );

     dfb_layer_context_unref( data->context );

     DIRECT_DEALLOCATE_INTERFACE( thiz );

     D_DEBUG_AT( Layer, "  -> done.\n" );
}

static DFBResult
IDirectFBDisplayLayer_AddRef( IDirectFBDisplayLayer *thiz )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBDisplayLayer)

     data->ref++;

     return DFB_OK;
}

static DFBResult
IDirectFBDisplayLayer_Release( IDirectFBDisplayLayer *thiz )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBDisplayLayer)

     if (--data->ref == 0)
          IDirectFBDisplayLayer_Destruct( thiz );

     return DFB_OK;
}

static DFBResult
IDirectFBDisplayLayer_GetID( IDirectFBDisplayLayer *thiz,
                             DFBDisplayLayerID     *id )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBDisplayLayer)

     if (!id)
          return DFB_INVARG;

     *id = dfb_layer_id_translated( data->layer );

     return DFB_OK;
}

static DFBResult
IDirectFBDisplayLayer_GetDescription( IDirectFBDisplayLayer      *thiz,
                                      DFBDisplayLayerDescription *desc )
{

     DIRECT_INTERFACE_GET_DATA(IDirectFBDisplayLayer)

     if (!desc)
          return DFB_INVARG;

     *desc = data->desc;

     return DFB_OK;
}

static DFBResult
IDirectFBDisplayLayer_GetSurface( IDirectFBDisplayLayer  *thiz,
                                  IDirectFBSurface      **interface )
{
     DFBResult         ret;
     CoreLayerRegion  *region;
     IDirectFBSurface *surface;

     DIRECT_INTERFACE_GET_DATA(IDirectFBDisplayLayer)

     if (!interface)
          return DFB_INVARG;

     *interface = NULL;

     if (data->level == DLSCL_SHARED) {
          D_WARN( "letting unprivileged IDirectFBDisplayLayer::GetSurface() "
                   "call pass until cooperative level handling is finished" );
     }

     ret = dfb_layer_context_get_primary_region( data->context, true, &region );
     if (ret)
          return ret;

     DIRECT_ALLOCATE_INTERFACE( surface, IDirectFBSurface );
     {
          DFBResult    ret;
          CoreSurface *core_surface;
          DFBRectangle rect = { 0,0,data->context->config.width,
                  data->context->config.height };
          DIRECT_ALLOCATE_INTERFACE_DATA(surface,IDirectFBSurface);

          if (dfb_layer_region_ref( region ))
               return DFB_FUSION;

          ret = dfb_layer_region_get_surface( region, &core_surface );

          if (ret) {
               dfb_layer_region_unref( region );
               DIRECT_DEALLOCATE_INTERFACE(surface);
               return ret;
          }

          ret = IDirectFBSurface_Construct( surface,&rect,NULL,NULL,
                                       core_surface, DSCAPS_NONE );

          dfb_surface_unref( core_surface );
          dfb_layer_region_unref( region );

     }

     *interface = ret ? NULL : surface;


     return ret;
}

static DFBResult
IDirectFBDisplayLayer_GetScreen( IDirectFBDisplayLayer  *thiz,
                                 IDirectFBScreen       **interface )
{
     DFBResult        ret;
     IDirectFBScreen *screen;

     DIRECT_INTERFACE_GET_DATA(IDirectFBDisplayLayer)

     if (!interface)
          return DFB_INVARG;

     DIRECT_ALLOCATE_INTERFACE( screen, IDirectFBScreen );

     ret = IDirectFBScreen_Construct( screen, data->screen );

     *interface = ret ? NULL : screen;

     return ret;
}

static DFBResult
IDirectFBDisplayLayer_SetCooperativeLevel( IDirectFBDisplayLayer           *thiz,
                                           DFBDisplayLayerCooperativeLevel  level )
{
     DFBResult         ret;
     CoreLayerContext *context;
     CoreLayerRegion  *region;

     DIRECT_INTERFACE_GET_DATA(IDirectFBDisplayLayer)

     if (data->level == level)
          return DFB_OK;

     switch (level) {
          case DLSCL_SHARED:
          case DLSCL_ADMINISTRATIVE:
               if (data->level == DLSCL_EXCLUSIVE) {
                    ret = dfb_layer_get_primary_context( data->layer, false, &context );
                    if (ret)
                         return ret;

                    ret = dfb_layer_context_get_primary_region( context, true, &region );
                    if (ret) {
                         dfb_layer_context_unref( context );
                         return ret;
                    }

                    dfb_layer_region_unref( data->region );
                    dfb_layer_context_unref( data->context );

                    data->context = context;
                    data->region  = region;
                }

               break;

          case DLSCL_EXCLUSIVE:
               ret = dfb_layer_create_context( data->layer, &context );
               if (ret)
                    return ret;

               ret = dfb_layer_activate_context( data->layer, context );
               if (ret) {
                    dfb_layer_context_unref( context );
                    return ret;
               }

               ret = dfb_layer_context_get_primary_region( context, true, &region );
               if (ret) {
                    dfb_layer_context_unref( context );
                    return ret;
               }

               dfb_layer_region_unref( data->region );
               dfb_layer_context_unref( data->context );

               data->context = context;
               data->region  = region;
               break;

          default:
               return DFB_INVARG;
     }

     data->level = level;

     return DFB_OK;
}

static DFBResult
IDirectFBDisplayLayer_SetOpacity( IDirectFBDisplayLayer *thiz,
                                  __u8                   opacity )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBDisplayLayer)

     if (data->level == DLSCL_SHARED)
          return DFB_ACCESSDENIED;

     return dfb_layer_context_set_opacity( data->context, opacity );
}

static DFBResult
IDirectFBDisplayLayer_GetCurrentOutputField( IDirectFBDisplayLayer *thiz, int *field )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBDisplayLayer)

     return dfb_layer_get_current_output_field( data->layer, field );
}

static DFBResult
IDirectFBDisplayLayer_SetFieldParity( IDirectFBDisplayLayer *thiz, int field )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBDisplayLayer)

     if (data->level != DLSCL_EXCLUSIVE)
          return DFB_ACCESSDENIED;

     return dfb_layer_context_set_field_parity( data->context, field );
}

static DFBResult
IDirectFBDisplayLayer_SetClipRegions( IDirectFBDisplayLayer *thiz,
                                      const DFBRegion       *regions,
                                      int                    num_regions,
                                      DFBBoolean             positive )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBDisplayLayer)

     if (!regions || num_regions < 1)
          return DFB_INVARG;

     if (num_regions > data->desc.clip_regions)
          return DFB_UNSUPPORTED;

     if (data->level != DLSCL_EXCLUSIVE)
          return DFB_ACCESSDENIED;

     return dfb_layer_context_set_clip_regions( data->context, regions, num_regions, positive );
}

static DFBResult
IDirectFBDisplayLayer_SetSourceRectangle( IDirectFBDisplayLayer *thiz,
                                          int                    x,
                                          int                    y,
                                          int                    width,
                                          int                    height )
{
     DFBRectangle source = { x, y, width, height };

     DIRECT_INTERFACE_GET_DATA(IDirectFBDisplayLayer)

     if (x < 0 || y < 0 || width <= 0 || height <= 0)
          return DFB_INVARG;

     if (data->level == DLSCL_SHARED)
          return DFB_ACCESSDENIED;

     return dfb_layer_context_set_sourcerectangle( data->context, &source );
}

static DFBResult
IDirectFBDisplayLayer_SetScreenLocation( IDirectFBDisplayLayer *thiz,
                                         float                  x,
                                         float                  y,
                                         float                  width,
                                         float                  height )
{
     DFBLocation location = { x, y, width, height };

     DIRECT_INTERFACE_GET_DATA(IDirectFBDisplayLayer)

     if (! D_FLAGS_IS_SET( data->desc.caps, DLCAPS_SCREEN_LOCATION ))
          return DFB_UNSUPPORTED;

     if (width <= 0 || height <= 0)
          return DFB_INVARG;

     if (data->level == DLSCL_SHARED)
          return DFB_ACCESSDENIED;

     return dfb_layer_context_set_screenlocation( data->context, &location );
}

static DFBResult
IDirectFBDisplayLayer_SetScreenPosition( IDirectFBDisplayLayer *thiz,
                                         int                    x,
                                         int                    y )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBDisplayLayer)

     if (! D_FLAGS_IS_SET( data->desc.caps, DLCAPS_SCREEN_POSITION ))
          return DFB_UNSUPPORTED;

     if (data->level == DLSCL_SHARED)
          return DFB_ACCESSDENIED;

     return dfb_layer_context_set_screenposition( data->context, x, y );
}

static DFBResult
IDirectFBDisplayLayer_SetScreenRectangle( IDirectFBDisplayLayer *thiz,
                                          int                    x,
                                          int                    y,
                                          int                    width,
                                          int                    height )
{
     DFBRectangle rect = { x, y, width, height };

     DIRECT_INTERFACE_GET_DATA(IDirectFBDisplayLayer)

     if (! D_FLAGS_IS_SET( data->desc.caps, DLCAPS_SCREEN_LOCATION ))
          return DFB_UNSUPPORTED;

     if (width <= 0 || height <= 0)
          return DFB_INVARG;

     if (data->level == DLSCL_SHARED)
          return DFB_ACCESSDENIED;

     return dfb_layer_context_set_screenrectangle( data->context, &rect );
}

static DFBResult
IDirectFBDisplayLayer_SetSrcColorKey( IDirectFBDisplayLayer *thiz,
                                      __u8                   r,
                                      __u8                   g,
                                      __u8                   b )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBDisplayLayer)

     if (data->level == DLSCL_SHARED)
          return DFB_ACCESSDENIED;

     return dfb_layer_context_set_src_colorkey( data->context, r, g, b );
}

static DFBResult
IDirectFBDisplayLayer_SetDstColorKey( IDirectFBDisplayLayer *thiz,
                                      __u8                   r,
                                      __u8                   g,
                                      __u8                   b )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBDisplayLayer)

     if (data->level == DLSCL_SHARED)
          return DFB_ACCESSDENIED;

     return dfb_layer_context_set_dst_colorkey( data->context, r, g, b );
}

static DFBResult
IDirectFBDisplayLayer_GetLevel( IDirectFBDisplayLayer *thiz,
                                int                   *level )
{
     DFBResult ret;
     int       lvl;

     DIRECT_INTERFACE_GET_DATA(IDirectFBDisplayLayer)

     if (!level)
          return DFB_INVARG;

     ret = dfb_layer_get_level( data->layer, &lvl );
     if (ret)
          return ret;

     *level = lvl;

     return DFB_OK;
}

static DFBResult
IDirectFBDisplayLayer_SetLevel( IDirectFBDisplayLayer *thiz,
                                int                    level )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBDisplayLayer)

     if (! D_FLAGS_IS_SET( data->desc.caps, DLCAPS_LEVELS ))
          return DFB_UNSUPPORTED;

     if (data->level == DLSCL_SHARED)
          return DFB_ACCESSDENIED;

     return dfb_layer_set_level( data->layer, level );
}

static DFBResult
IDirectFBDisplayLayer_GetConfiguration( IDirectFBDisplayLayer *thiz,
                                        DFBDisplayLayerConfig *config )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBDisplayLayer)

     if (!config)
          return DFB_INVARG;

     return dfb_layer_context_get_configuration( data->context, config );
}

static DFBResult
IDirectFBDisplayLayer_TestConfiguration( IDirectFBDisplayLayer       *thiz,
                                         const DFBDisplayLayerConfig *config,
                                         DFBDisplayLayerConfigFlags  *failed )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBDisplayLayer)

     if (!config)
          return DFB_INVARG;

     if (((config->flags & DLCONF_WIDTH) && (config->width < 0)) ||
         ((config->flags & DLCONF_HEIGHT) && (config->height < 0)))
          return DFB_INVARG;

     return dfb_layer_context_test_configuration( data->context, config, failed );
}

static DFBResult
IDirectFBDisplayLayer_SetConfiguration( IDirectFBDisplayLayer       *thiz,
                                        const DFBDisplayLayerConfig *config )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBDisplayLayer)

     if (!config)
          return DFB_INVARG;

     if (((config->flags & DLCONF_WIDTH) && (config->width < 0)) ||
         ((config->flags & DLCONF_HEIGHT) && (config->height < 0)))
          return DFB_INVARG;

     switch (data->level) {
          case DLSCL_EXCLUSIVE:
          case DLSCL_ADMINISTRATIVE:
               return dfb_layer_context_set_configuration( data->context, config );

          default:
               break;
     }

     return DFB_ACCESSDENIED;
}


static DFBResult
IDirectFBDisplayLayer_EnableCursor( IDirectFBDisplayLayer *thiz, int enable )
{
     DFBResult ret;

     DIRECT_INTERFACE_GET_DATA(IDirectFBDisplayLayer)

     if (data->level == DLSCL_SHARED)
          return DFB_ACCESSDENIED;

     if (dfb_layer_region_lock( data->region ))
          return DFB_FUSION;

     data->cursor.set = true;

     if (dfb_config->no_cursor || data->cursor.enabled == enable) {
          dfb_layer_region_unlock( data->region );
          return DFB_OK;
     }

     if (enable && !data->cursor.surface) {
          ret = load_default_cursor( data );
          if (ret) {
               dfb_layer_region_unlock( data->region );
               return ret;
          }
     }

     /* Keep state. */
     data->cursor.enabled = enable;

     /* Notify WM. */
     //dfb_wm_update_cursor( data, enable ? CCUF_ENABLE : CCUF_DISABLE );

     /* Unlock the layer data. */
     dfb_layer_region_unlock( data->region );
     return DFB_OK;
}

static DFBResult
IDirectFBDisplayLayer_GetCursorPosition( IDirectFBDisplayLayer *thiz,
                                         int *ret_x, int *ret_y )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBDisplayLayer)

     if (!ret_x && !ret_y)
          return DFB_INVARG;

     D_ASSERT( data != NULL );
     D_ASSUME( ret_x != NULL || ret_y != NULL );

     if (dfb_layer_region_lock( data->region ))
          return DFB_FUSION;

     if (ret_x)
          *ret_x = data->cursor.x;

     if (ret_y)
          *ret_y = data->cursor.y;

     /* Unlock the region. */
     dfb_layer_region_unlock( data->region );

     return DFB_OK;
}

static DFBResult
IDirectFBDisplayLayer_WarpCursor( IDirectFBDisplayLayer *thiz, int x, int y )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBDisplayLayer)

     if (data->level == DLSCL_SHARED)
          return DFB_ACCESSDENIED;

     D_ASSERT( data != NULL );

     /* Lock the region. */
     if (dfb_layer_region_lock( data->region ))
          return DFB_FUSION;

     if (x < 0)
          x = 0;
     else if (x > data->context->config.width - 1)
          x = data->context->config.width - 1;

     if (y < 0)
          y = 0;
     else if (y > data->context->config.height - 1)
          y = data->context->config.height - 1;

     if (data->cursor.x != x || data->cursor.y != y) {
          data->cursor.x = x;
          data->cursor.y = y;

          /* Notify the WM. */
          //if (data->cursor.enabled)
               //dfb_wm_update_cursor( data, CCUF_POSITION );
     }

     /* Unlock the region. */
     dfb_layer_region_unlock( data->region );

     return DFB_OK;
}

static DFBResult
IDirectFBDisplayLayer_SetCursorAcceleration( IDirectFBDisplayLayer *thiz,
                                             int                    numerator,
                                             int                    denominator,
                                             int                    threshold )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBDisplayLayer)

     if (numerator < 0  ||  denominator < 1  ||  threshold < 0)
          return DFB_INVARG;

     if (data->level == DLSCL_SHARED)
          return DFB_ACCESSDENIED;

     D_ASSERT( data != NULL );

     if (dfb_layer_region_lock( data->region ))
          return DFB_FUSION;

     data->cursor.numerator   = numerator;
     data->cursor.denominator = denominator;
     data->cursor.threshold   = threshold;

     /* Unlock the region. */
     dfb_layer_region_unlock( data->region );

     return DFB_OK;
}

static DFBResult
IDirectFBDisplayLayer_SetCursorShape( IDirectFBDisplayLayer *thiz,
                                      IDirectFBSurface      *shape,
                                      int                    hot_x,
                                      int                    hot_y )
{
     DFBResult              ret;
     CoreSurface           *cursor;
     CoreCursorUpdateFlags  flags = CCUF_SHAPE;
     IDirectFBSurface_data *shape_data;

     DIRECT_INTERFACE_GET_DATA(IDirectFBDisplayLayer)

     if (!shape)
          return DFB_INVARG;

     if (data->level == DLSCL_SHARED)
          return DFB_ACCESSDENIED;

     shape_data = (IDirectFBSurface_data*)shape->priv;

     if (hot_x < 0  ||
         hot_y < 0  ||
         hot_x >= shape_data->surface->width  ||
         hot_y >= shape_data->surface->height)
          return DFB_INVARG;

     D_DEBUG_AT( Layer, "%s( %p, %p, hot %d, %d ) <- size %dx%d\n",
                 __FUNCTION__, data, shape, hot_x, hot_y, shape_data->surface->width, shape_data->surface->height );

     D_ASSERT( data != NULL );
     D_ASSERT( shape != NULL );

     if (dfb_config->no_cursor)
          return DFB_OK;

     if (dfb_layer_region_lock( data->region ))
          return DFB_FUSION;

     cursor = data->cursor.surface;
     if (!cursor) {
          D_ASSUME( !data->cursor.enabled );

          /* Create the a surface for the shape. */
          ret = create_cursor_surface( data, shape_data->surface->width, shape_data->surface->height );
          if (ret) {
               dfb_layer_region_unlock( data->region );
               return ret;
          }

          cursor = data->cursor.surface;
     }
     else if (data->cursor.size.w != shape_data->surface->width || data->cursor.size.h != shape_data->surface->height) {
          dfb_surface_reformat( NULL, cursor, shape_data->surface->width, shape_data->surface->height, DSPF_ARGB );

          data->cursor.size.w = shape_data->surface->width;
          data->cursor.size.h = shape_data->surface->height;

          /* Notify about new size. */
          flags |= CCUF_SIZE;
     }

     if (data->cursor.hot.x != hot_x || data->cursor.hot.y != hot_y) {
          data->cursor.hot.x = hot_x;
          data->cursor.hot.y = hot_y;

          /* Notify about new position. */
          flags |= CCUF_POSITION;
     }

     /* Copy the content of the new shape. */
     dfb_gfx_copy( shape_data->surface, cursor, NULL );

     cursor->caps = ((cursor->caps & ~DSCAPS_PREMULTIPLIED) | (shape_data->surface->caps & DSCAPS_PREMULTIPLIED));

     /* Notify the WM. */
     //if (data->cursor.enabled)
          //dfb_wm_update_cursor( data, flags );

     /* Unlock the region. */
     dfb_layer_region_unlock( data->region );

     return DFB_OK;
}

static DFBResult
IDirectFBDisplayLayer_SetCursorOpacity( IDirectFBDisplayLayer *thiz,
                                        __u8                   opacity )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBDisplayLayer)

     if (data->level == DLSCL_SHARED)
          return DFB_ACCESSDENIED;

     D_DEBUG_AT( Layer, "%s( %p, 0x%02x )\n", __FUNCTION__, data, opacity );

     D_ASSERT( data != NULL );

     /* Lock the region. */
     if (dfb_layer_region_lock( data->region ))
          return DFB_FUSION;

     /* Set new opacity. */
     data->cursor.opacity = opacity;

     /* Notify WM. */
     //if (data->cursor.enabled)
          //dfb_wm_update_cursor( data, CCUF_OPACITY );

     /* Unlock the region. */
     dfb_layer_region_unlock( data->region );

     return DFB_OK;
}

static DFBResult
IDirectFBDisplayLayer_GetColorAdjustment( IDirectFBDisplayLayer *thiz,
                                          DFBColorAdjustment    *adj )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBDisplayLayer)

     if (!adj)
          return DFB_INVARG;

     return dfb_layer_context_get_coloradjustment( data->context, adj );
}

static DFBResult
IDirectFBDisplayLayer_SetColorAdjustment( IDirectFBDisplayLayer    *thiz,
                                          const DFBColorAdjustment *adj )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBDisplayLayer)

     if (!adj || (adj->flags & ~DCAF_ALL))
          return DFB_INVARG;

     if (data->level == DLSCL_SHARED)
          return DFB_ACCESSDENIED;

     if (!adj->flags)
          return DFB_OK;

     return dfb_layer_context_set_coloradjustment( data->context, adj );
}

static DFBResult
IDirectFBDisplayLayer_WaitForSync( IDirectFBDisplayLayer *thiz )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBDisplayLayer)

     return dfb_layer_wait_vsync( data->layer );
}

static DFBResult
IDirectFBDisplayLayer_GetSourceDescriptions( IDirectFBDisplayLayer            *thiz,
                                             DFBDisplayLayerSourceDescription *ret_descriptions )
{
     int i;

     DIRECT_INTERFACE_GET_DATA(IDirectFBDisplayLayer)

     if (!ret_descriptions)
          return DFB_INVARG;

     if (! D_FLAGS_IS_SET( data->desc.caps, DLCAPS_SOURCES ))
          return DFB_UNSUPPORTED;

     for (i=0; i<data->desc.sources; i++)
          dfb_layer_get_source_info( data->layer, i, &ret_descriptions[i] );

     return DFB_OK;
}

DFBResult
IDirectFBDisplayLayer_Construct( IDirectFBDisplayLayer *thiz,
                                 CoreLayer             *layer )
{
     DFBResult         ret;
     CoreLayerContext *context;
     CoreLayerRegion  *region;

     DIRECT_ALLOCATE_INTERFACE_DATA(thiz, IDirectFBDisplayLayer)

     ret = dfb_layer_get_primary_context( layer, true, &context );
     if (ret) {
          DIRECT_DEALLOCATE_INTERFACE( thiz )
          return ret;
     }

     ret = dfb_layer_context_get_primary_region( context, true, &region );
     if (ret) {
          dfb_layer_context_unref( context );
          DIRECT_DEALLOCATE_INTERFACE( thiz )
          return ret;
     }

     data->ref     = 1;
     data->screen  = dfb_layer_screen( layer );
     data->layer   = layer;
     data->context = context;
     data->region  = region;

     dfb_layer_get_description( data->layer, &data->desc );

     thiz->AddRef                = IDirectFBDisplayLayer_AddRef;
     thiz->Release               = IDirectFBDisplayLayer_Release;
     thiz->GetID                 = IDirectFBDisplayLayer_GetID;
     thiz->GetDescription        = IDirectFBDisplayLayer_GetDescription;
     thiz->GetSurface            = IDirectFBDisplayLayer_GetSurface;
     thiz->GetScreen             = IDirectFBDisplayLayer_GetScreen;
     thiz->SetCooperativeLevel   = IDirectFBDisplayLayer_SetCooperativeLevel;
     thiz->SetOpacity            = IDirectFBDisplayLayer_SetOpacity;
     thiz->GetCurrentOutputField = IDirectFBDisplayLayer_GetCurrentOutputField;
     thiz->SetSourceRectangle    = IDirectFBDisplayLayer_SetSourceRectangle;
     thiz->SetScreenLocation     = IDirectFBDisplayLayer_SetScreenLocation;
     thiz->SetSrcColorKey        = IDirectFBDisplayLayer_SetSrcColorKey;
     thiz->SetDstColorKey        = IDirectFBDisplayLayer_SetDstColorKey;
     thiz->GetLevel              = IDirectFBDisplayLayer_GetLevel;
     thiz->SetLevel              = IDirectFBDisplayLayer_SetLevel;
     thiz->GetConfiguration      = IDirectFBDisplayLayer_GetConfiguration;
     thiz->TestConfiguration     = IDirectFBDisplayLayer_TestConfiguration;
     thiz->SetConfiguration      = IDirectFBDisplayLayer_SetConfiguration;
     thiz->GetColorAdjustment    = IDirectFBDisplayLayer_GetColorAdjustment;
     thiz->SetColorAdjustment    = IDirectFBDisplayLayer_SetColorAdjustment;
     thiz->WarpCursor            = IDirectFBDisplayLayer_WarpCursor;
     thiz->SetCursorAcceleration = IDirectFBDisplayLayer_SetCursorAcceleration;
     thiz->EnableCursor          = IDirectFBDisplayLayer_EnableCursor;
     thiz->GetCursorPosition     = IDirectFBDisplayLayer_GetCursorPosition;
     thiz->SetCursorShape        = IDirectFBDisplayLayer_SetCursorShape;
     thiz->SetCursorOpacity      = IDirectFBDisplayLayer_SetCursorOpacity;
     thiz->SetFieldParity        = IDirectFBDisplayLayer_SetFieldParity;
     thiz->SetClipRegions        = IDirectFBDisplayLayer_SetClipRegions;
     thiz->WaitForSync           = IDirectFBDisplayLayer_WaitForSync;
     thiz->GetSourceDescriptions = IDirectFBDisplayLayer_GetSourceDescriptions;
     thiz->SetScreenPosition     = IDirectFBDisplayLayer_SetScreenPosition;
     thiz->SetScreenRectangle    = IDirectFBDisplayLayer_SetScreenRectangle;

     return DFB_OK;
}

