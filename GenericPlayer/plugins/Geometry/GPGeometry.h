/*
 * Copyright (C) 2004, 2006 Apple Computer, Inc.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE COMPUTER, INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE COMPUTER, INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 */

#ifndef GPGeometry_h
#define GPGeometry_h
#include "GPRuntime.h"

#ifdef __cplusplus
extern "C" {
#endif

#define GPGeometryDef "\
\
gp.Geometry.Plugin(\
    gp.Geometry.Plugin.function(function)\
    gp.Geometry.Plugin.impl(void*))\
\
    gp.Geometry.Plugin.PluginFunc({gp.Geometry.Plugin}\
            gp.GPPlugin/GPPluginFunc))\
\
    gp.Geometry.Plugin.DestroyFunc({gp.Geometry.Plugin}\
            gp.Geometry.Plugin.DestroyFunc.plugin(gp.Geometry.Plugin*))\
\
gp.Geometry.Point( \
    gp.Geometry.Point.x(int)\
    gp.Geometry.Point.y(int))\
\
gp.Geometry.Size( \
    gp.Geometry.Size.width(int)\
    gp.Geometry.Size.height(int))\
\
gp.Geometry.Rectangle( \
    gp.Geometry.Rectangle.location(gp.Geometry.Point)\
    gp.Geometry.Rectangle.size(gp.Geometry.Size))\
\
"

#define GP_GEOMETRY_MIMETYPE "utility/geometry"


typedef struct {
    int x;
    int y;
} gp_Geometry_Point;

typedef struct {
    int width;
    int height;
} gp_Geometry_Size;

typedef struct {
    gp_Geometry_Point location;
    gp_Geometry_Size size;
} gp_Geometry_Rectangle;


#ifdef __cplusplus
}  /* end extern "C" */
#endif

#endif /*GPGeometry_h*/
