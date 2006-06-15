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
#include <sys/mman.h>
#include <errno.h>
extern int errno;

#include <string.h>

#include <pthread.h>

#include <directfb_arib.h>

#include <direct/interface.h>
#include <direct/mem.h>

#include <media/idirectfbaribdrcs.h>

#include "misc/glyph_util.h"

/*
 * private structures
 */
typedef struct {
     __u8 *glyphs[256];
} aribdrcs_columns;

typedef struct {
     aribdrcs_columns* columns[256];
} aribdrcs_rows;

struct aribdrcs_table {
     __u8     fontId;
     __u8     width;
     __u8     height;
     int      glyph_length; /* glyph buffer length in bytes */
     aribdrcs_rows* rows;
};

static const struct {
     __u8  fontId;
     __u8  width;
     __u8  height;
} fonts_supported[] = {
     { 1, 16, 16}, /* MARUGOTHIC, 16 point */
     { 1, 20, 20}, /* MARUGOTHIC, 20 point */
     { 1, 24, 24}, /* MARUGOTHIC, 24 point */
     { 1, 30, 30}, /* MARUGOTHIC, 30 point */
     { 1, 36, 36}, /* MARUGOTHIC, 36 point */
     { 2, 20, 20}, /* KAKUGOTHIC, 20 point */
     { 2, 24, 24}, /* KAKUGOTHIC, 24 point */
     { 3, 30, 30}, /* FUTOMARUGOTHIC, 30 point */
};

#define N_FONTS_SUPPORTED (int)((sizeof(fonts_supported) / sizeof(fonts_supported[0])))


/*
 * subroutines
 */

static DFBResult
setup_aribdrcs_table(aribdrcs_table** pTable)
{
     aribdrcs_table* table;
     int i;

     D_ASSERT(pTable);
     table = (aribdrcs_table*)D_CALLOC(N_FONTS_SUPPORTED, sizeof(aribdrcs_table));
     if (!table)
          return (DFB_NOSYSTEMMEMORY);
     for (i=0; i<N_FONTS_SUPPORTED; i++) {
          table[i].fontId = fonts_supported[i].fontId;
          table[i].width  = fonts_supported[i].width;
          table[i].height = fonts_supported[i].height;
          table[i].glyph_length = (((2 * table[i].width) + 7)/8) * table[i].height;
     }
     *pTable = table;
     return DFB_OK;
}


static DFBResult
unload_drcs(aribdrcs_table *table)
{
     int i, j, k;
     aribdrcs_rows* rows;
     aribdrcs_columns* columns;
     __u8 *glyph;

     D_ASSERT(table);

     for (i=0; i<N_FONTS_SUPPORTED; i++) {
          rows = table[i].rows;
          if (rows == NULL) continue;
          for (j=0; j<256; j++) {
               columns = rows->columns[j];
               if (columns == NULL) continue;
               for (k=0; k<256; k++) {
                    glyph = columns->glyphs[k];
                    if (glyph == NULL) continue;
                    D_FREE(glyph);
               }
               D_FREE(columns);
          }
          D_FREE(rows);
          table[i].rows = NULL;
     }
     return DFB_OK;
}


static DFBResult
get_glyph(aribdrcs_table *table,
          __u8 fontId, __u8 width, __u8 height, __u16 CharacterCode,
          __u8 **glyph_p)
{
     int i;
     __u8 upper, lower;

     aribdrcs_rows* rows;
     aribdrcs_columns* columns;

     D_ASSERT(table);

     upper = (CharacterCode & 0xFF00) >> 8;
     lower = (CharacterCode & 0x00FF);
     *glyph_p = NULL;

     for (i=0; i<N_FONTS_SUPPORTED; i++) {
          if ((table[i].fontId == fontId) &&
              (table[i].width  == width) &&
              (table[i].height == height)) {
               rows = table[i].rows;
               if ((rows != NULL) &&
                   ((columns = rows->columns[upper]) != NULL)) {
                    *glyph_p = columns->glyphs[lower];
               }
               return DFB_OK;
          }
     }
     return DFB_INVARG;
}


static void
copy_pattern_to_a2(__u8* sbuf, __u8* dbuf, int w, int h)
{
     int   x,y;
     __u8 *s = sbuf;
     __u8 *d = dbuf;
     int   b_src, pad_bits;
     int   bit_width = 2 * w;

     pad_bits = ((bit_width+7)/8)*8 - bit_width;
     b_src = 0;
     for(y=0; y<h; y++) {
          for(x=0; x<bit_width; s++, d++, x+=8) {
               if (b_src == 0) {
                    *d = *s;
               }
               else {
                    *d = (*s << b_src) | (*(s+1) >> (8 - b_src));
               }
          }
          if (pad_bits) { *d= (*d >> pad_bits) << pad_bits; } /* clear pad bits */
          if ((b_src -= pad_bits) < 0) { s--; b_src += 8; }
     }
}

static void
copy_depth0pattern_to_a2(__u8* sbuf, __u8* dbuf, int w, int h)
{
     int   x,y;
     __u8 *s = sbuf;
     __u8 *d = dbuf;
     int  b_src, b_dst;

     b_src = 7;
     for(y=0; y<h; y++) {
          b_dst = 6;
          for(x=0; x<w; x++) {
               if (b_dst == 6)
                    *d = 0;
               if (((*s)>>b_src) & 0x01)
                    *d |= (0x03) << b_dst;
			if ((b_src -= 1)< 0) { b_src = 7; s++; }
			if ((b_dst -= 2)< 0) { b_dst = 6; d++; }
		}
          if (b_dst != 6) d++;
     }
}

static DFBResult
reserve_glyph(aribdrcs_table *table,
              __u8 fontId, __u8 width, __u8 height, __u16 CharacterCode,
              __u8 **glyph_p)
{
     int i;
     __u8 upper, lower;

     aribdrcs_rows* rows;
     aribdrcs_columns* columns;
     __u8 **glyph_p2;
     __u8  *glyph;

     D_ASSERT(table);

     upper = (CharacterCode & 0xFF00) >> 8;
     lower = (CharacterCode & 0x00FF);

     for (i=0; i<N_FONTS_SUPPORTED; i++) {
          if ((table[i].fontId == fontId) &&
              (table[i].width  == width) &&
              (table[i].height == height)) {

               if (table[i].rows == NULL) {
                    if ((table[i].rows = (aribdrcs_rows*)D_CALLOC(1, sizeof(aribdrcs_rows))) == NULL) {
                         return (DFB_NOSYSTEMMEMORY);
                    }
                    D_HEAVYDEBUG("table[%d].rows alloced(%p)\n",i, table[i].rows);
               }
               rows = table[i].rows;

               if (rows->columns[upper] == NULL) {
                    if ((rows->columns[upper] = (aribdrcs_columns*)D_CALLOC(1, sizeof(aribdrcs_columns))) == NULL) {
                         return (DFB_NOSYSTEMMEMORY);
                    }
                    D_HEAVYDEBUG("table[%d].rows->columns[%d] alloced(%p)\n", i, upper, rows->columns[upper]);
               }
               columns = rows->columns[upper];

               if ((glyph = (__u8 *)D_CALLOC(table[i].glyph_length, sizeof(__u8))) == NULL) {
                         return (DFB_NOSYSTEMMEMORY);
               }
               D_HEAVYDEBUG("table[%d].rows->columns[%d]->glyphs[%d] alloced(%p)\n", i, upper, lower, glyph);

               glyph_p2 = columns->glyphs + lower;

               if (*glyph_p2) { /* already registered */
                    D_HEAVYDEBUG("already\n");
                    /* unregister it */
                    D_FREE(*glyph_p2); *glyph_p2 = NULL;
               }

               *glyph_p = *glyph_p2 = glyph;
               D_HEAVYDEBUG("glyph reserved\n");
               return DFB_OK;
          }
     }
     return DFB_INVARG;
}




/*****************************************************/

void
IDirectFBARIBDrcs_Destruct( IDirectFBARIBDrcs *thiz )
{
     IDirectFBARIBDrcs_data *data = (IDirectFBARIBDrcs_data*) thiz->priv;

     if (data->table) {
          unload_drcs ( data->table );
          D_FREE( data->table );
     }

     DIRECT_DEALLOCATE_INTERFACE( thiz );
}

static DFBResult
IDirectFBARIBDrcs_AddRef( IDirectFBARIBDrcs *thiz )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBARIBDrcs)

     data->ref++;

     return DFB_OK;
}

static DFBResult
IDirectFBARIBDrcs_Release( IDirectFBARIBDrcs *thiz )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBARIBDrcs)

     if (--data->ref == 0)
          IDirectFBARIBDrcs_Destruct( thiz );

     return DFB_OK;
}

/*
 * Load a pattern data
 */


static DFBResult
IDirectFBARIBDrcs_Load( IDirectFBARIBDrcs *thiz,
                        DFBARIBDrcsOptions options,
                        const char        *pattern_data,
                        int                length )
{
     __u8   NumberOfCode;
     __u16  CharacterCode;
     __u8   NumberOfFont;
     __u8   fontId;  /* 4bits */
     __u8   mode;    /* 4bits */
     __u8   depth;   /* gray-level - 2 */
     __u8   width;   /* horizontal size in pixel */
     __u8   height;  /* vertical size in pixel */
     __u8   k;
     int    grayBits;
     __u16  patternData_length;
     __u8   regionX;
     __u8   regionY;
     __u16  geometricData_length;

     int    i,j;
     int    n = 0;     /* parsed byte length */
     __u8  *p;
     aribdrcs_table *table;
     __u8  *glyph_buffer;
     __u8   force_fontid;

     /* Move GET_DATA from lower because of a strict compiler check --ats-- */
     DIRECT_INTERFACE_GET_DATA(IDirectFBARIBDrcs)

     switch (options & 0x00000003) {
     case DARIBDRCSOP_LOADAS_RNDGOTHIC: force_fontid = 0x1; break;
     case DARIBDRCSOP_LOADAS_GOTHIC:    force_fontid = 0x2; break;
     case DARIBDRCSOP_LOADAS_BOLDRNDGOTHIC: force_fontid = 0x3; break;
     default: force_fontid = 0; break;
     }


     if (!pattern_data)
          return(DFB_INVARG);
	if (++n > length)
          return(DFB_INVARG);

     table = data->table;

     p = (__u8 *)pattern_data;
     NumberOfCode = *p++;

     D_HEAVYDEBUG("DirectFB/ARIBDrcs: Load ----------------\n");
     D_HEAVYDEBUG("NumberOfCode = %d\n", NumberOfCode);

     for(i=0; i<NumberOfCode; i++) {
          if ((n += 3) > length)
               return(DFB_FAILURE);

          CharacterCode = *p++;
          CharacterCode <<= 8;
          CharacterCode |= *p++;
          NumberOfFont = *p++;

          D_HEAVYDEBUG("  CharacterCode = 0x%04X, NumberOfFont=%d\n", CharacterCode, NumberOfFont);

          for (j=0; j<NumberOfFont; j++) {
               if (++n > length)
                    return(DFB_FAILURE);
               fontId = force_fontid ? force_fontid : (*p & 0xF0) >> 4;
               mode = *p & 0x0F;
               p++;

               D_HEAVYDEBUG("    fontId = %d, mode=%d\n", fontId, mode);

               if (mode == 0x0 ||   /* 2 level, no compression */
                   mode == 0x1) {   /* multi level, no compression */
                    if ((n+=3) > length)
                         return(DFB_FAILURE);
                    depth = *p++;
                    width = *p++;
                    height = *p++;

                    /* calculate bits/pixel */
                    for (k = depth +2 -1, grayBits = 0;
                         k != 0;
                         k>>=1, grayBits++);
                    patternData_length = ((grayBits * width * height) + 7) / 8;

                    D_HEAVYDEBUG("    depth=%d, width=%d, height=%d, length=%d\n", depth, width, height, patternData_length);

				/* register DRCS glyph */
                    if ((n+=patternData_length) > length)
                         return(DFB_FAILURE);

                    if (depth == 2 || depth == 1) { /* 4 level (3 lelve is treated as if it were 4 level) */
                         if (reserve_glyph(table, fontId, width, height, CharacterCode, &glyph_buffer) == DFB_OK) {
                              copy_pattern_to_a2(p, glyph_buffer, width, height);
                         }
                         else {
                              D_ERROR( "DirectFB/ARIBDrcs: "
                                       "reserve_glyph failed\n");
                         }
                    }
                    else if (depth == 0) { /* transform 2 level into 4 level */
                         if (reserve_glyph(table, fontId, width, height, CharacterCode, &glyph_buffer) == DFB_OK) {
                              copy_depth0pattern_to_a2(p, glyph_buffer, width, height);
                         }
                         else {
                              D_ERROR( "DirectFB/ARIBDrcs: "
                                       "reserve_glyph failed\n");
                         }
                    }
                    /* level more than 4 is not supported */
				p += patternData_length;
               }
               else { /* geometric data is not supported */
                    if ((n+=4) > length)
                         return(DFB_FAILURE);
                    regionX = *p++;
                    regionY = *p++;
                    geometricData_length = (*p++) << 8;
                    geometricData_length |= *p++;
                    if ((n+=geometricData_length) > length)
                         return(DFB_FAILURE);
                    /* skip */
                    p += geometricData_length;
               }
          }
     }
     D_HEAVYDEBUG("---------------- DirectFB/ARIBDrcs: Load\n");

     return(DFB_OK);
}

/*
 * Load a pattern file
 */
static DFBResult
IDirectFBARIBDrcs_LoadFile( IDirectFBARIBDrcs *thiz,
                            DFBARIBDrcsOptions options,
                            const char        *filename )
{
     DFBResult   ret;
     int         file;
     struct stat stat_buf = {0};
     void        *p_mmap;
     int         length;

     if (!filename)
          return DFB_INVARG;

     file = open(filename, O_RDONLY);
     if (file < 0) {
          D_ERROR("IDirectFBARIBDrcs: "
                  "Unable to open `%s' !\n", filename);
		return DFB_FILENOTFOUND;
     }

     if (fstat(file, &stat_buf) < 0) {
          D_ERROR("IDirectFBARIBDrcs: "
                  "fstat error for `%s' !\n", filename);
          return DFB_FAILURE;
     }
     length = stat_buf.st_size;

     D_HEAVYDEBUG( "IDirectFBARIBDrcs: "
                   "mmap-ing DRCS file(%s). : %d bytes.\n",
                   filename, length);
     p_mmap = mmap(NULL, length, PROT_READ, MAP_FILE | MAP_PRIVATE, file, 0);

     if ( (long)p_mmap == -1 ) {
          D_ERROR("IDirectFBARIBDrcs: "
                  "mmap error for `%s'!: errno = [%d]\n", filename, errno);
          close(file);
          return DFB_FAILURE;
     }

     close(file);

     ret = IDirectFBARIBDrcs_Load(thiz, options, (char *)p_mmap, length);

     munmap(p_mmap, length);

     return ret;
}


/*
 * Unload all pattern data
 */
static DFBResult
IDirectFBARIBDrcs_Unload( IDirectFBARIBDrcs *thiz )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBARIBDrcs)

     return (unload_drcs(data->table));
}


static DFBResult
adjust_metrics(DFBARIBFontDesc *desc,
               DFBARIBFontOptions options,
               DFBRectangle *extents)
{

     D_ASSERT(desc && extents);

     if (DARIBFOP_HALF_WIDTH == (options & (DARIBFOP_DOUBLE_WIDTH | DARIBFOP_HALF_WIDTH))) {
          extents->w /= 2;
          if (options & DARIBFOP_VERTICAL)
               extents->x += (extents->w)/2;
     }
     if (DARIBFOP_DOUBLE_WIDTH == (options & (DARIBFOP_DOUBLE_WIDTH | DARIBFOP_HALF_WIDTH))) {
          extents->w *= 2;
          if (options & DARIBFOP_VERTICAL)
               extents->x -= (extents->w)/4;
     }
     if (DARIBFOP_HALF_HEIGHT == (options & (DARIBFOP_DOUBLE_HEIGHT | DARIBFOP_HALF_HEIGHT))) {
          extents->h /= 2;
          if (!(options & DARIBFOP_VERTICAL))
               extents->y += extents->h;
     }
     if (DARIBFOP_DOUBLE_HEIGHT == (options & (DARIBFOP_DOUBLE_HEIGHT | DARIBFOP_HALF_HEIGHT))) {
          extents->h *= 2;
          if (!(options & DARIBFOP_VERTICAL))
               extents->y -= (extents->h)/2;
     }
     return DFB_OK;
}

/*
 * Get the extents of a glyph specified by its characterCode.
 */
static DFBResult
IDirectFBARIBDrcs_GetGlyphExtents(IDirectFBARIBDrcs *thiz,
                                  DFBARIBFontDesc   *desc,
                                  DFBARIBFontOptions options,
                                  __u16              characterCode,
                                  DFBRectangle      *ret_rect)
{
     DFBResult      ret;
     __u8           fontId;
     __u8           width, height;
     __u8          *glyph;

     DIRECT_INTERFACE_GET_DATA(IDirectFBARIBDrcs)

     if (!desc || !ret_rect)
          return DFB_INVARG;

     if (desc->style & DARIBFS_ITALIC)
          return DFB_UNSUPPORTED;

     switch (desc->family) {
     case (DARIBFF_RNDGOTHIC):     fontId = 1; break;
     case (DARIBFF_GOTHIC):        fontId = 2; break;
     case (DARIBFF_BOLDRNDGOTHIC): fontId = 3; break;
     default:
          return DFB_UNSUPPORTED;
     }
     width = height = desc->size;

     ret = get_glyph(data->table, fontId, width, height, characterCode, &glyph);
     if ((ret != DFB_OK) || (glyph == NULL)) {
          ret_rect->x = ret_rect->y = ret_rect->w = ret_rect->h = 0;
          return DFB_FAILURE;
     }
     else {
          ret_rect->x = ret_rect->y = 0;
          ret_rect->w = ret_rect->h = desc->size;
          adjust_metrics(desc, options, ret_rect);
     }

     return DFB_OK;
}


/*
 * Get a single gray scale glyph specified by its characterCode
 * into the specified buffer area without colorizing.
 */
static DFBResult
IDirectFBARIBDrcs_GetGlyphImage(IDirectFBARIBDrcs *thiz,
                                DFBARIBFontDesc   *desc,
                                DFBARIBFontOptions options,
                                __u16              characterCode,
                                void*              buf_ptr,
                                int                pitch)
{
     DFBResult      ret;
     __u8           fontId;
     __u8           width, height;
     __u8          *glyph;

     DFBRectangle   glyph_rect;
     int            glyph_pitch;

	 __u8			*work_buf;


     DIRECT_INTERFACE_GET_DATA(IDirectFBARIBDrcs)

     if (!desc || !buf_ptr)
          return DFB_INVARG;

     if (desc->style & DARIBFS_ITALIC)
          return DFB_UNSUPPORTED;

     switch (desc->family) {
     case (DARIBFF_RNDGOTHIC):     fontId = 1; break;
     case (DARIBFF_GOTHIC):        fontId = 2; break;
     case (DARIBFF_BOLDRNDGOTHIC): fontId = 3; break;
     default:
          return DFB_UNSUPPORTED;
     }
     width = height = desc->size;

     ret = get_glyph(data->table, fontId, width, height, characterCode, &glyph);
     if ((ret != DFB_OK) || (glyph == NULL)) {
          return DFB_FAILURE;
     }
     else {
          glyph_rect.x = glyph_rect.y = 0;
          glyph_rect.w = glyph_rect.h = desc->size;
          adjust_metrics(desc, options, &glyph_rect);
     }

     glyph_pitch = ((2 * width)+7)/8;

     if (desc->style & DARIBFS_BOLD)	{
        if((work_buf = (__u8*)D_CALLOC((pitch * glyph_rect.h), sizeof(__u8))) == NULL)
          	return DFB_FAILURE;
	 }
	 else
	 	work_buf = buf_ptr;

     if ((desc->size == glyph_rect.w) && (desc->size == glyph_rect.h)) {
          dfb_copy_glyph_a2(glyph, glyph_pitch,
                        work_buf, pitch,
                        glyph_rect.w, glyph_rect.h);
     }
     else
          dfb_scaling_copy_glyph_a2(glyph, glyph_pitch,
                                work_buf, pitch,
                                (int)width, (int)height,
                                glyph_rect.w - width,
                                glyph_rect.h - height);

     /* options & DARIBFS_BOLD branch here */
     if (desc->style & DARIBFS_BOLD)	{
          dfb_bold_glyph_a2(work_buf,   pitch,
                        buf_ptr,    pitch,
                        (int)glyph_rect.w,
						(int)glyph_rect.h);
	 	  D_FREE(work_buf);
	 }

     return DFB_OK;
}


     /*
      * Set the color palette used for drawing text function.
      *
      */
static DFBResult
IDirectFBARIBDrcs_SetGrayLevelPalette(IDirectFBARIBDrcs *thiz,
                                      int               *palette,
                                      int                level)
{

     DIRECT_INTERFACE_GET_DATA(IDirectFBARIBDrcs);

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


/*
 * Draw a single gray scale glyph specified by its characterCode
 * into the specified buffer area with the given gray-level palette.
 */
static DFBResult
IDirectFBARIBDrcs_DrawGlyphImage(IDirectFBARIBDrcs *thiz,
                                 DFBARIBFontDesc   *desc,
                                 DFBARIBFontOptions options,
                                 __u16              characterCode,
                                 int                frame_color,
                                 void*              buf_ptr,
                                 int                pitch)
{
     DFBResult      ret;
     DFBRectangle   glyph_rect;
     int            glyph_pitch;

     __u8          *glyphBuf = NULL;
     int            glyphBufLen = 0;
     int           *palette;

     DIRECT_INTERFACE_GET_DATA(IDirectFBARIBDrcs)

     if (!desc || !buf_ptr)
          return DFB_INVARG;

     if (data->graylevel >= 4) {
          palette = data->palette;
     }
     else {
          D_WARN(" DirecFB/IDirectFBARIBDrcs:"
                 " palette is empty or insufficient.\n");
          return DFB_OK;
     }

     if ((ret = IDirectFBARIBDrcs_GetGlyphExtents(thiz, desc, options,
                                                  characterCode, &glyph_rect)) == DFB_OK) {
          glyph_pitch = ((2*glyph_rect.w)+7)/8;
          glyphBufLen = glyph_pitch * glyph_rect.h;
          glyphBuf = (__u8*)D_CALLOC(1,glyphBufLen);
          if (glyphBuf == NULL)
               return DFB_NOSYSTEMMEMORY;

          if ((ret = IDirectFBARIBDrcs_GetGlyphImage(thiz, desc, options, characterCode,
                                                     glyphBuf, glyph_pitch)) == DFB_OK) {
               if (frame_color < 0)
               dfb_draw_glyph_a2(glyphBuf, glyph_pitch,
                             (__u8 *)buf_ptr + glyph_rect.y * pitch + glyph_rect.x,
                             pitch,
                             glyph_rect.w, glyph_rect.h, palette);
               else
                  dfb_draw_framed_glyph_a2(glyphBuf, glyph_pitch,
                                           (__u8 *)buf_ptr + (glyph_rect.y - 1) * pitch + (glyph_rect.x - 1),
                                           pitch,
                                           glyph_rect.w, glyph_rect.h, palette, frame_color);
          }
          D_FREE(glyphBuf); glyphBuf = NULL;
     }

     return ret;
}


DFBResult
IDirectFBARIBDrcs_Construct( IDirectFBARIBDrcs *thiz )
{
     DFBResult ret;

     DIRECT_ALLOCATE_INTERFACE_DATA(thiz, IDirectFBARIBDrcs)

     if ((ret = setup_aribdrcs_table(&(data->table))) != DFB_OK) {
          DIRECT_DEALLOCATE_INTERFACE( thiz )
          return ret;
     }

     data->ref = 1;
     data->graylevel = 0; /* may not need this due to calloc */
     data->palette = NULL; /* may not need this due to calloc */

     thiz->AddRef                 = IDirectFBARIBDrcs_AddRef;
     thiz->Release                = IDirectFBARIBDrcs_Release;
     thiz->Load                   = IDirectFBARIBDrcs_Load;
     thiz->LoadFile               = IDirectFBARIBDrcs_LoadFile;
     thiz->Unload                 = IDirectFBARIBDrcs_Unload;
     thiz->GetGlyphExtents        = IDirectFBARIBDrcs_GetGlyphExtents;
     thiz->GetGlyphImage          = IDirectFBARIBDrcs_GetGlyphImage;
     thiz->SetGrayLevelPalette    = IDirectFBARIBDrcs_SetGrayLevelPalette;
     thiz->DrawGlyphImage         = IDirectFBARIBDrcs_DrawGlyphImage;

     return DFB_OK;
}

/* end of idirectfbaribdrcs.c */
