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

#ifndef __DRAWABLE__DRAWABLE_H__
#define __DRAWABLE__DRAWABLE_H__

#include <directfb.h>
#include <core/coretypes.h>
#include <fusion/object.h>
#include <drawable/drawable_manager.h>

/*
 * Core data of a window.
 */
struct __Drawable_Drawable {
     FusionObject            object;
     int                    magic;

     DrawableManager       *manager;
     FusionSHMPoolShared   *shmpool;

     DrawableProcess         *process;

     DFBDrawableID            id;
};

/*
 *  * Creates a pool of drawable objects.
 *   */
FusionObjectPool *dfb_drawable_pool_create( const FusionWorld *world );

/*
 * Generates dfb_drawable_ref(), dfb_drawable_attach() etc.
 */
FUSION_OBJECT_METHODS( Drawable, dfb_drawable )

/*
 * creates a drawabl
 */
DFBResult
dfb_drawable_create( int                     x,
                     int                     y,
                     int                     width,
                     int                     height,
                     Drawable          **drawable );

/*
 * deinitializes a drawable and removes it 
 */
void
dfb_drawable_destroy( Drawable *drawable );


DFBResult dfb_drawble_init         ( Drawable        *drawable );

//DFBResult dfb_drawble_append_output ( CoreSurface *surface, DrawOpOut** output );

DFBResult dfb_drawable_set_property ( Drawable  *drawable,
                                       const char       *key,
                                       void             *value,
                                       void            **ret_old_value );

DFBResult dfb_drawable_get_property ( Drawable  *drawable,
                                       const char       *key,
                                       void            **ret_value );

DFBResult dfb_drawable_remove_property ( Drawable  *drawable,
                                          const char       *key,
                                          void            **ret_value );

DFBResult dfb_drawable_add             ( Drawable        *parent,
                                         Drawable             *child );

DFBResult dfb_drawable_remove      ( Drawable        *parent,
                                      Drawable             *child );

#endif
// vim: ts=5 sw=5 et
