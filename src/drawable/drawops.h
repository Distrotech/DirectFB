/*
   (c) Copyright 2001-2007  directfb.org
   (c) Copyright 2000-2004  convergence (integrated) media GmbH.

   All rights reserved.

   Written by Denis Oliver Kropp <dok@directfb.org>,
              Andreas Hundt <andi@fischlustig.de>,
              Sven Neumann <neo@directfb.org> ,
              Ville Syrjälä <syrjala@sci.fi> and ,
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

#ifndef __DRAWABLE_DRAWOP_H__
#define __DRAWABLE_DRAWOP_H__

#include <directfb.h>
#include <direct/types.h>
#include "core/surfaces.h"
#include "drawable/drawable_types.h"

struct __Drawable_DrawStyle {
  bool  hidden; 
  char* fill;     
  char* stroke;     
  char* stroke_width;     
};

#define DRAW_BASE \
     DirectResult (*draw) (DrawOp *thiz);

struct __Drawable_DrawOp {
     DRAW_BASE
};


struct __Drawable_DrawListOp {
     DRAW_BASE
     DirectLink     *ops;
};

struct __Drawable_DrawTreeOp {
     DRAW_BASE;
     DrawTreeOp *parent;
     DrawOp *thisOp; /*if non zero draws itself*/
     DrawTreeOp *firstChild;
     DrawTreeOp *lastChild;
     DrawTreeOp *prevSibling;
};

struct __Drawable_DrawRectOp {
     DRAW_BASE
     double x;
     double y;
     double width;
     double height;
     DrawStyle style;
};


struct __Drawable_DrawSurfaceOp  {
     DRAW_BASE
     DrawListOp* oplist;
     CoreSurface* surface;
};

/*Generic dynamic op ? */
DirectResult  drawop_create( char* name, char *data, DrawOp** ret_op); 

DirectResult  drawop_create_surface( CoreSurface* dest, DrawSurfaceOp** ret_op );

DirectResult  drawop_create_rect( double x,
                                          double y,
                                          double w,
                                          double h,
                                          DrawStyle* style,
                                          DrawRectOp** ret_op );

DirectResult  drawop_create_list_op( DrawListOp** ret_op );


#endif
// vim: ts=5 sw=5 et

