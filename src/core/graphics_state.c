/*
   (c) Copyright 2001-2009  The world wide DirectFB Open Source Community (directfb.org)
   (c) Copyright 2000-2004  Convergence (integrated media) GmbH

   All rights reserved.

   Written by Denis Oliver Kropp <dok@directfb.org>,
              Andreas Hundt <andi@fischlustig.de>,
              Sven Neumann <neo@directfb.org>,
              Ville Syrjälä <syrjala@sci.fi> and
              Claudio Ciccani <klan@users.sf.net>.

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

#include <direct/debug.h>
#include <direct/mem.h>
#include <direct/messages.h>

#include <core/core.h>
#include <core/graphics_internal.h>
#include <core/graphics_state.h>
#include <core/graphics_state_internal.h>
#include <core/state.h>
#include <core/surface.h>


D_DEBUG_DOMAIN( Core_GraphicsState, "Core/Graphics/State", "DirectFB Core Graphics State" );

/**********************************************************************************************************************/

DFBResult
CoreGraphicsStateClient_Init( CoreGraphicsStateClient *client,
                              CoreDFB                 *core )
{
     DFBResult              ret;
     VoodooResponseMessage *response;

     D_ASSERT( client != NULL );
     D_MAGIC_ASSERT( core, CoreDFB );

     client->core = core;

     ret = voodoo_manager_request( core->manager, core->instance, CORE_DFB_CREATE_GRAPHICS_STATE, VREQ_RESPOND, &response, VMBT_NONE );
     if (ret) {
          D_DERROR( ret, "%s: voodoo_manager_request( CORE_DFB_CREATE_GRAPHICS_STATE ) failed!\n", __FUNCTION__ );
          return ret;
     }

     ret = response->result;
     if (ret) {
          D_DERROR( ret, "%s: CORE_CREATE_GRAPHICS_STATE failed!\n", __FUNCTION__ );
          voodoo_manager_finish_request( core->manager, response );
          return ret;
     }

     voodoo_manager_finish_request( core->manager, response );

     client->manager  = core->manager;
     client->instance = response->instance;

     D_MAGIC_SET( client, CoreGraphicsStateClient );

     return DFB_OK;
}

DFBResult
CoreGraphicsStateClient_SetState( CoreGraphicsStateClient *client,
                                  CardState               *state,
                                  StateModificationFlags   flags )
{
     DFBResult ret;

     D_MAGIC_ASSERT( client, CoreGraphicsStateClient );
     D_MAGIC_ASSERT( state, CardState );

     if (flags & SMF_DESTINATION) {
          ret = voodoo_manager_request( client->manager, client->instance, CORE_GRAPHICS_STATE_SET_DESTINATION, VREQ_QUEUE, NULL,
                                        VMBT_UINT, state->destination->object.id,
                                        VMBT_NONE );
          if (ret) {
               D_DERROR( ret, "%s: voodoo_manager_request( CORE_GRAPHICS_STATE_SET_DESTINATION ) failed!\n", __FUNCTION__ );
               return ret;
          }
     }

     if (flags & SMF_CLIP) {
          ret = voodoo_manager_request( client->manager, client->instance, CORE_GRAPHICS_STATE_SET_CLIP, VREQ_QUEUE, NULL,
                                        VMBT_DATA, sizeof(state->clip), &state->clip,
                                        VMBT_NONE );
          if (ret) {
               D_DERROR( ret, "%s: voodoo_manager_request( CORE_GRAPHICS_STATE_SET_CLIP ) failed!\n", __FUNCTION__ );
               return ret;
          }
     }

     if (flags & SMF_COLOR) {
          ret = voodoo_manager_request( client->manager, client->instance, CORE_GRAPHICS_STATE_SET_COLOR, VREQ_QUEUE, NULL,
                                        VMBT_DATA, sizeof(state->color), &state->color,
                                        VMBT_NONE );
          if (ret) {
               D_DERROR( ret, "%s: voodoo_manager_request( CORE_GRAPHICS_STATE_SET_COLOR ) failed!\n", __FUNCTION__ );
               return ret;
          }
     }

     return DFB_OK;
}

// TEST
DFBResult
CoreGraphicsStateClient_FillRectangle( CoreGraphicsStateClient *client,
                                       const DFBRectangle      *rect )
{
     DFBResult ret;

     D_MAGIC_ASSERT( client, CoreGraphicsStateClient );
     DFB_RECTANGLE_ASSERT( rect );

     ret = voodoo_manager_request( client->manager, client->instance, CORE_GRAPHICS_STATE_FILL_RECTANGLE, VREQ_QUEUE, NULL,
                                   VMBT_DATA, sizeof(*rect), rect,
                                   VMBT_NONE );
     if (ret) {
          D_DERROR( ret, "%s: voodoo_manager_request( CORE_GRAPHICS_STATE_FILL_RECTANGLE ) failed!\n", __FUNCTION__ );
          return ret;
     }

     return DFB_OK;
}

/*********************************************************************************************************************/
/*********************************************************************************************************************/

static DFBResult
CoreGraphicsState_Dispatch_SetDestination( CoreGraphicsState    *state,
                                           VoodooManager        *manager,
                                           VoodooRequestMessage *msg )
{
     DFBResult            ret;
     VoodooMessageParser  parser;
     u32                  object_id;
     CoreSurface         *surface;

     D_DEBUG_AT( Core_GraphicsState, "%s( %p )\n", __FUNCTION__, state );

     D_MAGIC_ASSERT( state, CoreGraphicsState );

     VOODOO_PARSER_BEGIN( parser, msg );
     VOODOO_PARSER_GET_UINT( parser, object_id );
     VOODOO_PARSER_END( parser );

     ret = dfb_core_get_surface( state->core, object_id, &surface );
     if (ret)
          return ret;

     ret = dfb_state_set_destination( &state->state, surface );

     dfb_surface_unref( surface );

     return ret;
}

static DFBResult
CoreGraphicsState_Dispatch_SetClip( CoreGraphicsState    *state,
                                    VoodooManager        *manager,
                                    VoodooRequestMessage *msg )
{
     VoodooMessageParser  parser;
     const DFBRegion     *clip;

     D_DEBUG_AT( Core_GraphicsState, "%s( %p )\n", __FUNCTION__, state );

     D_MAGIC_ASSERT( state, CoreGraphicsState );

     VOODOO_PARSER_BEGIN( parser, msg );
     VOODOO_PARSER_GET_DATA( parser, clip );
     VOODOO_PARSER_END( parser );

     dfb_state_set_clip( &state->state, clip );

     return DFB_OK;
}

static DFBResult
CoreGraphicsState_Dispatch_SetColor( CoreGraphicsState    *state,
                                     VoodooManager        *manager,
                                     VoodooRequestMessage *msg )
{
     VoodooMessageParser  parser;
     const DFBColor      *color;

     D_DEBUG_AT( Core_GraphicsState, "%s( %p )\n", __FUNCTION__, state );

     D_MAGIC_ASSERT( state, CoreGraphicsState );

     VOODOO_PARSER_BEGIN( parser, msg );
     VOODOO_PARSER_GET_DATA( parser, color );
     VOODOO_PARSER_END( parser );

     dfb_state_set_color( &state->state, color );

     return DFB_OK;
}

static DFBResult
CoreGraphicsState_Dispatch_FillRectangle( CoreGraphicsState    *state,
                                          VoodooManager        *manager,
                                          VoodooRequestMessage *msg )
{
     VoodooMessageParser  parser;
     const DFBRectangle  *rect;

     D_DEBUG_AT( Core_GraphicsState, "%s( %p )\n", __FUNCTION__, state );

     D_MAGIC_ASSERT( state, CoreGraphicsState );

     VOODOO_PARSER_BEGIN( parser, msg );
     VOODOO_PARSER_GET_DATA( parser, rect );
     VOODOO_PARSER_END( parser );

     dfb_gfxcard_fillrectangles( rect, 1, &state->state );

     return DFB_OK;
}

/**********************************************************************************************************************/

DirectResult
CoreGraphicsState_Dispatch( void                 *dispatcher,
                            void                 *real,
                            VoodooManager        *manager,
                            VoodooRequestMessage *msg )
{
     switch (msg->method) {
          case CORE_GRAPHICS_STATE_SET_DESTINATION:
               D_DEBUG_AT( Core_GraphicsState, "=-> CORE_GRAPHICS_STATE_SET_DESTINATION\n" );

               return CoreGraphicsState_Dispatch_SetDestination( real, manager, msg );

          case CORE_GRAPHICS_STATE_SET_CLIP:
               D_DEBUG_AT( Core_GraphicsState, "=-> CORE_GRAPHICS_STATE_SET_CLIP\n" );

               return CoreGraphicsState_Dispatch_SetClip( real, manager, msg );

          case CORE_GRAPHICS_STATE_SET_COLOR:
               D_DEBUG_AT( Core_GraphicsState, "=-> CORE_GRAPHICS_STATE_SET_COLOR\n" );

               return CoreGraphicsState_Dispatch_SetColor( real, manager, msg );

          case CORE_GRAPHICS_STATE_FILL_RECTANGLE:
               D_DEBUG_AT( Core_GraphicsState, "=-> CORE_GRAPHICS_STATE_FILL_RECTANGLE\n" );

               return CoreGraphicsState_Dispatch_FillRectangle( real, manager, msg );

          default:
               D_BUG( "invalid method %d", msg->method );
     }

     return DR_NOSUCHINSTANCE;
}

