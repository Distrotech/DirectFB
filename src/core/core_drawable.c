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

#include <stdlib.h>
#include <string.h>

#include <fusion/shmalloc.h>

#include <core/core.h>
#include <core/core_parts.h>
#include <core/core_drawable.h>

#include <direct/memcpy.h>


static CoreDrawable *core_drawable_local = NULL;
static CoreDrawable *core_drawable = NULL;


DFB_CORE_PART( drawable, sizeof(CoreDrawableLocal), sizeof(CoreDrawable) )


static DFBResult
dfb_drawable_initialize( CoreDFB *core, void *data_local, void *data_shared )
{
     D_ASSERT( core_drawable == NULL );
     D_ASSERT( core_drawable_local == NULL );

     core_drawable = data_shared;
     core_drawable_local = data_local;

     core_drawable->shmpool = dfb_core_shmpool( core );

     fusion_skirmish_init( &core_drawable->lock, "Drawable Core", dfb_core_world(core) );

     return DFB_OK;
}

static DFBResult
dfb_drawable_join( CoreDFB *core, void *data_local, void *data_shared )
{
     D_ASSERT( core_drawable == NULL );

     core_drawable = data_shared;

     return DFB_OK;
}

static DFBResult
dfb_drawable_shutdown( CoreDFB *core, bool emergency )
{
     D_ASSERT( core_drawable != NULL );

     fusion_skirmish_destroy( &core_drawable->lock );

     if (core_drawable->data)
          SHFREE( core_drawable->shmpool, core_drawable->data );

     if (core_drawable->name)
          SHFREE( core_drawable->shmpool, core_drawable->name );

     core_drawable = NULL;

     return DFB_OK;
}

static DFBResult
dfb_drawable_leave( CoreDFB *core, bool emergency )
{
     D_ASSERT( core_drawable != NULL );

     core_drawable = NULL;

     return DFB_OK;
}

static DFBResult
dfb_drawable_suspend( CoreDFB *core )
{
     D_ASSERT( core_drawable != NULL );

     return DFB_OK;
}

static DFBResult
dfb_drawable_resume( CoreDFB *core )
{
     D_ASSERT( core_drawable != NULL );

     return DFB_OK;
}


DFBResult
dfb_drawable_set( const char     *name,
                   const void     *data,
                   unsigned int    size)
{
     char *new_mime;
     void *new_data;

     D_ASSERT( core_drawable != NULL );

     D_ASSERT( name != NULL );
     D_ASSERT( data != NULL );
     D_ASSERT( size > 0 );

     new_mime = SHSTRDUP( core_drawable->shmpool, name );
     if (!new_mime)
          return D_OOSHM();

     new_data = SHMALLOC( core_drawable->shmpool, size );
     if (!new_data) {
          SHFREE( core_drawable->shmpool, new_mime );
          return D_OOSHM();
     }

     direct_memcpy( new_data, data, size );

     if (fusion_skirmish_prevail( &core_drawable->lock )) {
          SHFREE( core_drawable->shmpool, new_data );
          SHFREE( core_drawable->shmpool, new_mime );
          return DFB_FUSION;
     }

     if (core_drawable->data)
          SHFREE( core_drawable->shmpool, core_drawable->data );

     if (core_drawable->name)
          SHFREE( core_drawable->shmpool, core_drawable->name );

     core_drawable->name = new_mime;
     core_drawable->data      = new_data;
     core_drawable->size      = size;

     fusion_skirmish_dismiss( &core_drawable->lock );

     return DFB_OK;
}

DFBResult
dfb_drawable_get( char **name, void **data, unsigned int *size )
{
     D_ASSERT( core_drawable != NULL );

     if (fusion_skirmish_prevail( &core_drawable->lock ))
          return DFB_FUSION;

     if (!core_drawable->name || !core_drawable->data) {
          fusion_skirmish_dismiss( &core_drawable->lock );
          return DFB_BUFFEREMPTY;
     }

     if (name)
          *name = strdup( core_drawable->name );

     if (data) {
          *data = malloc( core_drawable->size );
          direct_memcpy( *data, core_drawable->data, core_drawable->size );
     }

     if (size)
          *size = core_drawable->size;

     fusion_skirmish_dismiss( &core_drawable->lock );

     return DFB_OK;
}

DFBResult
dfb_drawable_get_timestamp( struct timeval *timestamp )
{
     D_ASSERT( core_drawable != NULL );
     D_ASSERT( timestamp != NULL );

     if (fusion_skirmish_prevail( &core_drawable->lock ))
          return DFB_FUSION;

     *timestamp = core_drawable->timestamp;

     fusion_skirmish_dismiss( &core_drawable->lock );

     return DFB_OK;
}

