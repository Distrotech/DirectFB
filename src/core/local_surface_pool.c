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

#include <config.h>

#include <direct/debug.h>
#include <direct/mem.h>

#include <core/surface_pool.h>


/**********************************************************************************************************************/

typedef struct {
} LocalPoolData;

typedef struct {
     void *addr;
     int   pitch;
     int   size;
} LocalAllocationData;

/**********************************************************************************************************************/

static int
localPoolDataSize()
{
     return sizeof(LocalPoolData);
}

static int
localAllocationDataSize()
{
     return sizeof(LocalAllocationData);
}

static DFBResult
localInitPool( CoreDFB                    *core,
               CoreSurfacePool            *pool,
               void                       *pool_data,
               void                       *system_data,
               CoreSurfacePoolDescription *ret_desc )
{
     D_MAGIC_ASSERT( pool, CoreSurfacePool );
     D_ASSERT( ret_desc != NULL );

     ret_desc->caps     = CSPCAPS_NONE;
     ret_desc->access   = CSAF_CPU_READ | CSAF_CPU_WRITE;
     ret_desc->priority = CSPP_PREFERED;

     snprintf( ret_desc->name, DFB_SURFACE_POOL_DESC_NAME_LENGTH, "System Memory" );

     return DFB_OK;
}

static DFBResult
localDestroyPool( CoreSurfacePool *pool,
                  void            *pool_data )
{
     D_MAGIC_ASSERT( pool, CoreSurfacePool );

     return DFB_OK;
}

static DFBResult
localAllocateBuffer( CoreSurfacePool   *pool,
                     void              *pool_data,
                     CoreSurfaceBuffer *buffer,
                     void              *alloc_data )
{
     CoreSurface         *surface;
     LocalAllocationData *alloc = alloc_data;

     D_MAGIC_ASSERT( pool, CoreSurfacePool );
     D_MAGIC_ASSERT( buffer, CoreSurfaceBuffer );

     surface = buffer->surface;

     D_MAGIC_ASSERT( surface, CoreSurface );

     dfb_surface_calc_buffer_size( surface, 8, 0, &alloc->pitch, &alloc->size );

     alloc->addr = D_MALLOC( alloc->size );
     if (!alloc->addr)
          return D_OOM();

     return DFB_OK;
}

static DFBResult
localDeallocateBuffer( CoreSurfacePool   *pool,
                       void              *pool_data,
                       CoreSurfaceBuffer *buffer,
                       void              *alloc_data )
{
     LocalAllocationData *alloc = alloc_data;

     D_MAGIC_ASSERT( pool, CoreSurfacePool );
     D_MAGIC_ASSERT( buffer, CoreSurfaceBuffer );

     D_FREE( alloc->addr );

     return DFB_OK;
}

static DFBResult
localLock( CoreSurfacePool       *pool,
           void                  *pool_data,
           CoreSurfaceAllocation *allocation,
           void                  *alloc_data,
           CoreSurfaceBufferLock *lock )
{
     LocalAllocationData *alloc = alloc_data;

     D_MAGIC_ASSERT( pool, CoreSurfacePool );
     D_MAGIC_ASSERT( allocation, CoreSurfaceAllocation );
     D_MAGIC_ASSERT( lock, CoreSurfaceBufferLock );

     lock->addr  = alloc->addr;
     lock->pitch = alloc->pitch;

     return DFB_OK;
}

static DFBResult
localUnlock( CoreSurfacePool       *pool,
             void                  *pool_data,
             CoreSurfaceAllocation *allocation,
             void                  *alloc_data,
             CoreSurfaceBufferLock *lock )
{
     D_MAGIC_ASSERT( pool, CoreSurfacePool );
     D_MAGIC_ASSERT( allocation, CoreSurfaceAllocation );
     D_MAGIC_ASSERT( lock, CoreSurfaceBufferLock );

     return DFB_OK;
}

const SurfacePoolFuncs localSurfacePoolFuncs = {
     PoolDataSize:       localPoolDataSize,
     AllocationDataSize: localAllocationDataSize,
     InitPool:           localInitPool,
     DestroyPool:        localDestroyPool,

     AllocateBuffer:     localAllocateBuffer,
     DeallocateBuffer:   localDeallocateBuffer,

     Lock:               localLock,
     Unlock:             localUnlock
};

