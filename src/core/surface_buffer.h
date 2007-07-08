/*
   (c) Copyright 2001-2007  The DirectFB Organization (directfb.org)
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

#ifndef __CORE__SURFACE_BUFFER_H__
#define __CORE__SURFACE_BUFFER_H__

#include <direct/list.h>

#include <fusion/vector.h>

#include <core/surface.h>

#include <directfb.h>

/*
 * Configuration and State flags of a Surface Buffer
 */
typedef enum {
     CSBF_NONE      = 0x00000000,  /* None of these. */

     CSBF_STICKED   = 0x00000001,  /* Sticked to one Surface Pool, e.g. system only. */

     CSBF_ALL       = 0x00000001   /* All of these. */
} CoreSurfaceBufferFlags;

/*
 * Configuration and State flags of a Surface Buffer Allocation
 */
typedef enum {
     CSALF_NONE      = 0x00000000,  /* None of these. */

     CSALF_ONEFORALL = 0x00000001,  /* Only one allocation in pool for all buffers. */

     CSALF_ALL       = 0x00000001   /* All of these. */
} CoreSurfaceAllocationFlags;

/*
 * An Allocation of a Surface Buffer
 */
typedef struct {
     int                            magic;

     CoreSurfaceBuffer             *buffer;       /* Surface Buffer owning this Allocation. */
     CoreSurfacePool               *pool;         /* Surface Pool providing the Allocation. */
     void                          *data;         /* Pool's private data for this Allocation. */

     CoreSurfaceAccessFlags         access;       
     CoreSurfaceAllocationFlags     flags;        /* Pool can return CSALF_ONEFORALL upon allocation of first buffer. */
} CoreSurfaceAllocation;

/*
 * A Lock on a Surface Buffer
 */
typedef struct {
     int                      magic;

     CoreSurfaceAccessFlags   access;

     CoreSurfaceBuffer       *buffer;
     CoreSurfaceAllocation   *allocation;

     void                    *addr;
     unsigned long            base;
     unsigned int             pitch;

     void                    *handle;
} CoreSurfaceBufferLock;

/*
 * A Surface Buffer of a Surface
 */
struct __DFB_CoreSurfaceBuffer {
     int                      magic;

     CoreSurface             *surface;       /* Surface owning this Surface Buffer. */
     CoreSurfacePolicy        policy;

     CoreSurfaceBufferFlags   flags;         /* Configuration and State flags. */
     DFBSurfacePixelFormat    format;        /* Pixel format of buffer data. */

     FusionVector             allocs;        /* Allocations within Surface Pools. */
};


DFBResult dfb_surface_buffer_new    ( CoreSurface             *surface,
                                      CoreSurfaceBufferFlags   flags,
                                      CoreSurfaceBuffer      **ret_buffer );

DFBResult dfb_surface_buffer_destroy( CoreSurfaceBuffer       *buffer );


DFBResult dfb_surface_buffer_lock   ( CoreSurfaceBuffer       *buffer,
                                      CoreSurfaceAccessFlags   access,
                                      CoreSurfaceBufferLock   *ret_lock );

DFBResult dfb_surface_buffer_unlock ( CoreSurfaceBufferLock   *lock );

DFBResult dfb_surface_buffer_read   ( CoreSurfaceBuffer       *buffer,
                                      void                    *destination,
                                      int                      pitch,
                                      const DFBRectangle      *rect );

DFBResult dfb_surface_buffer_write  ( CoreSurfaceBuffer       *buffer,
                                      const void              *source,
                                      int                      pitch,
                                      const DFBRectangle      *rect );

DFBResult dfb_surface_buffer_dump   ( CoreSurfaceBuffer       *buffer,
                                      const char              *directory,
                                      const char              *prefix );

#endif

