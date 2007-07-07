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

#include <string.h>

#include <directfb_util.h>

#include <direct/debug.h>
#include <direct/memcpy.h>

#include <fusion/shmalloc.h>

#include <core/surface.h>
#include <core/surface_buffer.h>
#include <core/surface_pool.h>

#include <gfx/convert.h>


D_DEBUG_DOMAIN( Core_SurfBuffer, "Core/SurfBuffer", "DirectFB Core Surface Buffer" );

/**********************************************************************************************************************/

DFBResult
dfb_surface_buffer_new( CoreSurface             *surface,
                        CoreSurfaceBufferFlags   flags,
                        CoreSurfaceBuffer      **ret_buffer )
{
     CoreSurfaceBuffer *buffer;

     D_MAGIC_ASSERT( surface, CoreSurface );
     D_FLAGS_ASSERT( flags, CSBF_ALL );
     D_ASSERT( ret_buffer != NULL );

#if DIRECT_BUILD_DEBUG
     D_DEBUG_AT( Core_SurfBuffer, "dfb_surface_buffer_new( %s )\n", dfb_pixelformat_name( surface->config.format ) );

     if (flags & CSBF_STICKED)
          D_DEBUG_AT( Core_SurfBuffer, "  -> STICKED\n" );
#endif

     buffer = SHCALLOC( surface->shmpool, 1, sizeof(CoreSurfaceBuffer) );
     if (!buffer)
          return D_OOSHM();

     buffer->surface = surface;
     buffer->flags   = flags;
     buffer->format  = surface->config.format;

     D_MAGIC_SET( buffer, CoreSurfaceBuffer );

     *ret_buffer = buffer;

     return DFB_OK;
}

DFBResult
dfb_surface_buffer_destroy( CoreSurfaceBuffer *buffer )
{
     CoreSurface           *surface;
     CoreSurfaceAllocation *allocation, *next;

     D_MAGIC_ASSERT( buffer, CoreSurfaceBuffer );

     surface = buffer->surface;

     D_MAGIC_ASSERT( surface, CoreSurface );

     D_DEBUG_AT( Core_SurfBuffer, "dfb_surface_buffer_destroy( %p [%dx%d] )\n",
                 buffer, surface->config.size.h, surface->config.size.h );

     direct_list_foreach_safe (allocation, next, buffer->allocs)
          dfb_surface_pool_deallocate( allocation->pool, allocation );

     D_MAGIC_CLEAR( buffer );

     SHFREE( surface->shmpool, buffer );

     return DFB_OK;
}

DFBResult
dfb_surface_buffer_lock( CoreSurfaceBuffer      *buffer,
                         CoreSurfaceAccessFlags  access,
                         CoreSurfaceBufferLock  *lock )
{
     DFBResult              ret;
     CoreSurfaceAllocation *alloc;
     CoreSurfaceAllocation *allocation = NULL;

#if DIRECT_BUILD_DEBUG
     D_DEBUG_AT( Core_SurfBuffer, "dfb_surface_buffer_lock( %p, 0x%08x, %p )\n", buffer, access, lock );

     if (access & CSAF_GPU_READ)
          D_DEBUG_AT( Core_SurfBuffer, "  -> GPU READ\n" );
     if (access & CSAF_GPU_WRITE)
          D_DEBUG_AT( Core_SurfBuffer, "  -> GPU WRITE\n" );
     if (access & CSAF_CPU_READ)
          D_DEBUG_AT( Core_SurfBuffer, "  -> CPU READ\n" );
     if (access & CSAF_CPU_WRITE)
          D_DEBUG_AT( Core_SurfBuffer, "  -> CPU WRITE\n" );
#endif

     D_MAGIC_ASSERT( buffer, CoreSurfaceBuffer );
     D_FLAGS_ASSERT( access, CSAF_ALL );
     D_ASSERT( lock != NULL );

     direct_list_foreach (alloc, buffer->allocs) {
          if (D_FLAGS_ARE_SET( alloc->access, access )) {
               allocation = alloc;
               break;
          }
     }

     if (!allocation) {
          CoreSurfacePool *pool;

          ret = dfb_surface_pool_negotiate( buffer, access, &pool );
          if (ret) {
               D_DERROR( ret, "Core/SurfBuffer: Surface pool negotiation failed!\n" );
               return ret;
          }

          ret = dfb_surface_pool_allocate( pool, buffer, &allocation );
          if (ret) {
               D_DERROR( ret, "Core/SurfBuffer: Allocation in '%s' failed!\n", pool->desc.name );
               return ret;
          }

          direct_list_append( &buffer->allocs, &allocation->link );
     }

     lock->buffer     = buffer;
     lock->access     = access;
     lock->allocation = NULL;

     D_MAGIC_SET( lock, CoreSurfaceBufferLock );

     ret = dfb_surface_pool_lock( allocation->pool, allocation, lock );
     if (ret) {
          D_DERROR( ret, "Core/SurfBuffer: Locking allocation failed! [%s]\n",
                    allocation->pool->desc.name );
          D_MAGIC_CLEAR( lock );
          return ret;
     }

     lock->allocation = allocation;

     return DFB_OK;
}

DFBResult
dfb_surface_buffer_unlock( CoreSurfaceBufferLock *lock )
{
     DFBResult        ret;
     CoreSurfacePool *pool;

     D_DEBUG_AT( Core_SurfBuffer, "dfb_surface_buffer_unlock( %p )\n", lock );

     D_MAGIC_ASSERT( lock, CoreSurfaceBufferLock );

     D_MAGIC_ASSERT( lock->buffer, CoreSurfaceBuffer );
     D_MAGIC_ASSERT( lock->allocation, CoreSurfaceAllocation );

     pool = lock->allocation->pool;

     D_MAGIC_ASSERT( pool, CoreSurfacePool );

     ret = dfb_surface_pool_unlock( pool, lock->allocation, lock );
     if (ret) {
          D_DERROR( ret, "Core/SurfBuffer: Unlocking allocation failed! [%s]\n", pool->desc.name );
          return ret;
     }

     lock->buffer     = NULL;
     lock->allocation = NULL;

     lock->addr       = NULL;
     lock->base       = 0;
     lock->pitch      = 0;

     D_MAGIC_CLEAR( lock );

     return DFB_OK;
}

DFBResult
dfb_surface_buffer_read( CoreSurfaceBuffer  *buffer,
                         void               *destination,
                         int                 pitch,
                         const DFBRectangle *prect )
{
     DFBResult              ret;
     DFBRectangle           rect;
     int                    y;
     int                    bytes;
     CoreSurface           *surface;
     CoreSurfaceBufferLock  lock;
     CoreSurfaceAllocation *alloc;
     CoreSurfaceAllocation *allocation = NULL;
     DFBSurfacePixelFormat  format;

     D_DEBUG_AT( Core_SurfBuffer, "%s( %p, %p [%d] )\n", __FUNCTION__, buffer, destination, pitch );

     D_MAGIC_ASSERT( buffer, CoreSurfaceBuffer );
     D_ASSERT( destination != NULL );
     D_ASSERT( pitch > 0 );
     DFB_RECTANGLE_ASSERT_IF( prect );

     surface = buffer->surface;
     D_MAGIC_ASSERT( surface, CoreSurface );

     /* Determine area. */
     rect.x = 0;
     rect.y = 0;
     rect.w = surface->config.size.w;
     rect.h = surface->config.size.h;

     if (prect && (!dfb_rectangle_intersect( &rect, prect ) || !DFB_RECTANGLE_EQUAL( rect, *prect )))
          return DFB_INVAREA;

     /* Calculate bytes per read line. */
     format = surface->config.format;
     bytes  = DFB_BYTES_PER_LINE( format, rect.w );

     D_DEBUG_AT( Core_SurfBuffer, "  -> %d,%d - %dx%d (%s)\n", DFB_RECTANGLE_VALS(&rect),
                 dfb_pixelformat_name( format ) );

     /* If no allocations exists, simply clear the destination. */
     if (!buffer->allocs) {
          for (y=0; y<rect.h; y++) {
               memset( destination, 0, bytes );

               destination += pitch;
          }

          return DFB_OK;
     }

     /* Look for allocation with CPU access. */
     direct_list_foreach (alloc, buffer->allocs) {
          if (D_FLAGS_ARE_SET( alloc->access, CSAF_CPU_READ )) {
               allocation = alloc;
               break;
          }
     }

     /* FIXME: use Read() */
     if (!allocation)
          return DFB_UNIMPLEMENTED;

     /* Lock the allocation. */
     lock.buffer = buffer;
     lock.access = CSAF_CPU_READ;

     D_MAGIC_SET( &lock, CoreSurfaceBufferLock );

     ret = dfb_surface_pool_lock( allocation->pool, allocation, &lock );
     if (ret) {
          D_DERROR( ret, "Core/SurfBuffer: Locking allocation failed! [%s]\n",
                    allocation->pool->desc.name );
          D_MAGIC_CLEAR( &lock );
          return ret;
     }

     /* Move to start of read. */
     lock.addr += DFB_BYTES_PER_LINE( format, rect.x ) + rect.y * lock.pitch;

     /* Copy the data. */
     for (y=0; y<rect.h; y++) {
          direct_memcpy( destination, lock.addr, bytes );

          destination += pitch;
          lock.addr   += lock.pitch;
     }

     /* Unlock the allocation. */
     ret = dfb_surface_pool_unlock( allocation->pool, allocation, &lock );
     if (ret)
          D_DERROR( ret, "Core/SurfBuffer: Unlocking allocation failed! [%s]\n", allocation->pool->desc.name );

     D_MAGIC_CLEAR( &lock );

     return ret;
}

DFBResult
dfb_surface_buffer_write( CoreSurfaceBuffer  *buffer,
                          const void         *source,
                          int                 pitch,
                          const DFBRectangle *prect )
{
     DFBResult              ret;
     DFBRectangle           rect;
     int                    y;
     int                    bytes;
     CoreSurface           *surface;
     CoreSurfaceBufferLock  lock;
     CoreSurfaceAllocation *alloc;
     CoreSurfaceAllocation *allocation = NULL;
     DFBSurfacePixelFormat  format;

     D_DEBUG_AT( Core_SurfBuffer, "%s( %p, %p [%d] )\n", __FUNCTION__, buffer, source, pitch );

     D_MAGIC_ASSERT( buffer, CoreSurfaceBuffer );
     D_ASSERT( pitch > 0 || source == NULL );
     DFB_RECTANGLE_ASSERT_IF( prect );

     surface = buffer->surface;
     D_MAGIC_ASSERT( surface, CoreSurface );

     /* Determine area. */
     rect.x = 0;
     rect.y = 0;
     rect.w = surface->config.size.w;
     rect.h = surface->config.size.h;

     if (prect && (!dfb_rectangle_intersect( &rect, prect ) || !DFB_RECTANGLE_EQUAL( rect, *prect )))
          return DFB_INVAREA;

     /* Calculate bytes per written line. */
     format = surface->config.format;
     bytes  = DFB_BYTES_PER_LINE( format, rect.w );

     D_DEBUG_AT( Core_SurfBuffer, "  -> %d,%d - %dx%d (%s)\n", DFB_RECTANGLE_VALS(&rect),
                 dfb_pixelformat_name( format ) );

     /* If no allocations exists, create one. */
     if (!buffer->allocs) {
          CoreSurfacePool *pool;

          ret = dfb_surface_pool_negotiate( buffer, CSAF_CPU_WRITE, &pool );
          if (ret) {
               D_DERROR( ret, "Core/SurfBuffer: Surface pool negotiation failed!\n" );
               return ret;
          }

          ret = dfb_surface_pool_allocate( pool, buffer, &allocation );
          if (ret) {
               D_DERROR( ret, "Core/SurfBuffer: Allocation in '%s' failed!\n", pool->desc.name );
               return ret;
          }

          direct_list_append( &buffer->allocs, &allocation->link );
     }

     /* Look for allocation with CPU access. */
     direct_list_foreach (alloc, buffer->allocs) {
          if (D_FLAGS_ARE_SET( alloc->access, CSAF_CPU_WRITE )) {
               allocation = alloc;
               break;
          }
     }

     /* FIXME: use Write() */
     if (!allocation)
          return DFB_UNIMPLEMENTED;

     /* Lock the allocation. */
     lock.buffer = buffer;
     lock.access = CSAF_CPU_WRITE;

     D_MAGIC_SET( &lock, CoreSurfaceBufferLock );

     ret = dfb_surface_pool_lock( allocation->pool, allocation, &lock );
     if (ret) {
          D_DERROR( ret, "Core/SurfBuffer: Locking allocation failed! [%s]\n",
                    allocation->pool->desc.name );
          D_MAGIC_CLEAR( &lock );
          return ret;
     }

     /* Move to start of write. */
     lock.addr += DFB_BYTES_PER_LINE( format, rect.x ) + rect.y * lock.pitch;

     /* Copy the data. */
     for (y=0; y<rect.h; y++) {
          if (source) {
               direct_memcpy( lock.addr, source, bytes );

               source += pitch;
          }
          else
               memset( lock.addr, 0, bytes );

          lock.addr += lock.pitch;
     }

     /* Unlock the allocation. */
     ret = dfb_surface_pool_unlock( allocation->pool, allocation, &lock );
     if (ret)
          D_DERROR( ret, "Core/SurfBuffer: Unlocking allocation failed! [%s]\n", allocation->pool->desc.name );

     D_MAGIC_CLEAR( &lock );

     return ret;
}

DFBResult
dfb_surface_buffer_dump( CoreSurfaceBuffer *buffer,
                         const char        *directory,
                         const char        *prefix )
{
     D_DEBUG_AT( Core_SurfBuffer, "dfb_surface_buffer_dump( %p, %p, %p )\n", buffer, directory, prefix );

     return DFB_OK;
}

