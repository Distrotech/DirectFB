/*
 * (c) Copyright 2004-2006 Mitsubishi Electric Corp.
 *
 * All rights reserved.
 *
 * Written by Toshiyuki Takahashi,
 *            Koichi Hiramatsu,
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

#ifndef __GLYPH_UTIL_H__
#define __GLYPH_UTIL_H__

#include <dfb_types.h>

void dfb_copy_glyph_a2(__u8* sbuf, int spitch,
                       __u8* dbuf, int dpitch,
                       int w, int h);

void dfb_scaling_copy_glyph_a2(__u8* sbuf, int spitch,
                               __u8* dbuf, int dpitch,
                               int w, int h,
                               int horizontal,
                               int vertical
                               );

void dfb_draw_glyph_a2(__u8* sbuf, int spitch,
                       __u8* dbuf, int dpitch,
                       int w, int h, int palette[]);

void dfb_bold_glyph_a2(__u8* sbuf, int spitch,
                       __u8* dbuf, int dpitch,
                       int	 w  , int h);

void dfb_frame_glyph_a2(__u8* sbuf, int spitch,
                        __u8* dbuf, int dpitch,
                        int	 w,    int h);

void dfb_draw_framed_glyph_a2(__u8* sbuf, int spitch,
                              __u8* dbuf, int dpitch,
                              int w, int h, int palette[], int frame_color);

#endif /* __GLYPH_UTIL_H__ */
