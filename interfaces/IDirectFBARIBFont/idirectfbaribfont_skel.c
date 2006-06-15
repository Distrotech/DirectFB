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
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>


#define MAX_SKEL_FONTS 6
#define SKEL_WORKING_BUFFER 16*1024

#define HEAD_CUSTOM_ID  0x38 /* HEAD_MARK      +32 */
#define HEAD_METRICSPTR 0x40 /* HEAD_TYPEFACE  +4  */
#define HEAD_GLYPHPTR   0x50 /* HEAD_ICONLEN   +4  */

/*
 * -1: 1bpp bitmap
 *  0: 2bpp bitmap (simple doubling)
 *  1: 2bpp grayscale, smooth but heavy processing, not applied to bitmap font
 *  2: 2bpp grayscale, sharp and lower processing,
 *  3: 2bpp grayscale, smooth but heavy processing, applied to bitmap font
 *  5: 2bpp grayscale, bolden small font, applied to bitmap font
 *  7: 2bpp grayscale, compatible with FT2, mode 5 applied to bitmap font
 *  8: 2bpp grayscale, same as 7 but bolder
 *  9: 8bpp grayscale, compatible with FT2, mode 5 applied to bitmap font
 */
#define SKEL_GRAY_MODE 1 /* 0 */

#include <directfb_arib.h>

#include <core/aribfonts.h>

#include <media/idirectfbaribfont.h>

#include <direct/mem.h>
#include <direct/memcpy.h>
#include <direct/messages.h>
#include <direct/util.h>

#include <misc/conf.h>
#include <misc/util.h>


static DFBResult
Probe( IDirectFBARIBFont_ProbeContext *ctx );

static DFBResult
Construct( IDirectFBARIBFont  *thiz,
           CoreDFB            *core,
           const char         *filename );

#include <direct/interface_implementation.h>

DIRECT_INTERFACE_IMPLEMENTATION( IDirectFBARIBFont, SKEL )

typedef struct SKELHandle {
     int     stream;
     int     num_fonts;
     char    FontName[256];
} SKELHandle;

static SKELHandle*     library           = NULL;
static int             library_ref_count = 0;
static pthread_mutex_t library_mutex     = DIRECT_UTIL_RECURSIVE_PTHREAD_MUTEX_INITIALIZER;


/**********************************************
 * static functions
 **********************************************/
static DFBResult
jis2seq(const __u16 jiscode,
        __s16* skel_seq,
        int* isMedium)
{
     DFBResult ret = DFB_OK; /* it may help in future */
     __s16 upper, lower;

     /* JIS0201 */
     if ((jiscode & 0xFF00) == 0x0000) {
          /* SKEL mixtures ASCII and JIS0201
           *  0x7E: tilde    ( not overscore ) -- ASCII
           *  0x5C: yen mark ( not backslash ) -- JIS0201
           */
          if ((jiscode >= 0x0020) && (jiscode <= 0x007E)) { /* ASCII */
               *skel_seq = jiscode;
          }
          else if ((jiscode >= 0x00A1) && (jiscode <= 0x00DF)) { /* JIS ROMA */
               *skel_seq = jiscode;
          }
          else {
               *skel_seq = 0x0000; /* replace to space */
          }
          *isMedium = 1; /* HANKAKU */
     }
     else { /* JIS0208 */
          *isMedium = 0; /* ZENKAKU */
          upper = (jiscode & 0xFF00)>>8;
          lower = jiscode & 0x00FF;
          if ((upper < 0x21) || (upper > 0x7E) ||
              (lower < 0x21) || (lower > 0x7E)) {
               *skel_seq = 0x0000; /* replace to SPACE */
          }
          else {
               *skel_seq = (upper - 0x21) * (0x7E - 0x21 + 1) + (lower - 0x21);
          }
     }
     return ret;
}


static DFBResult
adjust_metrics(DFBARIBFontDesc *desc,
               DFBARIBFontOptions options,
               DFBRectangle *extents)
{

     D_ASSERT(desc && extents);

     if (options & DARIBFOP_VERTICAL) {
          if (DARIBFOP_HALF_WIDTH == (options & (DARIBFOP_DOUBLE_WIDTH | DARIBFOP_HALF_WIDTH))) {
               extents->x += (extents->w)/2;
          }
          if (DARIBFOP_DOUBLE_WIDTH == (options & (DARIBFOP_DOUBLE_WIDTH | DARIBFOP_HALF_WIDTH))) {
               extents->x -= (extents->w)/4;
          }
     }
     else {
          if (DARIBFOP_HALF_HEIGHT == (options & (DARIBFOP_DOUBLE_HEIGHT | DARIBFOP_HALF_HEIGHT))) {
               extents->y += extents->h;
          }
          if (DARIBFOP_DOUBLE_HEIGHT == (options & (DARIBFOP_DOUBLE_HEIGHT | DARIBFOP_HALF_HEIGHT))) {
               extents->y -= (extents->h)/2;
          }
     }
     return DFB_OK;
}

static DFBResult
set_request_glyph(DFBARIBFontDesc *desc,
                  DFBARIBFontOptions options,
                  int isMedium,
                  DFBRectangle *ret_rect,
                  DFBBoolean   doIt)
{
     __s16 FontID;
     __s32 req_width, req_height;

     D_ASSERT(desc != NULL);

     if (isMedium) {
          switch (desc->family) {
          case DARIBFF_RNDGOTHIC:     FontID = 0x1300; break; /* EHG11G */
          case DARIBFF_GOTHIC:        FontID = 0x1100; break; /* EHH11M */
          case DARIBFF_BOLDRNDGOTHIC: FontID = 0x1300; break; /* EHG11G: tentative */
          default: return DFB_FAILURE;
          }
     }
     else {
          switch (desc->family) {
          case DARIBFF_RNDGOTHIC:     FontID = 0x2310; break; /* JGTR00M */
          case DARIBFF_GOTHIC:        FontID = 0x2110; break; /* JHEI00M */
          case DARIBFF_BOLDRNDGOTHIC: FontID = 0x2313; break; /* JGTR00B */
          default: return DFB_FAILURE;
          }
     }

     req_width = req_height = desc->size;

     if (options & DARIBFOP_DOUBLE_WIDTH)
          req_width *= 2;
     if (options & DARIBFOP_DOUBLE_HEIGHT)
          req_height *= 2;

     if (isMedium) { /* ignore HALF_WIDTH option */
          /* req_width will be reported to user as halfen later */
          if (options & DARIBFOP_HALF_HEIGHT)
               req_height /= 2;
     }
     else {
          if (options & DARIBFOP_HALF_WIDTH)
               req_width /= 2;
          if (options & DARIBFOP_HALF_HEIGHT)
               req_height /= 2;
     }

     if (ret_rect) {
          ret_rect->x = ret_rect->y = 0;
          ret_rect->w = req_width;
          ret_rect->h = req_height;

          if (isMedium)
               ret_rect->w /= 2;

          /* adjust x,y point */
          adjust_metrics(desc, options, ret_rect);

          if (isMedium)
               ret_rect->w = req_width;
     }

     if (doIt) {
		;
     }

     return DFB_OK;
}

/**********************************************
 * class methods
 **********************************************/
static DFBResult
get_glyph_extents( CoreARIBFont *thiz,
                   DFBARIBFontDesc *desc,
                   DFBARIBFontOptions options,
                   __u16 jiscode, /* 0x00XX: JIS0201, else: JIS0208 */
                   DFBRectangle *ret_rect,
                   int* ret_buflen)
{
     DFBResult ret;

     __s16 skel_seq;
     int isMedium;
     DFBRectangle glyph_rect;
     int          glyph_pitch;

     D_ASSERT((desc != NULL) && (ret_rect != NULL));

     jis2seq(jiscode, &skel_seq, &isMedium);

     ret = set_request_glyph(desc, options, isMedium, &glyph_rect, DFB_FALSE);
     if (ret != DFB_OK)
          return ret;

     glyph_pitch = (glyph_rect.w + 7)/8 * 2;

     *ret_rect = glyph_rect;
     if (isMedium)
          ret_rect->w /= 2;

     if (ret_buflen) {
          *ret_buflen = glyph_pitch * glyph_rect.h;
     }

     return ret;
}


static DFBResult
get_glyph_image(CoreARIBFont *thiz,
                DFBARIBFontDesc *desc,
                DFBARIBFontOptions options,
                __u16 jiscode, /* 0x00XX: JIS0201, else: JIS0208 */
                void* bufptr, int in_buflen,
                DFBRectangle *ret_rect)
{
     DFBResult ret = DFB_OK;

     __s16        skel_seq;
     int          isMedium;
     DFBRectangle glyph_rect;
     int          glyph_pitch;
     int          buf_length;

     D_ASSERT((desc != NULL) && (bufptr != NULL));

     jis2seq(jiscode, &skel_seq, &isMedium);

     pthread_mutex_lock ( &library_mutex );

     if (library == NULL) {
          ret = DFB_FAILURE; goto error;
     }

     ret = set_request_glyph(desc, options, isMedium, &glyph_rect, DFB_TRUE);
     if (ret != DFB_OK) goto error;

     glyph_pitch = (glyph_rect.w + 7)/8 * 2;
     if (ret_rect) {
          *ret_rect = glyph_rect;
          if (isMedium)
               ret_rect->w /= 2;
     }
     if (skel_seq == 0x0000) {
          ret = DFB_FAILURE; goto error;
     }
     if (skel_seq == 0x0020) {
          memset(bufptr,0,in_buflen);
          ret = DFB_OK; goto error;
     }

     memset(bufptr,0xff,in_buflen);	/* dummy */
     buf_length = glyph_pitch * ret_rect->h;
     if (buf_length <= 0) {
          D_ERROR( "DirectFB/ARIBFontSKEL "
                   "EgtGetFntBmp failed (seq#=0x%04X, rtn=%d)\n", skel_seq, buf_length);
          ret = DFB_FAILURE; goto error;
     }

error:
     pthread_mutex_unlock ( &library_mutex );
     return ret;
}

static void
final_skel(SKELHandle* handle)
{
     if (handle == NULL) return;

     if (handle->stream) {
          close(handle->stream); handle->stream = 0;
     }
     D_FREE(handle);
}

static DFBResult
init_skel(const char* filename)
{
     DFBResult ret = DFB_OK;
     SKELHandle* handle;
     int    stream;

     pthread_mutex_lock ( &library_mutex );

     if (!library) {
          D_HEAVYDEBUG( "DirectFB/ARIBFontSKEL: "
                        "Initializing the SKEL library.\n" );
          handle = (SKELHandle*)D_CALLOC(1, sizeof(SKELHandle));
          if (handle == NULL) {
               ret = DFB_NOSYSTEMMEMORY; goto error1;
          }
          library = handle;

          if ((stream = open(filename, O_RDONLY)) < 0) {
               D_ERROR( "DirectFB/ARIBFontSKEL: "
                        "Unable to open %s !\n", filename);
               ret = DFB_FILENOTFOUND; goto error2;
          }

          /* Set file stream for get font data */
          handle->stream = stream;
          handle->num_fonts = 1;
          memcpy(handle->FontName, filename, strlen(filename));
     }

     library_ref_count++;
     pthread_mutex_unlock( &library_mutex );

     return ret;

error2:
     final_skel(handle);
error1:
     library = NULL;
     pthread_mutex_unlock( &library_mutex );
     return ret;
}

static void
release_skel( void )
{
     pthread_mutex_lock( &library_mutex );

     if (library && --library_ref_count == 0) {
          D_HEAVYDEBUG( "DirectFB/ARIBFontSKEL: "
                         "Releasing the SKEL library.\n" );
          final_skel( library );
          library = NULL;
     }

     pthread_mutex_unlock( &library_mutex );
}

static void
IDirectFBARIBFont_SKEL_Destruct(IDirectFBARIBFont *thiz)
{
     IDirectFBARIBFont_Destruct( thiz );

     release_skel();
}


static DFBResult
IDirectFBARIBFont_SKEL_Release(IDirectFBARIBFont *thiz)
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBARIBFont)

     if (--data->ref == 0) {
          IDirectFBARIBFont_SKEL_Destruct(thiz);
     }

     return DFB_OK;
}


static DFBResult
Probe( IDirectFBARIBFont_ProbeContext *ctx )
{
     DFBResult ret = DFB_OK;
     int file;

     if (!ctx->filename)
          return DFB_UNSUPPORTED;

     file = open(ctx->filename, O_RDONLY);
     if (file < 0) {
          ret = DFB_UNSUPPORTED;
     }
     close(file);
     return ret;
}


static DFBResult
Construct( IDirectFBARIBFont  *thiz,
           CoreDFB            *core,
           const char         *filename)
{
     CoreARIBFont          *font;

     D_HEAVYDEBUG( "DirectFB/ARIBFontSKEL: "
                    "Construct font from file `%s'.\n",
                    filename);

     if (init_skel(filename) != DFB_OK) {
          DIRECT_DEALLOCATE_INTERFACE( thiz );
          return DFB_FAILURE;
     }

     font = dfb_aribfont_create(core);

     D_ASSERT( font->pixel_format == DSPF_A2 );

     font->GetGlyphExtents = get_glyph_extents;
     font->GetGlyphImage   = get_glyph_image;

     IDirectFBARIBFont_Construct( thiz, font );

     thiz->Release = IDirectFBARIBFont_SKEL_Release;

     return DFB_OK;
}

/* end of idirectfbaribfont_skel.c */
