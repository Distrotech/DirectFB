/*
 * (c) Copyright 2004-2006 Mitsubishi Electric Corp.
 *
 * All rights reserved.
 *
 * Written by Koichi Hiramatsu,
 *            Seishi Takahashi,
 *            Atsushi Hori
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef __DIRECT__INTERFACE_ARIB_H__
#define __DIRECT__INTERFACE_ARIB_H__

#include <directfb_arib.h>

#define DIRECT_ALLOCATE_CHILD_INTERFACE_DATA(p,i)                                    \
     i##_data *data;                                                                 \
                                                                                     \
     D_MAGIC_ASSERT( (IAny*)(p), DirectInterface );                                  \
                                                                                     \
     if (!(p)->parent.priv)                                                          \
          (p)->parent.priv = D_CALLOC( 1, sizeof(i##_data) );                        \
                                                                                     \
     data = (i##_data*)((p)->parent.priv);


#define DIRECT_DEALLOCATE_CHILD_INTERFACE(p)                                         \
     direct_dbg_interface_remove( __FUNCTION__, __FILE__, __LINE__, #p, p );         \
                                                                                     \
     if ((p)->parent.priv) {                                                         \
          D_FREE( (p)->parent.priv );                                                \
          (p)->parent.priv = NULL;                                                          \
     }                                                                               \
                                                                                     \
     D_MAGIC_CLEAR( (IAny*)(p) );                                                    \
                                                                                     \
     D_FREE( (p) );


#define DIRECT_CHILD_INTERFACE_GET_DATA(i)                                           \
     i##_data *data;                                                                 \
                                                                                     \
     if (!thiz)                                                                      \
          return DFB_THIZNULL;                                                       \
                                                                                     \
     D_MAGIC_ASSERT( (IAny*)thiz, DirectInterface );                                 \
                                                                                     \
     data = (i##_data*) thiz->parent.priv;                                           \
                                                                                     \
     if (!data)                                                                      \
          return DFB_DEAD;

#endif

