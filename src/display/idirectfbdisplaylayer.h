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

#ifndef __IDIRECTFBDISPLAYLAYER_H__
#define __IDIRECTFBDISPLAYLAYER_H__

#include <directfb.h>
#include <core/coretypes.h>

/* FIXME: move to cursor.h when it's there */
typedef enum {
     CCUF_NONE      = 0x00000000,

     CCUF_ENABLE    = 0x00000001,
     CCUF_DISABLE   = 0x00000002,

     CCUF_POSITION  = 0x00000010,
     CCUF_SIZE      = 0x00000020,
     CCUF_SHAPE     = 0x00000040,
     CCUF_OPACITY   = 0x00000080,

     CCUF_ALL       = 0x000000F3
} CoreCursorUpdateFlags;


/*
 * initializes interface struct and private data
 */
DFBResult IDirectFBDisplayLayer_Construct( IDirectFBDisplayLayer *thiz,
                                           CoreLayer             *layer );


#endif
