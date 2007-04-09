/*
   (c) Copyright 2006-2007  directfb.org

   All rights reserved.

   Written by Denis Oliver Kropp <dok@directfb.org>
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

#include <unistd.h>

#include <direct/debug.h>
#include <misc/conf.h>
#include <misc/util.h>

#include <direct/debug.h>
#include <direct/mem.h>
#include <direct/messages.h>
#include <direct/modules.h>

#include <fusion/fusion.h>
#include <fusion/shmalloc.h>
#include <core/layer_region.h>
#include <core/core_drawable.h>
#include "drawable/drawable_manager.h"
//#include "idrawable.h"

static int
process_watcher( int   caller,
                 int   call_arg,
                 void *call_ptr,
                 void *ctx );

static DirectResult
register_process( DrawableManager             *manager,
                  DrawableProcessFlags  flags,
                  FusionWorld        *world );

static DirectResult
unregister_process( DrawableManager        *manager,
                    DrawableProcess *process );

/* FIXME: avoid globals use core drawable */
static DrawableManager *m_manager;
static DrawableProcess *m_process;
static FusionWorld     *m_world;


/**********************************************************************************************************************/

#if 0
__attribute__((constructor)) void directfb_register_drawable_manager();

DEFINE_MODULE_DIRECTORY( drawble_modules, "drawable", DFB_DRAWABLE_ABI_VERSION );

                                                            
void
directfb_register_drawable_manager()
{
    direct_modules_register( &drawable_modules,
                              DFB_DRAWABLE_ABI_VERSION,
                              "DrawableManager", &wm_funcs );
}

DirectResult
DrawableCreate(IDirectFBSurface *surface, IDrawable **ret_manager )
{
     DirectResult  ret;
     IDrawable      *manager;

     if (!ret_manager)
          return DFB_INVARG;

     if (!m_manager) {
          D_ERROR( "DrawableCreate: No running Drawable Manager detected!\n" );
          return DFB_NOIMPL;
     }

     D_MAGIC_ASSERT( m_manager, Drawable );
     D_MAGIC_ASSERT( m_process, DrawableProcess );

     DIRECT_ALLOCATE_INTERFACE( manager, IDrawable );

     ret = IDrawable_Construct( manager, m_manager, m_process );
     if (ret)
          return ret;

     *ret_manager = manager;

     return DFB_OK;
}
#endif

DirectResult drawable_manager_attach_surface ( CoreSurface *surface )
{
     return DFB_OK;
}

static int
drawable_manager_call_handler( int   caller,
                      int   call_arg,
                      void *call_ptr,
                      void *ctx )
{
     DirectResult  ret;
     DrawableManager       *manager = ctx;
     DrawableManagerCallID  call   = call_arg;

     D_MAGIC_ASSERT( manager, DrawableManager );

     /* Last mile of dispatch. */
     switch (call) {
          case DMCID_START:
          case DMCID_STOP:
          case DMCID_PROCESS_ADDED:
          case DMCID_PROCESS_REMOVED:
          case DMCID_DRAWABLE_ADDED:
          case DMCID_DRAWABLE_REMOVED:
          break;
     }

     return DFB_NOIMPL;
}

/**********************************************************************************************************************/

DirectResult
drawable_manager_initialize( DrawableManager         *manager,
                   FusionWorld    *world,
                   DrawableProcess **ret_process )
{
     DirectResult       ret;

     D_ASSERT( manager != NULL );
     D_ASSERT( world != NULL );

     D_ASSERT( m_manager == NULL );

     /* Initialize process watcher call. */
     ret = fusion_call_init( &manager->process_watch, process_watcher, manager, world );
     if (ret)
          return ret;

     /* Create shared memory pool. */
     ret = fusion_shm_pool_create( world, "DrawableManager Pool", 0x1000000, true, &manager->shmpool );
     if (ret)
          goto error;

     /* Initialize lock. */
     fusion_skirmish_init( &manager->lock, "DrawableManager", world );

     D_MAGIC_SET( manager, DrawableManager );

     /* Set global singleton. */
     m_manager = manager;
     m_world  = world;

     /* Register ourself as a new process. */
     ret = register_process( manager, DRAWMGR_MASTER, world );
     if (ret) {
          D_MAGIC_CLEAR( manager );
          goto error;
     }

     if (ret_process)
          *ret_process = m_process;

     return DFB_OK;


error:
     fusion_call_destroy( &manager->process_watch );

     m_manager  = NULL;
     m_world   = NULL;
     m_process = NULL;

     return ret;
}

DirectResult
drawable_manager_join( DrawableManager         *manager,
             FusionWorld    *world,
             DrawableProcess **ret_process )
{
     DirectResult ret;

     D_MAGIC_ASSERT( manager, Drawable );
     D_ASSERT( world != NULL );

     D_ASSERT( m_manager == NULL );

     /* Set global singleton. */
     m_manager = manager;
     m_world  = world;

     /* Lock DrawableManager. */
     ret = drawable_manager_lock( manager );
     if (ret)
          goto error;

     /* Register ourself as a new process. */
     ret = register_process( manager, DRAWMGR_NONE, world );

     /* Unlock DrawableManager. */
     drawable_manager_unlock( manager );

     if (ret)
          goto error;

     if (ret_process)
          *ret_process = m_process;

     return DFB_OK;


error:
     m_manager  = NULL;
     m_world   = NULL;
     m_process = NULL;

     return ret;
}

static int
process_watcher( int   caller,
                 int   call_arg,
                 void *call_ptr,
                 void *ctx )
{
     DrawableManager *manager  = ctx;
     DrawableProcess *process;

     D_MAGIC_ASSERT( manager, DrawableManager );

     /* Lookup process by pid. */
     direct_list_foreach (process, manager->processes) {
          D_MAGIC_ASSERT( process, SaWManProcess );

          if (process->pid == call_arg)
               break;
     }

     if (!process) {
          D_BUG( "process with pid %d not found", call_arg );
          return DFB_BUG;
     }

     D_INFO( "SaWMan/Watcher: Process %d [%lu] has exited%s\n", process->pid,
             process->fusion_id, (process->flags & DRAWMGR_EXITING) ? "." : " ABNORMALLY!" );

     unregister_process( manager, process );

     return DFB_OK;
}

static DirectResult
register_process( DrawableManager             *manager,
                  DrawableProcessFlags  flags,
                  FusionWorld        *world )
{
     DirectResult   ret;
     DrawableProcess *process;

     D_MAGIC_ASSERT( manager, DrawableManager );

     /* Allocate process data. */
     process = SHCALLOC( manager->shmpool, 1, sizeof(DrawableProcess) );
     if (!process)
          return D_OOSHM();

     /* Initialize process data. */
     process->pid       = getpid();
     process->fusion_id = fusion_id( world );
     process->flags     = flags;

     /* Initialize reference counter. */
     ret = fusion_ref_init( &process->ref, "Drawable Process", world );
     if (ret) {
          D_DERROR( ret, "Drawable/Register: fusion_ref_init() failed!\n" );
          goto error_ref;
     }

     /* Add a local reference. */
     ret = fusion_ref_up( &process->ref, false );
     if (ret) {
          D_DERROR( ret, "Drawable/Register: fusion_ref_up() failed!\n" );
          goto error;
     }

     /* Set the process watcher on this. */
     ret = fusion_ref_watch( &process->ref, &manager->process_watch, process->pid );
     if (ret) {
          D_DERROR( ret, "Drawable/Register: fusion_ref_watch() failed!\n" );
          goto error;
     }

     D_MAGIC_SET( process, DrawableProcess );

     /* Add process to list. */
     direct_list_append( &manager->processes, &process->link );

     /* Call application manager executable. */
     drawable_manager_call( manager, DMCID_PROCESS_ADDED, process );

     /* Set global singleton. */
     m_process = process;

     return DFB_OK;


error:
     fusion_ref_destroy( &process->ref );

error_ref:
     SHFREE( manager->shmpool, process );

     return ret;
}

static DirectResult
unregister_process( DrawableManager        *manager,
                    DrawableProcess *process )
{
     D_MAGIC_ASSERT( manager, DrawableManager );
     D_MAGIC_ASSERT( process, DrawableProcess );

     /* Destroy reference counter. */
     fusion_ref_destroy( &process->ref );

     /* Remove process from list. */
     direct_list_remove( &manager->processes, &process->link );

     /* Unregister manager process? */
     if (process->flags & DRAWMGR_MANAGER) {
          D_ASSERT( manager->manager.present );

          /* Destroy manager call, unless it was another process. */
          if (m_process == process)
               fusion_call_destroy( &manager->manager.call );
          else
               manager->manager.call.handler = NULL;    /* FIXME: avoid failing assertion in fusion_call_init() */

          /* Ready for new manager. */
          manager->manager.present = false;
     }
     else {
          /* Call application manager executable. */
          drawable_manager_call( manager, DMCID_PROCESS_REMOVED, process );
     }

     D_MAGIC_CLEAR( process );

     /* Deallocate process data. */
     SHFREE( manager->shmpool, process );

     return DFB_OK;
}
DirectResult
drawable_manager_shutdown( DrawableManager *manager,
                 FusionWorld *world )
{
     DirectResult   ret;
     DrawableProcess *process;

     D_MAGIC_ASSERT( manager, DrawableManager );
     D_ASSERT( world != NULL );

     D_ASSERT( m_managerManager == manager );
     D_ASSERT( m_world == world );

     D_ASSERT( manager->processes != NULL );
     D_ASSERT( manager->processes->next == NULL );

     process = (DrawableProcess*) manager->processes;

     D_ASSERT( process == m_process );
     D_ASSERT( process->fusion_id == fusion_id( world ) );

     /* Lock DrawableManager. */
     ret = drawable_manager_lock( manager );
     if (ret)
          return ret;

     /* Shutdown our own process. */
     unregister_process( manager, process );

     /* Clear global singleton. */
     m_process = NULL;

     /* Destroy process watcher call. */
     fusion_call_destroy( &manager->process_watch );

     D_ASSERT( manager->processes == NULL );
     D_ASSERT( !manager->manager.present );

     /* Destroy lock. */
     fusion_skirmish_destroy( &manager->lock );

     D_MAGIC_CLEAR( manager );

     /* Destroy shared memory pool. */
     fusion_shm_pool_destroy( world, manager->shmpool );

     /* Clear global singleton. */
     m_manager = NULL;
     m_world  = NULL;

     return DFB_OK;
}

DirectResult
drawable_manager_leave( DrawableManager      *manager,
              FusionWorld *world )
{
     D_MAGIC_ASSERT( manager, Drawable );
     D_ASSERT( world != NULL );

     D_ASSERT( m_manager == manager );
     D_ASSERT( m_world == world );
     D_MAGIC_ASSERT( m_process, DrawableProcess );

     /* Set 'cleanly exiting' flag. */
     m_process->flags |= DRAWMGR_EXITING;

     /* Clear global singletons. */
     m_manager  = NULL;
     m_world   = NULL;
     m_process = NULL;

     return DFB_OK;
}
// vim: ts=5 sw=5 et
