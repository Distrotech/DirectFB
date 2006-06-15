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

#ifndef __DIRECTFB_ARIB_H__
#define __DIRECTFB_ARIB_H__

#include <fusion/call.h>
#include <directfb.h>

#ifdef __cplusplus
extern "C"
{
#endif

/*
 * @internal
 *
 * Macro for inheriting an interface definition.
 */
#define DEFINE_CHILD_INTERFACE( IFACECHILD, IFACEPARENT, IDATA... )  \
     struct _##IFACECHILD  {                                         \
          struct _##IFACEPARENT   parent;                            \
          DFBResult (*AddRef)( IFACECHILD *thiz );                   \
          DFBResult (*Release)( IFACECHILD *thiz );                  \
          IDATA                                                      \
     };

/*
 * An aspect ratio type
 */
typedef enum {
     DLASPECT_INVALID    = 0,
     DLASPECT_4_3,
     DLASPECT_16_9,
     DLASPECT_221,
     DLASPECT_1_1,
} DFBAspectRatio;

/*
 * Monitor Type
 */
typedef enum {
     DMONITOR_TYPE_1080I = 0,
     DMONITOR_TYPE_D4,
     DMONITOR_TYPE_D3,
     DMONITOR_TYPE_D2,
     DMONITOR_TYPE_D1,
} DFBMonitorType;

/*
 * Monitor Aspect Ratio
 */
typedef enum {
     DMONITOR_ASPECT_4_3 = 0,
     DMONITOR_ASPECT_16_9,
     DMONITOR_ASPECT_4_3WIDE,
} DFBMonitorAspectRatio;

/*
 * Call Back Id
 */
typedef enum {
	CB_RESOLUTION_CHANGE = 0,
	CB_LOCATION_CHANGE,
	CB_REG_MAX,
} DFBCallBackId;


/*
 * BML Information
 */
typedef struct {
	DFBBoolean  bml_valid;
	DFBBoolean  bml_invisible;
	DFBBoolean  bml_has_video;
} DFBAribBmlInfo;

/*
 * Resolution Information
 */
typedef struct {
	DFBDimension input_res;
	DFBBoolean   input_aspect;
	DFBDimension output_res;
	DFBBoolean   output_aspect;
} DFBResolutionInfo;

/*
 * ARIB Resolution Table
 */
typedef struct {
	unsigned long seqhead_w;			/* Mpeg SequenceHeader horizontal size */
	unsigned long seqhead_h;			/* Mpeg SequenceHeader vertical size */
	unsigned long seqdisp_w;			/* Mpeg SequenceDisplayExtension horizontal size */
	unsigned long seqdisp_h;			/* Mpeg SequenceDisplayExtension vertical size */
	DFBBoolean    seqhead_aspect;		/* Mpeg SequenceHeader aspect ratio */
	unsigned long mpeg_w;				/* Mpeg horizontal size */
	unsigned long mpeg_h;				/* Mpeg vertical size */
	DFBBoolean    mpeg_aspect;			/* Mpeg aspect ratio */
	unsigned long display_w;			/* Display horizontal size */
	unsigned long display_h;			/* Display vertical size */
	DFBBoolean    display_aspect;		/* Display aspect ratio */
	unsigned long gfx_input_w;			/* Graphics input horizontal size */
	unsigned long gfx_input_h;			/* Graphics input vertical size */
	DFBBoolean    gfx_input_aspect;		/* Graphics input aspect ratio */
	unsigned long gfx_output_w;			/* Graphics output horizontal size */
	unsigned long gfx_output_h;			/* Graphics output vertical size */
	DFBBoolean    gfx_output_aspect;	/* Graphics output aspect ratio */
	unsigned long x_offset;				/* Display X Offset */
	unsigned long y_offset;				/* Display Y Offset */
} DFB_RESOLUTION_ITEM;

typedef struct {
	DFBMonitorType        monitor_type;
	DFBMonitorAspectRatio monitor_aspect;
	unsigned long         num;
	DFB_RESOLUTION_ITEM   *ptr;
} DFB_RESOLUTION_TABLE;

/*
 *	Resolution Change Level
 *
 */
#define	DFB_DISPLAY_CHANGE		1
#define	DFB_ARIBGFX_CHANGE		2
#define	DFB_CANVAS_CHANGE		4

/*
 * predefined ARIB window ids
 */

#define DARIBWID_NONE                0x0000
#define DARIBWID_STILLPICTURE_PLANE  0x0001 /* Still picture plane */
#define DARIBWID_TEXT_GRAPHIC_PLANE  0x0002 /* Text and graphics plane */
#define DARIBWID_CAPTION_PLANE       0x0003 /* Subtitle plane */
#define DARIBWID_SUPERIMPOSE_PLANE   0x0004 /* (optional) Subtitle plane */
#define DARIBWID_DEBUG_PLANE1        0x0005 /* Debug Plane1 (LUT8) */
#define DARIBWID_DEBUG_PLANE2        0x0006 /* Debug Plane2 (LUT8AYCbCr) */
#define DARIBWID_DEBUG_PLANE3        0x0007 /* Debug Plane3 (AYCbCr) */
#define DARIBWID_PLANE_MAX           DARIBWID_DEBUG_PLANE3

#define	DFB_ARIB_HD_DIMENSION_W		960
#define	DFB_ARIB_HD_DIMENSION_H		540
#define	DFB_ARIB_SD_DIMENSION_W		720
#define	DFB_ARIB_SD_DIMENSION_H		480

#define	DFB_MPEG_1080I_DIMENSION_W	1920
#define	DFB_MPEG_1080I_DIMENSION_H	1080

#define	DFB_MPEG_720P_DIMENSION_W	1280
#define	DFB_MPEG_720P_DIMENSION_H	720

#define	DFB_MPEG_480_DIMENSION_W	720
#define	DFB_MPEG_480_DIMENSION_H	480

#define DFB_RECTANGLE_SET(rect, xx, yy, ww, hh) { \
		(rect)->x = (xx); \
		(rect)->y = (yy); \
		(rect)->w = (ww); \
		(rect)->h = (hh); \
}

#define DFB_REGION_SET(region, xx1, yy1, xx2, yy2) { \
		(region)->x1 = (xx1); \
		(region)->y1 = (yy1); \
		(region)->x2 = (xx2); \
		(region)->y2 = (yy2); \
}

/*
 * ARIB still window switching Infomation
 */
typedef struct {
	DFBBoolean   active;
	DFBBoolean   attribute;
	DFBRectangle *rectangle[DFB_SWITCHING_WINDOW_NUM];
} DFBSwitchingInfo;

/*
 * ARIB extended Display Layer.
 */
DECLARE_INTERFACE( IDirectFBARIBDisplayLayer )

/*
 * ARIB extended Window.
 */
DECLARE_INTERFACE( IDirectFBARIBWindow )


/******************************
 * IDirectFBARIBDisplayLayer  *
 *****************************/

DEFINE_CHILD_INTERFACE( IDirectFBARIBDisplayLayer, IDirectFBDisplayLayer,

     /*
      * Create an ARIB window within this ARIB layer given a
      * predefined ARIB window id (= ARIB plane id), and a
      * description of the window that is to be created.
      *
      */
     DFBResult (*CreateAribWindow) (
          IDirectFBARIBDisplayLayer  *thiz,
          DFBWindowID                arib_id,
          const DFBWindowDescription *desc,
          IDirectFBARIBWindow        **ret_interface
     );

     /*
      * Get an ARIB window within this ARIB layer given a
      * predefined ARIB window id (= ARIB plane id), and a
      * description of the window that is to be created.
      * for DEBUG
      */
     DFBResult (*GetAribWindow) (
          IDirectFBARIBDisplayLayer *thiz,
          DFBWindowID               arib_id,
          IDirectFBARIBWindow       **ret_interface
     );

     /*
      * Setting BML Visibility.
      */
     DFBResult (*SetBmlVisibility) (
          IDirectFBARIBDisplayLayer *thiz,
          DFBBoolean                bml_valid,
          DFBBoolean                bml_invisible,
          DFBBoolean                bml_has_video
     );

     /*
      * Set Mpeg Resolution.
      */
     DFBResult (*SetMpegResolution) (
          IDirectFBARIBDisplayLayer *thiz,
          DFBDimension              *seqhead_resolution,
          DFBDimension              *seqdisp_resolution,
          DFBBoolean                seqhead_aspect_ratio,
          DFBBoolean                progressive
     );

     /*
      * Set Data Broadcast Resolution.
      */
     DFBResult (*SetDBcastResolution) (
          IDirectFBARIBDisplayLayer *thiz,
          DFBDimension              *resolution,
          DFBBoolean                aspect_ratio
     );

     /*
      * Set a resolution change listener.
      *
      * [FIXME] needs a way to cancel ?
      */
     DFBResult (*SetResolutionListener) (
          IDirectFBARIBDisplayLayer *thiz,
          FusionCallHandler         callback,
          void                      *ctx
     );

     /*
      * Set a location change listener.
      *
      * [FIXME] needs a way to cancel ?
      */
     DFBResult (*SetLocationListener) (
          IDirectFBARIBDisplayLayer *thiz,
          FusionCallHandler         callback,
          void                      *ctx
     );

     /*
      * Get Monitor Infomation.
      *
      * [FIXME] needs a way to cancel ?
      */
     DFBResult (*GetMonitorInfo) (
          IDirectFBARIBDisplayLayer *thiz,
          DFBMonitorType            *monitortype,
          DFBMonitorAspectRatio     *monitor_aspect
     );

     /*
      * Set Arib Rssolution Table.
      *
      * [FIXME] needs a way to cancel ?
      */
     DFBResult (*SetResolutionTable) (
          IDirectFBARIBDisplayLayer *thiz,
          DFB_RESOLUTION_TABLE      *resolution_table
     );
)



/*
 * Specifies remote key-groups
 * cf. ARIB STD-B24 BML used-key-list CSS property.
 */
typedef enum {
     DARIBIKG_NONE           = 0x00000000,
     DARIBIKG_BASIC          = 0x00000001,  /* arrows, enter, back */
     DARIBIKG_DATA_BUTTON    = 0x00000002,  /* red, green, blue, yellow */
     DARIBIKG_NUMERIC_TUNING = 0x00000004,  /* 0-9,10,11,12 */
     DARIBIKG_OTHER_TUNING   = 0x00000008,  /* CHUP/DOWN etc. */
     DARIBIKG_SPECIAL1       = 0x00000010,
     DARIBIKG_SPECIAL2       = 0x00000020,
     DARIBIKG_SPECIAL3       = 0x00000040,
     DARIBIKG_SPECIAL4       = 0x00000080,
     DARIBIKG_MISC           = 0x00000100,

     DARIBIKG_ALL            = 0x000000FF
} DFBARIBInputDeviceKeyGroup;


/*****************************
 * IDirectFBARIBInputDevice  *
 *****************************/

DEFINE_CHILD_INTERFACE( IDirectFBARIBInputDevice, IDirectFBInputDevice,

     /*
      * Set Used Key List.
      */
     DFBResult (*SetARIBUsedKeyList) (
          IDirectFBARIBInputDevice      *thiz,
          DFBARIBInputDeviceKeyGroup     key_groups
     );

     /*
      * Get Used Key List.
      */
     DFBResult (*GetARIBUsedKeyList) (
          IDirectFBARIBInputDevice      *thiz,
          DFBARIBInputDeviceKeyGroup    *key_groups
     );

     /*
      * Set KeySuppressState.
      */
     DFBResult (*SetARIBKeySuppressState) (
          IDirectFBARIBInputDevice      *thiz,
          DFBBoolean                    suppress
     );
     /*
      * Get KeySuppressState.
      */
     DFBResult (*GetARIBKeySuppressState) (
          IDirectFBARIBInputDevice      *thiz,
          DFBBoolean                    *suppress
     );

     /*
      * Query KeyGroup from DirectFB KeySymbol
      */
     DFBResult (*QueryARIBKeyGroup) ( IDirectFBARIBInputDevice   *thiz,
                                      DFBARIBInputDeviceKeyGroup *key_groups,
                                      DFBInputDeviceKeySymbol    key_symbol);
)


/************************
 * IDirectFBARIBWindow  *
 ***********************/
DEFINE_CHILD_INTERFACE( IDirectFBARIBWindow, IDirectFBWindow,

     /*
      * Setting switching region.
      *
      * primarily for realizing ARIB Video and still picture switching plane.
      */
     DFBResult (*SetSwitchingRegion) (
          IDirectFBARIBWindow           *thiz,
          DFBRectangle                  *rect1,
          DFBRectangle                  *rect2,
          DFBRectangle                  *rect3,
          DFBRectangle                  *rect4,
          DFBBoolean                    attribute
     );

     /*
      *  Batch count up.
      */
     DFBResult (*BatchStart) (
          IDirectFBARIBWindow           *thiz
     );

     /*
      *  Batch count down.
      */
     DFBResult (*BatchEnd) (
          IDirectFBARIBWindow           *thiz
     );

)

/*
 *	ARIB Extension
 */
typedef enum {
     DARIBFF_RNDGOTHIC, /* round gothic */
     DARIBFF_GOTHIC,    /* gothic */
     DARIBFF_BOLDRNDGOTHIC, /* bold round gothic */
} DFBARIBFontFamily;

/*
 *	ARIB Extension
 */
typedef enum {
     DARIBFS_NORMAL = 0x00000000,
     DARIBFS_BOLD   = 0x00000001, /* bold */
     DARIBFS_ITALIC = 0x00000002, /* italic (not used) */
} DFBARIBFontStyle;

/*
 *	ARIB Extension
 */
typedef enum {
     DARIBFOP_NORMAL        = 0x00000000,
     DARIBFOP_HALF_WIDTH    = 0x00000001, /* half width */
     DARIBFOP_HALF_HEIGHT   = 0x00000002, /* half height */
     DARIBFOP_DOUBLE_WIDTH  = 0x00000004, /* double width */
     DARIBFOP_DOUBLE_HEIGHT = 0x00000008, /* double height */
     DARIBFOP_VERTICAL      = 0x00000010, /* glyph for vertical layout */
} DFBARIBFontOptions;

/*
 *	ARIB Extension
 */
typedef struct {
     DFBARIBFontFamily      family;    /* fontid */
     DFBARIBFontStyle       style;     /* style (or'd flags) */
     int                    size;      /* width, height in pixel */
} DFBARIBFontDesc;

/*
 *	ARIB Extension
 */
typedef enum {
     DARIBDRCSOP_LOADAS_RNDGOTHIC     = 0x00000001, /* force to load as round gothic */
     DARIBDRCSOP_LOADAS_GOTHIC        = 0x00000002, /* force to load as gothic */
     DARIBDRCSOP_LOADAS_BOLDRNDGOTHIC = 0x00000003, /* force to load as round gothic */
} DFBARIBDrcsOptions;

/*********************
 * IDirectFBARIBFont *
 *********************/

/*
 * <i><b>IDirectFBARIBFont</b></i> is a special FontProvider
 * dedicated to ARIB BML presentation and ARIB caption.
 *
 * ARIB has a number of differences from ordinary font system,
 * such as FreeType2:
 * <ol><li>Four level grayscale font must be used. Normally glyph images
 * are returned as 2bpp.
 *     <li>Gray-level palletes are strictly specified by the broadcasters
 * in its broadcast contents. We can not freely assign those intermediate
 * colors.
 *     <li>ARIB character code is not Unicode, rather EUC-JP for BML,
 * JIS0208 for ARIB caption. Moreover, both of them includes digital
 * broadcasting specific characters whose corresponding unicode code points
 * are not established.
 *     <li>DRCS (Dynamically Redefinable Character Sets). Broadcasters send
 * glyph pattern data which are mapped into the specified code points.
 *     <li>There is no proportional font. All are fixed.
 *     <li>ARIB fonts are drawn on only LUT8(AYCbCr CLUT) surfaces.
 * </ol>
 *
 * Considering the differences above, we have decided to make another font
 * provider for ARIB broadcasting.
 *
 * As we don't want to bother DFBSurface for ARIB specific APIs,
 * ARIBFontProvider also provides drawing methods. If you want to draw a
 * string on a surface, you need to provide its buffer to the ARIB Font
 * Provider.
 *    surface->Lock(surface, flags, &ptr, &pitch);
 *    aribFont->DrawEUCString(aribFont, desc, text, bytes, 0,
 *              ptr + (y * pitch) + x, pitch);
 *    surface->Unlock(surface);
 *
 * ARIBFontProvider is a process local object, and does not share common
 * data structure each other.
 */
DEFINE_INTERFACE(   IDirectFBARIBFont,

   /*** methods for BML ***/

     /*
	 * Set an ARIB DRCS manager to the ARIB FontProvider.
      *
      * Certain parts of character code area are assigned for DRCS.
      * They are 0x77xx-0x78xx(xx:21-7E) for JIS0208, and 0xF7yy-0xF8yy
      * (yy:A1-FE) for EUC. When a glyph for a character in the code area
      * requested, ARIBFontProvider requests associated ARIB DRCS manager
      * for the glyph.
      *
      * ARIB FontProvider can hold maximum of one ARIB DRCS manager.
      * Duplicated set will overrides the already held manager.
      * If argument drcs is NULL, the ARIB FontProvider just releases
      * the manager it has held.
      */
     DFBResult (*SetARIBDrcs) (
          IDirectFBARIBFont   *thiz,
          IDirectFBARIBDrcs   *drcs
     );


   /** String extents measurement **/

     /*
      * Get the extents of the specified EUC string as if it
      * were drawn with the font specified with argument desc.
      *
      * Bytes specifies the number of bytes to take from the
      * string or -1 for the complete NULL-terminated string.
      *
      * Letter_space specifies the number of pixels between
      * glyphs drawn. The space won't be inserted before the
      * first glyph nor after the last glyph.
      *
      * The ret_rect describes the the typographic extents.
      * The rectangle offsets are reported relative to the
      * left-top corner of the drawn string, so that (x,y) is
      * normally (0,0).
      */
     DFBResult (*GetEUCStringExtents) (
          IDirectFBARIBFont   *thiz,
          DFBARIBFontDesc     *desc,
          const char          *euc_text,
          int                  bytes,
          int                  letter_space,
          DFBRectangle        *ret_rect
     );

   /** Drawing control **/

     /*
      * Set the color palette used for drawing text function.
      *
      */

     DFBResult (*SetGrayLevelPalette) (
          IDirectFBARIBFont   *thiz,
          int                 *palette, /* array of color indexes for LUT8. minus for transparent */
          int                  level   /* number of entries in the gray-level pallet */
     );


   /** Text functions **/

     /*
      * Draw an EUC string at the specified buffer area with the
      * given gray-level palette, and with the font specified by
      * argument desc. A pixel in the glyph with gray level n is
      * colorized as a pixel with gray palette[n] set with the
      * SetGrayLevelPalette.
      *
      * Bytes specifies the number of bytes to take from the
      * string or -1 for the complete NULL-terminated string.
      *
      * Letter_space specifies the number of pixels between
      * glyphs drawn. The space won't be inserted before the
      * first glyph nor after the last glyph.
      *
      * The buf_ptr should address a point in a surfaces buffer
      * with an indexed pixelformat, e.g. DSPF_LUT8, and the point
      * will be the left-top corner of the string drawn.
      * The pitch describes the pitch of the surface buffer in bytes.
      */
     DFBResult (*DrawEUCString) (
          IDirectFBARIBFont   *thiz,
          DFBARIBFontDesc     *desc,
          const char          *euc_text,
          int                  bytes,
          int                  letter_space,
          void*                buf_ptr,
          int                  pitch
     );


   /*** methods for ARIB caption ***/

   /** Glyph handling **/

     /*
      * Get the extents of a glyph specified by its JIS0208 code.
      *
      * The ret_rect describes the the typographic extents.
      * The rectangle offsets are reported relative to the
      * left-top corner of the drawn string, so that (x,y) is
      * normally (0,0).
      */
     DFBResult (*GetJISGlyphExtents) (
          IDirectFBARIBFont   *thiz,
          DFBARIBFontDesc     *desc,
          DFBARIBFontOptions   options,
          __u16                jis0208_code,
          DFBRectangle        *ret_rect
     );

     /*
      * Get a single gray scale glyph specified by its JIS0208 code.
      * into the specified buffer area without colorizing.
      *
      * The font id, style, size are specified by arguments desc
      * and options.
      *
      * The buf_ptr should address a point in an application buffer
      * which is to save 2bpp gray-scale pixmap. The left-top corner
      * of the glyph is copied to the point the buf_ptr addressed.
      * The pitch describes the pitch of the buffer in bytes.
      */
     DFBResult (*GetJISGlyphImage) (
          IDirectFBARIBFont   *thiz,
          DFBARIBFontDesc     *desc,
          DFBARIBFontOptions   options,
          __u16                 jis0208_code,
          void*               buf_ptr,
          int                 pitch
     );


     /*
      * Draw a single gray scale glyph specified by its JIS0208 code.
      * into the specified buffer area with the given gray-level palette,
      * and with the font specified by argument desc. A pixel in the glyph
      * with gray level n is colorized as a pixel with gray palette[n] set
      * with the SetGrayLevelPalette.
      *
      * The font id, style, size are specified by arguments desc
      * and options.
      *
      * The buf_ptr should address a point in a surfaces buffer
      * with an indexed pixelformat, e.g. DSPF_LUT8, and the point
      * will be the left-top corner of the normal size glyph drawn.
      * The pitch describes the pitch of the buffer in bytes.
      *
      * If frame_color is not minus, additional glyph frame is drawn with the
      * frame color. In that case, drawn width and height are two pixel wider,
      * taller than the extents returned by GetJISGlyphExtents.
      */
     DFBResult (*DrawJISGlyphImage) (
          IDirectFBARIBFont   *thiz,
          DFBARIBFontDesc     *desc,
          DFBARIBFontOptions   options,
          __u16                jis0208_code,
          int                  frame_color,
          void*                buf_ptr,
          int                  pitch
     );
 )


/*********************
 * IDirectFBARIBDrcs *
 *********************/

/*
 * <i><b>IDirectFBARIBDrcs</b></i> is a DRCS manager for ARIB BML
 * presentation and ARIB caption.
 *
 * Certain parts of character code area are assigned for DRCS.
 * They are 0x77xx-0x78xx(xx:21-7E) for JIS0208, and 0xF7yy-0xF8yy
 * (yy:A1-FE) for EUC. The purpose of the DRCS is to complement rare
 * characters the standard character set does not have, e.g., historic
 * kanji characters. Broadcasters send DRCS pattern data, which contains
 * multiple sets of {fontid, size, characterCode, gray-scale-glyph-image}.
 */
DEFINE_INTERFACE(   IDirectFBARIBDrcs,

	/*
	 * Load a pattern data
	 *
	 * You may load pattern_data multiple times. It may overwrite a
	 * glyph image if the {fontid, size, characterCode} matches with
	 * already loaded one.
      *
      * When FontFamily indicated by the options argument, fontId's
      * contained in the pattern_data are ignored, and loaded as if they
      * were ones of indicated FontFamily.
	 */
     DFBResult (*Load) (
          IDirectFBARIBDrcs   *thiz,
          DFBARIBDrcsOptions   options,
          const char          *pattern_data,
          int                  length
     );

	/*
	 * Load a pattern file
	 *
      * You can load a pattern_data from file instead of memory.
	 */
     DFBResult (*LoadFile) (
          IDirectFBARIBDrcs   *thiz,
          DFBARIBDrcsOptions   options,
          const char          *filename
     );

	/*
	 * Unload all pattern data
	 */
     DFBResult (*Unload) (
          IDirectFBARIBDrcs   *thiz
     );

   /** String extents measurement **/

     /*
      * Get the extents of a glyph specified by its characterCode.
      *
      * The ret_rect describes the the typographic extents.
      * The rectangle offsets are reported relative to the
      * left-top corner of the drawn string, so that (x,y) is
	  * normally (0,0).
      */
    DFBResult (*GetGlyphExtents) (
          IDirectFBARIBDrcs   *thiz,
		  DFBARIBFontDesc     *desc,
		  DFBARIBFontOptions   options,
		  __u16                 characterCode,
          DFBRectangle        *ret_rect
     );

     /*
      * Get a single gray scale glyph specified by its characterCode
	  * into the specified buffer area without colorizing.
	  *
	  * The font id, style, size are specified by arguments desc
	  * and options.
       *
	  * The buf_ptr should address a point in an application buffer
	  * which is to save 2bpp gray-scale pixmap. The left-top corner
	  * of the glyph is copied to the point the buf_ptr addressed.
	  * The pitch describes the pitch of the buffer in bytes.
      */
     DFBResult (*GetGlyphImage) (
          IDirectFBARIBDrcs   *thiz,
          DFBARIBFontDesc     *desc,
          DFBARIBFontOptions   options,
          __u16                characterCode,
          void*                buf_ptr,
          int                  pitch
     );

   /** Drawing control **/

     /*
      * Set the color palette used for drawing text function.
      *
      */

     DFBResult (*SetGrayLevelPalette) (
          IDirectFBARIBDrcs   *thiz,
          int                 *palette, /* array of color indexes for LUT8. minus for transparent */
          int                  level   /* number of entries in the gray-level pallet */
     );

     /*
      * Draw a single gray scale glyph specified by its characterCode
      * into the specified buffer area with the given gray-level palette.
      *
      * The font id, style, size are specified by arguments desc
      * and options. A pixel in the glyph with gray level n is colorized
      * as a pixel with gray palette[n] set with the SetGrayLevelPalette.
      *
      * The buf_ptr should address a point in a surfaces buffer
      * with an indexed pixelformat, e.g. DSPF_LUT8, and the point
      * will be the left-top corner of the glyph drawn.
      * The pitch describes the pitch of the buffer in bytes.
      */
     DFBResult (*DrawGlyphImage) (
          IDirectFBARIBDrcs   *thiz,
          DFBARIBFontDesc     *desc,
          DFBARIBFontOptions   options,
          __u16                characterCode,
          int                  frame_color,
          void*                buf_ptr,
          int                  pitch
     );

 )
#ifdef __cplusplus
}
#endif

#endif /* __DIRECTFB_ARIB_H__ */
