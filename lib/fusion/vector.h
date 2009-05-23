/*
   (c) Copyright 2001-2008  The world wide DirectFB Open Source Community (directfb.org)
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

#ifndef __FUSION__VECTOR_H__
#define __FUSION__VECTOR_H__

#include <fusion/types.h>

#include <direct/debug.h>

typedef struct {
     int    magic;

     void **elements;
     int    count;

     int    capacity;

     FusionSHMPoolShared *pool;
} FusionVector;


#if D_DEBUG_ENABLED
#define FUSION_VECTOR_ASSERT( vector )                                          \
     do {                                                                       \
          D_MAGIC_ASSERT( vector, FusionVector );                               \
          D_ASSERT( (vector)->count >= 0 );                                     \
          D_ASSERT( (vector)->count <= (vector)->capacity );                    \
          D_ASSERT( (vector)->capacity > 0 );                                   \
          D_ASSERT( (vector)->elements != NULL || (vector)->count == 0 );       \
     } while (0)
#else
#define FUSION_VECTOR_ASSERT( vector )                                          \
     do {                                                                       \
     } while (0)
#endif


void         fusion_vector_init       ( FusionVector        *vector,
                                        int                  capacity,
                                        FusionSHMPoolShared *pool );

void         fusion_vector_destroy    ( FusionVector        *vector );

DirectResult fusion_vector_add        ( FusionVector        *vector,
                                        void                *element );

DirectResult fusion_vector_insert     ( FusionVector        *vector,
                                        void                *element,
                                        int                  index );

DirectResult fusion_vector_move       ( FusionVector        *vector,
                                        int                  from,
                                        int                  to );

DirectResult fusion_vector_remove     ( FusionVector        *vector,
                                        int                  index );

DirectResult fusion_vector_remove_last( FusionVector        *vector );

DirectResult fusion_vector_remove_with( FusionVector        *vector,
                                        void                *element );


static inline bool
fusion_vector_has_elements( const FusionVector *vector )
{
     FUSION_VECTOR_ASSERT( vector );

     return vector->count > 0;
}

static inline bool
fusion_vector_is_empty( const FusionVector *vector )
{
     FUSION_VECTOR_ASSERT( vector );

     return vector->count == 0;
}

static inline int
fusion_vector_size( const FusionVector *vector )
{
     FUSION_VECTOR_ASSERT( vector );

     return vector->count;
}

static inline void *
fusion_vector_at( const FusionVector *vector, int index )
{
     FUSION_VECTOR_ASSERT( vector );
     D_ASSERT( index >= 0 );
     D_ASSERT( index < vector->count );

     return vector->elements[index];
}

static inline bool
fusion_vector_contains( const FusionVector *vector, const void *element )
{
     int           i;
     int           count;
     void * const *elements;

     FUSION_VECTOR_ASSERT( vector );
     D_ASSERT( element != NULL );

     count    = vector->count;
     elements = vector->elements;

     /* Start with more recently added elements. */
     for (i=count-1; i>=0; i--)
          if (elements[i] == element)
               return true;

     return false;
}

static inline int
fusion_vector_index_of( const FusionVector *vector, const void *element )
{
     int i;

     FUSION_VECTOR_ASSERT( vector );
     D_ASSERT( element != NULL );

     /* Start with more recently added elements. */
     for (i=vector->count-1; i>=0; i--)
          if (vector->elements[i] == element)
               return i;

     /*
      * In case the return value isn't checked
      * this will most likely generate a bad address.
      */
     return INT_MIN >> 2;
}


#define fusion_vector_foreach(element, index, vector)                                               \
     for ((index) = 0;                                                                              \
          (index) < (vector).count && (element = (typeof(element))(vector).elements[index]);        \
          (index)++)

#define fusion_vector_foreach_reverse(element, index, vector)                                       \
     for ((index) = (vector).count - 1;                                                             \
          (index) >= 0 && (element = (typeof(element))(vector).elements[index]);                    \
          (index)--)

#define fusion_vector_foreach_remove(element, vector)                                               \
     while ((vector).count > 0 && (element = (typeof(element))(vector).elements[--(vector).count]))

#endif

