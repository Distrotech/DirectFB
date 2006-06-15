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

#ifndef __ARIBFONTS_H__
#define __ARIBFONTS_H__

#include <pthread.h>

/* #include <fusion/lock.h> */

#include <directfb.h>

#include <core/coretypes.h>

/* #include <core/state.h> */

#include <direct/debug.h>

/*
 * arib font struct
 */

struct _CoreARIBFont {
     int                      magic;
     DFBSurfacePixelFormat    pixel_format; /* fixed to DSPF_A2 for now */
     CoreDFB         *core;
     pthread_mutex_t  lock;             /* lock during access to the font   */
     void            *impl_data;        /* a pointer used by the impl.      */

     DFBResult   (* GetGlyphExtents) (CoreARIBFont *thiz,
                                      DFBARIBFontDesc *desc,
                                      DFBARIBFontOptions options,
                                      __u16 jiscode, /* 0x00XX: JIS0201, else: JIS0208 */
                                      DFBRectangle *ret_rect,
                                      int* ret_buflen);
     DFBResult   (* GetGlyphImage) (CoreARIBFont *thiz,
                                    DFBARIBFontDesc *desc,
                                    DFBARIBFontOptions options,
                                    __u16 jiscode, /* 0x00XX: JIS0201, else: JIS0208 */
                                    void* bufptr, int buflen,
                                    DFBRectangle *ret_rect);
};

/*
 * allocates and initializes a new font structure
 */
CoreARIBFont *dfb_aribfont_create( CoreDFB *core );

/*
 * destroy all data in the CoreFont struct
 */
void dfb_aribfont_destroy( CoreARIBFont *font );

/*
 * lock the font before accessing it
 */
static inline void
dfb_aribfont_lock( CoreARIBFont *font )
{
     D_MAGIC_ASSERT( font, CoreARIBFont );

     pthread_mutex_lock( &font->lock );

/*     dfb_state_lock( &font->state ); */
}

/*
 * unlock the font after access
 */
static inline void
dfb_aribfont_unlock( CoreARIBFont *font )
{
     D_MAGIC_ASSERT( font, CoreARIBFont );

/*     dfb_state_unlock( &font->state ); */

     pthread_mutex_unlock( &font->lock );
}

/*
 * get a glyph extents from font
 */
DFBResult dfb_aribfont_get_glyph_extents(CoreARIBFont *thiz,
                                         DFBARIBFontDesc *desc,
                                         DFBARIBFontOptions options,
                                         __u16 jiscode, /* 0x00XX: JIS0201, else: JIS0208 */
                                         DFBRectangle *ret_rect,
                                         int* ret_buflen);

/*
 * get a glyph image from font
 */
DFBResult dfb_aribfont_get_glyph_image(CoreARIBFont *thiz,
                                       DFBARIBFontDesc *desc,
                                       DFBARIBFontOptions options,
                                       __u16 jiscode, /* 0x00XX: JIS0201, else: JIS0208 */
                                       void* bufptr, int buflen,
                                       DFBRectangle *ret_rect);
#endif
