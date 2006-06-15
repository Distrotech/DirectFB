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

#include <config.h>

#include <stdlib.h>
#include <string.h>

#include <pthread.h>

#include <directfb_arib.h>

#include <core/core.h>
#include <core/coredefs.h>
#include <core/coretypes.h>

#include <core/aribfonts.h>
/* #include <core/gfxcard.h> */
/* #include <core/surfaces.h> */

#include <direct/mem.h>
/* #include <direct/messages.h> */
/* #include <direct/tree.h> */
#include <direct/util.h>

/* #include <misc/conf.h> */
/* #include <misc/util.h> */

CoreARIBFont *
dfb_aribfont_create( CoreDFB *core )
{
     CoreARIBFont *font;

     font = (CoreARIBFont *) D_CALLOC( 1, sizeof(CoreARIBFont) );
     font->core = core;
     direct_util_recursive_pthread_mutex_init( &font->lock );

     /* ARIB Font is always represented as 2bpp greyscale (for now) */
     font->pixel_format = DSPF_A2;

     D_MAGIC_SET( font, CoreARIBFont );
     return font;
}

void
dfb_aribfont_destroy( CoreARIBFont *font )
{

     D_MAGIC_ASSERT( font, CoreARIBFont );
     D_MAGIC_CLEAR( font );
     pthread_mutex_lock( &font->lock );
     /* if there is something to protect in the future, finalize it here. */
     pthread_mutex_unlock( &font->lock );
     pthread_mutex_destroy( &font->lock );

     D_FREE( font );
}


DFBResult
dfb_aribfont_get_glyph_extents( CoreARIBFont *font,
                                DFBARIBFontDesc *desc,
                                DFBARIBFontOptions options,
                                __u16 jiscode, /* 0x00XX: JIS0201, else: JIS0208 */
                                DFBRectangle *ret_rect,
                                int* ret_buflen)

{
     DFBResult      ret;

     D_MAGIC_ASSERT( font, CoreARIBFont );
     D_ASSERT( desc != NULL );
     D_ASSERT( ret_rect != NULL );

     if (font->GetGlyphExtents) {
          ret = font->GetGlyphExtents(font, desc, options, jiscode, ret_rect, ret_buflen);
     }
     else {
          ret = DFB_UNSUPPORTED;
     }
     return ret;
}

DFBResult
dfb_aribfont_get_glyph_image( CoreARIBFont *font,
                              DFBARIBFontDesc *desc,
                              DFBARIBFontOptions options,
                              __u16 jiscode, /* 0x00XX: JIS0201, else: JIS0208 */
                              void* bufptr, int buflen,
                              DFBRectangle *ret_rect)
{
     DFBResult      ret;

     D_MAGIC_ASSERT( font, CoreARIBFont );
     D_ASSERT( desc != NULL );
     D_ASSERT( bufptr != NULL );
     D_ASSERT( ret_rect != NULL );


     if (font->GetGlyphImage) {
          ret = font->GetGlyphImage(font, desc, options, jiscode, bufptr, buflen, ret_rect);
     }
     else {
          ret = DFB_UNSUPPORTED;
     }
     return ret;
}

