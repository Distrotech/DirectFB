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

#include <directfb_util.h>

#include <direct/debug.h>
#include <direct/mem.h>
#include <direct/messages.h>

#include <core/core.h>
#include <core/window.h>
#include <core/window_internal.h>

#include <core/windows_internal.h>


D_DEBUG_DOMAIN( Core_Window, "Core/Window", "DirectFB Core Window" );

/*********************************************************************************************************************/

DFBResult
CoreWindow_SetConfig( CoreDFB                *core,
                      CoreWindow             *window,
                      const CoreWindowConfig *config,
                      CoreWindowConfigFlags   flags )
{
     DFBResult ret;

     D_DEBUG_AT( Core_Window, "%s( %p )\n", __FUNCTION__, window );

     D_MAGIC_ASSERT( window, CoreWindow );
     D_ASSERT( config != NULL );

     ret = voodoo_manager_request( core->manager, core->instance, CORE_DFB_WINDOW_SET_CONFIG, VREQ_NONE, NULL,
                                   VMBT_UINT, window->object.id,
                                   VMBT_DATA, sizeof(*config), config,
                                   VMBT_INT, flags,
                                   VMBT_NONE );
     if (ret)
          D_DERROR( ret, "%s: voodoo_manager_request( CORE_DFB_WINDOW_SET_CONFIG ) failed!\n", __FUNCTION__ );

     return ret;
}

DFBResult
CoreWindow_Repaint( CoreDFB             *core,
                    CoreWindow          *window,
                    const DFBRegion     *left,
                    const DFBRegion     *right,
                    DFBSurfaceFlipFlags  flags )
{
     DFBResult ret;

     D_DEBUG_AT( Core_Window, "%s( %p )\n", __FUNCTION__, window );

     D_MAGIC_ASSERT( window, CoreWindow );
     DFB_REGION_ASSERT( left );
     DFB_REGION_ASSERT( right );

     ret = voodoo_manager_request( core->manager, core->instance, CORE_DFB_WINDOW_REPAINT, VREQ_NONE, NULL,
                                   VMBT_UINT, window->object.id,
                                   VMBT_DATA, sizeof(*left), left,
                                   VMBT_DATA, sizeof(*right), right,
                                   VMBT_INT, flags,
                                   VMBT_NONE );
     if (ret)
          D_DERROR( ret, "%s: voodoo_manager_request( CORE_DFB_WINDOW_REPAINT ) failed!\n", __FUNCTION__ );

     return ret;
}

/*********************************************************************************************************************/
/*********************************************************************************************************************/

static DFBResult
CoreWindow_Dispatch_SetConfig( CoreWindow           *window,
                               VoodooManager        *manager,
                               VoodooRequestMessage *msg )
{
     DFBResult               ret;
     VoodooMessageParser     parser;
     const CoreWindowConfig *config;
     CoreWindowConfigFlags   flags;

     D_DEBUG_AT( Core_Window, "%s( %p )\n", __FUNCTION__, window );

     D_MAGIC_ASSERT( window, CoreWindow );

     VOODOO_PARSER_BEGIN( parser, msg );
     VOODOO_PARSER_GET_DATA( parser, config );
     VOODOO_PARSER_GET_INT( parser, flags );
     VOODOO_PARSER_END( parser );

     ret = dfb_window_set_config( window, config, flags );

     return voodoo_manager_respond( manager, true, msg->header.serial, ret, VOODOO_INSTANCE_NONE, VMBT_NONE );
}

static DFBResult
CoreWindow_Dispatch_Repaint( CoreWindow           *window,
                             VoodooManager        *manager,
                             VoodooRequestMessage *msg )
{
     DFBResult            ret;
     VoodooMessageParser  parser;
     const DFBRegion     *left;
     const DFBRegion     *right;
     DFBSurfaceFlipFlags  flags;

     D_DEBUG_AT( Core_Window, "%s( %p )\n", __FUNCTION__, window );

     D_MAGIC_ASSERT( window, CoreWindow );

     VOODOO_PARSER_BEGIN( parser, msg );
     VOODOO_PARSER_GET_DATA( parser, left );
     VOODOO_PARSER_GET_DATA( parser, right );
     VOODOO_PARSER_GET_INT( parser, flags );
     VOODOO_PARSER_END( parser );

     ret = dfb_window_repaint( window, left, right, flags );

     return voodoo_manager_respond( manager, true, msg->header.serial, ret, VOODOO_INSTANCE_NONE, VMBT_NONE );
}

DirectResult
CoreWindow_Dispatch( void                 *dispatcher,
                     void                 *real,
                     VoodooManager        *manager,
                     VoodooRequestMessage *msg )
{
     switch (msg->method) {
          case CORE_WINDOW_SET_CONFIG:
               D_DEBUG_AT( Core_Window, "=-> CORE_WINDOW_SET_CONFIG\n" );

               return CoreWindow_Dispatch_SetConfig( real, manager, msg );

          case CORE_WINDOW_REPAINT:
               D_DEBUG_AT( Core_Window, "=-> CORE_WINDOW_REPAINT\n" );

               return CoreWindow_Dispatch_Repaint( real, manager, msg );

          default:
               D_BUG( "invalid method %d", msg->method );
     }

     return DR_NOSUCHINSTANCE;
}

