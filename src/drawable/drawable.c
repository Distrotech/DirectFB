/*
   (c) Copyright 2001-2007  The DirectFB Organization (directfb.org)
   (c) Copyright 2000-2004  Convergence (integrated media) GmbH

   All rights reserved.

   Written by Denis Oliver Kropp <dok@directfb.org>,
              Andreas Hundt <andi@fischlustig.de>,
              Sven Neumann <neo@directfb.org>,
              Ville Syrjälä <syrjala@sci.fi> and
              Claudio Ciccani <klan@users.sf.net>
              Michael Emmel <mike.emmel@gmail.com>.

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
#include <string.h>

#include <directfb.h>

#include <direct/list.h>

#include <fusion/shmalloc.h>

#include <core/core.h>
#include <core/coredefs.h>
#include <core/coretypes.h>
#include <core/core_parts.h>
#include <drawable/drawable.h>


/**************************************************************************************************/



DFBResult
dfb_drawable_set_active( Drawable *drawable,
                   bool             active )
{
     return DFB_OK;
}

DFBResult
dfb_drawable_drawable_at( Drawable  *drawable,
                  int               x,
                  int               y,
                  Drawable      **ret_drawable )
{
     return DFB_OK;
}

DFBResult
dfb_drawable_drawable_lookup( Drawable  *drawable,
                      DFBDrawableID       drawable_id,
                      Drawable      **ret_drawable )
{
     return DFB_OK;
}


DFBResult
dfb_drawable_add_drawable( Drawable *parent,
                   Drawable      *child )
{
     return DFB_OK;
}

DFBResult
dfb_drawable_remove_drawable( Drawable *parent,
                      Drawable      *child )
{
     DFBResult ret = DFB_OK;
     return ret;
}

/**
 * Let the drawable set a property on a drawable 
 */
DFBResult
dfb_drawable_set_drawable_property( Drawable  *drawable,
                            const char       *key,
                            void             *value,
                            void            **ret_old_value )
{
     D_ASSERT( drawable != NULL );
     D_ASSERT( key != NULL );
     fusion_object_set_property((FusionObject*)drawable,
                     key,value,ret_old_value);
}

/**
 * get the drawable  property on a drawable 
 */
DFBResult
dfb_drawable_get_property( Drawable  *drawable,
                                    const char       *key,
                                    void            **ret_value )
{
     D_ASSERT( drawable != NULL );
     D_ASSERT( key != NULL );
     D_ASSERT( ret_value != NULL );

     *ret_value=fusion_object_get_property((FusionObject*)drawable,key);
     return DFB_OK;

}

/**
 * remove the drawable  property 
 */
DFBResult
dfb_drawable_remove_property( Drawable       *drawable,
                               const char       *key,
                               void            **ret_value )
{
     D_ASSERT( drawable != NULL );
     D_ASSERT( key != NULL );
     fusion_object_remove_property((FusionObject*)drawable,key,ret_value);
     return DFB_OK;
}

// vim: ts=5 sw=5 et
