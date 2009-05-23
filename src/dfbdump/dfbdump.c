/*
   (c) Copyright 2001-2008  The world wide DirectFB Open Source Community (directfb.org)
   (c) Copyright 2000-2004  Convergence (integrated media) GmbH

   All rights reserved.

   Written by Denis Oliver Kropp <dok@directfb.org>,
              Andreas Hundt <andi@fischlustig.de>,
              Sven Neumann <neo@directfb.org>,
              Ville Syrjälä <syrjala@sci.fi> and
              Claudio Ciccani <klan@users.sf.net>.

   This file is subject to the terms and conditions of the MIT License:

   Permission is hereby granted, free of charge, to any person
   obtaining a copy of this software and associated documentation
   files (the "Software"), to deal in the Software without restriction,
   including without limitation the rights to use, copy, modify, merge,
   publish, distribute, sublicense, and/or sell copies of the Software,
   and to permit persons to whom the Software is furnished to do so,
   subject to the following conditions:

   The above copyright notice and this permission notice shall be
   included in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
   EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
   IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
   CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
   TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
   SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

//#define DIRECT_ENABLE_DEBUG

#include <config.h>

#include <directfb.h>
#include <directfb_strings.h>

#include <direct/debug.h>

#include <fusion/fusion.h>
#include <fusion/object.h>
#include <fusion/ref.h>
#include <fusion/shmalloc.h>
#include <fusion/shm/shm.h>
#include <fusion/shm/shm_internal.h>

#include <core/core.h>
#include <core/layer_context.h>
#include <core/layer_control.h>
#include <core/layers_internal.h>
#include <core/surface.h>
#include <core/surface_buffer.h>
#include <core/surface_pool.h>
#include <core/windows_internal.h>
#include <core/windowstack.h>
#include <core/wm.h>

#include <dfbdump/dfbdump.h>


static DirectFBPixelFormatNames( format_names );

/**********************************************************************************************************************/

typedef struct {
     int video;
     int system;
     int presys;
} MemoryUsage;

typedef struct {
     CoreDFB     *core;
     MemoryUsage  mem;
     int          dump_surface;
} ShowSurfaceContext;

static inline int
buffer_size( CoreSurface *surface, CoreSurfaceBuffer *buffer, bool video )
{
     int                    i, mem = 0;
     CoreSurfaceAllocation *allocation;

     fusion_vector_foreach (allocation, i, buffer->allocs) {
          int size = allocation->size;
          if (allocation->flags & CSALF_ONEFORALL)
               size /= surface->num_buffers;
          if (video) {
               if (allocation->access[CSAID_GPU])
                    mem += size;
          }
          else if (!allocation->access[CSAID_GPU])
               mem += size;
     }

     return mem;
}

static int
buffer_sizes( CoreSurface *surface, bool video )
{
     int i, mem = 0;

     for (i=0; i<surface->num_buffers; i++) {
          CoreSurfaceBuffer *buffer = surface->buffers[i];

          mem += buffer_size( surface, buffer, video );
     }

     return mem;
}

static int
buffer_locks( CoreSurface *surface, bool video )
{
     int i, locks = 0;

     for (i=0; i<surface->num_buffers; i++) {
          CoreSurfaceBuffer *buffer = surface->buffers[i];

          locks += buffer->locked;
     }

     return locks;
}

static bool
surface_callback( FusionObjectPool *pool,
                  FusionObject     *object,
                  void             *ctx )
{
     DirectResult        ret;
     int                 i;
     int                 refs;
     CoreSurface        *surface = (CoreSurface*) object;
     ShowSurfaceContext *context = ctx;
     MemoryUsage        *mem     = &context->mem;
     int                 vmem;
     int                 smem;

     if (object->state != FOS_ACTIVE)
          return true;

     ret = fusion_ref_stat( &object->ref, &refs );
     if (ret) {
          D_DERROR( ret, "Tools/Dump: Fusion error!\n" );
          return false;
     }

     if (context->dump_surface && ((context->dump_surface < 0 && surface->type & CSTF_SHARED) ||
                                   (context->dump_surface == object->ref.multi.id)) && surface->num_buffers)
     {
          char buf[32];

          direct_snprintf( buf, sizeof(buf), "dfb_surface_0x%08x", object->ref.multi.id );

          dfb_surface_dump_buffer( surface, CSBR_FRONT, ".", buf );
     }

#if FUSION_BUILD_MULTI
     direct_log_printf( NULL, "0x%08x [%3lx] : ", object->ref.multi.id, object->ref.multi.creator );
#else
     direct_log_printf( NULL, "N/A              : " );
#endif

     direct_log_printf( NULL, "%3d   ", refs );

     direct_log_printf( NULL, "%4dx%4d   ", surface->config.size.w, surface->config.size.h );

     for (i=0; format_names[i].format; i++) {
          if (surface->config.format == format_names[i].format)
               direct_log_printf( NULL, "%8s ", format_names[i].name );
     }

     vmem = buffer_sizes( surface, true );
     smem = buffer_sizes( surface, false );

     mem->video += vmem;

     /* FIXME: assumes all buffers have this flag (or none) */
     if (surface->type & CSTF_PREALLOCATED)
          mem->presys += smem;
     else
          mem->system += smem;

     if (vmem && vmem < 1024)
          vmem = 1024;

     if (smem && smem < 1024)
          smem = 1024;

     direct_log_printf( NULL, "%5dk%c  ", vmem >> 10, buffer_locks( surface, true ) ? '*' : ' ' );
     direct_log_printf( NULL, "%5dk%c  ", smem >> 10, buffer_locks( surface, false ) ? '*' : ' ' );

     /* FIXME: assumes all buffers have this flag (or none) */
     if (surface->type & CSTF_PREALLOCATED)
          direct_log_printf( NULL, "preallocated " );

     if (surface->config.caps & DSCAPS_SYSTEMONLY)
          direct_log_printf( NULL, "system only  " );

     if (surface->config.caps & DSCAPS_VIDEOONLY)
          direct_log_printf( NULL, "video only   " );

     if (surface->config.caps & DSCAPS_INTERLACED)
          direct_log_printf( NULL, "interlaced   " );

     if (surface->config.caps & DSCAPS_DOUBLE)
          direct_log_printf( NULL, "double       " );

     if (surface->config.caps & DSCAPS_TRIPLE)
          direct_log_printf( NULL, "triple       " );

     if (surface->config.caps & DSCAPS_PREMULTIPLIED)
          direct_log_printf( NULL, "premultiplied" );

     direct_log_printf( NULL, "\n" );

     return true;
}

void
dfbdump_show_surfaces( CoreDFB *core, int dump_surface )
{
     ShowSurfaceContext context;

     memset( &context, 0, sizeof(context) );

     context.core         = core;
     context.dump_surface = dump_surface;


     direct_log_printf( NULL, "\n"
                        "-----------------------------[ Surfaces ]-------------------------------------\n" );
     direct_log_printf( NULL, "Reference   FID  . Refs Width Height Format     Video   System  Capabilities\n" );
     direct_log_printf( NULL, "------------------------------------------------------------------------------\n" );

     dfb_core_enum_surfaces( core, surface_callback, &context );

     direct_log_printf( NULL, "                                                ------   ------\n" );
     direct_log_printf( NULL, "                                               %6dk  %6dk   -> %dk total\n",
                        context.mem.video >> 10, (context.mem.system + context.mem.presys) >> 10,
                        (context.mem.video + context.mem.system + context.mem.presys) >> 10);
}

/**********************************************************************************************************************/

static DFBEnumerationResult
alloc_callback( CoreSurfaceAllocation *alloc,
                void                  *ctx )
{
     int                i, index;
     CoreSurface       *surface;
     CoreSurfaceBuffer *buffer;

     D_MAGIC_ASSERT( alloc, CoreSurfaceAllocation );

     buffer  = alloc->buffer;
     D_MAGIC_ASSERT( buffer, CoreSurfaceBuffer );

     surface = buffer->surface;
     D_MAGIC_ASSERT( surface, CoreSurface );

     direct_log_printf( NULL, "%9lu %8d  ", alloc->offset, alloc->size );

     direct_log_printf( NULL, "%4d x %4d   ", surface->config.size.w, surface->config.size.h );

     for (i=0; format_names[i].format; i++) {
          if (surface->config.format == format_names[i].format)
               direct_log_printf( NULL, "%8s ", format_names[i].name );
     }

     index = dfb_surface_buffer_index( alloc->buffer );

     direct_log_printf( NULL, " %-5s ",
                        (dfb_surface_get_buffer( surface, CSBR_FRONT ) == buffer) ? "front" :
                        (dfb_surface_get_buffer( surface, CSBR_BACK  ) == buffer) ? "back"  :
                        (dfb_surface_get_buffer( surface, CSBR_IDLE  ) == buffer) ? "idle"  : "" );

     direct_log_printf( NULL, direct_serial_check(&alloc->serial, &buffer->serial) ? " * " : "   " );

     direct_log_printf( NULL, "%d  %2lu  ", fusion_vector_size( &buffer->allocs ), surface->resource_id );

     if (surface->type & CSTF_SHARED)
          direct_log_printf( NULL, "SHARED  " );
     else
          direct_log_printf( NULL, "PRIVATE " );

     if (surface->type & CSTF_LAYER)
          direct_log_printf( NULL, "LAYER " );

     if (surface->type & CSTF_WINDOW)
          direct_log_printf( NULL, "WINDOW " );

     if (surface->type & CSTF_CURSOR)
          direct_log_printf( NULL, "CURSOR " );

     if (surface->type & CSTF_FONT)
          direct_log_printf( NULL, "FONT " );

     direct_log_printf( NULL, " " );

     if (surface->type & CSTF_INTERNAL)
          direct_log_printf( NULL, "INTERNAL " );

     if (surface->type & CSTF_EXTERNAL)
          direct_log_printf( NULL, "EXTERNAL " );

     direct_log_printf( NULL, " " );

     if (surface->config.caps & DSCAPS_SYSTEMONLY)
          direct_log_printf( NULL, "system only  " );

     if (surface->config.caps & DSCAPS_VIDEOONLY)
          direct_log_printf( NULL, "video only   " );

     if (surface->config.caps & DSCAPS_INTERLACED)
          direct_log_printf( NULL, "interlaced   " );

     if (surface->config.caps & DSCAPS_DOUBLE)
          direct_log_printf( NULL, "double       " );

     if (surface->config.caps & DSCAPS_TRIPLE)
          direct_log_printf( NULL, "triple       " );

     if (surface->config.caps & DSCAPS_PREMULTIPLIED)
          direct_log_printf( NULL, "premultiplied" );

     direct_log_printf( NULL, "\n" );

     return DFENUM_OK;
}

static DFBEnumerationResult
surface_pool_callback( CoreSurfacePool *pool,
                       void            *ctx )
{
     int      length;
     CoreDFB *core = ctx;

     direct_log_printf( NULL, "\n" );
     direct_log_printf( NULL, "--------------------[ Surface Buffer Allocations in %s ]-------------------%n\n", pool->desc.name, &length );
     direct_log_printf( NULL, "Offset    Length   Width Height     Format  Role  Up nA ID  Usage   Type / Storage / Caps\n" );

     while (length--)
          direct_log_printf( NULL, "-" );

     direct_log_printf( NULL, "\n" );

     dfb_surface_pool_enumerate( pool, alloc_callback, core );

     return DFENUM_OK;
}

void
dfbdump_show_surface_pools( CoreDFB *core )
{
     dfb_surface_pools_enumerate( surface_pool_callback, core );
}

/**********************************************************************************************************************/

static DFBEnumerationResult
surface_pool_info_callback( CoreSurfacePool *pool,
                            void            *ctx )
{
     int                    i;
     unsigned long          total = 0;
     CoreSurfaceAllocation *alloc;

     fusion_vector_foreach (alloc, i, pool->allocs)
          total += alloc->size;

     direct_log_printf( NULL, "%-20s ", pool->desc.name );

     switch (pool->desc.priority) {
          case CSPP_DEFAULT:
               direct_log_printf( NULL, "DEFAULT  " );
               break;

          case CSPP_PREFERED:
               direct_log_printf( NULL, "PREFERED " );
               break;

          case CSPP_ULTIMATE:
               direct_log_printf( NULL, "ULTIMATE " );
               break;

          default:
               direct_log_printf( NULL, "unknown  " );
               break;
     }

     direct_log_printf( NULL, "%6lu/%6luk  ", total / 1024, pool->desc.size / 1024 );

     if (pool->desc.types & CSTF_SHARED)
          direct_log_printf( NULL, "* " );
     else
          direct_log_printf( NULL, "  " );


     if (pool->desc.types & CSTF_INTERNAL)
          direct_log_printf( NULL, "INT " );
     else
          direct_log_printf( NULL, "    " );

     if (pool->desc.types & CSTF_EXTERNAL)
          direct_log_printf( NULL, "EXT " );
     else
          direct_log_printf( NULL, "    " );


     if (pool->desc.types & CSTF_LAYER)
          direct_log_printf( NULL, "LAYER " );
     else
          direct_log_printf( NULL, "      " );

     if (pool->desc.types & CSTF_WINDOW)
          direct_log_printf( NULL, "WINDOW " );
     else
          direct_log_printf( NULL, "       " );

     if (pool->desc.types & CSTF_CURSOR)
          direct_log_printf( NULL, "CURSOR " );
     else
          direct_log_printf( NULL, "       " );

     if (pool->desc.types & CSTF_FONT)
          direct_log_printf( NULL, "FONT " );
     else
          direct_log_printf( NULL, "     " );


     for (i=CSAID_CPU; i<=CSAID_GPU; i++) {
          direct_log_printf( NULL, " %c%c%c",
                             (pool->desc.access[i] & CSAF_READ)   ? 'r' : '.',
                             (pool->desc.access[i] & CSAF_WRITE)  ? 'w' : '.',
                             (pool->desc.access[i] & CSAF_SHARED) ? 's' : '.' );
     }

     for (i=CSAID_LAYER0; i<=CSAID_LAYER2; i++) {
          direct_log_printf( NULL, " %c%c%c",
                             (pool->desc.access[i] & CSAF_READ)   ? 'r' : '.',
                             (pool->desc.access[i] & CSAF_WRITE)  ? 'w' : '.',
                             (pool->desc.access[i] & CSAF_SHARED) ? 's' : '.' );
     }

     direct_log_printf( NULL, "\n" );

     return DFENUM_OK;
}

void
dfbdump_show_surface_pool_info( CoreDFB *core )
{
     direct_log_printf( NULL, "\n" );
     direct_log_printf( NULL, "---------------------------------------[ Surface Buffer Pools ]--------------------------------------\n" );
     direct_log_printf( NULL, "Name                 Priority   Used/Capacity S I/E Resource Type Support         CPU GPU Layer 0 - 2\n" );
     direct_log_printf( NULL, "-----------------------------------------------------------------------------------------------------\n" );

     dfb_surface_pools_enumerate( surface_pool_info_callback, NULL );
}

/**********************************************************************************************************************/

typedef struct {
     CoreDFB     *core;
     CoreLayer   *layer;
     int          dump_context;
} ShowLayerContext;

static bool
context_callback( FusionObjectPool *pool,
                  FusionObject     *object,
                  void             *ctx )
{
     DirectResult       ret;
     int                i;
     int                refs;
     int                level;
     ShowLayerContext  *show    = (ShowLayerContext*) ctx;
     CoreLayerContext  *context = (CoreLayerContext*) object;
     CoreLayerRegion   *region  = NULL;
     CoreSurface       *surface = NULL;

     if (object->state != FOS_ACTIVE)
          return true;

     if (context->layer_id != dfb_layer_id( show->layer ))
          return true;

     ret = fusion_ref_stat( &object->ref, &refs );
     if (ret) {
          D_DERROR( ret, "Tools/Dump: Fusion error!\n" );
          return false;
     }

     if (show->dump_context && (show->dump_context < 0 || show->dump_context == object->ref.multi.id)) {
          if (dfb_layer_context_get_primary_region( context, false, &region ) == DFB_OK) {
               if (dfb_layer_region_get_surface( region, &surface ) == DFB_OK) {
                    if (surface->num_buffers) {
                         char buf[32];

                         direct_snprintf( buf, sizeof(buf), "dfb_layer_context_0x%08x", object->ref.multi.id );

                         dfb_surface_dump_buffer( surface, CSBR_FRONT, ".", buf );
                    }

                    dfb_surface_unref( surface );
               }
          }
     }

#if FUSION_BUILD_MULTI
     direct_log_printf( NULL, "0x%08x [%3lx] : ", object->ref.multi.id, object->ref.multi.creator );
#else
     direct_log_printf( NULL, "N/A              : " );
#endif

     direct_log_printf( NULL, "%3d   ", refs );

     direct_log_printf( NULL, "%4d x %4d  ", context->config.width, context->config.height );

     for (i=0; format_names[i].format; i++) {
          if (context->config.pixelformat == format_names[i].format) {
               direct_log_printf( NULL, "%-8s ", format_names[i].name );
               break;
          }
     }

     if (!format_names[i].format)
          direct_log_printf( NULL, "unknown  " );

     direct_log_printf( NULL, "%.1f, %.1f -> %.1f, %.1f   ",
                        context->screen.location.x,  context->screen.location.y,
                        context->screen.location.x + context->screen.location.w,
                        context->screen.location.y + context->screen.location.h );

     direct_log_printf( NULL, "%2d     ", fusion_vector_size( &context->regions ) );

     direct_log_printf( NULL, "%s    ", context->active ? "(*)" : "   " );

     if (context == show->layer->shared->contexts.primary)
          direct_log_printf( NULL, "SHARED   " );
     else
          direct_log_printf( NULL, "PRIVATE  " );

     if (context->rotation)
          direct_log_printf( NULL, "ROTATED %d ", context->rotation);

     if (dfb_layer_get_level( show->layer, &level ))
          direct_log_printf( NULL, "N/A" );
     else
          direct_log_printf( NULL, "%3d", level );

     direct_log_printf( NULL, "\n" );

     return true;
}

void
dfbdump_show_layer_contexts( CoreDFB   *core,
                             CoreLayer *layer,
                             int        dump_context )
{
     ShowLayerContext context;

     context.core         = core;
     context.layer        = layer;
     context.dump_context = dump_context;

     if (fusion_vector_size( &layer->shared->contexts.stack ) == 0)
          return;

     direct_log_printf( NULL, "\n"
                        "----------------------------------[ Contexts of Layer %d ]----------------------------------------\n", dfb_layer_id( layer ));
     direct_log_printf( NULL, "Reference   FID  . Refs  Width Height Format   Location on screen  Regions  Active  Info    Level\n" );
     direct_log_printf( NULL, "-------------------------------------------------------------------------------------------------\n" );

     dfb_core_enum_layer_contexts( core, context_callback, &context );
}

/**********************************************************************************************************************/

static DFBEnumerationResult
window_callback( CoreWindow *window,
                 void       *ctx )
{
     DirectResult      ret;
     int               refs;
     CoreWindowConfig *config = &window->config;
     DFBRectangle     *bounds = &config->bounds;

     ret = fusion_ref_stat( &window->object.ref, &refs );
     if (ret) {
          D_DERROR( ret, "Tools/Dump: Fusion error!\n" );
          return DFENUM_OK;
     }

#if FUSION_BUILD_MULTI
     direct_log_printf( NULL, "0x%08x [%3lx] : ", window->object.ref.multi.id, window->object.ref.multi.creator );
#else
     direct_log_printf( NULL, "N/A              : " );
#endif

     direct_log_printf( NULL, "%3d   ", refs );

     direct_log_printf( NULL, "%4d, %4d   ", bounds->x, bounds->y );

     direct_log_printf( NULL, "%4d x %4d    ", bounds->w, bounds->h );

     direct_log_printf( NULL, "0x%02x ", config->opacity );

     direct_log_printf( NULL, "%5d  ", window->id );

     switch (config->stacking) {
          case DWSC_UPPER:
               direct_log_printf( NULL, "^  " );
               break;
          case DWSC_MIDDLE:
               direct_log_printf( NULL, "-  " );
               break;
          case DWSC_LOWER:
               direct_log_printf( NULL, "v  " );
               break;
          default:
               direct_log_printf( NULL, "?  " );
               break;
     }

     if (window->caps & DWCAPS_ALPHACHANNEL)
          direct_log_printf( NULL, "alphachannel   " );

     if (window->caps & DWCAPS_INPUTONLY)
          direct_log_printf( NULL, "input only     " );

     if (window->caps & DWCAPS_DOUBLEBUFFER)
          direct_log_printf( NULL, "double buffer  " );

     if (config->options & DWOP_GHOST)
          direct_log_printf( NULL, "GHOST          " );

     if (DFB_WINDOW_FOCUSED( window ))
          direct_log_printf( NULL, "FOCUSED        " );

     if (DFB_WINDOW_DESTROYED( window ))
          direct_log_printf( NULL, "DESTROYED      " );

     if (window->config.rotation)
          direct_log_printf( NULL, "ROTATED %d     ", window->config.rotation);

     direct_log_printf( NULL, "\n" );

     return DFENUM_OK;
}

void
dfbdump_show_layer_windows( CoreDFB   *core,
                            CoreLayer *layer )
{
     DFBResult         ret;
     CoreLayerShared  *shared;
     CoreLayerContext *context;
     CoreWindowStack  *stack;

     shared = layer->shared;

     ret = fusion_skirmish_prevail( &shared->lock );
     if (ret) {
          D_DERROR( ret, "DirectFB/Dump: Could not lock the shared layer data!\n" );
          return;
     }

     context = layer->shared->contexts.primary;
     if (!context) {
          fusion_skirmish_dismiss( &shared->lock );
          return;
     }

     stack = dfb_layer_context_windowstack( context );
     if (!stack) {
          fusion_skirmish_dismiss( &shared->lock );
          return;
     }

     dfb_windowstack_lock( stack );

     if (stack->num) {
          direct_log_printf( NULL, "\n"
                  "-----------------------------------[ Windows of Layer %d ]-----------------------------------------\n", dfb_layer_id( layer ) );
          direct_log_printf( NULL, "Reference   FID  . Refs     X     Y   Width Height Opacity   ID     Capabilities   State & Options\n" );
          direct_log_printf( NULL, "--------------------------------------------------------------------------------------------------\n" );

          dfb_wm_enum_windows( stack, window_callback, NULL );
     }

     dfb_windowstack_unlock( stack );

     fusion_skirmish_dismiss( &shared->lock );
}

/**********************************************************************************************************************/

typedef struct {
     CoreDFB     *core;
     int          dump_context;
} ShowLayersContext;

static DFBEnumerationResult
layer_callback( CoreLayer *layer,
                void      *ctx )
{
     ShowLayersContext *context = ctx;

     dfbdump_show_layer_windows( context->core, layer );
     dfbdump_show_layer_contexts( context->core, layer, context->dump_context );

     return DFENUM_OK;
}

void
dfbdump_show_layers( CoreDFB *core,
                     int      dump_context )
{
     ShowLayersContext context;

     context.core         = core;
     context.dump_context = dump_context;

     dfb_layers_enumerate( layer_callback, &context );
}

/**********************************************************************************************************************/

static DirectEnumerationResult
dump_shmpool( FusionSHMPool *pool,
              void          *ctx )
{
     DFBResult     ret;
     SHMemDesc    *desc;
     unsigned int  total = 0;
     int           length;
     FusionSHMPoolShared *shared = pool->shared;

     direct_log_printf( NULL, "\n" );
     direct_log_printf( NULL, "----------------------------[ Shared Memory in %s ]----------------------------%n\n", shared->name, &length );
     direct_log_printf( NULL, "      Size          Address      Offset      Function                     FusionID\n" );

     while (length--)
          direct_log_printf( NULL, "-" );

     direct_log_printf( NULL, "\n" );

     ret = fusion_skirmish_prevail( &shared->lock );
     if (ret) {
          D_DERROR( ret, "Could not lock shared memory pool!\n" );
          return DFENUM_OK;
     }

     if (shared->allocs) {
          direct_list_foreach (desc, shared->allocs) {
               direct_log_printf( NULL, " %9zu bytes at %p [%8lu] in %-30s [%3lx] (%s: %u)\n",
                                  desc->bytes, desc->mem, (ulong)desc->mem - (ulong)shared->heap,
                                  desc->func, desc->fid, desc->file, desc->line );

               total += desc->bytes;
          }

          direct_log_printf( NULL, "   -------\n  %7dk total\n", total >> 10 );
     }

     direct_log_printf( NULL, "\nShared memory file size: %dk\n", shared->heap->size >> 10 );

     fusion_skirmish_dismiss( &shared->lock );

     return DFENUM_OK;
}

void
dfbdump_show_shmpools( FusionWorld *world )
{
     fusion_shm_enum_pools( world, dump_shmpool, NULL );
}

