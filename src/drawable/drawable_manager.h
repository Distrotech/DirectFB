/*
   (c) Copyright 2006-2007  directfb.org

   All rights reserved.

   Written by Denis Oliver Kropp <dok@directfb.org>.

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

#ifndef __DRAWABLE_MANAGER_H__
#define __DRAWABLE_MANAGER_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <directfb_util.h>

#include <direct/list.h>
#include <direct/interface.h>

#include <fusion/call.h>
#include <fusion/lock.h>
#include <fusion/ref.h>
#include <fusion/vector.h>
#include <direct/modules.h>
#include "drawable_types.h"

#define DFB_DRAWABLE_ABI_VERSION        1 

DECLARE_MODULE_DIRECTORY( dfb_core_drawabe_modules );

typedef enum {
     DRAWMGR_NONE     = 0x00000000,

     DRAWMGR_MASTER   = 0x00000001,
     DRAWMGR_MANAGER  = 0x00000002,

     DRAWMGR_EXITING  = 0x00000010,

     DRAWMGR_ALL      = 0x00000013
} DrawableProcessFlags;


typedef enum {
     DMCID_START,
     DMCID_STOP,
     DMCID_PROCESS_ADDED,
     DMCID_PROCESS_REMOVED,
     DMCID_DRAWABLE_ADDED,
     DMCID_DRAWABLE_REMOVED
} DrawableManagerCallID;

struct __Drawable_DrawableManager {
     int                   magic;

     FusionSkirmish        lock;
     CoreSurface          *surface;
     FusionSHMPoolShared  *shmpool;
     DirectLink           *processes;
     DirectLink           *drawables;
     FusionCall            process_watch;
     struct {
          bool                      present;
          FusionCall                call;
          void                     *context;
     } manager;
};


struct __Drawable_DrawableProcess {
     DirectLink             link;

     int                    magic;

     pid_t                  pid;
     FusionID               fusion_id;
     DrawableProcessFlags     flags;

     FusionRef              ref;
};

struct __Drawable_DrawableManagerDrawable {
     DirectLink             link;

     int                    magic;

     DrawableManager       *drawable_manager;
     FusionSHMPoolShared   *shmpool;

     DrawableProcess       *process;

     DFBDrawableID         id;

};

DirectResult drawable_manager_attach_surface ( CoreSurface *surface );

DirectResult drawable_manager_initialize( DrawableManager         *manager,
                                FusionWorld    *world,
                                DrawableProcess **ret_process );

DirectResult drawable_manager_join      ( DrawableManager         *manager,
                                FusionWorld    *world,
                                DrawableProcess **ret_process );

DirectResult drawable_manager_shutdown  ( DrawableManager         *manager,
                                FusionWorld    *world );

DirectResult drawable_manager_leave     ( DrawableManager         *manager,
                                FusionWorld    *world );


DirectResult drawable_manager_call      ( DrawableManager         *manager,
                                DrawableManagerCallID    call,
                                void           *ptr );


DirectResult drawable_manager_register       ( DrawableManager                *manager,
                                     void                  *context );

static inline DirectResult
drawable_manager_lock( DrawableManager *manager )
{
     D_MAGIC_ASSERT( manager, DrawableManager );

     return fusion_skirmish_prevail( &manager->lock );
}

static inline DirectResult
drawable_manager_unlock( DrawableManager *manager )
{
     D_MAGIC_ASSERT( manager, DrawableManager );

     return fusion_skirmish_dismiss( &manager->lock );
}

#ifdef __cplusplus
}
#endif

#endif
// vim: ts=5 sw=5 et
