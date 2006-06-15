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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>


#include "directfb_arib.h"

#include "core/coretypes.h"

#include "core/aribfonts.h"

#include "idirectfbaribfont.h"

#include <direct/interface.h>
#include <direct/mem.h>
#include <direct/tree.h>
#include <direct/utf8.h>

#include "misc/util.h"
#include "misc/glyph_util.h"

/*-----------------------
 * utility functions
 *-----------------------
 */
static int
eucjp2jis(const __u8 *p, __u16* jis)
{
     if ((p[0] >= 0x20) && (p[0] <= 0x7E)) { /* JIS0201 + ascii space */
          *jis = p[0]; return 1;
     }
     else if (((p[0] >= 0xA1) && (p[0] <= 0xFE)) && ((p[1] >= 0xA1) && (p[1] <= 0xFE))) {
          /* JIS0208 */
          *jis = ((p[0] & 0x7F) << 8) | (p[1] & 0x7F); return 2;
     }
     else if ((p[0] == 0x8E) && ((p[1] >= 0xA1) && (p[1] <= 0xDF))) {
          /* JIS0201 upper half */
          *jis = p[1]; return 2;
     }
     else if ((p[0] == 0x8F) && (((p[1] >= 0xA1) && (p[1] <= 0xFE)) && ((p[2] >= 0xA1) && (p[2] <= 0xFE)))) {
          /* JIS0208 not supported */
          *jis = 0x2129; /* substitute character (?) */
          return 3;
     }

     /* violation */
     *jis = (__u16) -1; return 1;

}

/*----------------------------------------------------------------------*/
void
IDirectFBARIBFont_Destruct( IDirectFBARIBFont *thiz )
{
     IDirectFBARIBFont_data *data = (IDirectFBARIBFont_data*)thiz->priv;

     dfb_aribfont_destroy (data->font);

     DIRECT_DEALLOCATE_INTERFACE( thiz );
}

/*
 * increments reference count of font
 */
static DFBResult
IDirectFBARIBFont_AddRef( IDirectFBARIBFont *thiz )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBARIBFont)

     data->ref++;

     return DFB_OK;
}

/*
 * decrements reference count, destructs interface data if reference count is 0
 */
static DFBResult
IDirectFBARIBFont_Release( IDirectFBARIBFont *thiz )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBARIBFont)

     if (--data->ref == 0)
          IDirectFBARIBFont_Destruct( thiz );

     return DFB_OK;
}

/*
 * Set an ARIB DRCS manager to the ARIB FontProvider.
 */
static DFBResult
IDirectFBARIBFont_SetARIBDrcs(IDirectFBARIBFont *thiz,
                              IDirectFBARIBDrcs *drcs)
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBARIBFont);

     data->aribdrcs = drcs;

     return DFB_OK;
}

/** String extents measurement **/

/*
 * Get the extents of the specified EUC string
 */
static DFBBoolean
isDrcs(__u16 jiscode) {
     __u8 upper = (jiscode & 0xFF00) >> 8;
     __u8 lower = jiscode & 0x00FF;
     if ((upper != 0x77) && /* 87 ku */
         (upper != 0x78))   /* 88 ku */
          return DFB_FALSE; /* not DRCS code area */
     if ((lower < 0x21) || (lower > 0x7E))
          return DFB_FALSE; /* not DRCS code area */
     return DFB_TRUE; /* DRCS code area */
}

static DFBResult
IDirectFBARIBFont_GetEUCStringExtents(IDirectFBARIBFont   *thiz,
                                      DFBARIBFontDesc     *desc,
                                      const char          *euc_text,
                                      int                  bytes,
                                      int                  letter_space,
                                      DFBRectangle        *ret_rect)
{
     CoreARIBFont  *font;
     const __u8    *string;
     const __u8    *end;
     __u16          jiscode;
     int            skip;
     DFBRectangle   glyph_rect;
     DFBARIBFontOptions options;
     DFBRegion      region = {0,0,0,0};

     DIRECT_INTERFACE_GET_DATA(IDirectFBARIBFont)

     if (!desc || !euc_text || !ret_rect)
          return DFB_INVARG;
     if (bytes < 0)
          bytes = strlen(euc_text);
     if (!bytes) {
          ret_rect->x = ret_rect->y = ret_rect->w = ret_rect->h = 0;
          return DFB_OK;
     }

     font   = data->font;
     string = euc_text;
     end    = string + bytes;

     options = DARIBFOP_NORMAL;

     dfb_aribfont_lock( font );

     do {
          skip = eucjp2jis(string, &jiscode);
          if (jiscode & 0x8000) { /* code violation */
               string += skip; continue;
          }

          if ((isDrcs(jiscode) &&
               data->aribdrcs &&
               (data->aribdrcs->GetGlyphExtents(data->aribdrcs, desc, options, jiscode, &glyph_rect) == DFB_OK)) ||

              (dfb_aribfont_get_glyph_extents(font, desc, options, jiscode, &glyph_rect, NULL) == DFB_OK)) {
               if (region.x2 + glyph_rect.x < region.x1)
                    region.x1 = region.x2 + glyph_rect.x;
               if (glyph_rect.x + glyph_rect.w > 0)
                    region.x2 += glyph_rect.x + glyph_rect.w;
               if (glyph_rect.y < region.y1)
                    region.y1 = glyph_rect.y;
               if (region.y2 < glyph_rect.y + glyph_rect.h)
                    region.y2 = glyph_rect.y + glyph_rect.h;

               if (letter_space > 0) {
                    region.x2 += letter_space;
               }
          }

          string += skip;
     } while (string < end);

     if ((letter_space > 0) && (region.x2 != 0)) {
          region.x2 -= letter_space;
     }

     dfb_aribfont_unlock(font);

     ret_rect->x = region.x1;
     ret_rect->y = region.y1;
     ret_rect->w = region.x2 - region.x1;
     ret_rect->h = region.y2 - region.y1;

     return DFB_OK;
}

/** Drawing control **/

/*
 * Set the color palette used for drawing text function.
 */

static DFBResult
IDirectFBARIBFont_SetGrayLevelPalette(IDirectFBARIBFont *thiz,
                                      int *palette,
                                      int level)
{

     DIRECT_INTERFACE_GET_DATA(IDirectFBARIBFont)

     if (!palette || (level < 0))
          return DFB_INVARG;

     if (data->graylevel != level) {
          if (data->palette) {
               D_FREE(data->palette);
               data->palette = NULL;
          }
          data->graylevel = level;
          if (level > 0) {
               data->palette = (int*)D_MALLOC(level * sizeof(int));
               if (data->palette == NULL) {
                    return DFB_NOSYSTEMMEMORY;
               }
          }
     }

     if (level > 0) {
          memcpy(data->palette, palette, level * sizeof(int));
     }

     return DFB_OK;
}

/** Text functions **/

/*
 * Draw an EUC string at the specified buffer area with the given gray-level palette
 */
static DFBResult
IDirectFBARIBFont_DrawEUCString(IDirectFBARIBFont *thiz,
                                DFBARIBFontDesc   *desc,
                                const char        *euc_text,
                                int                bytes,
                                int                letter_space,
                                void*              buf_ptr,
                                int                pitch)
{
     CoreARIBFont  *font;
     const __u8    *string;
     const __u8    *end;
     __u16          jiscode;
     int            skip;

     DFBARIBFontOptions options;
     DFBRectangle   glyph_rect;
     int            glyph_pitch;
     int            ret_buflen;

     __u8          *glyphBuf = NULL;
     int            glyphBufLen = 0;
     int           *palette;

     DIRECT_INTERFACE_GET_DATA(IDirectFBARIBFont)

     if (!desc || !euc_text || !buf_ptr)
          return DFB_INVARG;
     if (bytes < 0)
          bytes = strlen(euc_text);
     if (!bytes) {
          return DFB_OK;
     }
     if (data->graylevel >= 4) {
          palette = data->palette;
     }
     else {
          D_WARN(" DirecFB/IDirectFBARIBFont:"
                 " palette is empty or insufficient.\n");
          return DFB_OK;
     }

     font   = data->font;
     string = euc_text;
     end    = string + bytes;

     options = DARIBFOP_NORMAL;

     dfb_aribfont_lock( font );

     do {
          skip = eucjp2jis(string, &jiscode);
          if (jiscode & 0x8000) { /* code violation */
               string += skip; continue;
          }

          if (isDrcs(jiscode) &&
              data->aribdrcs &&
              (data->aribdrcs->GetGlyphExtents(data->aribdrcs, desc, options, jiscode, &glyph_rect) == DFB_OK)) {
               glyph_pitch = (2 * glyph_rect.w + 7)/8;
               ret_buflen = glyph_pitch * glyph_rect.h;
               if ((ret_buflen > glyphBufLen) && (glyphBuf != NULL)) {
                    D_FREE(glyphBuf); glyphBuf = NULL;
               }
               if (glyphBuf == NULL) {
                    glyphBufLen = ret_buflen;
                    glyphBuf = (__u8*)D_CALLOC(1,glyphBufLen);
               }
               else {
                    memset(glyphBuf, 0, glyphBufLen);
               }

               if (data->aribdrcs->GetGlyphImage(data->aribdrcs, desc, options, jiscode,
                                                 glyphBuf, glyph_pitch) == DFB_OK) {

                    dfb_draw_glyph_a2(glyphBuf, glyph_pitch,
                                  buf_ptr + pitch * glyph_rect.y + glyph_rect.x,
                                  pitch,
                                  glyph_rect.w, glyph_rect.h, palette);

                    buf_ptr += glyph_rect.x + glyph_rect.w;
                    if (letter_space > 0) {
                         buf_ptr += letter_space;
                    }
               }
          }
          else if (dfb_aribfont_get_glyph_extents(font, desc, options, jiscode, &glyph_rect, &ret_buflen) == DFB_OK) {
               glyph_pitch = ret_buflen / glyph_rect.h;
               if ((ret_buflen > glyphBufLen) && (glyphBuf != NULL)) {
                    D_FREE(glyphBuf); glyphBuf = NULL;
               }
               if (glyphBuf == NULL) {
                    glyphBufLen = ret_buflen;
                    glyphBuf = (__u8*)D_CALLOC(1,glyphBufLen);
               }
               else {
                    memset(glyphBuf, 0, glyphBufLen);
               }

               if (dfb_aribfont_get_glyph_image(font, desc, options, jiscode,
                                                glyphBuf, glyphBufLen, &glyph_rect) == DFB_OK) {

                    dfb_draw_glyph_a2(glyphBuf, glyph_pitch,
                                  buf_ptr + pitch * glyph_rect.y + glyph_rect.x ,
                                  pitch,
                                  glyph_rect.w, glyph_rect.h, palette);

                    buf_ptr += glyph_rect.x + glyph_rect.w;
                    if (letter_space > 0) {
                         buf_ptr += letter_space;
                    }
               }
          }

          string += skip;
     } while (string < end);

     if (glyphBuf != NULL) {
          D_FREE(glyphBuf); glyphBuf = NULL;
     }

     dfb_aribfont_unlock(font);

     return DFB_OK;
}


/*** methods for ARIB caption ***/
/** Glyph handling **/

/*
 * Get the extents of a glyph specified by its JIS0208 code.
 */
static DFBResult
IDirectFBARIBFont_GetJISGlyphExtents(IDirectFBARIBFont *thiz,
                                     DFBARIBFontDesc   *desc,
                                     DFBARIBFontOptions options,
                                     __u16              jis0208_code,
                                     DFBRectangle      *ret_rect)
{
     DFBResult      ret;
     CoreARIBFont  *font;

     DIRECT_INTERFACE_GET_DATA(IDirectFBARIBFont)

     if (!desc || !ret_rect)
          return DFB_INVARG;

     font   = data->font;

     dfb_aribfont_lock( font );

     ret = dfb_aribfont_get_glyph_extents(font, desc, options, jis0208_code, ret_rect, NULL);

     dfb_aribfont_unlock(font);

     if (ret != DFB_OK) {
          ret_rect->x = ret_rect->y = ret_rect->w = ret_rect->h = 0;
     }
     return DFB_OK;
}

/*
 * Get a single gray scale glyph specified by its JIS0208 code.
 * into the specified buffer area without colorizing.
 */

static DFBResult
IDirectFBARIBFont_GetJISGlyphImage(IDirectFBARIBFont *thiz,
                                   DFBARIBFontDesc   *desc,
                                   DFBARIBFontOptions options,
                                   __u16              jis0208_code,
                                   void*              buf_ptr,
                                   int                pitch)
{
     DFBResult      ret;
     CoreARIBFont  *font;

     DFBRectangle   glyph_rect;
     int            glyph_pitch;
     int            ret_buflen;

     __u8*          glyphBuf = NULL;
     int            glyphBufLen = 0;

     DIRECT_INTERFACE_GET_DATA(IDirectFBARIBFont)

     if (!desc || !buf_ptr)
          return DFB_INVARG;

     font = data->font;

     dfb_aribfont_lock( font );

     if ((ret = dfb_aribfont_get_glyph_extents(font, desc, options, jis0208_code, &glyph_rect, &ret_buflen)) == DFB_OK) {
          glyph_pitch = ret_buflen / glyph_rect.h;
          glyphBufLen = ret_buflen;
          glyphBuf = (__u8*)D_CALLOC(1,glyphBufLen);

          if ((ret = dfb_aribfont_get_glyph_image(font, desc, options, jis0208_code,
                                                  glyphBuf, glyphBufLen, &glyph_rect)) == DFB_OK) {
               dfb_copy_glyph_a2(glyphBuf, glyph_pitch,
                             buf_ptr, pitch,
                             glyph_rect.w, glyph_rect.h);
          }
          D_FREE(glyphBuf); glyphBuf = NULL;
     }

     dfb_aribfont_unlock(font);

     return ret;
}


/*
 * Draw a single gray scale glyph specified by its JIS0208 code.
 * into the specified buffer area with the given gray-level palette.
 *
 * If frame_color is not minus, additional glyph frame is drawn with the
 * frame color. In that case, drawn width and height are two pixel wider
 * and taller than the extents returned by GetJISGlyphExtents.
 */

static DFBResult
IDirectFBARIBFont_DrawJISGlyphImage(IDirectFBARIBFont *thiz,
                                    DFBARIBFontDesc   *desc,
                                    DFBARIBFontOptions options,
                                    __u16              jis0208_code,
                                    int                frame_color,
                                    void*              buf_ptr,
                                    int                pitch)
{
     DFBResult      ret;
     CoreARIBFont  *font;

     DFBRectangle   glyph_rect;
     int            glyph_pitch;
     int            ret_buflen;

     __u8*          glyphBuf = NULL;
     int            glyphBufLen = 0;
     int           *palette;

     DIRECT_INTERFACE_GET_DATA(IDirectFBARIBFont)

     if (!desc || !buf_ptr)
          return DFB_INVARG;

     if (data->graylevel >= 4) {
          palette = data->palette;
     }
     else {
          D_WARN(" DirecFB/IDirectFBARIBFont:"
                 " palette is empty or insufficient.\n");
          return DFB_OK;
     }

     font = data->font;

     dfb_aribfont_lock( font );

     if ((ret = dfb_aribfont_get_glyph_extents(font, desc, options, jis0208_code, &glyph_rect, &ret_buflen)) == DFB_OK) {
          glyph_pitch = ret_buflen / glyph_rect.h;
          glyphBufLen = ret_buflen;
          glyphBuf = (__u8*)D_CALLOC(1,glyphBufLen);
          if (glyphBuf == NULL)
               return DFB_NOSYSTEMMEMORY;

          if ((ret = dfb_aribfont_get_glyph_image(font, desc, options, jis0208_code,
                                                  glyphBuf, glyphBufLen, &glyph_rect)) == DFB_OK) {
               if (frame_color < 0) {
                    dfb_draw_glyph_a2(glyphBuf, glyph_pitch,
                                      (__u8 *)buf_ptr + glyph_rect.y * pitch + glyph_rect.x,
                                      pitch,
                                      glyph_rect.w, glyph_rect.h, palette);
               }
               else { /* add frame */
                    dfb_draw_framed_glyph_a2(glyphBuf, glyph_pitch,
                                             (__u8 *)buf_ptr + (glyph_rect.y - 1) * pitch + (glyph_rect.x - 1),
                                             pitch,
                                             glyph_rect.w, glyph_rect.h, palette, frame_color);
               }
          }
          D_FREE(glyphBuf); glyphBuf = NULL;
     }

     dfb_aribfont_unlock(font);

     return ret;
}


/* called at the end of Construct() in the idirectfbaribfont_xxx.c */
DFBResult
IDirectFBARIBFont_Construct( IDirectFBARIBFont *thiz, CoreARIBFont *font )
{
     DIRECT_ALLOCATE_INTERFACE_DATA(thiz, IDirectFBARIBFont)

     data->ref = 1;
     data->font = font;
     data->graylevel = 0; /* may not need this due to calloc */
     data->palette = NULL; /* may not need this due to calloc */
     data->aribdrcs = NULL; /* may not need this due to calloc */

     thiz->AddRef = IDirectFBARIBFont_AddRef;
     thiz->Release = IDirectFBARIBFont_Release;
     thiz->SetARIBDrcs = IDirectFBARIBFont_SetARIBDrcs;
     thiz->GetEUCStringExtents = IDirectFBARIBFont_GetEUCStringExtents;
     thiz->SetGrayLevelPalette = IDirectFBARIBFont_SetGrayLevelPalette;
     thiz->DrawEUCString = IDirectFBARIBFont_DrawEUCString;
     thiz->GetJISGlyphExtents = IDirectFBARIBFont_GetJISGlyphExtents;
     thiz->GetJISGlyphImage = IDirectFBARIBFont_GetJISGlyphImage;
     thiz->DrawJISGlyphImage = IDirectFBARIBFont_DrawJISGlyphImage;

     return DFB_OK;
}

/* end of idirectfbaribfont.c */
