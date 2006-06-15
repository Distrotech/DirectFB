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

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <strings.h>
#if defined(HAVE_SYSIO)
#include <sys/io.h>
#endif
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/kd.h>

#include <linux/fb.h>

#include <pthread.h>

#ifdef USE_SYSFS
# include <sysfs/libsysfs.h>
#endif

#include <fusion/shmalloc.h>
#include <fusion/reactor.h>
#include <fusion/arena.h>
#include <fusion/call.h>

#include <directfb_arib.h>
#include <directfb_strings.h>

#include <core/core.h>
#include <core/coredefs.h>
#include <core/coretypes.h>

#include <core/layer_control.h>
#include <core/layers_internal.h>
#include <core/layers.h>
#include <core/gfxcard.h>
#include <core/palette.h>
#include <core/screen.h>
#include <core/screens.h>
#include <core/surfaces.h>
#include <core/surfacemanager.h>
#include <core/state.h>
#include <core/windows.h>
#include <core/windowstack_arib.h>

#include <gfx/convert.h>
#include <gfx/util.h>

#include <direct/mem.h>
#include <direct/memcpy.h>
#include <direct/messages.h>
#include <direct/signals.h>
#include <direct/system.h>
#include <direct/util.h>

#include <misc/conf.h>
#include <misc/util.h>

#include "aribdev.h"

#include "vt.h"

#include <core/core_system.h>

/* DLBM_BACKVIDEO or DLBM_FRONTONLY */
#define CANVAS_BUFFER_MODE	DLBM_BACKVIDEO
#define ARIBDEV_LAYER_NUM	2

#undef	USED_GUI_TMP_SURFACE
#define	USED_ARIB_TMP_SURFACE
#define	EXPAND_MODIFY_RECT
#define	EXPAND_PIXEL	4

#if 0
static char SRC_FILE[] = "(+) [DirectFB/systems/aribdev/aribdev] ";
#define ARIBDEV_PRINT(x)	{ printf x; }
#else
#define ARIBDEV_PRINT(x)	{}
#endif


DFB_CORE_SYSTEM( aribdev )

#define FBDEV_IOCTL( request,arg )	fbdev_ioctl( request, arg, sizeof(*(arg)) )

static int fbdev_ioctl_call_handler(
					int  caller,
					int  call_arg,
					void *call_ptr,
					void *ctx );

static int fbdev_ioctl(
					int  request,
					void *arg,
					int  arg_size );

static DirectFBPixelFormatNames( format_names );

FBDev *dfb_fbdev = NULL;

/******************************************************************************/
/*                          primary layer function                            */
/******************************************************************************/
static int primaryLayerDataSize ();

static int primaryRegionDataSize();

static DFBResult primaryInitLayer(
						CoreLayer                  *layer,
						void                       *driver_data,
						void                       *layer_data,
						DFBDisplayLayerDescription *description,
						DFBDisplayLayerConfig      *config,
						DFBColorAdjustment         *adjustment );

static DFBResult primaryClearTmpSurface(
						CoreLayer                  *layer,
						void                       *driver_data,
						void                       *layer_data,
						DFBRectangle               *clear_rect );

static DFBResult primarySetColorAdjustment(
						CoreLayer                  *layer,
						void                       *driver_data,
						void                       *layer_data,
						DFBColorAdjustment         *adjustment );

static DFBResult primaryTestRegion(
						CoreLayer                  *layer,
						void                       *driver_data,
						void                       *layer_data,
						CoreLayerRegionConfig      *config,
						CoreLayerRegionConfigFlags *failed );

static DFBResult primaryAddRegion(
						CoreLayer                  *layer,
						void                       *driver_data,
						void                       *layer_data,
						void                       *region_data,
						CoreLayerRegionConfig      *config );

static DFBResult primarySetRegion(
						CoreLayer                  *layer,
						void                       *driver_data,
						void                       *layer_data,
						void                       *region_data,
						CoreLayerRegionConfig      *config,
						CoreLayerRegionConfigFlags updated,
						CoreSurface                *surface,
						CorePalette                *palette );

static DFBResult primaryRemoveRegion(
						CoreLayer                  *layer,
						void                       *driver_data,
						void                       *layer_data,
						void                       *region_data );

static DFBResult primaryFlipRegion(
						CoreLayer                  *layer,
						void                       *driver_data,
						void                       *layer_data,
						void                       *region_data,
						CoreSurface                *surface,
						DFBSurfaceFlipFlags        flags );

static DFBResult primaryAllocateSurface(
						CoreLayer                  *layer,
						void                       *driver_data,
						void                       *layer_data,
						void                       *region_data,
						CoreLayerRegionConfig      *config,
						CoreSurface                **ret_surface );

static DFBResult primaryReallocateSurface(
						CoreLayer                  *layer,
						void                       *driver_data,
						void                       *layer_data,
						void                       *region_data,
						CoreLayerRegionConfig      *config,
						CoreSurface                *surface );

static DFBResult primaryBlitLayerSurface(
						CoreLayer                  *layer,
						void                       *driver_data,
						void                       *layer_data,
						DFBRectangle               *src_full_rect,
						DFBRectangle               *dst_full_rect );

static int primaryCheckResolution(
                        CoreLayer                  *layer,
                        void                       *driver_data,
                        void                       *layer_data,
                        DFBDimension               *seqhead_resolution,
                        DFBDimension               *seqdisp_resolution,
                        DFBBoolean                 seqhead_aspect_ratio,
                        DFBBoolean                 seqhead_progressive );

static DFBBoolean primarySetMpegResolution(
						CoreLayer                  *layer,
						void                       *driver_data,
						void                       *layer_data,
						DFBDimension               *seqhead_resolution,
						DFBDimension               *seqdisp_resolution,
						DFBBoolean                 seqhead_aspect_ratio,
						DFBBoolean                 progressive,
						DFBDimension               *ret_resolution );

static DFBBoolean primarySetDBcastResolution(
						CoreLayer                  *layer,
						void                       *driver_data,
						void                       *layer_data,
						DFBDimension               *resolution,
						DFBBoolean                 aspect_ratio,
						DFBDimension               *ret_resolution );

static DFBBoolean primarySetBmlVisibility(
						CoreLayer                  *layer,
						void                       *driver_data,
						void                       *layer_data,
						DFBAribBmlInfo             *bml,
						DFBDimension               *ret_resolution );

static DFBResult primaryGetResolution(
						CoreLayer                  *layer,
						void                       *driver_data,
						void                       *layer_data,
						DFBDimension               *displayRes,
						DFBBoolean                 *displayAspect,
						DFBDimension               *canvasRes,
						DFBBoolean                 *canvasAspect,
						DFBDimension               *aribGfxRes,
						DFBBoolean                 *aribGfxAspect );

static DFBResult primaryGetBmlVisibility(
						CoreLayer                  *layer,
						void                       *driver_data,
						void                       *layer_data,
						DFBAribBmlInfo             *bml );

static DFBResult primaryGetVideoRectangle(
						CoreLayer                  *layer,
						void                       *driver_data,
						void                       *layer_data,
						DFBRectangle               *resolution,
						DFBRectangle               *ret_resolution,
                        DFBRectangle               *ret_mpeg_resolution,
						DFBBoolean                 *full_video,
                        DFBBoolean                 *aspect );

static DFBResult primaryGetDstRefRect(
						CoreLayer                  *layer,
						void                       *driver_data,
						void                       *layer_data,
						DFBRectangle               *src_rect,
						DFBRectangle               *src_full_rect,
						DFBRectangle               *dst_rect,
						DFBRectangle               *dst_full_rect,
						DFBRectangle               *modify_rect );

static DFBResult primaryGetTmpSurface(
						CoreLayer                  *layer,
						void                       *driver_data,
						void                       *layer_data,
						CoreSurface                **ret_surface );

static DFBResult primarySetCallBackFunction(
						CoreLayer                  *layer,
						void                       *driver_data,
						void                       *layer_data,
						DFBCallBackId              call_id,
						FusionCallHandler          callHandler,
						void                       *ctx );

static DFBResult primarySetResolutionTable(
						CoreLayer                  *layer,
						void                       *driver_data,
						void                       *layer_data,
						DFB_RESOLUTION_TABLE       *resolution_table );

/******************************************************************************/
/*                          arib layer function                               */
/******************************************************************************/
static DFBResult aribInitLayer(
						CoreLayer                  *layer,
						void                       *driver_data,
						void                       *layer_data,
						DFBDisplayLayerDescription *description,
						DFBDisplayLayerConfig      *config,
						DFBColorAdjustment         *adjustment );

static DFBResult aribFlipRegionMulti(
						CoreLayer                  *layer,
						void                       *driver_data,
						void                       *layer_data,
						void                       *region_data,
						DFBRegion                  *update,
						DFBSurfaceFlipFlags        flags );

static DFBResult aribBlitLayerSurface(
						CoreLayer                  *layer,
						void                       *driver_data,
						void                       *layer_data,
						DFBRectangle               *src_full_rect,
						DFBRectangle               *dst_full_rect );

static DFBResult aribClearTmpSurface(
						CoreLayer                  *layer,
						void                       *driver_data,
						void                       *layer_data,
						DFBRectangle               *clear_rect );

static int aribCheckResolution(
                        CoreLayer                  *layer,
                        void                       *driver_data,
                        void                       *layer_data,
                        DFBDimension               *seqhead_resolution,
                        DFBDimension               *seqdisp_resolution,
                        DFBBoolean                 seqhead_aspect_ratio,
                        DFBBoolean                 seqhead_progressive );

static DFBBoolean aribSetMpegResolution(
						CoreLayer                  *layer,
						void                       *driver_data,
						void                       *layer_data,
						DFBDimension               *seqhead_resolution,
						DFBDimension               *seqdisp_resolution,
						DFBBoolean                 seqhead_aspect_ratio,
						DFBBoolean                 progressive,
						DFBDimension               *ret_resolution );

static DFBBoolean aribSetDBcastResolution(
						CoreLayer                  *layer,
						void                       *driver_data,
						void                       *layer_data,
						DFBDimension               *resolution,
						DFBBoolean                 aspect_ratio,
						DFBDimension               *ret_resolution );

static DFBResult aribSetBmlVisibility(
						CoreLayer                  *layer,
						void                       *driver_data,
						void                       *layer_data,
						DFBAribBmlInfo             *bml,
						DFBDimension               *ret_resolution );

static DFBResult aribGetResolution(
						CoreLayer                  *layer,
						void                       *driver_data,
						void                       *layer_data,
						DFBDimension               *displayRes,
						DFBBoolean                 *displayAspect,
						DFBDimension               *canvasRes,
						DFBBoolean                 *canvasAspect,
						DFBDimension               *aribGfxRes,
						DFBBoolean                 *aribGfxAspect );

static DFBResult aribGetBmlVisibility(
						CoreLayer                  *layer,
						void                       *driver_data,
						void                       *layer_data,
						DFBAribBmlInfo             *bml );

static DFBResult aribGetVideoRectangle(
						CoreLayer                  *layer,
						void                       *driver_data,
						void                       *layer_data,
						DFBRectangle               *resolution,
						DFBRectangle               *ret_resolution,
                        DFBRectangle               *ret_mpeg_resolution,
						DFBBoolean                 *full_video,
                        DFBBoolean                 *aspect );

static DFBResult aribSetAribLocation(
						CoreLayer                  *layer,
						void                       *driver_data,
						void                       *layer_data,
						const DFBLocation          *location );

static DFBResult aribGetTmpSurface(
						CoreLayer                  *layer,
						void                       *driver_data,
						void                       *layer_data,
						CoreSurface                **ret_surface );

static DFBResult aribGetDstRefRect(
						CoreLayer                  *layer,
						void                       *driver_data,
						void                       *layer_data,
						DFBRectangle               *src_rect,
						DFBRectangle               *src_full_rect,
						DFBRectangle               *dst_rect,
						DFBRectangle               *dst_full_rect,
						DFBRectangle               *modify_rect );

static DFBResult aribGetCanvasSurface(
						CoreLayer                  *layer,
						void                       *driver_data,
						void                       *layer_data,
						CoreSurface                **ret_surface );

static DFBResult aribFbDevInformation(
						CoreLayer                  *layer,
						void                       *driver_data,
						void                       *layer_data );

static DFBResult aribSetCallBackFunction(
						CoreLayer                  *layer,
						void                       *driver_data,
						void                       *layer_data,
						DFBCallBackId              call_id,
						FusionCallHandler          callHandler,
						void                       *ctx );

static DFBResult aribSetResolutionTable(
						CoreLayer                  *layer,
						void                       *driver_data,
						void                       *layer_data,
						DFB_RESOLUTION_TABLE       *resolution_table );

/******************************************************************************/
/*                          layer function setting                            */
/******************************************************************************/
static DisplayLayerFuncs primaryLayerFuncs = {
		LayerDataSize:          primaryLayerDataSize,
		RegionDataSize:         primaryRegionDataSize,
		InitLayer:              primaryInitLayer,
		SetColorAdjustment:     primarySetColorAdjustment,
		TestRegion:             primaryTestRegion,
		AddRegion:              primaryAddRegion,
		SetRegion:              primarySetRegion,
		RemoveRegion:           primaryRemoveRegion,
		FlipRegion:             primaryFlipRegion,
		FlipRegionMulti:        aribFlipRegionMulti,
		AllocateSurface:        primaryAllocateSurface,
		ReallocateSurface:      primaryReallocateSurface,
		BlitLayerSurface:       primaryBlitLayerSurface,
		ClearTmpSurface:        primaryClearTmpSurface,
		CheckResolution:        primaryCheckResolution,
		SetMpegResolution:      primarySetMpegResolution,
		SetDBcastResolution:    primarySetDBcastResolution,
		SetBmlVisibility:       primarySetBmlVisibility,
		GetResolution:          primaryGetResolution,
		GetBmlVisibility:       primaryGetBmlVisibility,
		GetVideoRectangle:      primaryGetVideoRectangle,
		GetDstRefRect:          primaryGetDstRefRect,
		GetTmpSurface:          primaryGetTmpSurface,
		SetCallBackFunction:    primarySetCallBackFunction,
		GetCanvasSurface:       aribGetCanvasSurface,
		DspFbDevInformation:    aribFbDevInformation,
		SetResolutionTable:     primarySetResolutionTable,
    /* default DeallocateSurface copes with our chunkless video buffers */
};

static DisplayLayerFuncs aribLayerFuncs = {
		LayerDataSize:          primaryLayerDataSize,
		RegionDataSize:         primaryRegionDataSize,
		InitLayer:              aribInitLayer,
		SetColorAdjustment:     primarySetColorAdjustment,
		TestRegion:             primaryTestRegion,
		AddRegion:              primaryAddRegion,
		SetRegion:              primarySetRegion,
		RemoveRegion:           primaryRemoveRegion,
		FlipRegion:             primaryFlipRegion,
		FlipRegionMulti:        aribFlipRegionMulti,
		AllocateSurface:        primaryAllocateSurface,
		ReallocateSurface:      primaryReallocateSurface,
		BlitLayerSurface:       aribBlitLayerSurface,
		ClearTmpSurface:        aribClearTmpSurface,
		CheckResolution:        aribCheckResolution,
		SetMpegResolution:      aribSetMpegResolution,
		SetDBcastResolution:    aribSetDBcastResolution,
		SetBmlVisibility:       aribSetBmlVisibility,
		GetResolution:          aribGetResolution,
		GetBmlVisibility:       aribGetBmlVisibility,
		GetVideoRectangle:      aribGetVideoRectangle,
		SetAribLocation:        aribSetAribLocation,
		GetDstRefRect:          aribGetDstRefRect,
		GetTmpSurface:          aribGetTmpSurface,
		SetCallBackFunction:    aribSetCallBackFunction,
		GetCanvasSurface:       aribGetCanvasSurface,
		DspFbDevInformation:    aribFbDevInformation,
		SetResolutionTable:     aribSetResolutionTable,
    /* default DeallocateSurface copes with our chunkless video buffers */
};

/******************************************************************************/
static DFBResult primaryInitScreen(
						CoreScreen           *screen,
						GraphicsDevice       *device,
						void                 *driver_data,
						void                 *screen_data,
						DFBScreenDescription *description );
static DFBResult primarySetPowerMode(
						CoreScreen           *screen,
						void                 *driver_data,
						void                 *screen_data,
						DFBScreenPowerMode   mode );
static DFBResult primaryWaitVSync(
						CoreScreen           *screen,
						void                 *driver_data,
						void                 *layer_data );
static DFBResult primaryGetScreenSize(
						CoreScreen           *screen,
						void                 *driver_data,
						void                 *screen_data,
						int                  *ret_width,
						int                  *ret_height );
static ScreenFuncs primaryScreenFuncs = {
		.InitScreen    = primaryInitScreen,
		.SetPowerMode  = primarySetPowerMode,
		.WaitVSync     = primaryWaitVSync,
		.GetScreenSize = primaryGetScreenSize
};

/******************************************************************************/
static DFBResult dfb_fbdev_read_modes();
static DFBResult dfb_fbdev_set_gamma_ramp(
						DFBSurfacePixelFormat format );
static DFBResult dfb_fbdev_set_rgb332_palette();
static DFBResult dfb_fbdev_pan(
						int   offset,
						bool  onsync );
static DFBResult dfb_fbdev_blank(
						int  level );
static DFBResult dfb_fbdev_set_mode_canvas(
						CoreSurface *surface,
						VideoMode   *mode );
static DFBResult dfb_fbdev_chg_mode_canvas(
						DFBDimension *video_resolution,
						DFBDimension *canvas_resolution,
						unsigned int canvas_aspect,
						DFBBoolean   progressive );
static DFBResult initial_arib_construct( void );
static DFBResult clear_surface(
						CardState    *state,
						CoreSurface  *surface,
						DFBRectangle *rect );
static DFBResult blit_tmp_to_layer(
						CardState    *state,
						CoreSurface  *source,
						CoreSurface  *destination,
						DFBRectangle *src_rect,
						DFBRectangle *dst_rect );
static DFBResult blit_layer_to_canvas(
						int          num,
						DFBRectangle *src_rect,
						CoreSurface  **src_surface,
						DFBRectangle *dst_rect,
						CoreSurface  *dst_surface,
						CardState    *state );

static DFBResult allocate_canvas_surface(
						DFBDisplayLayerBufferMode buffermode );
static DFBResult allocate_tmp_surface(
						CoreSurfacePolicy policy );
static DFBResult get_canvas_dstrect(
						DFBRectangle *src_rect,
						DFBRectangle *dst_rect );
static DFBResult expand_rectangle(
						DFBRectangle *modify_rect,
						DFBRectangle *maximum_rect,
						int          expand_pixel );
static DFBResult callback_execute(
						int           request,
						void          *arg,
						int           arg_size,
						DFBCallBackId call_id );
static char *format_string(
						DFBSurfacePixelFormat fmt );
static char *caps_string(
						DFBSurfaceCapabilities caps );
static char *aspect_string(
						DFBBoolean aspect );
static int buffer_sizes(
						CoreSurface *surface,
						bool        video );
static inline int buffer_size(
						CoreSurface   *surface,
						SurfaceBuffer *buffer,
						bool          video );
static void	dfb_fbdev_var_to_mode(
						struct fb_var_screeninfo *var,
						VideoMode *mode );

/******************************************************************************/
static DFBEnumerationResult
GetLayer_Callback(
			CoreLayer *layer,
			void      *ctx )
{
	CoreLayerContext *context;
	CoreLayerRegion  *region;
	Layers_Context   *layers = (Layers_Context *)ctx;
	DFBDisplayLayerDescription desc;

	layers->layer   = NULL;
	layers->surface = NULL;

	context = layer->shared->contexts.primary;
	if (!context)
		return DFENUM_OK;

	region = context->primary.region;
	if (!region)
		return DFENUM_OK;

	if (layers->layer_type) {
		dfb_layer_get_description( layer, &desc );

		if (desc.type & layers->layer_type) {
			layers->layer   = layer;
			layers->id      = layer->shared->layer_id;
			layers->surface = region->surface;
			return DFENUM_CANCEL;
		}
	}
	return DFENUM_OK;
}

/******************************************************************************/

static inline
void waitretrace (void)
{
#if defined(HAVE_INB_OUTB_IOPL)
	if (iopl(3))
		return;

	if (!(inb (0x3cc) & 1)) {
		while ((inb (0x3ba) & 0x8))
			;

		while (!(inb (0x3ba) & 0x8))
			;
	}
	else {
		while ((inb (0x3da) & 0x8))
			;

		while (!(inb (0x3da) & 0x8))
			;
	}
#endif
}

/******************************************************************************/

static DFBResult dfb_fbdev_open()
{
	ARIBDEV_PRINT(("%s%s: \n", SRC_FILE, __FUNCTION__));

	if (dfb_config->fb_device) {
		dfb_fbdev->fd = open( dfb_config->fb_device, O_RDWR );
		if (dfb_fbdev->fd < 0) {
			D_PERROR( "DirectFB/ARIBDev: Error opening `%s'!\n",
					dfb_config->fb_device);
			return errno2result( errno );
		}
	}
	else if (getenv( "FRAMEBUFFER" ) && *getenv( "FRAMEBUFFER" ) != '\0') {
		dfb_fbdev->fd = open( getenv ("FRAMEBUFFER"), O_RDWR );
		if (dfb_fbdev->fd < 0) {
			D_PERROR( "DirectFB/ARIBDev: Error opening `%s'!\n",
					getenv ("FRAMEBUFFER"));

			return errno2result( errno );
		}
	}
	else {
		dfb_fbdev->fd = direct_try_open( "/dev/fb0", "/dev/fb/0", O_RDWR, true );
		if (dfb_fbdev->fd < 0) {
			D_ERROR( "DirectFB/ARIBDev: Error opening framebuffer device!\n" );
			D_ERROR( "DirectFB/ARIBDev: Use 'fbdev' option or set FRAMEBUFFER environment variable.\n" );
			return DFB_INIT;
		}
	}

    return DFB_OK;
}

static void
dfb_fbdev_get_pci_info( FBDevShared *shared )
{
     char buf[512];
     int  vendor = -1;
     int  model  = -1;

#ifdef USE_SYSFS
     if (!sysfs_get_mnt_path( buf, 512 )) {
          struct sysfs_class_device *classdev;
          struct sysfs_device       *device;
          struct sysfs_attribute    *attr;
          char                      *fbdev;
          char                       dev[5] = { 'f', 'b', '0', 0, 0 };

          fbdev = dfb_config->fb_device;
          if (!fbdev)
               fbdev = getenv( "FRAMEBUFFER" );

          if (fbdev) {
               if (!strncmp( fbdev, "/dev/fb/", 8 ))
                    snprintf( dev, 5, "fb%s", fbdev+8 );
               else if (!strncmp( fbdev, "/dev/fb", 7 ))
                    snprintf( dev, 5, "fb%s", fbdev+7 );
          }

          classdev = sysfs_open_class_device( "graphics", dev );
          if (classdev) {
               device = sysfs_get_classdev_device( classdev );

               if (device) {
                    attr = sysfs_get_device_attr( device, "vendor" );
                    if (attr)
                           sscanf( attr->value, "0x%04x", &vendor );

                    attr = sysfs_get_device_attr( device, "device" );
                    if (attr)
                         sscanf( attr->value, "0x%04x", &model );

                    if (vendor != -1 && model != -1) {
                         sscanf( device->name, "0000:%02x:%02x.%1x",
                                 &shared->pci.bus,
                                 &shared->pci.dev,
                                 &shared->pci.func );

                         shared->device.vendor = vendor;
                         shared->device.model  = model;
                    }
               }

               sysfs_close_class_device( classdev );
          }
     }
#endif /* USE_SYSFS */

     /* try /proc interface */
     if (vendor == -1 || model == -1) {
          FILE *fp;
          int   id;
          int   bus;
          int   dev;
          int   func;

          fp = fopen( "/proc/bus/pci/devices", "r" );
          if (!fp) {
               D_PERROR( "DirectFB/FBDev: "
                         "couldn't access /proc/bus/pci/devices!\n" );
               return;
          }

          while (fgets( buf, 512, fp )) {
               if (sscanf( buf, "%04x\t%04x%04x", &id, &vendor, &model ) == 3) {
                    bus  = (id & 0xff00) >> 8;
                    dev  = (id & 0x00ff) >> 3;
                    func = (id & 0x0007);

                    if (bus  == dfb_config->pci.bus &&
                        dev  == dfb_config->pci.dev &&
                        func == dfb_config->pci.func)
                    {
                         shared->pci.bus  = bus;
                         shared->pci.dev  = dev;
                         shared->pci.func = func;

                         shared->device.vendor = vendor;
                         shared->device.model  = model;

                         break;
                    }
               }
          }

          fclose( fp );
     }
}

/** public **/

static void
system_get_info(
			CoreSystemInfo *info )
{
	ARIBDEV_PRINT(("%s%s: \n", SRC_FILE, __FUNCTION__));

	info->type = CORE_FBDEV;    /* ARIB extension */

	info->caps = CSCAPS_ACCELERATION;

	info->version.major = 0;

	info->version.minor = 1;

	snprintf( info->name, DFB_CORE_SYSTEM_INFO_NAME_LENGTH, "Aribdev" );

	snprintf( info->vendor, DFB_CORE_SYSTEM_INFO_VENDOR_LENGTH, "Mitsubishi Electric Corp" );
}

static DFBResult
system_initialize(
			CoreDFB *core,
			void    **data )
{
	DFBResult  ret;
	long       page_size;
	CoreScreen *screen;
	CoreSystemInfo info;

	ARIBDEV_PRINT(("%s%s: \n", SRC_FILE, __FUNCTION__));

	system_get_info( &info );
	D_INFO( "DirectFB/Core/system: %s %d.%d (%s)\n",
		info.name, info.version.major, info.version.minor, info.vendor );

	D_ASSERT( dfb_fbdev == NULL );

	dfb_fbdev = D_CALLOC( 1, sizeof(FBDev) );

	dfb_fbdev->shared = (FBDevShared*) SHCALLOC( 1, sizeof(FBDevShared) );

	fusion_arena_add_shared_field( dfb_core_arena( core ),
	                               "aribdev", dfb_fbdev->shared );

	dfb_fbdev->core = core;

	page_size = direct_pagesize();
	dfb_fbdev->shared->page_mask = page_size < 0 ? 0 : (page_size - 1);

	ret = dfb_fbdev_open();
	if (ret) {
		SHFREE( dfb_fbdev->shared );
		D_FREE( dfb_fbdev );
		dfb_fbdev = NULL;

		return ret;
	}

	if (dfb_config->vt) {
		ret = dfb_vt_initialize();
		if (ret) {
			SHFREE( dfb_fbdev->shared );
			D_FREE( dfb_fbdev );
			dfb_fbdev = NULL;

			return ret;
		}
	}

	/* Retrieve fixed informations like video ram size */
	if (ioctl( dfb_fbdev->fd, FBIOGET_FSCREENINFO, &dfb_fbdev->shared->fix ) < 0) {
		D_PERROR( "DirectFB/ARIBDev: "
				"Could not get fixed screen information!\n" );
		SHFREE( dfb_fbdev->shared );
		close( dfb_fbdev->fd );
		D_FREE( dfb_fbdev );
		dfb_fbdev = NULL;

		return DFB_INIT;
	}

    /* Map the framebuffer */
	dfb_fbdev->framebuffer_base = mmap( NULL, dfb_fbdev->shared->fix.smem_len,
										PROT_READ | PROT_WRITE, MAP_SHARED,
										dfb_fbdev->fd, 0 );
	if ((int)(dfb_fbdev->framebuffer_base) == -1) {
		D_PERROR( "DirectFB/ARIBDev: "
				"Could not mmap the framebuffer!\n");
		SHFREE( dfb_fbdev->shared );
		close( dfb_fbdev->fd );
		D_FREE( dfb_fbdev );
		dfb_fbdev = NULL;

		return DFB_INIT;
	}

	if (ioctl( dfb_fbdev->fd, FBIOGET_VSCREENINFO, &dfb_fbdev->shared->orig_var ) < 0) {
		D_PERROR( "DirectFB/ARIBDev: "
				"Could not get variable screen information!\n" );
		SHFREE( dfb_fbdev->shared );
		munmap( dfb_fbdev->framebuffer_base, dfb_fbdev->shared->fix.smem_len );
		close( dfb_fbdev->fd );
		D_FREE( dfb_fbdev );
		dfb_fbdev = NULL;

		return DFB_INIT;
	}

	dfb_fbdev->shared->current_var = dfb_fbdev->shared->orig_var;
	dfb_fbdev->shared->current_var.accel_flags = 0;

	if (ioctl( dfb_fbdev->fd, FBIOPUT_VSCREENINFO, &dfb_fbdev->shared->current_var ) < 0) {
		D_PERROR( "DirectFB/ARIBDev: "
				"Could not disable console acceleration!\n" );
		SHFREE( dfb_fbdev->shared );
		munmap( dfb_fbdev->framebuffer_base, dfb_fbdev->shared->fix.smem_len );
		close( dfb_fbdev->fd );
		D_FREE( dfb_fbdev );
		dfb_fbdev = NULL;

		return DFB_INIT;
	}
	dfb_fbdev_var_to_mode( &dfb_fbdev->shared->current_var,
	                        &dfb_fbdev->shared->current_mode );

	dfb_fbdev->shared->orig_cmap.start  = 0;
	dfb_fbdev->shared->orig_cmap.len    = 256;
	dfb_fbdev->shared->orig_cmap.red    = (__u16*)SHMALLOC( 2 * 256 );
	dfb_fbdev->shared->orig_cmap.green  = (__u16*)SHMALLOC( 2 * 256 );
	dfb_fbdev->shared->orig_cmap.blue   = (__u16*)SHMALLOC( 2 * 256 );
	dfb_fbdev->shared->orig_cmap.transp = (__u16*)SHMALLOC( 2 * 256 );

	if (ioctl( dfb_fbdev->fd, FBIOGETCMAP, &dfb_fbdev->shared->orig_cmap ) < 0) {
		D_PERROR( "DirectFB/ARIBDev: "
				"Could not retrieve palette for backup!\n" );
		SHFREE( dfb_fbdev->shared->orig_cmap.red );
		SHFREE( dfb_fbdev->shared->orig_cmap.green );
		SHFREE( dfb_fbdev->shared->orig_cmap.blue );
		SHFREE( dfb_fbdev->shared->orig_cmap.transp );
		dfb_fbdev->shared->orig_cmap.len = 0;
	}

	dfb_fbdev->shared->temp_cmap.len       = 256;
	dfb_fbdev->shared->temp_cmap.red       = SHCALLOC( 256, 2 );
	dfb_fbdev->shared->temp_cmap.green     = SHCALLOC( 256, 2 );
	dfb_fbdev->shared->temp_cmap.blue      = SHCALLOC( 256, 2 );
	dfb_fbdev->shared->temp_cmap.transp    = SHCALLOC( 256, 2 );

	dfb_fbdev->shared->current_cmap.len    = 256;
	dfb_fbdev->shared->current_cmap.red    = SHCALLOC( 256, 2 );
	dfb_fbdev->shared->current_cmap.green  = SHCALLOC( 256, 2 );
	dfb_fbdev->shared->current_cmap.blue   = SHCALLOC( 256, 2 );
	dfb_fbdev->shared->current_cmap.transp = SHCALLOC( 256, 2 );

	dfb_fbdev_get_pci_info( dfb_fbdev->shared );

	/* Init Definition Information */
	initial_arib_construct();

	fusion_call_init( &dfb_fbdev->shared->fbdev_ioctl,
				 	  fbdev_ioctl_call_handler, NULL );

	/* Register primary screen functions */
	screen = dfb_screens_register( NULL, NULL, &primaryScreenFuncs );

	/* Register primary layer functions */
	dfb_layers_register( screen, NULL, &primaryLayerFuncs );

	/* DFB_ARIB :Register ARIB layer functions */
	dfb_layers_register( screen, NULL, &aribLayerFuncs );

	*data = dfb_fbdev;

	return DFB_OK;
}

static DFBResult
system_join(
			CoreDFB *core,
			void    **data )
{
	DFBResult  ret;
	CoreScreen *screen;

	D_ASSERT( dfb_fbdev == NULL );

	ARIBDEV_PRINT(("%s%s: \n", SRC_FILE, __FUNCTION__));

	if (dfb_config->vt) {
		ret = dfb_vt_join();
		if (ret)
			return ret;
	}

	dfb_fbdev = D_CALLOC( 1, sizeof(FBDev) );

	fusion_arena_get_shared_field( dfb_core_arena( core ),
	                               "aribdev",
	                               (void**) &dfb_fbdev->shared );

	dfb_fbdev->core = core;

	/* Open framebuffer device */
	ret = dfb_fbdev_open();
	if (ret) {
		D_FREE( dfb_fbdev );
		dfb_fbdev = NULL;
		return ret;
	}

	/* Map the framebuffer */
	dfb_fbdev->framebuffer_base = mmap( NULL, dfb_fbdev->shared->fix.smem_len,
										PROT_READ | PROT_WRITE, MAP_SHARED,
										dfb_fbdev->fd, 0 );
	if ((int)(dfb_fbdev->framebuffer_base) == -1) {
		D_PERROR( "DirectFB/ARIBDev: "
				"Could not mmap the framebuffer!\n");
		close( dfb_fbdev->fd );
		D_FREE( dfb_fbdev );
		dfb_fbdev = NULL;

		return DFB_INIT;
	}

	/* Register primary screen functions */
	screen = dfb_screens_register( NULL, NULL, &primaryScreenFuncs );

	/* Register primary layer functions */
	dfb_layers_register( screen, NULL, &primaryLayerFuncs );

	/* Register ARIB layer functions */
	dfb_layers_register( screen, NULL, &aribLayerFuncs );

	*data = dfb_fbdev;

	return DFB_OK;
}

static DFBResult
system_shutdown(
			bool emergency )
{
	DFBResult ret;
	VideoMode *m;

	ARIBDEV_PRINT(("%s%s: \n", SRC_FILE, __FUNCTION__));

	D_ASSERT( dfb_fbdev != NULL );

	m = dfb_fbdev->shared->modes;
	while (m) {
		VideoMode *next = m->next;
		SHFREE( m );
		m = next;
	}

	if (ioctl( dfb_fbdev->fd, FBIOPUT_VSCREENINFO, &dfb_fbdev->shared->orig_var ) < 0) {
		D_PERROR( "DirectFB/ARIBDev: "
				"Could not restore variable screen information!\n" );
	}

	if (dfb_fbdev->shared->orig_cmap.len) {
		if (ioctl( dfb_fbdev->fd, FBIOPUTCMAP, &dfb_fbdev->shared->orig_cmap ) < 0)
			D_PERROR( "DirectFB/ARIBDev: "
					"Could not restore palette!\n" );

		SHFREE( dfb_fbdev->shared->orig_cmap.red );
		SHFREE( dfb_fbdev->shared->orig_cmap.green );
		SHFREE( dfb_fbdev->shared->orig_cmap.blue );
		SHFREE( dfb_fbdev->shared->orig_cmap.transp );
	}

	SHFREE( dfb_fbdev->shared->temp_cmap.red );
	SHFREE( dfb_fbdev->shared->temp_cmap.green );
	SHFREE( dfb_fbdev->shared->temp_cmap.blue );
	SHFREE( dfb_fbdev->shared->temp_cmap.transp );

	SHFREE( dfb_fbdev->shared->current_cmap.red );
	SHFREE( dfb_fbdev->shared->current_cmap.green );
	SHFREE( dfb_fbdev->shared->current_cmap.blue );
	SHFREE( dfb_fbdev->shared->current_cmap.transp );

	SHFREE( dfb_fbdev->shared->resolution_table.ptr );

	fusion_call_destroy( &dfb_fbdev->shared->fbdev_ioctl );

	munmap( dfb_fbdev->framebuffer_base, dfb_fbdev->shared->fix.smem_len );

	if (dfb_config->vt) {
		ret = dfb_vt_shutdown( emergency );
		if (ret)
			return ret;
	}

	close( dfb_fbdev->fd );

	SHFREE( dfb_fbdev->shared );
	D_FREE( dfb_fbdev );
	dfb_fbdev = NULL;

	return DFB_OK;
}

static DFBResult
system_leave(
			bool emergency )
{
	DFBResult ret;

	D_ASSERT( dfb_fbdev != NULL );

	ARIBDEV_PRINT(("%s%s: \n", SRC_FILE, __FUNCTION__));

	munmap( dfb_fbdev->framebuffer_base,
	dfb_fbdev->shared->fix.smem_len );

	if (dfb_config->vt) {
		ret = dfb_vt_leave( emergency );
		if (ret)
			return ret;
	}

	close( dfb_fbdev->fd );

	D_FREE( dfb_fbdev );
	dfb_fbdev = NULL;

	return DFB_OK;
}

static DFBResult
system_suspend()
{
	return DFB_OK;
}

static DFBResult
system_resume()
{
	return DFB_OK;
}

/******************************************************************************/

static volatile void *
system_map_mmio(
			unsigned int offset,
			int          length )
{
	void *addr;

	ARIBDEV_PRINT(("%s%s: \n", SRC_FILE, __FUNCTION__));

	if (length <= 0)
		length = dfb_fbdev->shared->fix.mmio_len;

	addr = mmap( NULL, length, PROT_READ | PROT_WRITE, MAP_SHARED,
	dfb_fbdev->fd, dfb_fbdev->shared->fix.smem_len + offset );
	if ((int)(addr) == -1) {
		D_PERROR( "DirectFB/ARIBDev: Could not mmap MMIO region "
				"(offset %d, length %d)!\n", offset, length );
		return NULL;
	}

	return(volatile void*) ((__u8*) addr + (dfb_fbdev->shared->fix.mmio_start &
							dfb_fbdev->shared->page_mask));
}

static void
system_unmap_mmio(
			volatile void *addr,
			int            length )
{
	ARIBDEV_PRINT(("%s%s: \n", SRC_FILE, __FUNCTION__));

	if (length <= 0)
		length = dfb_fbdev->shared->fix.mmio_len;

	if (munmap( (void*) ((__u8*) addr - (dfb_fbdev->shared->fix.mmio_start &
						dfb_fbdev->shared->page_mask)), length ) < 0)
		D_PERROR( "DirectFB/ARIBDev: Could not unmap MMIO region "
				"at %p (length %d)!\n", addr, length );
}

static int
system_get_accelerator()
{
#ifdef FB_ACCEL_MATROX_MGAG400
	if (!strcmp( dfb_fbdev->shared->fix.id, "MATROX DH" ))
		return FB_ACCEL_MATROX_MGAG400;
#endif

	return dfb_fbdev->shared->fix.accel;
}

static VideoMode *
system_get_modes()
{
	return dfb_fbdev->shared->modes;
}

static VideoMode *
system_get_current_mode()
{
	return &dfb_fbdev->shared->current_mode;
}

static DFBResult
system_thread_init()
{
	ARIBDEV_PRINT(("%s%s: \n", SRC_FILE, __FUNCTION__));

	if (dfb_config->block_all_signals)
		direct_signals_block_all();

	if (dfb_config->vt)
		return dfb_vt_detach( false );

	return DFB_OK;
}

static bool
system_input_filter(
			CoreInputDevice *device,
			DFBInputEvent   *event )
{
	ARIBDEV_PRINT(("%s%s: \n", SRC_FILE, __FUNCTION__));

	if (dfb_config->vt && dfb_config->vt_switching) {
		switch (event->type) {
		case DIET_KEYPRESS:
			if (DFB_KEY_TYPE(event->key_symbol) == DIKT_FUNCTION &&
				event->modifiers == (DIMM_CONTROL | DIMM_ALT))
			return dfb_vt_switch( event->key_symbol - DIKS_F1 + 1 );

			break;

		case DIET_KEYRELEASE:
			if (DFB_KEY_TYPE(event->key_symbol) == DIKT_FUNCTION &&
				event->modifiers == (DIMM_CONTROL | DIMM_ALT))
			return true;

			break;

		default:
			break;
		}
	}

	return false;
}

static unsigned long
system_video_memory_physical(
			unsigned int offset )
{
	ARIBDEV_PRINT(("%s%s: \n", SRC_FILE, __FUNCTION__));

	return dfb_fbdev->shared->fix.smem_start + offset;
}

static void *
system_video_memory_virtual(
			unsigned int offset )
{
	ARIBDEV_PRINT(("%s%s: \n", SRC_FILE, __FUNCTION__));

	return(void*)((__u8*)(dfb_fbdev->framebuffer_base) + offset);
}

static unsigned int
system_videoram_length()
{
	return dfb_fbdev->shared->fix.smem_len;
}

static void
system_get_busid( int *ret_bus, int *ret_dev, int *ret_func )
{
     *ret_bus  = dfb_fbdev->shared->pci.bus;
     *ret_dev  = dfb_fbdev->shared->pci.dev;
     *ret_func = dfb_fbdev->shared->pci.func;
}

static void
system_get_deviceid( unsigned int *ret_vendor_id,
                     unsigned int *ret_device_id )
{
     *ret_vendor_id = dfb_fbdev->shared->device.vendor;
     *ret_device_id = dfb_fbdev->shared->device.model;
}

static DFBResult
system_canvas_surface_unref(
			CoreDFB *core )
{
	if (dfb_core_is_master( core )) {
		ARIBDEV_PRINT(("%s%s: \n", SRC_FILE, __FUNCTION__));

		if (dfb_fbdev->shared->canvas_surface) {
			dfb_surface_unref( dfb_fbdev->shared->canvas_surface );
			dfb_fbdev->shared->canvas_surface = NULL;
		}

		if (dfb_fbdev->shared->arib_tmp_surface) {
			dfb_surface_unref( dfb_fbdev->shared->arib_tmp_surface );
			dfb_fbdev->shared->arib_tmp_surface = NULL;
		}
		if (dfb_fbdev->shared->gui_tmp_surface) {
			dfb_surface_unref( dfb_fbdev->shared->gui_tmp_surface );
			dfb_fbdev->shared->gui_tmp_surface = NULL;
		}
	}
	return DFB_OK;
}

/******************************************************************************/
static DFBResult
init_modes()
{
	ARIBDEV_PRINT(("%s%s: \n", SRC_FILE, __FUNCTION__));

	dfb_fbdev_read_modes();

	if (!dfb_fbdev->shared->modes) {
		/* try to use current mode*/
		dfb_fbdev->shared->modes = (VideoMode*) SHCALLOC( 1, sizeof(VideoMode) );

		*dfb_fbdev->shared->modes = dfb_fbdev->shared->current_mode;
	}
    return DFB_OK;
}

/******************************************************************************/
static DFBResult
primaryInitScreen(
			CoreScreen           *screen,
			GraphicsDevice       *device,
			void                 *driver_data,
			void                 *screen_data,
			DFBScreenDescription *description )
{
	A_TRACE("%s:\n", __FUNCTION__);

	/* Set the screen capabilities. */
	description->caps = DSCCAPS_VSYNC | DSCCAPS_POWER_MANAGEMENT;

	/* Set the screen name. */
	snprintf( description->name,
			DFB_SCREEN_DESC_NAME_LENGTH, "ARIBDev Primary Screen" );

	/* initialize mode table */
	init_modes();

	/* DLBM_BACKVIDEO DLBM_FRONTONLY */
	allocate_canvas_surface( CANVAS_BUFFER_MODE );

	allocate_tmp_surface( CSP_VIDEOONLY );

	return DFB_OK;
}

static DFBResult
primarySetPowerMode(
			CoreScreen         *screen,
			void               *driver_data,
			void               *screen_data,
			DFBScreenPowerMode mode )
{
	int level;

	ARIBDEV_PRINT(("%s%s: \n", SRC_FILE, __FUNCTION__));

	switch (mode) {
	case DSPM_OFF:
		level = 4;
		break;
	case DSPM_SUSPEND:
		level = 3;
		break;
	case DSPM_STANDBY:
		level = 2;
		break;
	case DSPM_ON:
		level = 0;
		break;
	default:
		return DFB_INVARG;
	}

	return dfb_fbdev_blank( level );
}

static DFBResult
primaryWaitVSync(
			CoreScreen *screen,
			void       *driver_data,
			void       *screen_data )
{
	static const int zero = 0;

	ARIBDEV_PRINT(("%s%s: \n", SRC_FILE, __FUNCTION__));

	if (dfb_config->pollvsync_none)
		return DFB_OK;

	if (ioctl( dfb_fbdev->fd, FBIO_WAITFORVSYNC, &zero ))
		waitretrace();

	return DFB_OK;
}

static DFBResult
primaryGetScreenSize(
			CoreScreen *screen,
			void       *driver_data,
			void       *screen_data,
			int        *ret_width,
			int        *ret_height )
{
	D_ASSERT( dfb_fbdev != NULL );
	D_ASSERT( dfb_fbdev->shared != NULL );

	ARIBDEV_PRINT(("%s%s: \n", SRC_FILE, __FUNCTION__));

	*ret_width  = dfb_fbdev->shared->current_mode.xres;
	*ret_height = dfb_fbdev->shared->current_mode.yres;

	return DFB_OK;
}

/******************************************************************************/

static int
primaryLayerDataSize()
{
    return 0;
}

static int
primaryRegionDataSize()
{
    return 0;
}

static DFBResult
primaryInitLayer(
			CoreLayer                  *layer,
			void                       *driver_data,
			void                       *layer_data,
			DFBDisplayLayerDescription *description,
			DFBDisplayLayerConfig      *config,
			DFBColorAdjustment         *adjustment )
{
	A_TRACE("%s: layer[%p]\n", __FUNCTION__, layer);

	/* set capabilities and type */
	description->caps = DLCAPS_SURFACE
					  | DLCAPS_CONTRAST
					  | DLCAPS_SATURATION
					  | DLCAPS_BRIGHTNESS;

	description->type = DLTF_GRAPHICS;

	/* set name */
	snprintf( description->name, DFB_DISPLAY_LAYER_DESC_NAME_LENGTH, "ARIBDev Primary Layer" );

	/* fill out default color adjustment */
	adjustment->flags = DCAF_BRIGHTNESS
					  | DCAF_CONTRAST
					  | DCAF_SATURATION;

	adjustment->brightness = 0x8000;
	adjustment->contrast   = 0x8000;
	adjustment->saturation = 0x8000;

	/* fill out the default configuration */
	config->flags = DLCONF_WIDTH
				  | DLCONF_HEIGHT
				  | DLCONF_PIXELFORMAT
				  | DLCONF_BUFFERMODE;

	config->buffermode = DLBM_FRONTONLY;
	config->width  = dfb_config->mode.width;
	config->height = dfb_config->mode.height;

	if (dfb_config->g_format != DSPF_UNKNOWN) {
		config->pixelformat = dfb_config->g_format;
	}
	else {
		config->pixelformat  = dfb_config->mode.format;
		dfb_config->g_format = dfb_config->mode.format;
	}

	return DFB_OK;
}

static DFBResult
primaryClearTmpSurface(
			CoreLayer    *layer,
			void         *driver_data,
			void         *layer_data,
			DFBRectangle *clear_rect )
{
	CardState *state;
	DFBResult ret = DFB_OK;

	if (!dfb_fbdev->shared->gui_tmp_surface)
		return ret;

	D_ASSERT( layer != NULL );

	A_TRACE("%s: layer_id[%d] clear_rect[%d,%d,%d,%d]\n",
			__FUNCTION__, dfb_layer_id( layer ),
			clear_rect->x, clear_rect->y, clear_rect->w, clear_rect->h);

	state = &layer->state;

	clear_surface( state, dfb_fbdev->shared->gui_tmp_surface, clear_rect );

	return ret;
}

static DFBResult
primarySetColorAdjustment(
			CoreLayer          *layer,
			void               *driver_data,
			void               *layer_data,
			DFBColorAdjustment *adjustment )
{
	struct fb_cmap *cmap = &dfb_fbdev->shared->current_cmap;
	struct fb_cmap *temp = &dfb_fbdev->shared->temp_cmap;
	int    contrast      = adjustment->contrast >> 8;
	int    brightness    = (adjustment->brightness >> 8) - 128;
	int    saturation    = adjustment->saturation >> 8;
	int    r, g, b, i;

	ARIBDEV_PRINT(("%s%s: \n", SRC_FILE, __FUNCTION__));

	if (dfb_fbdev->shared->fix.visual != FB_VISUAL_DIRECTCOLOR)
	return DFB_UNIMPLEMENTED;

	/* Use gamma ramp to set color attributes */
	for (i = 0; i < (int)cmap->len; i++) {
		r = cmap->red[i];
		g = cmap->green[i];
		b = cmap->blue[i];
		r >>= 8;
		g >>= 8;
		b >>= 8;

		/*
		* Brightness Adjustment: Increase/Decrease each color channels
		* by a constant amount as specified by value of brightness.
		*/
		if (adjustment->flags & DCAF_BRIGHTNESS) {
			r += brightness;
			g += brightness;
			b += brightness;

			r = (r < 0) ? 0 : r;
			g = (g < 0) ? 0 : g;
			b = (b < 0) ? 0 : b;

			r = (r > 255) ? 255 : r;
			g = (g > 255) ? 255 : g;
			b = (b > 255) ? 255 : b;
		}

		/*
		* Contrast Adjustment:  We increase/decrease the "separation"
		* between colors in proportion to the value specified by the
		* contrast control. Decreasing the contrast has a side effect
		* of decreasing the brightness.
		*/

		if (adjustment->flags & DCAF_CONTRAST) {
			/* Increase contrast */
			if (contrast > 128) {
				int c = contrast - 128;

				r = ((r + c/2)/c) * c;
				g = ((g + c/2)/c) * c;
				b = ((b + c/2)/c) * c;
			}
			/* Decrease contrast */
			else if (contrast < 127) {
				float c = (float)contrast/128.0;

				r = (int)((float)r * c);
				g = (int)((float)g * c);
				b = (int)((float)b * c);
			}
			r = (r < 0) ? 0 : r;
			g = (g < 0) ? 0 : g;
			b = (b < 0) ? 0 : b;

			r = (r > 255) ? 255 : r;
			g = (g > 255) ? 255 : g;
			b = (b > 255) ? 255 : b;
		}

		/*
		* Saturation Adjustment:  This is is a better implementation.
		* Saturation is implemented by "mixing" a proportion of medium
		* gray to the color value.  On the other side, "removing"
		* a proportion of medium gray oversaturates the color.
		*/
		if (adjustment->flags & DCAF_SATURATION) {
			if (saturation > 128) {
				float gray = ((float)saturation - 128.0)/128.0;
				float color = 1.0 - gray;

				r = (int)(((float)r - 128.0 * gray)/color);
				g = (int)(((float)g - 128.0 * gray)/color);
				b = (int)(((float)b - 128.0 * gray)/color);
			}
			else if (saturation < 128) {
				float color = (float)saturation/128.0;
				float gray = 1.0 - color;

				r = (int)(((float) r * color) + (128.0 * gray));
				g = (int)(((float) g * color) + (128.0 * gray));
				b = (int)(((float) b * color) + (128.0 * gray));
			}

			r = (r < 0) ? 0 : r;
			g = (g < 0) ? 0 : g;
			b = (b < 0) ? 0 : b;

			r = (r > 255) ? 255 : r;
			g = (g > 255) ? 255 : g;
			b = (b > 255) ? 255 : b;
		}
		r |= r << 8;
		g |= g << 8;
		b |= b << 8;

		temp->red[i]    =  (unsigned short)r;
		temp->green[i] =  (unsigned short)g;
		temp->blue[i]  =  (unsigned short)b;
	}

	temp->len = cmap->len;
	temp->start = cmap->start;
	if (FBDEV_IOCTL( FBIOPUTCMAP, temp ) < 0) {
		D_PERROR( "DirectFB/ARIBDev: Could not set the palette!\n" );

		return errno2result(errno);
	}

	return DFB_OK;
}

static DFBResult
primaryTestRegion(
			CoreLayer                  *layer,
			void                       *driver_data,
			void                       *layer_data,
			CoreLayerRegionConfig      *config,
			CoreLayerRegionConfigFlags *failed )
{
	A_TRACE("%s: \n",__FUNCTION__);

	return DFB_OK;
}

static DFBResult
primaryAddRegion(
			CoreLayer             *layer,
			void                  *driver_data,
			void                  *layer_data,
			void                  *region_data,
			CoreLayerRegionConfig *config )
{
	A_TRACE("%s: \n",__FUNCTION__);

	return DFB_OK;

}

static DFBResult
primarySetRegion(
			CoreLayer                  *layer,
			void                       *driver_data,
			void                       *layer_data,
			void                       *region_data,
			CoreLayerRegionConfig      *config,
			CoreLayerRegionConfigFlags updated,
			CoreSurface                *surface,
			CorePalette                *palette )
{
	A_TRACE("%s: \n",__FUNCTION__);

	return DFB_OK;
}

static DFBResult
primaryRemoveRegion(
			CoreLayer *layer,
			void      *driver_data,
			void      *layer_data,
			void      *region_data )
{
	A_TRACE("%s: \n",__FUNCTION__);

	return DFB_OK;
}

static DFBResult
primaryFlipRegion(
			CoreLayer           *layer,
			void                *driver_data,
			void                *layer_data,
			void                *region_data,
			CoreSurface         *surface,
			DFBSurfaceFlipFlags flags )
{
	DFBResult ret;

	A_TRACE("%s: \n",__FUNCTION__);

	if (((flags & DSFLIP_WAITFORSYNC) == DSFLIP_WAITFORSYNC) &&
		!dfb_config->pollvsync_after)
		dfb_screen_wait_vsync( dfb_screens_at(DSCID_PRIMARY) );

	ret = dfb_fbdev_pan( surface->back_buffer->video.offset /
						 surface->back_buffer->video.pitch,
						 (flags & DSFLIP_WAITFORSYNC) == DSFLIP_ONSYNC );
	if (ret)
		return ret;

	if ((flags & DSFLIP_WAIT) &&
		(dfb_config->pollvsync_after || !(flags & DSFLIP_ONSYNC)))
		dfb_screen_wait_vsync( dfb_screens_at(DSCID_PRIMARY) );

	dfb_surface_flip_buffers( surface, false );

	return DFB_OK;
}

static DFBResult
primaryAllocateSurface(
			CoreLayer             *layer,
			void                  *driver_data,
			void                  *layer_data,
			void                  *region_data,
			CoreLayerRegionConfig *config,
			CoreSurface           **ret_surface )
{
	DFBResult              ret;
	DFBSurfaceCapabilities caps = DSCAPS_VIDEOONLY;
	CoreSurface            *surface;
	DFBDisplayLayerDescription desc;
	DFBSurfacePixelFormat      format;

	format = dfb_config->mode.format;

	dfb_layer_get_description( layer, &desc );
	if (D_FLAGS_IS_SET(desc.type, DLTF_ARIB)) {
		format = dfb_config->a_format;
	}
	if (D_FLAGS_IS_SET(desc.type, DLTF_GRAPHICS)) {
		format = dfb_config->g_format;
	}

	caps |= config->surface_caps & DSCAPS_PREMULTIPLIED;

	/* Use the default surface creation. */
	ret = dfb_surface_create( layer->core,
							  dfb_config->mode.width, dfb_config->mode.height,
							  format, CSP_VIDEOONLY,
							  caps, NULL, &surface );
	if (ret) {
		D_ERROR( "DirectFB/core/layers: Surface creation failed!\n" );
		return ret;
	}
	A_TRACE("%s: (%d x %d) %s\n",__FUNCTION__,
			dfb_config->mode.width, dfb_config->mode.height, format_string(format));

	*ret_surface = surface;

     return DFB_OK;
}

static DFBResult
primaryReallocateSurface(
			CoreLayer             *layer,
			void                  *driver_data,
			void                  *layer_data,
			void                  *region_data,
			CoreLayerRegionConfig *config,
			CoreSurface           *surface )
{
	/* reallocation is done during SetConfiguration,
	   because the pitch can only be determined AFTER setting the mode */
	if (DFB_PIXELFORMAT_IS_INDEXED(config->format) && !surface->palette) {
		DFBResult   ret;
		CorePalette *palette;

		A_TRACE("%s: \n",__FUNCTION__);

		ret = dfb_palette_create( dfb_fbdev->core,
								  1 << DFB_COLOR_BITS_PER_PIXEL( config->format ),
								  &palette );
		if (ret)
			return ret;

		if (config->format == DSPF_LUT8)
			dfb_palette_generate_rgb332_map( palette );

		dfb_surface_set_palette( surface, palette );

		dfb_palette_unref( palette );
	}

	return DFB_OK;
}



static DFBResult
primaryBlitLayerSurface(
			CoreLayer    *layer,
			void         *driver_data,
			void         *layer_data,
			DFBRectangle *src_full_rect,
			DFBRectangle *dst_full_rect )
{
	CardState         *state;
	CoreSurface       *src;
	CoreSurface       *dst;
	Layers_Context    gui_layer;
	DFBResult         ret = DFB_OK;
	DFBDisplayLayerID layer_id;


	if (!dfb_fbdev->shared->gui_tmp_surface)
		return ret;

	D_ASSERT( layer != NULL );

	layer_id = dfb_layer_id( layer );
	state = &layer->state;

	D_ASSERT( layer->funcs != NULL );

	gui_layer.layer_type = DLTF_GRAPHICS;
	dfb_layers_enumerate( GetLayer_Callback, &gui_layer );
	if (!gui_layer.surface) {
		A_TRACE("%s: primaryBlitLayerSurface surface:NULL\n", __FUNCTION__);
		return DFB_OK;
	}

	src = dfb_fbdev->shared->gui_tmp_surface;
	dst = gui_layer.surface;

	A_TRACE("%s: src_full_rect[%d,%d,%d,%d]->dst_full_rect[%d,%d,%d,%d]\n",
			__FUNCTION__,
			src_full_rect->x, src_full_rect->y, src_full_rect->w, src_full_rect->h,
			dst_full_rect->x, dst_full_rect->y, dst_full_rect->w, dst_full_rect->h);

	blit_tmp_to_layer( state, src, dst, src_full_rect, dst_full_rect );

	return ret;
}

static int
primaryCheckResolution(
			CoreLayer    *layer,
			void         *driver_data,
			void         *layer_data,
			DFBDimension *seqhead_resolution,
			DFBDimension *seqdisp_resolution,
			DFBBoolean   seqhead_aspect_ratio,
			DFBBoolean   seqhead_progressive )
{
	int change_level = 0;

    return change_level;
}

static DFBBoolean
primarySetMpegResolution(
			CoreLayer    *layer,
			void         *driver_data,
			void         *layer_data,
			DFBDimension *seqhead_resolution,
			DFBDimension *seqdisp_resolution,
			DFBBoolean   seqhead_aspect_ratio,
			DFBBoolean   progressive,
			DFBDimension *ret_resolution )
{
    return DFB_FALSE;
}

static DFBBoolean
primarySetDBcastResolution(
			CoreLayer    *layer,
			void         *driver_data,
			void         *layer_data,
			DFBDimension *resolution,
			DFBBoolean   aspect_ratio,
			DFBDimension *ret_resolution )
{
    return DFB_FALSE;
}

static DFBBoolean
primarySetBmlVisibility(
			CoreLayer      *layer,
			void           *driver_data,
			void           *layer_data,
			DFBAribBmlInfo *bml,
			DFBDimension   *ret_resolution )
{
	return DFB_FALSE;
}

static DFBResult
primaryGetResolution(
			CoreLayer    *layer,
			void         *driver_data,
			void         *layer_data,
			DFBDimension *displayRes,
			DFBBoolean   *displayAspect,
			DFBDimension *canvasRes,
			DFBBoolean   *canvasAspect,
			DFBDimension *aribGfxRes,
			DFBBoolean   *aribGfxAspect )
{
	return DFB_OK;
}

static DFBResult
primaryGetBmlVisibility(
			CoreLayer      *layer,
			void           *driver_data,
			void           *layer_data,
			DFBAribBmlInfo *bml )
{
	return DFB_OK;
}

static DFBResult
primaryGetVideoRectangle(
			CoreLayer    *layer,
			void         *driver_data,
			void         *layer_data,
			DFBRectangle *resolution,
			DFBRectangle *ret_resolution,
			DFBRectangle *ret_mpeg_resolution,
			DFBBoolean   *full_video,
			DFBBoolean   *aspect )
{
	return DFB_OK;
}

static DFBResult
primaryGetDstRefRect(
			CoreLayer    *layer,
			void         *driver_data,
			void         *layer_data,
			DFBRectangle *src_rect,
			DFBRectangle *src_full_rect,
			DFBRectangle *dst_rect,
			DFBRectangle *dst_full_rect,
			DFBRectangle *modify_rect )
{
	DFBRectangle *guiRefRect;
	DFBDimension *guiGfxRes;

	D_ASSERT( layer != NULL );

	guiRefRect = &dfb_fbdev->shared->guiRefRect;
	guiGfxRes  = &dfb_fbdev->shared->guiGfxRes;

	/* Initialize fullscreen rectangle. */
	DFB_RECTANGLE_SET( src_full_rect, 0, 0, guiGfxRes->w, guiGfxRes->h );
	dst_full_rect->x = (float)src_full_rect->x * (float)guiRefRect->w / (float)guiGfxRes->w;
	dst_full_rect->y = (float)src_full_rect->y * (float)guiRefRect->h / (float)guiGfxRes->h;
	dst_full_rect->w = (float)src_full_rect->w * (float)guiRefRect->w / (float)guiGfxRes->w;
	dst_full_rect->h = (float)src_full_rect->h * (float)guiRefRect->h / (float)guiGfxRes->h;

	/* Initialize update rectangle. */
	dfb_rectangle_intersect( src_rect, src_full_rect );
	dst_rect->x = (float)src_rect->x * (float)guiRefRect->w / (float)guiGfxRes->w;
	dst_rect->y = (float)src_rect->y * (float)guiRefRect->h / (float)guiGfxRes->h;
	dst_rect->w = (float)src_rect->w * (float)guiRefRect->w / (float)guiGfxRes->w;
	dst_rect->h = (float)src_rect->h * (float)guiRefRect->h / (float)guiGfxRes->h;

	*modify_rect = *dst_rect;

#ifdef EXPAND_MODIFY_RECT
	if ((src_full_rect->w != dst_full_rect->w)
	||  (src_full_rect->h != dst_full_rect->h)) {
		/* Update rectangle is expanded. */
		expand_rectangle( modify_rect, dst_full_rect, EXPAND_PIXEL );
	}
#endif

	dst_full_rect->x += guiRefRect->x;
	dst_full_rect->y += guiRefRect->y;

	dst_rect->x      += guiRefRect->x;
	dst_rect->y      += guiRefRect->y;

	modify_rect->x   += guiRefRect->x;
	modify_rect->y   += guiRefRect->y;

	A_TRACE("%s: rect[%d,%d,%d,%d]->[%d,%d,%d,%d] modify[%d,%d,%d,%d]\n",
			__FUNCTION__,
			src_rect->x, src_rect->y, src_rect->w, src_rect->h,
			dst_rect->x, dst_rect->y, dst_rect->w, dst_rect->h,
			modify_rect->x, modify_rect->y, modify_rect->w, modify_rect->h);

	A_TRACE("                    : full_rect[%d,%d,%d,%d]->[%d,%d,%d,%d]\n",
			src_full_rect->x, src_full_rect->y, src_full_rect->w, src_full_rect->h,
			dst_full_rect->x, dst_full_rect->y, dst_full_rect->w, dst_full_rect->h);

	return DFB_OK;
}

static DFBResult
primaryGetTmpSurface(
			CoreLayer   *layer,
			void        *driver_data,
			void        *layer_data,
			CoreSurface **ret_surface )
{
	*ret_surface = dfb_fbdev->shared->gui_tmp_surface;

	return DFB_OK;
}

static DFBResult
primarySetCallBackFunction(
			CoreLayer          *layer,
			void               *driver_data,
			void               *layer_data,
			DFBCallBackId      call_id,
			FusionCallHandler  callHandler,
			void               *ctx )
{
    return DFB_OK;
}

static DFBResult
primarySetResolutionTable(
			CoreLayer            *layer,
			void                 *driver_data,
			void                 *layer_data,
			DFB_RESOLUTION_TABLE *resolution_table )
{
    return DFB_UNSUPPORTED;
}

/******************************************************************************/
static DFBResult
aribInitLayer(
			CoreLayer                  *layer,
			void                       *driver_data,
			void                       *layer_data,
			DFBDisplayLayerDescription *description,
			DFBDisplayLayerConfig      *config,
			DFBColorAdjustment         *adjustment )
{
	A_TRACE("%s: layer[%p]\n", __FUNCTION__, layer);

	/* set capabilities and type */
	description->caps = DLCAPS_SURFACE
					  | DLCAPS_CONTRAST
					  | DLCAPS_SATURATION
					  | DLCAPS_BRIGHTNESS;

	description->type = DLTF_ARIB;

	/* set name */
	snprintf( description->name, DFB_DISPLAY_LAYER_DESC_NAME_LENGTH, "ARIBDev Arib Layer" );

	/* fill out default color adjustment */
	adjustment->flags = DCAF_BRIGHTNESS
					  | DCAF_CONTRAST
					  | DCAF_SATURATION;

	adjustment->brightness = 0x8000;
	adjustment->contrast   = 0x8000;
	adjustment->saturation = 0x8000;

	/* fill out the default configuration */
	config->flags = DLCONF_WIDTH
				  | DLCONF_HEIGHT
				  | DLCONF_PIXELFORMAT
				  | DLCONF_BUFFERMODE;

	config->buffermode = DLBM_FRONTONLY;
	config->width  = dfb_config->mode.width;
	config->height = dfb_config->mode.height;

	if (dfb_config->a_format != DSPF_UNKNOWN) {
		config->pixelformat = dfb_config->a_format;
	}
	else {
		config->pixelformat  = dfb_config->mode.format;
		dfb_config->a_format = dfb_config->mode.format;
	}

	return DFB_OK;
}

static DFBResult
aribFlipRegionMulti(
			CoreLayer           *layer,
			void                *driver_data,
			void                *layer_data,
			void                *region_data,
			DFBRegion           *update,
			DFBSurfaceFlipFlags flags )
{
	int              i;
	int              num = 0;
	DFBResult        ret = DFB_OK;
	DFBRectangle     src_rect;
	DFBRectangle     dst_rect;
	Layers_Context   layers[ARIBDEV_LAYER_NUM];
	CardState        *state;
	CoreSurface      *canvas_surface;
	CoreSurface      *src_surface[ARIBDEV_LAYER_NUM];
	DFBRegion        canvas_update;

	D_ASSERT( layer  != NULL );
	D_ASSERT( update != NULL );

	state = &layer->state;
	canvas_surface = dfb_fbdev->shared->canvas_surface;

	layers[0].layer_type = DLTF_GRAPHICS;
	dfb_layers_enumerate( GetLayer_Callback, &layers[0] );

	layers[1].layer_type = DLTF_ARIB;
	dfb_layers_enumerate( GetLayer_Callback, &layers[1] );

	dfb_rectangle_from_region( &src_rect, update );

	for (i = ARIBDEV_LAYER_NUM-1; i >= 0; i--) {
		if (!layers[i].surface)
			continue;

		A_TRACE("%s: layer_id[%d] update[%d,%d,%d,%d] flags[0x%x]\n",
				__FUNCTION__, i,
				update->x1,update->y1,update->x2,update->y2, flags);

		src_surface[num++] = layers[i].surface;
	}

	if (num) {
		get_canvas_dstrect( &src_rect, &dst_rect );
		/* MultiBLIT does not support it in the case that dst_rect differs from src_rect */
		if ((src_rect.w == dst_rect.w)
		&&  (src_rect.h == dst_rect.h)) {
			dfb_gfxcard_multi_blit( num,
									&src_rect, src_surface,
									&dst_rect, canvas_surface,
									state, NULL );
		}
		else {
			blit_layer_to_canvas( num,
								  &src_rect, src_surface,
								  &dst_rect, canvas_surface,
								  state );
		}
		/* Depending on the buffer mode... */
		switch (dfb_fbdev->shared->canvas_buffermode) {
			case DLBM_TRIPLE:
			case DLBM_BACKVIDEO:
			case DLBM_BACKSYSTEM:
				if ((flags & DSFLIP_WAITFORSYNC) == DSFLIP_WAITFORSYNC)
					dfb_layer_wait_vsync( layer );

				dfb_region_from_rectangle( &canvas_update, &dst_rect );

				/* ...or copy updated contents from back to front buffer. */
				dfb_back_to_front_copy( canvas_surface, &canvas_update );

				if ((flags & DSFLIP_WAITFORSYNC) == DSFLIP_WAIT)
					dfb_layer_wait_vsync( layer );
				break;
			case DLBM_FRONTONLY:
				break;

			default:
				D_BUG("unknown buffer mode");
				ret = DFB_BUG;
		}
	}
	A_TRACE("===================================================================================\n");

	return ret;
}

static DFBResult
aribBlitLayerSurface(
			CoreLayer    *layer,
			void         *driver_data,
			void         *layer_data,
			DFBRectangle *src_full_rect,
			DFBRectangle *dst_full_rect )
{
	CardState         *state;
	CoreSurface       *src;
	CoreSurface       *dst;
	Layers_Context    arib_layer;
	DFBResult         ret = DFB_OK;
	DFBDisplayLayerID layer_id;

	if (!dfb_fbdev->shared->arib_tmp_surface)
		return ret;

	D_ASSERT( layer != NULL );

	layer_id = dfb_layer_id( layer );
	state = &layer->state;

	D_ASSERT( layer->funcs != NULL );

	arib_layer.layer_type = DLTF_ARIB;
	dfb_layers_enumerate( GetLayer_Callback, &arib_layer );
	if (!arib_layer.surface) {
		A_TRACE("%s: aribBlitLayerSurface surface:NULL\n", __FUNCTION__);
		return DFB_OK;
	}

	src = dfb_fbdev->shared->arib_tmp_surface;
	dst = arib_layer.surface;

	A_TRACE("%s: full_rect[%d,%d,%d,%d]->[%d,%d,%d,%d]\n", __FUNCTION__,
			src_full_rect->x, src_full_rect->y, src_full_rect->w, src_full_rect->h,
			dst_full_rect->x, dst_full_rect->y, dst_full_rect->w, dst_full_rect->h);

	blit_tmp_to_layer( state, src, dst, src_full_rect, dst_full_rect );

	return ret;
}

static DFBResult
aribClearTmpSurface(
			CoreLayer    *layer,
			void         *driver_data,
			void         *layer_data,
			DFBRectangle *clear_rect )
{
	CardState *state;
	DFBResult ret = DFB_OK;

	if (!dfb_fbdev->shared->arib_tmp_surface)
		return ret;

	D_ASSERT( layer != NULL );

	A_TRACE("%s: layer_id[%d] clear_rect[%d,%d,%d,%d]\n",
			__FUNCTION__, dfb_layer_id( layer ),
			clear_rect->x, clear_rect->y, clear_rect->w, clear_rect->h);

	state = &layer->state;

	clear_surface( state, dfb_fbdev->shared->arib_tmp_surface, clear_rect );

	return ret;
}

static void
*Resolution_Thread( void *arg )
{
	DFBResolutionInfo res_info;
	struct timespec   req;

	req.tv_sec  = 0;
	req.tv_nsec = 50000000;

	while (1) {
		if (dfb_fbdev->shared->resolution_change) {
			res_info.input_res     = dfb_fbdev->shared->aribGfxRes;
			res_info.input_aspect  = dfb_fbdev->shared->aribGfxAspect;
			res_info.output_res    = dfb_fbdev->shared->guiGfxRes;
			res_info.output_aspect = dfb_fbdev->shared->canvasAspect;
			A_TRACE("%s: input[%d,%d][%d] output[%d,%d][%d]\n", __FUNCTION__,
					res_info.input_res.w, res_info.input_res.h, res_info.input_aspect,
					res_info.output_res.w, res_info.output_res.h, res_info.output_aspect);
			callback_execute( 1, &res_info, sizeof(DFBResolutionInfo), CB_RESOLUTION_CHANGE );
			dfb_fbdev->shared->resolution_change = DFB_FALSE;
		}
		nanosleep( &req, NULL );
	}
	return NULL;
}

static void
*Location_Thread( void *arg )
{
	DFBRectangle    videoPosition;
	struct timespec req;

	req.tv_sec  = 0;
	req.tv_nsec = 50000000;

	while (1) {
		if (dfb_fbdev->shared->location_change) {
			videoPosition = dfb_fbdev->shared->videoPosition;
			A_TRACE("%s: videoPosition[%d,%d,%d,%d]\n", __FUNCTION__,
					videoPosition.x, videoPosition.y, videoPosition.w, videoPosition.h);
			callback_execute( 1, &videoPosition, sizeof(DFBRectangle), CB_LOCATION_CHANGE );
			dfb_fbdev->shared->location_change = DFB_FALSE;
		}
		nanosleep( &req, NULL );
	}
	return NULL;
}

static int
aribCheckResolution(
			CoreLayer    *layer,
			void         *driver_data,
			void         *layer_data,
			DFBDimension *seqhead_resolution,
			DFBDimension *seqdisp_resolution,
			DFBBoolean   seqhead_aspect_ratio,
			DFBBoolean   seqhead_progressive )
{
	DFBDimension  *displayRes;
	DFBBoolean    *displayAspect;
	DFBDimension  *aribGfxRes;
	DFBBoolean    *aribGfxAspect;
	DFBDimension  *canvasRes;
	DFBBoolean    *canvasAspect;
	DFBBoolean    *progressive;
	int           i;
	int           change_level = 0;
	DFBBoolean    table_find = DFB_FALSE;
	unsigned long table_num;
	DFB_RESOLUTION_ITEM  *ptr = NULL;
	DFB_RESOLUTION_ITEM  *table_ptr;
	DFB_RESOLUTION_TABLE *resolution_table;

	A_TRACE("%s: seqhead[%d,%d] seqdisp[%d,%d] aspect[%d] proge[%d]\n",
			__FUNCTION__,
			seqhead_resolution->w, seqhead_resolution->h,
			seqdisp_resolution->w, seqdisp_resolution->h,
			seqhead_aspect_ratio, seqhead_progressive);

	displayRes       = &dfb_fbdev->shared->displayRes;
	displayAspect    = &dfb_fbdev->shared->displayAspect;
	canvasRes        = &dfb_fbdev->shared->canvasRes;
	canvasAspect     = &dfb_fbdev->shared->canvasAspect;
	aribGfxRes       = &dfb_fbdev->shared->aribGfxRes;
	aribGfxAspect    = &dfb_fbdev->shared->aribGfxAspect;
	progressive      = &dfb_fbdev->shared->progressive;
	resolution_table = &dfb_fbdev->shared->resolution_table;

	if ((dfb_config->monitor_type   != resolution_table->monitor_type)
	||	(dfb_config->monitor_aspect != resolution_table->monitor_aspect)) {
		return DFB_BUG;
	}

	table_num = resolution_table->num;
	table_ptr = resolution_table->ptr;

    for (i = 0; i < table_num; i++) {
		ptr = (DFB_RESOLUTION_ITEM *)&table_ptr[i];

		A_TRACE("%s: [%d]seqhead[%d,%d] seqdisp[%d,%d] aspect[%d]\n",
				__FUNCTION__, i,
				(int)ptr->seqhead_w, (int)ptr->seqhead_h,
				(int)ptr->seqdisp_w, (int)ptr->seqdisp_h,
				(int)ptr->seqhead_aspect);

		/* Sequence Display Extension */
		if ((seqdisp_resolution->w > 0)
		&&  (seqdisp_resolution->h > 0)) {
			if ((ptr->seqhead_w      == seqhead_resolution->w)
			&&  (ptr->seqhead_h      == seqhead_resolution->h)
			&&  (ptr->seqdisp_w      == seqdisp_resolution->w)
			&&  (ptr->seqdisp_h      == seqdisp_resolution->h)
			&&  (ptr->seqhead_aspect == seqhead_aspect_ratio)) {
				A_TRACE("%s: Sequence Display Extension table[%d]\n",
						__FUNCTION__, i);
				table_find = DFB_TRUE;
				break;

			}
		}
		/* Non Sequence Display Extension */
		else {
			if ((ptr->seqhead_w      == seqhead_resolution->w)
			&&  (ptr->seqhead_h      == seqhead_resolution->h)
			&&  (ptr->seqdisp_w      == seqhead_resolution->w)
			&&  (ptr->seqdisp_h      == seqhead_resolution->h)
			&&  (ptr->seqhead_aspect == seqhead_aspect_ratio)) {
				A_TRACE("%s: Non Sequence Display Extension table[%d]\n",
						__FUNCTION__, i);
				table_find = DFB_TRUE;
				break;
			}
		}
    }

    /* not find resolution table */
	if (!table_find) {
		A_TRACE("%s: table not found\n", __FUNCTION__);
		return change_level;
	}

	/* noting to do */
	if ((displayRes->w  == ptr->display_w)
	&&	(displayRes->h  == ptr->display_h)
	&&	(*displayAspect == ptr->display_aspect)
	&&  (aribGfxRes->w  == ptr->gfx_input_w)
	&&  (aribGfxRes->h  == ptr->gfx_input_h)
	&&  (*aribGfxAspect == ptr->gfx_input_aspect)
	&&  (canvasRes->w   == ptr->gfx_output_w)
	&&  (canvasRes->h   == ptr->gfx_output_h)
	&&  (*canvasAspect  == ptr->gfx_output_aspect)) {
		A_TRACE("%s: noting to do\n", __FUNCTION__);
		return change_level;
	}

	/* change video resolution ? */
	if ((displayRes->w  != ptr->display_w)
	||	(displayRes->h  != ptr->display_h)
	||	(*displayAspect != ptr->display_aspect)) {
		change_level |= DFB_DISPLAY_CHANGE;
	}
	if (*progressive != seqhead_progressive) {
		change_level |= DFB_DISPLAY_CHANGE;
	}
	if ((aribGfxRes->w  != ptr->gfx_input_w)
	||	(aribGfxRes->h  != ptr->gfx_input_h)
	||	(*aribGfxAspect != ptr->gfx_input_aspect)) {
		change_level |= DFB_ARIBGFX_CHANGE;
	}

	/* change output resolution ? */
	if ((canvasRes->w  != ptr->gfx_output_w)
	||  (canvasRes->h  != ptr->gfx_output_h)
	||  (*canvasAspect != ptr->gfx_output_aspect)) {
		change_level |= DFB_CANVAS_CHANGE;
	}

    return change_level;
}

static DFBBoolean
aribSetMpegResolution(
			CoreLayer    *layer,
			void         *driver_data,
			void         *layer_data,
			DFBDimension *seqhead_resolution,
			DFBDimension *seqdisp_resolution,
			DFBBoolean   seqhead_aspect_ratio,
			DFBBoolean   seqhead_progressive,
			DFBDimension *ret_resolution )
{
	DFBAribBmlInfo *aribBmlInfo;
	DFBDimension   *canvasRes;
	DFBDimension   *displayRes;
	DFBDimension   *mpegRes;
	DFBDimension   *dbcastRes;
	DFBDimension   *guiGfxRes;
	DFBDimension   *aribGfxRes;
	DFBBoolean     *canvasAspect;
	DFBBoolean     *displayAspect;
	DFBBoolean     *mpegAspect;
	DFBBoolean     *dbcastAspect;
	DFBBoolean     *guiGfxAspect;
	DFBBoolean     *aribGfxAspect;
	DFBRectangle   *canvasRefRect;
	DFBRectangle   *displayRefRect;
	DFBBoolean     *progressive;
	unsigned long  *x_offset;
	unsigned long  *y_offset;
	int            i;
	int            change_level = 0;
	DFBBoolean     arib_res_change = DFB_FALSE;
	DFBBoolean     table_find = DFB_FALSE;
	unsigned long  table_num;
	DFB_RESOLUTION_ITEM  *ptr = NULL;
	DFB_RESOLUTION_ITEM  *table_ptr;
	DFB_RESOLUTION_TABLE *resolution_table;

	A_TRACE("%s: seqhead[%d,%d] seqdisp[%d,%d] aspect[%d] proge[%d]\n",
			__FUNCTION__,
			seqhead_resolution->w, seqhead_resolution->h,
			seqdisp_resolution->w, seqdisp_resolution->h,
			seqhead_aspect_ratio, seqhead_progressive);

	aribBmlInfo      = &dfb_fbdev->shared->aribBmlInfo;
	displayRes       = &dfb_fbdev->shared->displayRes;
	displayAspect    = &dfb_fbdev->shared->displayAspect;
	displayRefRect   = &dfb_fbdev->shared->displayRefRect;
	canvasRes        = &dfb_fbdev->shared->canvasRes;
	canvasAspect     = &dfb_fbdev->shared->canvasAspect;
	canvasRefRect    = &dfb_fbdev->shared->canvasRefRect;
	guiGfxRes        = &dfb_fbdev->shared->guiGfxRes;
	guiGfxAspect     = &dfb_fbdev->shared->guiGfxAspect;
	aribGfxRes       = &dfb_fbdev->shared->aribGfxRes;
	aribGfxAspect    = &dfb_fbdev->shared->aribGfxAspect;
	mpegRes          = &dfb_fbdev->shared->mpegRes;
	mpegAspect       = &dfb_fbdev->shared->mpegAspect;
	dbcastRes        = &dfb_fbdev->shared->dbcastRes;
	dbcastAspect     = &dfb_fbdev->shared->dbcastAspect;
	x_offset         = &dfb_fbdev->shared->x_offset;
	y_offset         = &dfb_fbdev->shared->y_offset;
	progressive      = &dfb_fbdev->shared->progressive;
	resolution_table = &dfb_fbdev->shared->resolution_table;

	/* initial set. */
	*ret_resolution = *aribGfxRes;
	*progressive    = seqhead_progressive;

	if ((dfb_config->monitor_type   != resolution_table->monitor_type)
	||	(dfb_config->monitor_aspect != resolution_table->monitor_aspect)) {
		return DFB_BUG;
	}

	table_num = resolution_table->num;
	table_ptr = resolution_table->ptr;

    for (i = 0; i < table_num; i++) {
		ptr = (DFB_RESOLUTION_ITEM *)&table_ptr[i];

		/* Sequence Display Extension */
		if ((seqdisp_resolution->w > 0)
		&&  (seqdisp_resolution->h > 0)) {
			if ((ptr->seqhead_w      == seqhead_resolution->w)
			&&  (ptr->seqhead_h      == seqhead_resolution->h)
			&&  (ptr->seqdisp_w      == seqdisp_resolution->w)
			&&  (ptr->seqdisp_h      == seqdisp_resolution->h)
			&&  (ptr->seqhead_aspect == seqhead_aspect_ratio)) {
				A_TRACE(" Sequence Display Extension table[%d]\n", i);
				table_find = DFB_TRUE;
				break;

			}
		}
		/* Non Sequence Display Extension */
		else {
			if ((ptr->seqhead_w      == seqhead_resolution->w)
			&&  (ptr->seqhead_h      == seqhead_resolution->h)
			&&  (ptr->seqdisp_w      == seqhead_resolution->w)
			&&  (ptr->seqdisp_h      == seqhead_resolution->h)
			&&  (ptr->seqhead_aspect == seqhead_aspect_ratio)) {
				A_TRACE(" Non Sequence Display Extension table[%d]\n", i);
				table_find = DFB_TRUE;
				break;
			}
		}
    }

    /* not find resolution table */
	if (!table_find) {
		A_TRACE("%s: table not found\n", __FUNCTION__);
		return DFB_FALSE;
	}

	A_TRACE("*****[ %s ] ***********************************\n", __FUNCTION__);
	A_TRACE("***** ptr->seqhead_w         :[%ld]\n",ptr->seqhead_w);
	A_TRACE("***** ptr->seqhead_h         :[%ld]\n",ptr->seqhead_h);
	A_TRACE("***** ptr->seqhead_aspect    :[%d]\n", ptr->seqhead_aspect);
	A_TRACE("***** ptr->mpeg_w            :[%ld]\n",ptr->mpeg_w);
	A_TRACE("***** ptr->mpeg_h            :[%ld]\n",ptr->mpeg_h);
	A_TRACE("***** ptr->mpeg_aspect       :[%d]\n", ptr->mpeg_aspect);
	A_TRACE("***** ptr->display_w         :[%ld]\n",ptr->display_w);
	A_TRACE("***** ptr->display_h         :[%ld]\n",ptr->display_h);
	A_TRACE("***** ptr->display_aspect    :[%d]\n", ptr->display_aspect);
	A_TRACE("***** ptr->gfx_input_w       :[%ld]\n",ptr->gfx_input_w);
	A_TRACE("***** ptr->gfx_input_h       :[%ld]\n",ptr->gfx_input_h);
	A_TRACE("***** ptr->gfx_input_aspect  :[%d]\n", ptr->gfx_input_aspect);
	A_TRACE("***** ptr->gfx_output_w      :[%ld]\n",ptr->gfx_output_w);
	A_TRACE("***** ptr->gfx_output_h      :[%ld]\n",ptr->gfx_output_h);
	A_TRACE("***** ptr->gfx_output_aspect :[%d]\n", ptr->gfx_output_aspect);
	A_TRACE("***** ptr->x_offset          :[%ld]\n",ptr->x_offset);
	A_TRACE("***** ptr->y_offset          :[%ld]\n",ptr->y_offset);
	A_TRACE("******************************************************************\n\n");

	/* change display resolution ? */
	if ((displayRes->w  != ptr->display_w)
	||	(displayRes->h  != ptr->display_h)
	||  (*displayAspect != ptr->display_aspect)) {
		change_level |= DFB_DISPLAY_CHANGE;
		DFB_RECTANGLE_SET( displayRefRect, 0, 0, ptr->display_w, ptr->display_h );
	}
	/* set display resolution. */
	displayRes->w  = ptr->display_w;
	displayRes->h  = ptr->display_h;
	*displayAspect = ptr->display_aspect;

	/* set mpeg resolution */
	if ((mpegRes->w  != ptr->gfx_input_w)
	||  (mpegRes->h  != ptr->gfx_input_h)
	||  (*mpegAspect != ptr->gfx_input_aspect)) {
		change_level |= DFB_ARIBGFX_CHANGE;
	}
	mpegRes->w  = ptr->gfx_input_w;
	mpegRes->h  = ptr->gfx_input_h;
	*mpegAspect = ptr->gfx_input_aspect;

	/* set arib resolution */
	if (!aribBmlInfo->bml_valid || aribBmlInfo->bml_invisible) {
		if ((aribGfxRes->w != mpegRes->w)
		||  (aribGfxRes->h != mpegRes->h)) {
			arib_res_change = DFB_TRUE;
		}
		aribGfxRes->w  = mpegRes->w;
		aribGfxRes->h  = mpegRes->h;
		*aribGfxAspect = *mpegAspect;
	}

	/* change output resolution ? */
	if ((canvasRes->w  != ptr->gfx_output_w)
	||  (canvasRes->h  != ptr->gfx_output_h)
	||  (*canvasAspect != ptr->gfx_output_aspect)) {
		change_level |= DFB_CANVAS_CHANGE;
	}
	canvasRes->w  = ptr->gfx_output_w;
	canvasRes->h  = ptr->gfx_output_h;
	*canvasAspect = ptr->gfx_output_aspect;

	/* change x,y offset ? */
	*x_offset = ptr->x_offset;
	*y_offset = ptr->y_offset;

	canvasRefRect->x = *x_offset;
	canvasRefRect->y = *y_offset;
	canvasRefRect->w = canvasRes->w - (*x_offset) * 2;
	canvasRefRect->h = canvasRes->h - (*y_offset) * 2;

	if (change_level) {
		if ((change_level & DFB_DISPLAY_CHANGE)
		||  (change_level & DFB_CANVAS_CHANGE)) {
			dfb_fbdev_chg_mode_canvas( displayRes, canvasRes, *canvasAspect, seqhead_progressive );
		}
		dfb_fbdev->shared->resolution_change = DFB_TRUE;
	}
	if (arib_res_change) {
		DFBRectangle clear_rect = {0, 0, DFB_ARIB_HD_DIMENSION_W, DFB_ARIB_HD_DIMENSION_H};
		aribClearTmpSurface( layer, driver_data, layer_data, &clear_rect );
	}
	*ret_resolution = *aribGfxRes;

	A_TRACE("****** [ %s ] *********************************\n", __FUNCTION__);
	A_TRACE("****** displayRes    :[%d,%d][%d]\n",
			displayRes->w, displayRes->h, *displayAspect);
	A_TRACE("****** displayRefRect:[%d,%d,%d,%d]\n",
			displayRefRect->x, displayRefRect->y, displayRefRect->w, displayRefRect->h);
	A_TRACE("****** canvasRes     :[%d,%d][%d]\n",
			canvasRes->w, canvasRes->h, *canvasAspect);
	A_TRACE("****** canvasRefRect :[%d,%d,%d,%d]\n",
			canvasRefRect->x, canvasRefRect->y, canvasRefRect->w, canvasRefRect->h);
	A_TRACE("****** mpegRes       :[%d,%d][%d]\n",
			mpegRes->w, mpegRes->h, *mpegAspect);
	A_TRACE("****** dbcastRes     :[%d,%d][%d] DATA:%s\n",
			dbcastRes->w, dbcastRes->h, *dbcastAspect,
			(!aribBmlInfo->bml_valid || aribBmlInfo->bml_invisible)? "OFF":"ON");
	A_TRACE("****** aribGfxRes    :[%d,%d][%d] CHANGE STATUS[%d]\n",
			aribGfxRes->w, aribGfxRes->h, *aribGfxAspect, arib_res_change);
	A_TRACE("******************************************************************\n\n");

	return arib_res_change;
}

static DFBBoolean
aribSetDBcastResolution(
			CoreLayer    *layer,
			void         *driver_data,
			void         *layer_data,
			DFBDimension *resolution,
			DFBBoolean   aspect_ratio,
			DFBDimension *ret_resolution )
{
	DFBAribBmlInfo *aribBmlInfo;
	DFBDimension   *canvasRes;
	DFBDimension   *displayRes;
	DFBDimension   *mpegRes;
	DFBDimension   *dbcastRes;
	DFBDimension   *guiGfxRes;
	DFBDimension   *aribGfxRes;
	DFBBoolean     *canvasAspect;
	DFBBoolean     *displayAspect;
	DFBBoolean     *mpegAspect;
	DFBBoolean     *dbcastAspect;
	DFBBoolean     *guiGfxAspect;
	DFBBoolean     *aribGfxAspect;
	DFBBoolean     arib_res_change = DFB_FALSE;
	int            change_level = 0;

	aribBmlInfo   = &dfb_fbdev->shared->aribBmlInfo;
	displayRes    = &dfb_fbdev->shared->displayRes;
	displayAspect = &dfb_fbdev->shared->displayAspect;
	canvasRes     = &dfb_fbdev->shared->canvasRes;
	canvasAspect  = &dfb_fbdev->shared->canvasAspect;
	guiGfxRes     = &dfb_fbdev->shared->guiGfxRes;
	guiGfxAspect  = &dfb_fbdev->shared->guiGfxAspect;
	aribGfxRes    = &dfb_fbdev->shared->aribGfxRes;
	aribGfxAspect = &dfb_fbdev->shared->aribGfxAspect;
	mpegRes       = &dfb_fbdev->shared->mpegRes;
	mpegAspect    = &dfb_fbdev->shared->mpegAspect;
	dbcastRes     = &dfb_fbdev->shared->dbcastRes;
	dbcastAspect  = &dfb_fbdev->shared->dbcastAspect;

	*dbcastRes    = *resolution;
	*dbcastAspect = aspect_ratio;

	A_TRACE("%s: dbcastRes[%d,%d][%d] valid[%d] invisible[%d] DATA:%s\n",
			__FUNCTION__,
			dbcastRes->w, dbcastRes->h, *dbcastAspect,
			aribBmlInfo->bml_valid, aribBmlInfo->bml_invisible,
			(!aribBmlInfo->bml_valid || aribBmlInfo->bml_invisible)? "OFF":"ON");


	if (dbcastRes->w > DFB_ARIB_HD_DIMENSION_W)
		dbcastRes->w = DFB_ARIB_HD_DIMENSION_W;
	if (dbcastRes->h > DFB_ARIB_HD_DIMENSION_H)
		dbcastRes->h = DFB_ARIB_HD_DIMENSION_H;

	/* set mpeg resolution */
	if (!aribBmlInfo->bml_valid || aribBmlInfo->bml_invisible) {
		if ((aribGfxRes->w  != mpegRes->w)
		||  (aribGfxRes->h  != mpegRes->h)
		||  (*aribGfxAspect != *mpegAspect)) {
			change_level |= DFB_ARIBGFX_CHANGE;
			if ((aribGfxRes->w != mpegRes->w)
			||  (aribGfxRes->h != mpegRes->h)) {
				arib_res_change = DFB_TRUE;
			}
		}
		*aribGfxRes    = *mpegRes;
		*aribGfxAspect = *mpegAspect;
	}
	/* set dbcast resolution */
	else {
		if ((aribGfxRes->w  != dbcastRes->w)
		||  (aribGfxRes->h  != dbcastRes->h)
		||  (*aribGfxAspect != *dbcastAspect)) {
			change_level |= DFB_ARIBGFX_CHANGE;
			if ((aribGfxRes->w != dbcastRes->w)
			||  (aribGfxRes->h != dbcastRes->h)) {
				arib_res_change = DFB_TRUE;
			}
		}
		*aribGfxRes    = *dbcastRes;
		*aribGfxAspect = *dbcastAspect;
	}
	if (change_level) {
		dfb_fbdev->shared->resolution_change = DFB_TRUE;
	}
	if (arib_res_change) {
		DFBRectangle clear_rect = {0, 0, DFB_ARIB_HD_DIMENSION_W, DFB_ARIB_HD_DIMENSION_H};
		aribClearTmpSurface( layer, driver_data, layer_data, &clear_rect );
	}
	*ret_resolution = *aribGfxRes;

	A_TRACE("****** [ %s ] *******************************\n", __FUNCTION__);
	A_TRACE("****** aribGfxRes    :[%d,%d][%d] CHANGE STATUS[%d]\n",
			aribGfxRes->w, aribGfxRes->h, *aribGfxAspect, arib_res_change);
	A_TRACE("******************************************************************\n\n");

	return arib_res_change;
}

static DFBBoolean
aribSetBmlVisibility(
			CoreLayer      *layer,
			void           *driver_data,
			void           *layer_data,
			DFBAribBmlInfo *bml,
			DFBDimension   *ret_resolution )
{
	DFBAribBmlInfo *aribBmlInfo;
	DFBDimension   *canvasRes;
	DFBDimension   *displayRes;
	DFBDimension   *mpegRes;
	DFBDimension   *dbcastRes;
	DFBDimension   *aribGfxRes;
	DFBBoolean     *canvasAspect;
	DFBBoolean     *displayAspect;
	DFBBoolean     *mpegAspect;
	DFBBoolean     *dbcastAspect;
	DFBBoolean     *aribGfxAspect;
	DFBBoolean     arib_res_change = DFB_FALSE;
	int            change_level = 0;

	A_TRACE("%s: layer_id[%d] valid[%d] invisible[%d] video[%d] DATA:%s\n",
			__FUNCTION__, dfb_layer_id( layer ),
			bml->bml_valid, bml->bml_invisible, bml->bml_has_video,
			(!bml->bml_valid || bml->bml_invisible)? "OFF":"ON");

	aribBmlInfo   = &dfb_fbdev->shared->aribBmlInfo;
	displayRes    = &dfb_fbdev->shared->displayRes;
	displayAspect = &dfb_fbdev->shared->displayAspect;
	canvasRes     = &dfb_fbdev->shared->canvasRes;
	canvasAspect  = &dfb_fbdev->shared->canvasAspect;
	aribGfxRes    = &dfb_fbdev->shared->aribGfxRes;
	aribGfxAspect = &dfb_fbdev->shared->aribGfxAspect;
	mpegRes       = &dfb_fbdev->shared->mpegRes;
	mpegAspect    = &dfb_fbdev->shared->mpegAspect;
	dbcastRes     = &dfb_fbdev->shared->dbcastRes;
	dbcastAspect  = &dfb_fbdev->shared->dbcastAspect;

	*aribBmlInfo = *bml;

	/* set mpeg resolution */
	if (!aribBmlInfo->bml_valid || aribBmlInfo->bml_invisible) {
		if ((aribGfxRes->w  != mpegRes->w)
		||  (aribGfxRes->h  != mpegRes->h)
		||  (*aribGfxAspect != *mpegAspect)) {
			change_level |= DFB_ARIBGFX_CHANGE;
			if ((aribGfxRes->w != mpegRes->w)
			||  (aribGfxRes->h != mpegRes->h)) {
				arib_res_change = DFB_TRUE;
			}
		}
		*aribGfxRes    = *mpegRes;
		*aribGfxAspect = *mpegAspect;
	}
	/* set dbcast resolution */
	else {
		if ((aribGfxRes->w  != dbcastRes->w)
		||  (aribGfxRes->h  != dbcastRes->h)
		||  (*aribGfxAspect != *dbcastAspect)) {
			change_level |= DFB_ARIBGFX_CHANGE;
			if ((aribGfxRes->w != dbcastRes->w)
			||  (aribGfxRes->h != dbcastRes->h)) {
				arib_res_change = DFB_TRUE;
			}
		}
		*aribGfxRes    = *dbcastRes;
		*aribGfxAspect = *dbcastAspect;
	}
	if (change_level) {
		dfb_fbdev->shared->resolution_change = DFB_TRUE;
	}
	if (arib_res_change) {
		DFBRectangle clear_rect = {0, 0, DFB_ARIB_HD_DIMENSION_W, DFB_ARIB_HD_DIMENSION_H};
		aribClearTmpSurface( layer, driver_data, layer_data, &clear_rect );
	}
	*ret_resolution = *aribGfxRes;

	A_TRACE("****** [ %s ] **********************************\n", __FUNCTION__);
	A_TRACE("****** aribGfxRes    :[%d,%d][%d] CHANGE STATUS[%d]\n",
			aribGfxRes->w, aribGfxRes->h, *aribGfxAspect, arib_res_change);
	A_TRACE("******************************************************************\n\n");

	return arib_res_change;
}

static DFBResult
aribGetResolution(
			CoreLayer    *layer,
			void         *driver_data,
			void         *layer_data,
			DFBDimension *displayRes,
			DFBBoolean   *displayAspect,
			DFBDimension *canvasRes,
			DFBBoolean   *canvasAspect,
			DFBDimension *aribGfxRes,
			DFBBoolean   *aribGfxAspect )
{
	A_TRACE("%s: layer_id[%d]\n", __FUNCTION__, dfb_layer_id( layer ));

	*displayRes    = dfb_fbdev->shared->displayRes;
	*displayAspect = dfb_fbdev->shared->displayAspect;
	*canvasRes     = dfb_fbdev->shared->canvasRes;
	*canvasAspect  = dfb_fbdev->shared->canvasAspect;
	*aribGfxRes    = dfb_fbdev->shared->aribGfxRes;
	*aribGfxAspect = dfb_fbdev->shared->aribGfxAspect;

	return DFB_OK;
}

static DFBResult
aribGetBmlVisibility(
			CoreLayer      *layer,
			void           *driver_data,
			void           *layer_data,
			DFBAribBmlInfo *bml)
{
	A_TRACE("%s: layer_id[%d]\n", __FUNCTION__, dfb_layer_id( layer ));

	*bml = dfb_fbdev->shared->aribBmlInfo;

    return DFB_OK;
}

static DFBResult
aribGetVideoRectangle(
			CoreLayer    *layer,
			void         *driver_data,
			void         *layer_data,
			DFBRectangle *resolution,
			DFBRectangle *ret_resolution,
			DFBRectangle *ret_mpeg_resolution,
			DFBBoolean   *full_video,
			DFBBoolean   *aspect )
{
	DFBAribBmlInfo *aribBmlInfo;
	DFBDimension   *displayRes;
	DFBDimension   *aribGfxRes;
	DFBDimension   *mpegRes;
	DFBRectangle   *displayRefRect;
	DFBRectangle   *aribRefRect;
	DFBRectangle   *videoPosRect;
	DFBRectangle   *videoPosition;
	DFBBoolean     *displayAspect;
	DFBBoolean     *aribGfxAspect;
	DFBBoolean     *mpegAspect;
	DFBLocation    *aribLocation;
	DFBRectangle   realPosition;

	aribBmlInfo    = &dfb_fbdev->shared->aribBmlInfo;
	displayRes     = &dfb_fbdev->shared->displayRes;
	displayRefRect = &dfb_fbdev->shared->displayRefRect;
	displayAspect  = &dfb_fbdev->shared->displayAspect;
	aribGfxRes     = &dfb_fbdev->shared->aribGfxRes;
	aribRefRect    = &dfb_fbdev->shared->aribRefRect;
	aribGfxAspect  = &dfb_fbdev->shared->aribGfxAspect;
	mpegRes        = &dfb_fbdev->shared->mpegRes;
	mpegAspect     = &dfb_fbdev->shared->mpegAspect;
	videoPosRect   = &dfb_fbdev->shared->videoPosRect;
	videoPosition  = &dfb_fbdev->shared->videoPosition;
	aribLocation   = &dfb_fbdev->shared->aribLocation;

	if (resolution) {
		if ((resolution->x != 0 || resolution->y != 0)
		||	(resolution->w != 0 || resolution->h != 0)) {
 			*videoPosition = *resolution;
		}
		realPosition = *videoPosition;
		if (*mpegAspect != *aribGfxAspect) {
			if (*aribGfxAspect) {
				realPosition.x += realPosition.w / 8;
				realPosition.w  = realPosition.w * 3 / 4;
			}
			else {
				realPosition.y += realPosition.h / 8;
				realPosition.h  = realPosition.h * 3 / 4;
			}
		}
		*full_video = DFB_FALSE;
	}
	else {
		DFB_RECTANGLE_SET( videoPosition, 0, 0, aribGfxRes->w, aribGfxRes->h );
		realPosition = *videoPosition;
		*full_video = DFB_TRUE;
	}
	realPosition.x &= -2;
	realPosition.y &= -2;
	realPosition.w +=  2;
	realPosition.w &= -4;
	realPosition.h +=  2;
	realPosition.h &= -4;

	videoPosRect->x  = (float)realPosition.x * (float)displayRefRect->w / (float)aribGfxRes->w;
	videoPosRect->y  = (float)realPosition.y * (float)displayRefRect->h / (float)aribGfxRes->h;
	videoPosRect->w  = (float)realPosition.w * (float)displayRefRect->w / (float)aribGfxRes->w;
	videoPosRect->h  = (float)realPosition.h * (float)displayRefRect->h / (float)aribGfxRes->h;
	videoPosRect->x += displayRefRect->x;
	videoPosRect->y += displayRefRect->y;

	*aspect = *displayAspect;

	/* full location */
	if ((aribLocation->w == 1.0) && (aribLocation->h == 1.0)) {
		if (*aribGfxAspect != *displayAspect) {
			/* narrow display &  wide aribGfxAspect */
			if (*aribGfxAspect) {
				videoPosRect->y += videoPosRect->h / 8;
				videoPosRect->h  = videoPosRect->h * 3 / 4;
			}
			/* wide display & narrow aribGfxAspect */
			else {
				videoPosRect->x += videoPosRect->w / 8;
				videoPosRect->w  = videoPosRect->w * 3 / 4;
			}
		}
	}
	*ret_resolution = *videoPosRect;
	DFB_RECTANGLE_SET( ret_mpeg_resolution, 0, 0, displayRes->w, displayRes->h );

	A_TRACE("*****[ %s ]************************\n", __FUNCTION__);
	if (resolution) {
		A_TRACE("***** layer_id[%d] resolution :[%d,%d,%d,%d]\n", dfb_layer_id( layer ),
				resolution->x, resolution->y, resolution->w, resolution->h);
	}
	else {
		A_TRACE("***** layer_id[%d] resolution :[NULL]\n", dfb_layer_id( layer ));
	}
	A_TRACE("***** displayRes             :[%d,%d][%d]\n",
			displayRes->w, displayRes->h, *displayAspect);
	A_TRACE("***** displayRefRect         :[%d,%d,%d,%d]\n",
			displayRefRect->x, displayRefRect->y, displayRefRect->w, displayRefRect->h);
	A_TRACE("***** aribGfxRes             :[%d,%d][%d]\n",
			aribGfxRes->w, aribGfxRes->h, *aribGfxAspect);
	A_TRACE("***** aribRefRect            :[%d,%d,%d,%d]\n",
			aribRefRect->x, aribRefRect->y, aribRefRect->w, aribRefRect->h);
	A_TRACE("***** videoPosition          :[%d,%d,%d,%d]\n",
			videoPosition->x, videoPosition->y, videoPosition->w, videoPosition->h);
	A_TRACE("***** videoPosRect           :[%d,%d,%d,%d]\n",
			videoPosRect->x, videoPosRect->y, videoPosRect->w, videoPosRect->h);
	A_TRACE("***** full_video             :[%d]\n", *full_video);
	A_TRACE("***** aspect                 :[%d]\n", *aspect);
	A_TRACE("******************************************************\n\n");

	return DFB_OK;
}

static DFBResult
aribSetAribLocation(
			CoreLayer         *layer,
			void              *driver_data,
			void              *layer_data,
			const DFBLocation *location )
{
	DFBLocation    *aribLocation;
	DFBDimension   *displayRes;
	DFBDimension   *canvasRes;
	DFBDimension   *guiGfxRes;
	DFBDimension   *aribGfxRes;
	DFBRectangle   *displayRefRect;
	DFBRectangle   *aribRefRect;
	DFBRectangle   *videoPosRect;
	DFBRectangle   *videoPosition;
	DFBBoolean     *aribGfxAspect;
	Layers_Context arib_layer;

	aribLocation   = &dfb_fbdev->shared->aribLocation;
	canvasRes      = &dfb_fbdev->shared->canvasRes;
	displayRes     = &dfb_fbdev->shared->displayRes;
	displayRefRect = &dfb_fbdev->shared->displayRefRect;
	guiGfxRes      = &dfb_fbdev->shared->guiGfxRes;
	aribGfxRes     = &dfb_fbdev->shared->aribGfxRes;
	aribRefRect    = &dfb_fbdev->shared->aribRefRect;
	aribGfxAspect  = &dfb_fbdev->shared->aribGfxAspect;
	videoPosRect   = &dfb_fbdev->shared->videoPosRect;
	videoPosition  = &dfb_fbdev->shared->videoPosition;

	*aribLocation  = *location;
	aribRefRect->x = aribLocation->x * (float)guiGfxRes->w;
	aribRefRect->y = aribLocation->y * (float)guiGfxRes->h;
	aribRefRect->w = aribLocation->w * (float)guiGfxRes->w;
	aribRefRect->h = aribLocation->h * (float)guiGfxRes->h;

	displayRefRect->x = aribLocation->x * (float)displayRes->w;
	displayRefRect->y = aribLocation->y * (float)displayRes->h;
	displayRefRect->w = aribLocation->w * (float)displayRes->w;
	displayRefRect->h = aribLocation->h * (float)displayRes->h;

	videoPosRect->x = (float)videoPosition->x * (float)displayRefRect->w / (float)aribGfxRes->w;
	videoPosRect->y = (float)videoPosition->y * (float)displayRefRect->h / (float)aribGfxRes->h;
	videoPosRect->w = (float)videoPosition->w * (float)displayRefRect->w / (float)aribGfxRes->w;
	videoPosRect->h = (float)videoPosition->h * (float)displayRefRect->h / (float)aribGfxRes->h;
	videoPosRect->x += displayRefRect->x;
	videoPosRect->y += displayRefRect->y;

	A_TRACE("****** [ %s ]************************************\n", __FUNCTION__);
	A_TRACE("****** aribLocation  :[%f,%f,%f,%f]\n",
			dfb_fbdev->shared->aribLocation.x,dfb_fbdev->shared->aribLocation.y,
			dfb_fbdev->shared->aribLocation.w,dfb_fbdev->shared->aribLocation.h );
	A_TRACE("****** displayRes    :[%d,%d][%d]\n",
			dfb_fbdev->shared->displayRes.w,dfb_fbdev->shared->displayRes.h,
			dfb_fbdev->shared->displayAspect);
	A_TRACE("****** displayRefRect:[%d,%d,%d,%d]\n",
			dfb_fbdev->shared->displayRefRect.x,dfb_fbdev->shared->displayRefRect.y,
			dfb_fbdev->shared->displayRefRect.w,dfb_fbdev->shared->displayRefRect.h);
	A_TRACE("****** canvasRes     :[%d,%d][%d]\n",
			dfb_fbdev->shared->canvasRes.w,dfb_fbdev->shared->canvasRes.h,
			dfb_fbdev->shared->canvasAspect);
	A_TRACE("****** canvasRefRect :[%d,%d,%d,%d]\n",
			dfb_fbdev->shared->canvasRefRect.x,dfb_fbdev->shared->canvasRefRect.y,
			dfb_fbdev->shared->canvasRefRect.w,dfb_fbdev->shared->canvasRefRect.h);
	A_TRACE("****** guiGfxRes     :[%d,%d][%d]\n",
			dfb_fbdev->shared->guiGfxRes.w,dfb_fbdev->shared->guiGfxRes.h,
			dfb_fbdev->shared->guiGfxAspect);
	A_TRACE("****** mpegRes       :[%d,%d][%d]\n",
			dfb_fbdev->shared->mpegRes.w,dfb_fbdev->shared->mpegRes.h,
			dfb_fbdev->shared->mpegAspect);
	A_TRACE("****** dbcastRes     :[%d,%d][%d]\n",
			dfb_fbdev->shared->dbcastRes.w,dfb_fbdev->shared->dbcastRes.h,
			dfb_fbdev->shared->dbcastAspect);
	A_TRACE("****** aribGfxRes    :[%d,%d][%d]\n",
			dfb_fbdev->shared->aribGfxRes.w,dfb_fbdev->shared->aribGfxRes.h,
			dfb_fbdev->shared->aribGfxAspect);
	A_TRACE("****** aribRefRect   :[%d,%d,%d,%d]\n",
			dfb_fbdev->shared->aribRefRect.x,dfb_fbdev->shared->aribRefRect.y,
			dfb_fbdev->shared->aribRefRect.w,dfb_fbdev->shared->aribRefRect.h);
	A_TRACE("****** videoPosition :[%d,%d,%d,%d]\n",
			dfb_fbdev->shared->videoPosition.x,dfb_fbdev->shared->videoPosition.y,
			dfb_fbdev->shared->videoPosition.w,dfb_fbdev->shared->videoPosition.h);
	A_TRACE("****** videoPosRect  :[%d,%d,%d,%d]\n",
			dfb_fbdev->shared->videoPosRect.x,dfb_fbdev->shared->videoPosRect.y,
			dfb_fbdev->shared->videoPosRect.w,dfb_fbdev->shared->videoPosRect.h);
	A_TRACE("******************************************************************\n\n");

	arib_layer.layer_type = DLTF_ARIB;
	dfb_layers_enumerate( GetLayer_Callback, &arib_layer );
	if (arib_layer.surface) {
		DFBRectangle aribGfxRect;
		DFBRectangle clearRect;
		CoreSurface  *src;
		CoreSurface  *dst;
		CardState    *state;

		src   = dfb_fbdev->shared->arib_tmp_surface;
		dst   = arib_layer.surface;
		state = &layer->state;

		DFB_RECTANGLE_SET( &clearRect, 0, 0, guiGfxRes->w, guiGfxRes->h );
		clear_surface( state, arib_layer.surface, &clearRect );

		if (dfb_fbdev->shared->arib_tmp_surface) {
			DFB_RECTANGLE_SET( &aribGfxRect, 0, 0, aribGfxRes->w, aribGfxRes->h );
			blit_tmp_to_layer( state, src, dst, &aribGfxRect, aribRefRect );
		}
	}
	dfb_fbdev->shared->location_change = DFB_TRUE;
	return DFB_OK;
}

static DFBResult
aribGetDstRefRect(
			CoreLayer    *layer,
			void         *driver_data,
			void         *layer_data,
			DFBRectangle *src_rect,
			DFBRectangle *src_full_rect,
			DFBRectangle *dst_rect,
			DFBRectangle *dst_full_rect,
			DFBRectangle *modify_rect )
{
	DFBRectangle *aribRefRect;
	DFBDimension *aribGfxRes;
	DFBDimension *guiGfxRes;

	D_ASSERT( layer != NULL );

	aribRefRect = &dfb_fbdev->shared->aribRefRect;
	aribGfxRes  = &dfb_fbdev->shared->aribGfxRes;
	guiGfxRes   = &dfb_fbdev->shared->guiGfxRes;

	/* Initialize fullscreen rectangle. */
	DFB_RECTANGLE_SET( src_full_rect, 0, 0, aribGfxRes->w, aribGfxRes->h );
	dst_full_rect->x = (float)src_full_rect->x * (float)aribRefRect->w / (float)aribGfxRes->w;
	dst_full_rect->y = (float)src_full_rect->y * (float)aribRefRect->h / (float)aribGfxRes->h;
	dst_full_rect->w = (float)src_full_rect->w * (float)aribRefRect->w / (float)aribGfxRes->w;
	dst_full_rect->h = (float)src_full_rect->h * (float)aribRefRect->h / (float)aribGfxRes->h;

	/* Initialize update rectangle. */
	dfb_rectangle_intersect( src_rect, src_full_rect );
	dst_rect->x = (float)src_rect->x * (float)aribRefRect->w / (float)aribGfxRes->w;
	dst_rect->y = (float)src_rect->y * (float)aribRefRect->h / (float)aribGfxRes->h;
	dst_rect->w = (float)src_rect->w * (float)aribRefRect->w / (float)aribGfxRes->w;
	dst_rect->h = (float)src_rect->h * (float)aribRefRect->h / (float)aribGfxRes->h;

	*modify_rect = *dst_rect;

#ifdef EXPAND_MODIFY_RECT
	if ((src_full_rect->w != dst_full_rect->w)
	||  (src_full_rect->h != dst_full_rect->h)) {
		/* Update rectangle is expanded. */
		expand_rectangle( modify_rect, dst_full_rect, EXPAND_PIXEL );
	}
#endif

	dst_full_rect->x += aribRefRect->x;
	dst_full_rect->y += aribRefRect->y;

	dst_rect->x      += aribRefRect->x;
	dst_rect->y      += aribRefRect->y;

	modify_rect->x   += aribRefRect->x;
	modify_rect->y   += aribRefRect->y;

	A_TRACE("%s: rect[%d,%d,%d,%d]->[%d,%d,%d,%d] modify[%d,%d,%d,%d]\n",
			__FUNCTION__,
			src_rect->x, src_rect->y, src_rect->w, src_rect->h,
			dst_rect->x, dst_rect->y, dst_rect->w, dst_rect->h,
			modify_rect->x, modify_rect->y, modify_rect->w, modify_rect->h);

	A_TRACE("                 : full_rect[%d,%d,%d,%d]->[%d,%d,%d,%d]\n",
			src_full_rect->x, src_full_rect->y, src_full_rect->w, src_full_rect->h,
			dst_full_rect->x, dst_full_rect->y, dst_full_rect->w, dst_full_rect->h);

	return DFB_OK;
}

static DFBResult
aribGetTmpSurface(
			CoreLayer   *layer,
			void        *driver_data,
			void        *layer_data,
			CoreSurface **ret_surface )
{
	*ret_surface = dfb_fbdev->shared->arib_tmp_surface;

	return DFB_OK;
}

static DFBResult
aribGetCanvasSurface(
			CoreLayer   *layer,
			void        *driver_data,
			void        *layer_data,
			CoreSurface **ret_surface)
{
	*ret_surface = dfb_fbdev->shared->canvas_surface;

    return DFB_OK;
}

static DFBResult
aribSetCallBackFunction(
			CoreLayer         *layer,
			void              *driver_data,
			void              *layer_data,
			DFBCallBackId     call_id,
			FusionCallHandler callHandler,
			void              *ctx )
{
	A_TRACE("%s: call_id[%d] callHandler[%p]\n", __FUNCTION__, call_id, callHandler);

	if (call_id < CB_REG_MAX) {
		fusion_call_init( &dfb_fbdev->shared->fusion_call[call_id], callHandler, ctx );
		if (call_id == CB_RESOLUTION_CHANGE) {
			pthread_create( &dfb_fbdev->shared->cb_thread[call_id], NULL, Resolution_Thread, NULL );
		}
		else if (call_id == CB_LOCATION_CHANGE) {
			pthread_create( &dfb_fbdev->shared->cb_thread[call_id], NULL, Location_Thread, NULL );
		}
	}
   	return DFB_OK;
}

static DFBResult
aribSetResolutionTable(
			CoreLayer         *layer,
			void              *driver_data,
			void              *layer_data,
			DFB_RESOLUTION_TABLE *resolution_table )
{
	if (resolution_table) {
		A_TRACE("%s: arib_resolution[%p] num[%d] type(%d) aspect(%d)\n",
		__FUNCTION__, resolution_table, (int)resolution_table->num,
		(int)resolution_table->monitor_type, (int)resolution_table->monitor_aspect);
	}

	if (!resolution_table) {
		return DFB_BUG;
	}
	if (!resolution_table->ptr) {
		return DFB_BUG;
	}

	if (dfb_fbdev->shared->resolution_table.ptr) {
		SHFREE( dfb_fbdev->shared->resolution_table.ptr );
	}

	dfb_fbdev->shared->resolution_table.ptr =
				SHMALLOC(sizeof(DFB_RESOLUTION_ITEM) * resolution_table->num);
	if (!dfb_fbdev->shared->resolution_table.ptr) {
		return DFB_BUG;
	}

	dfb_fbdev->shared->resolution_table.monitor_type   = resolution_table->monitor_type;
	dfb_fbdev->shared->resolution_table.monitor_aspect = resolution_table->monitor_aspect;
	dfb_fbdev->shared->resolution_table.num            = resolution_table->num;
	direct_memcpy( (char *)dfb_fbdev->shared->resolution_table.ptr,
				   (char *)resolution_table->ptr,
				   sizeof(DFB_RESOLUTION_ITEM) * resolution_table->num );

   	return DFB_OK;
}

static DFBResult
aribFbDevInformation(
			CoreLayer *layer,
			void      *driver_data,
			void      *layer_data)
{
	struct fb_var_screeninfo var;
	DFBDimension   *canvasRes;
	DFBBoolean     *canvasAspect;
	DFBDimension   *displayRes;
	DFBBoolean     *displayAspect;
	DFBRectangle   *displayRefRect;
	DFBDimension   *mpegRes;
	DFBBoolean     *mpegAspect;
	DFBDimension   *dbcastRes;
	DFBBoolean     *dbcastAspect;
	DFBDimension   *aribGfxRes;
	DFBBoolean     *aribGfxAspect;
	DFBRectangle   *aribRefRect;
	DFBAribBmlInfo *aribBmlInfo;
	DFBRectangle   *videoPosRect;
	DFBRectangle   *videoPosition;
	DFBLocation    *aribLocation;
	CoreSurface    *c_surface;
	CoreSurface    *at_surface;
	CoreSurface    *pt_surface;
	int            vmem;
	int            smem;

	canvasRes      = &dfb_fbdev->shared->canvasRes;
	canvasAspect   = &dfb_fbdev->shared->canvasAspect;
	displayRes     = &dfb_fbdev->shared->displayRes;
	displayAspect  = &dfb_fbdev->shared->displayAspect;
	displayRefRect = &dfb_fbdev->shared->displayRefRect;
	mpegRes        = &dfb_fbdev->shared->mpegRes;
	mpegAspect     = &dfb_fbdev->shared->mpegAspect;
	dbcastRes      = &dfb_fbdev->shared->dbcastRes;
	dbcastAspect   = &dfb_fbdev->shared->dbcastAspect;
	aribGfxRes     = &dfb_fbdev->shared->aribGfxRes;
	aribGfxAspect  = &dfb_fbdev->shared->aribGfxAspect;
	aribRefRect    = &dfb_fbdev->shared->aribRefRect;
	aribBmlInfo    = &dfb_fbdev->shared->aribBmlInfo;
	videoPosRect   = &dfb_fbdev->shared->videoPosRect;
	videoPosition  = &dfb_fbdev->shared->videoPosition;
	aribLocation   = &dfb_fbdev->shared->aribLocation;

	var        = dfb_fbdev->shared->current_var;
	c_surface  = dfb_fbdev->shared->canvas_surface;
	pt_surface = dfb_fbdev->shared->gui_tmp_surface;
	at_surface = dfb_fbdev->shared->arib_tmp_surface;

	vmem = buffer_sizes( c_surface, true );
	smem = buffer_sizes( c_surface, false );

	printf("************************ ARIB DEV INFORMATION ****************************\n");
	vmem = buffer_sizes( c_surface, true );
	smem = buffer_sizes( c_surface, false );
	printf("*-------------------------------------------------------------------------\n");
	printf("* canvas surface    :[%4d x %4d  %dK]\n",
			c_surface->width, c_surface->height, (vmem + smem) >> 10);
	printf("*                    [%6s(0x%08x)  %s(0x%08x)]\n",
			format_string( c_surface->format ), c_surface->format,
			caps_string( c_surface->caps ), c_surface->caps);
	if (pt_surface) {
		vmem = buffer_sizes( pt_surface, true );
		smem = buffer_sizes( pt_surface, false );
		printf("* gui_temp surface  :[%4d x %4d  %dK]\n",
				pt_surface->width, pt_surface->height, (vmem + smem) >> 10);
		printf("*                    [%6s(0x%08x)  %s(0x%08x)]\n",
				format_string( pt_surface->format ), pt_surface->format,
				caps_string( pt_surface->caps ), pt_surface->caps);
	}
	if (at_surface) {
		vmem = buffer_sizes( at_surface, true );
		smem = buffer_sizes( at_surface, false );
		printf("* arib_temp surface :[%4d x %4d  %dK]\n",
				at_surface->width, at_surface->height, (vmem + smem) >> 10);
		printf("*                    [%6s(0x%08x)  %s(0x%08x)]\n",
				format_string( at_surface->format ), at_surface->format,
				caps_string( at_surface->caps ), at_surface->caps);
	}
	printf("*-------------------------------------------------------------------------\n");
	printf("* displayRes        :[%4d x %4d  %s(%d)]\n",
			displayRes->w, displayRes->h, aspect_string(*displayAspect), *displayAspect);
	printf("* canvasRes         :[%4d x %4d  %s(%d)]\n",
			canvasRes->w, canvasRes->h, aspect_string(*canvasAspect), *canvasAspect);
	printf("* mpegRes           :[%4d x %4d  %s(%d)]\n",
			mpegRes->w, mpegRes->h, aspect_string(*mpegAspect), *mpegAspect);
	printf("* dbcastRes         :[%4d x %4d  %s(%d)]\n",
			dbcastRes->w, dbcastRes->h, aspect_string(*dbcastAspect), *dbcastAspect);
	printf("* aribGfxRes        :[%4d x %4d  %s(%d)]\n",
			aribGfxRes->w, aribGfxRes->h, aspect_string(*aribGfxAspect), *aribGfxAspect);
	printf("*-------------------------------------------------------------------------\n");
	printf("* displayRefRect    :[%4d,%4d,%4d,%4d]\n",
			displayRefRect->x, displayRefRect->y, displayRefRect->w, displayRefRect->h);
	printf("* videoPosRect      :[%4d,%4d,%4d,%4d]\n",
			videoPosRect->x, videoPosRect->y, videoPosRect->w, videoPosRect->h);
	printf("* videoPosition     :[%4d,%4d,%4d,%4d]\n",
			videoPosition->x, videoPosition->y, videoPosition->w, videoPosition->h);
	printf("* aribRefRect       :[%4d,%4d,%4d,%4d]\n",
			aribRefRect->x, aribRefRect->y, aribRefRect->w, aribRefRect->h);
	printf("* aribLocation      :[%f,%f,%f,%f]\n",
			aribLocation->x, aribLocation->y, aribLocation->w, aribLocation->h);
	printf("*-------------------------------------------------------------------------\n");
	printf("* aribBmlInfo       :[valid:%d  invisible:%d  has_video:%d]\n",
			aribBmlInfo->bml_valid, aribBmlInfo->bml_invisible,aribBmlInfo->bml_has_video);
	printf("**************************************************************************\n\n");

	return DFB_OK;
}

/**********************/
/*  ARIBDev internal  */
/**********************/
static char *format_string( DFBSurfacePixelFormat fmt )
{
	int i;

	for (i = 0; format_names[i].format; i++) {
		if (fmt == format_names[i].format)
			return (char *)format_names[i].name;
	}
	return (char *)NULL;
}

static char str[128];
static char *caps_string( DFBSurfaceCapabilities caps )
{
	memset( str, 0, 128 );

	strcat( str, "/" );
	if (caps & DSCAPS_SYSTEMONLY)
		strcat( str, "system only/" );

	if (caps & DSCAPS_VIDEOONLY)
		strcat( str, "video only/" );

	if (caps & DSCAPS_DOUBLE)
		strcat( str, "double/" );

	if (caps & DSCAPS_TRIPLE)
		strcat( str, "triple/" );

	if (caps & DSCAPS_INTERLACED)
		strcat( str, "interlaced/" );

	return (char *)str;
}

static char *aspect_string( DFBBoolean aspect )
{
	if (aspect == DFB_TRUE)
		return ("16:9");
	else
		return (" 4:3");

	return (char *)NULL;
}

static inline int
buffer_size( CoreSurface *surface, SurfaceBuffer *buffer, bool video )
{
	return video ?
		(buffer->video.health == CSH_INVALID ?
		0 : buffer->video.pitch * DFB_PLANE_MULTIPLY( surface->format,
		                                             surface->height )) :
		(buffer->system.health == CSH_INVALID ?
		0 : buffer->system.pitch * DFB_PLANE_MULTIPLY( surface->format,
	                                              surface->height ));
}

static int
buffer_sizes( CoreSurface *surface, bool video )
{
	int mem;

	if (!surface) {
		return 0;
	}

	mem = buffer_size( surface, surface->front_buffer, video );
	if (surface->caps & DSCAPS_FLIPPING)
		mem += buffer_size( surface, surface->back_buffer, video );

	if (surface->caps & DSCAPS_TRIPLE)
		mem += buffer_size( surface, surface->idle_buffer, video );

	return (mem + 0x3ff) & ~0x3ff;
}

static DFBResult
initial_arib_construct( void )
{
	/* canvas offset initial set. */
	dfb_fbdev->shared->x_offset = 0;
	dfb_fbdev->shared->y_offset = 0;

	/* canvas resolution initial set. */
	dfb_fbdev->shared->canvasRes.w  = dfb_config->mode.width;
	dfb_fbdev->shared->canvasRes.h  = dfb_config->mode.height;
	dfb_fbdev->shared->canvasAspect = DFB_TRUE;

	/* video resolution initial set. */
	dfb_fbdev->shared->displayRes.w  = DFB_MPEG_1080I_DIMENSION_W;
	dfb_fbdev->shared->displayRes.h  = DFB_MPEG_1080I_DIMENSION_H;
	dfb_fbdev->shared->displayAspect = DFB_TRUE;

	/* mpeg resolution initial set. */
	dfb_fbdev->shared->mpegRes.w  = DFB_ARIB_HD_DIMENSION_W;
	dfb_fbdev->shared->mpegRes.h  = DFB_ARIB_HD_DIMENSION_H;
	dfb_fbdev->shared->mpegAspect = DFB_TRUE;

	/* dbcast resolution initial set. */
	dfb_fbdev->shared->dbcastRes.w  = DFB_ARIB_HD_DIMENSION_W;
	dfb_fbdev->shared->dbcastRes.h  = DFB_ARIB_HD_DIMENSION_H;
	dfb_fbdev->shared->dbcastAspect = DFB_TRUE;

	/* aribgfx resolution initial set. */
	dfb_fbdev->shared->aribGfxRes.w  = DFB_ARIB_HD_DIMENSION_W;
	dfb_fbdev->shared->aribGfxRes.h  = DFB_ARIB_HD_DIMENSION_H;
	dfb_fbdev->shared->aribGfxAspect = DFB_TRUE;

	/* guigfx resolution initial set. */
	dfb_fbdev->shared->guiGfxRes.w  = dfb_fbdev->shared->canvasRes.w;
	dfb_fbdev->shared->guiGfxRes.h  = dfb_fbdev->shared->canvasRes.h;
	dfb_fbdev->shared->guiGfxAspect = dfb_fbdev->shared->canvasAspect;

	/* arib video position intial set. */
	dfb_fbdev->shared->videoPosition.x = 0;
	dfb_fbdev->shared->videoPosition.y = 0;
	dfb_fbdev->shared->videoPosition.w = DFB_ARIB_HD_DIMENSION_W;
	dfb_fbdev->shared->videoPosition.h = DFB_ARIB_HD_DIMENSION_H;

	/* arib location initial set. */
	dfb_fbdev->shared->aribLocation.x = 0.0f;
	dfb_fbdev->shared->aribLocation.y = 0.0f;
	dfb_fbdev->shared->aribLocation.w = 1.0f;
	dfb_fbdev->shared->aribLocation.h = 1.0f;

	/* gui location initial set. */
	dfb_fbdev->shared->guiLocation.x = 0.0f;
	dfb_fbdev->shared->guiLocation.y = 0.0f;
	dfb_fbdev->shared->guiLocation.w = 1.0f;
	dfb_fbdev->shared->guiLocation.h = 1.0f;

	/* canvas rectangle initial set. */
	dfb_fbdev->shared->canvasRefRect.x = dfb_fbdev->shared->x_offset;
	dfb_fbdev->shared->canvasRefRect.y = dfb_fbdev->shared->y_offset;
	dfb_fbdev->shared->canvasRefRect.w = dfb_fbdev->shared->canvasRes.w - dfb_fbdev->shared->x_offset * 2;
	dfb_fbdev->shared->canvasRefRect.h = dfb_fbdev->shared->canvasRes.h - dfb_fbdev->shared->y_offset * 2;

	/* video display rectangle initial set. */
	dfb_fbdev->shared->displayRefRect.x = dfb_fbdev->shared->aribLocation.x * (float)dfb_fbdev->shared->displayRes.w;
	dfb_fbdev->shared->displayRefRect.y = dfb_fbdev->shared->aribLocation.y * (float)dfb_fbdev->shared->displayRes.h;
	dfb_fbdev->shared->displayRefRect.w = dfb_fbdev->shared->aribLocation.w * (float)dfb_fbdev->shared->displayRes.w;
	dfb_fbdev->shared->displayRefRect.h = dfb_fbdev->shared->aribLocation.h * (float)dfb_fbdev->shared->displayRes.h;

	/* arib rectangle initial set. */
	dfb_fbdev->shared->aribRefRect.x = dfb_fbdev->shared->aribLocation.x * (float)dfb_fbdev->shared->guiGfxRes.w;
	dfb_fbdev->shared->aribRefRect.y = dfb_fbdev->shared->aribLocation.y * (float)dfb_fbdev->shared->guiGfxRes.h;
	dfb_fbdev->shared->aribRefRect.w = dfb_fbdev->shared->aribLocation.w * (float)dfb_fbdev->shared->guiGfxRes.w;
	dfb_fbdev->shared->aribRefRect.h = dfb_fbdev->shared->aribLocation.h * (float)dfb_fbdev->shared->guiGfxRes.h;

	/* gui rectangle initial set. */
	dfb_fbdev->shared->guiRefRect.x = dfb_fbdev->shared->guiLocation.x * (float)dfb_fbdev->shared->guiGfxRes.w;
	dfb_fbdev->shared->guiRefRect.y = dfb_fbdev->shared->guiLocation.y * (float)dfb_fbdev->shared->guiGfxRes.h;
	dfb_fbdev->shared->guiRefRect.w = dfb_fbdev->shared->guiLocation.w * (float)dfb_fbdev->shared->guiGfxRes.w;
	dfb_fbdev->shared->guiRefRect.h = dfb_fbdev->shared->guiLocation.h * (float)dfb_fbdev->shared->guiGfxRes.h;

	/* video position rectangle initial set.*/
	dfb_fbdev->shared->videoPosRect = dfb_fbdev->shared->displayRefRect;

	/* bml information initial set.*/
	dfb_fbdev->shared->aribBmlInfo.bml_valid     = DFB_FALSE;
	dfb_fbdev->shared->aribBmlInfo.bml_invisible = DFB_TRUE;
	dfb_fbdev->shared->aribBmlInfo.bml_has_video = DFB_FALSE;

	return DFB_OK;
}

static DFBResult
clear_surface(
			CardState    *state,
			CoreSurface  *surface,
			DFBRectangle *rect )
{
	DFBColor     color;
	DFBRegion    clear_region;
	DFBRectangle clear_rect;

	dfb_get_clear_color( &color, surface->format );

	/* set drawing flags */
	dfb_state_set_drawing_flags( state, DSDRAW_NOFX );

	if (rect) {
		clear_rect = *rect;
	}
	else {
		DFB_RECTANGLE_SET( &clear_rect, 0, 0, surface->width, surface->height );
	}
	A_TRACE("%s: clear_rect[%d,%d,%d,%d] color(%d,%d,%d,%d)\n",
			__FUNCTION__,
			clear_rect.x,clear_rect.y,clear_rect.w,clear_rect.h,
			color.a,color.r,color.g,color.b);

	dfb_region_from_rectangle( &clear_region, &clear_rect );

	dfb_state_set_clip( state, &clear_region );

	/* Set blitting source,destination. */
	state->destination = surface;
	state->modified   |= SMF_DESTINATION;

	dfb_state_set_color( state, &color );
	dfb_gfxcard_fillrectangles( &clear_rect, 1, state );

	/* Reset blitting source,destination. */
	state->destination = NULL;
	state->modified   |= SMF_DESTINATION;

	return DFB_OK;
}

static DFBResult
blit_tmp_to_layer(
			CardState    *state,
			CoreSurface  *source,
			CoreSurface  *destination,
			DFBRectangle *src_rect,
			DFBRectangle *dst_rect )
{
	DFBSurfaceBlittingFlags blittingflags = DSBLIT_NOFX;
	DFBRegion region;

	if (!source || !destination)
		return DFB_OK;

	A_TRACE("%s: rect[%d,%d,%d,%d]->[%d,%d,%d,%d]\n",
			__FUNCTION__,
			src_rect->x,src_rect->y,src_rect->w,src_rect->h,
			dst_rect->x,dst_rect->y,dst_rect->w,dst_rect->h);

	dfb_region_from_rectangle( &region, dst_rect );

    /* Change clipping region. */
    dfb_state_set_clip( state, &region );

	/* Set blitting flags. */
	dfb_state_set_blitting_flags( state, blittingflags );

	/* Set blend function. */
	dfb_state_set_src_blend( state, DSBF_ONE );
	dfb_state_set_dst_blend( state, DSBF_ZERO );

	/* Set blitting source,destination. */
	state->source      = source;
	state->destination = destination;
	state->modified   |= SMF_SOURCE | SMF_DESTINATION;

	if ((src_rect->w == dst_rect->w)
	&&  (src_rect->h == dst_rect->h)) {
		dfb_gfxcard_blit( src_rect, dst_rect->x, dst_rect->y, state );
	}
	else {
		dfb_gfxcard_stretchblit( src_rect, dst_rect, state );
	}
	/* Reset blend function. */
	dfb_state_set_src_blend( state, DSBF_ONE );
	dfb_state_set_dst_blend( state, DSBF_INVSRCALPHA );

	/* Reset blitting source,destination. */
	state->source      = NULL;
	state->destination = NULL;
	state->modified   |= SMF_SOURCE | SMF_DESTINATION;

	return DFB_OK;
}

static DFBResult
blit_layer_to_canvas(
			int          num,
			DFBRectangle *src_rect,
			CoreSurface  **src_surface,
			DFBRectangle *dst_rect,
			CoreSurface  *dst_surface,
			CardState    *state )
{
	DFBSurfaceBlittingFlags blittingflags = DSBLIT_NOFX;
	DFBRegion region;
	int       i;

	A_TRACE("%s: num[%d] rect[%d,%d,%d,%d]->[%d,%d,%d,%d]\n",
			__FUNCTION__, num,
			src_rect->x,src_rect->y,src_rect->w,src_rect->h,
			dst_rect->x,dst_rect->y,dst_rect->w,dst_rect->h);

	dfb_region_from_rectangle( &region, dst_rect );

    /* Change clipping region. */
    dfb_state_set_clip( state, &region );

	/* Set blend function. */
	dfb_state_set_src_blend( state, DSBF_ONE );
	dfb_state_set_dst_blend( state, DSBF_ZERO );

	for (i = 0; i < num; i++) {
		/* Set blitting flags. */
		dfb_state_set_blitting_flags( state, blittingflags );

		/* Set blitting source,destination. */
		state->source      = src_surface[i];
		state->destination = dst_surface;
		state->modified   |= SMF_SOURCE | SMF_DESTINATION;

		if ((src_rect->w == dst_rect->w)
		&&  (src_rect->h == dst_rect->h)) {
			dfb_gfxcard_blit( src_rect, dst_rect->x, dst_rect->y, state );
		}
		else {
			dfb_gfxcard_stretchblit( src_rect, dst_rect, state );
		}
		/* Reset blend function. */
		dfb_state_set_src_blend( state, DSBF_ONE );
		dfb_state_set_dst_blend( state, DSBF_INVSRCALPHA );

		/* Reset blitting source,destination. */
		state->source      = NULL;
		state->destination = NULL;
		state->modified   |= SMF_SOURCE | SMF_DESTINATION;

		/* Set blitting flags. */
		blittingflags  = DSBLIT_NOFX;
		blittingflags |= DSBLIT_BLEND_ALPHACHANNEL;
		if (dfb_config->src_premultiply) {
			blittingflags |= DSBLIT_SRC_PREMULTIPLY;
		}
	}
	return DFB_OK;
}

static DFBResult
allocate_canvas_surface(
			DFBDisplayLayerBufferMode buffermode )
{
	DFBResult              ret;
	DFBSurfaceCapabilities caps = DSCAPS_VIDEOONLY;
	CoreSurface            *surface;
	VideoMode              *videomode = NULL;

	A_TRACE("%s: buffermode[%d]\n", __FUNCTION__, buffermode);

	/* determine further capabilities */
	if (buffermode == DLBM_TRIPLE)
		caps |= DSCAPS_TRIPLE;
	else if (buffermode != DLBM_FRONTONLY)
		caps |= DSCAPS_DOUBLE;

	/* allocate surface object */
	surface = dfb_core_create_surface( dfb_fbdev->core );
	if (!surface)
		return DFB_FAILURE;

	surface->idle_buffer = surface->back_buffer = surface->front_buffer
						 = SHCALLOC( 1, sizeof(SurfaceBuffer) );

	if (!surface->front_buffer) {
		fusion_object_destroy( &surface->object );
		return DFB_NOSYSTEMMEMORY;
	}
	/* initialize surface structure */
	ret = dfb_surface_init( dfb_fbdev->core, surface,
							dfb_config->mode.width, dfb_config->mode.height,
							dfb_config->mode.format, caps, NULL );
	if (ret) {
		SHFREE( surface->front_buffer );
		fusion_object_destroy( &surface->object );
		return ret;
	}
	/* activate object */
	fusion_object_activate( &surface->object );

	/* return surface */
	dfb_fbdev->shared->canvas_surface = surface;
	dfb_fbdev->shared->canvas_buffermode = buffermode;

	videomode = dfb_fbdev->shared->modes;
	while (videomode) {
		if ((videomode->xres == dfb_config->mode.width)
		&&	(videomode->yres == dfb_config->mode.height))
			break;

		videomode = videomode->next;
	}

	/* buffermode = DLBM_BACKVIDEO */
	if (videomode) {
		dfb_fbdev_set_mode_canvas( dfb_fbdev->shared->canvas_surface, videomode );
		return DFB_OK;
	}
	else {
		return DFB_FAILURE;
	}
}

static DFBResult
allocate_tmp_surface(
			CoreSurfacePolicy policy )
{
	DFBResult ret = DFB_OK;

	A_TRACE("%s: policy[%d]\n", __FUNCTION__, policy);

	/* Arib Tmp surface creation. CSP_VIDEOONLY */
	dfb_fbdev->shared->arib_tmp_surface = NULL;
#ifdef USED_ARIB_TMP_SURFACE
	ret = dfb_surface_create( dfb_fbdev->core,
							  DFB_ARIB_HD_DIMENSION_W,
							  DFB_ARIB_HD_DIMENSION_H,
							  dfb_config->a_format, policy,
							  DSCAPS_VIDEOONLY, NULL, &dfb_fbdev->shared->arib_tmp_surface );
	if (ret) {
		D_ERROR( "DirectFB/systems/fbdev: Arib Tmp Surface creation failed!\n" );
		return ret;
	}
#endif

	dfb_fbdev->shared->gui_tmp_surface = NULL;
#ifdef USED_GUI_TMP_SURFACE
	/* Gui Tmp surface creation. CSP_VIDEOONLY */
	ret = dfb_surface_create( dfb_fbdev->core,
							  dfb_config->mode.width,
							  dfb_config->mode.height,
							  dfb_config->g_format, policy,
							  DSCAPS_VIDEOONLY, NULL, &dfb_fbdev->shared->gui_tmp_surface );
	if (ret) {
		D_ERROR( "DirectFB/systems/fbdev: Gui Tmp Surface creation failed!\n" );
		return ret;
	}
#endif

	return ret;
}

static DFBResult
get_canvas_dstrect(
			DFBRectangle *src_rect,
			DFBRectangle *dst_rect )
{
	DFBDimension *canvasRes;
	DFBDimension *guiGfxRes;

	canvasRes = &dfb_fbdev->shared->canvasRes;
	guiGfxRes = &dfb_fbdev->shared->guiGfxRes;

	if ((canvasRes->w != guiGfxRes->w)
	||  (canvasRes->h != guiGfxRes->h)) {
		DFB_RECTANGLE_SET(src_rect, 0, 0, guiGfxRes->w, guiGfxRes->h);
	}
	dst_rect->x = (float)src_rect->x * (float)canvasRes->w / (float)guiGfxRes->w;
	dst_rect->y = (float)src_rect->y * (float)canvasRes->h / (float)guiGfxRes->h;
	dst_rect->w = (float)src_rect->w * (float)canvasRes->w / (float)guiGfxRes->w;
	dst_rect->h = (float)src_rect->h * (float)canvasRes->h / (float)guiGfxRes->h;

	A_TRACE("%s: canvasRes[%d,%d] guiGfxRes[%d,%d][%d,%d,%d,%d]->[%d,%d,%d,%d]\n",
			__FUNCTION__,
			canvasRes->w,canvasRes->h, guiGfxRes->w,guiGfxRes->h,
			src_rect->x, src_rect->y, src_rect->w, src_rect->h,
			dst_rect->x, dst_rect->y, dst_rect->w, dst_rect->h);

	return DFB_OK;
}

static DFBResult
expand_rectangle(
			DFBRectangle *modify_rect,
			DFBRectangle *maximum_rect,
			int          expand_pixel )
{
	/* Update rectangle is expanded. */
	if (modify_rect->x - expand_pixel >= maximum_rect->x)
		modify_rect->x -= expand_pixel;
	else
		modify_rect->x = maximum_rect->x;

	if (modify_rect->y - expand_pixel >= maximum_rect->y)
		modify_rect->y -= expand_pixel;
	else
		modify_rect->y = maximum_rect->y;

	if (modify_rect->y + (modify_rect->h + expand_pixel*2) <= maximum_rect->h)
		modify_rect->h += expand_pixel*2;
	else
		modify_rect->h = maximum_rect->h - modify_rect->y + maximum_rect->y;

	if (modify_rect->x + (modify_rect->w + expand_pixel*2) <= maximum_rect->w)
		modify_rect->w += expand_pixel*2;
	else
		modify_rect->w = maximum_rect->w - modify_rect->x + maximum_rect->x;

	return DFB_OK;
}

static DFBResult
callback_execute(
			int           request,
			void          *arg,
			int           arg_size,
			DFBCallBackId call_id )
{
	DirectResult ret;
	int          erno;
	void         *tmp_shm = NULL;
	FusionCall   *fusion_call;

	D_ASSERT( dfb_fbdev != NULL );
	D_ASSERT( dfb_fbdev->shared != NULL );

	if (call_id >= CB_REG_MAX) {
		return DFB_FAILURE;
	}

	fusion_call = &dfb_fbdev->shared->fusion_call[call_id];
	if (!fusion_call->handler) {
		return DFB_OK;
	}
	A_TRACE("%s: call_id[%d]\n",  __FUNCTION__, call_id);

	if (arg) {
		if (!fusion_is_shared( arg )) {
			tmp_shm = SHMALLOC( arg_size );
			if (!tmp_shm) {
				errno = ENOMEM;
				return DFB_FAILURE;
			}
			direct_memcpy( tmp_shm, arg, arg_size );
		}
	}
	ret = fusion_call_execute( fusion_call, request, tmp_shm ? tmp_shm : arg, &erno );
	if (tmp_shm) {
		direct_memcpy( arg, tmp_shm, arg_size );
		SHFREE( tmp_shm );
	}

	errno = erno;

	return errno ? DFB_FAILURE : DFB_OK;
}

static void dfb_fbdev_var_to_mode( struct fb_var_screeninfo *var,
                                   VideoMode                *mode )
{
     mode->xres          = var->xres;
     mode->yres          = var->yres;
     mode->bpp           = var->bits_per_pixel;
     mode->hsync_len     = var->hsync_len;
     mode->vsync_len     = var->vsync_len;
     mode->left_margin   = var->left_margin;
     mode->right_margin  = var->right_margin;
     mode->upper_margin  = var->upper_margin;
     mode->lower_margin  = var->lower_margin;
     mode->pixclock      = var->pixclock;
     mode->hsync_high    = (var->sync & FB_SYNC_HOR_HIGH_ACT) ? 1 : 0;
     mode->vsync_high    = (var->sync & FB_SYNC_VERT_HIGH_ACT) ? 1 : 0;
     mode->csync_high    = (var->sync & FB_SYNC_COMP_HIGH_ACT) ? 1 : 0;
     mode->sync_on_green = (var->sync & FB_SYNC_ON_GREEN) ? 1 : 0;
     mode->external_sync = (var->sync & FB_SYNC_EXT) ? 1 : 0;
     mode->broadcast     = (var->sync & FB_SYNC_BROADCAST) ? 1 : 0;
     mode->laced         = (var->vmode & FB_VMODE_INTERLACED) ? 1 : 0;
     mode->doubled       = (var->vmode & FB_VMODE_DOUBLE) ? 1 : 0;
}

static int
dfb_fbdev_compatible_format(
			struct fb_var_screeninfo *var,
			int al, int rl, int gl, int bl,
			int ao, int ro, int go, int bo )
{
	int ah, rh, gh, bh;
	int vah, vrh, vgh, vbh;

	ARIBDEV_PRINT(("%s%s: \n", SRC_FILE, __FUNCTION__));

	ah = al + ao - 1;
	rh = rl + ro - 1;
	gh = gl + go - 1;
	bh = bl + bo - 1;

	vah = var->transp.length + var->transp.offset - 1;
	vrh = var->red.length + var->red.offset - 1;
	vgh = var->green.length + var->green.offset - 1;
	vbh = var->blue.length + var->blue.offset - 1;

	if (ah == vah && al >= (int)var->transp.length &&
		rh == vrh && rl >= (int)var->red.length &&
		gh == vgh && gl >= (int)var->green.length &&
		bh == vbh && bl >= (int)var->blue.length)
		return 1;

	return 0;
}

static DFBSurfacePixelFormat
dfb_fbdev_get_pixelformat(
			struct fb_var_screeninfo *var )
{
	ARIBDEV_PRINT(("%s%s: \n", SRC_FILE, __FUNCTION__));

	switch (var->bits_per_pixel) {

	case 8:
		/*
		This check is omitted, since we want to use RGB332 even if the
		hardware uses a palette (in that case we initialize a calculated
		one to have correct colors)

		if (fbdev_compatible_format( var, 0, 3, 3, 2, 0, 5, 2, 0 ))*/

		return DSPF_RGB332;

	case 15:
		if (dfb_fbdev_compatible_format( var, 0, 5, 5, 5, 0, 10, 5, 0 ) |
			dfb_fbdev_compatible_format( var, 1, 5, 5, 5,15, 10, 5, 0 ) )
			return DSPF_ARGB1555;

		break;

	case 16:
		if (dfb_fbdev_compatible_format( var, 0, 5, 5, 5, 0, 10, 5, 0 ) |
			dfb_fbdev_compatible_format( var, 1, 5, 5, 5,15, 10, 5, 0 ) )
			return DSPF_ARGB1555;

		if (dfb_fbdev_compatible_format( var, 0, 4, 4, 4,  0, 8, 4, 0 ) |
			dfb_fbdev_compatible_format( var, 4, 4, 4, 4, 12, 8, 4, 0 ) )
			return DSPF_ARGB4444;


		if (dfb_fbdev_compatible_format( var, 0, 5, 6, 5, 0, 11, 5, 0 ))
			return DSPF_RGB16;

		break;

	case 24:
		if (dfb_fbdev_compatible_format( var, 0, 8, 8, 8, 0, 16, 8, 0 ))
			return DSPF_RGB24;

		break;

	case 32:
        if (dfb_fbdev_compatible_format( var, 0, 8, 8, 8, 0, 16, 8, 0 ))
            return DSPF_RGB32;

        if (dfb_fbdev_compatible_format( var, 8, 8, 8, 8, 24, 16, 8, 0 ))
            return DSPF_ARGB;

		break;
	}

	D_ERROR( "DirectFB/ARIBDev: Unsupported pixelformat: "
			"rgba %d/%d, %d/%d, %d/%d, %d/%d (%dbit)\n",
			var->red.length,    var->red.offset,
			var->green.length,  var->green.offset,
			var->blue.length,    var->blue.offset,
			var->transp.length, var->transp.offset,
			var->bits_per_pixel );

	return DSPF_UNKNOWN;
}

/*
 * pans display (flips buffer) using fbdev ioctl
 */
static DFBResult
dfb_fbdev_pan(
			int  offset,
			bool onsync )
{
	struct fb_var_screeninfo *var;

	var = &dfb_fbdev->shared->current_var;

	ARIBDEV_PRINT(("%s%s: \n", SRC_FILE, __FUNCTION__));

	if (var->yres_virtual < offset + var->yres) {
		D_ERROR( "DirectFB/ARIBDev: yres %d, vyres %d, offset %d\n",
				var->yres, var->yres_virtual, offset );
		D_BUG( "panning buffer out of range" );
		return DFB_BUG;
	}

	var->xoffset  = 0;
	var->yoffset  = offset;
	var->activate = onsync ? FB_ACTIVATE_VBL : FB_ACTIVATE_NOW;

	dfb_gfxcard_sync();

	if (FBDEV_IOCTL( FBIOPAN_DISPLAY, var ) < 0) {
		int erno = errno;

		D_PERROR( "DirectFB/ARIBDev: Panning display failed!\n" );

		return errno2result( erno );
	}

	return DFB_OK;
}

/*
 * blanks display using fbdev ioctl
 */
static DFBResult
dfb_fbdev_blank(
			int level )
{

	ARIBDEV_PRINT(("%s%s: \n", SRC_FILE, __FUNCTION__));

	if (ioctl( dfb_fbdev->fd, FBIOBLANK, level ) < 0) {
		D_PERROR( "DirectFB/ARIBDev: Display blanking failed!\n" );

		return errno2result( errno );
	}

	return DFB_OK;
}

/*
 * sets (if surface != NULL) or tests (if surface == NULL) video mode,
 * sets virtual y-resolution according to buffermode
 */
static DFBResult
dfb_fbdev_set_mode_canvas(
			CoreSurface *surface,
			VideoMode   *mode )
{
	unsigned int vyres;
	struct fb_var_screeninfo var;
	FBDevShared *shared = dfb_fbdev->shared;
	DFBDisplayLayerBufferMode buffermode = shared->canvas_buffermode;


	A_TRACE("%s: surface[%p] videomode[%p]\n", __FUNCTION__,surface, mode);

	if (!mode)
		mode = &shared->current_mode;

	vyres = mode->yres;

	var = shared->current_var;

	var.xoffset = 0;
	var.yoffset = 0;

	switch (buffermode) {
		case DLBM_TRIPLE:
			vyres *= 3;
			break;

		case DLBM_BACKVIDEO:
			vyres *= 2;
			break;

		case DLBM_BACKSYSTEM:
		case DLBM_FRONTONLY:
			break;

		default:
			return DFB_UNSUPPORTED;
	}

	var.bits_per_pixel = DFB_BYTES_PER_PIXEL(dfb_config->mode.format) * 8;
	var.transp.length = var.transp.offset = 0;
	switch (dfb_config->mode.format) {
		case DSPF_ARGB1555:
			var.transp.length = 1;
			var.red.length    = 5;
			var.green.length  = 5;
			var.blue.length   = 5;
			var.transp.offset = 15;
			var.red.offset    = 10;
			var.green.offset  = 5;
			var.blue.offset   = 0;
			break;

		case DSPF_ARGB4444:
			var.transp.length = 4;
			var.red.length    = 4;
			var.green.length  = 4;
			var.blue.length   = 4;
			var.transp.offset = 12;
			var.red.offset    = 8;
			var.green.offset  = 4;
			var.blue.offset   = 0;
			break;

		case DSPF_RGB16:
			var.red.length    = 5;
			var.green.length  = 6;
			var.blue.length   = 5;
			var.red.offset    = 11;
			var.green.offset  = 5;
			var.blue.offset   = 0;
			break;

		case DSPF_ARGB:
		case DSPF_AiRGB:
#ifdef DFB_YCBCR
        case DSPF_AYCbCr:
        case DSPF_AiYCbCr:
#endif // DFB_YCBCR
			var.transp.length = 8;
			var.red.length    = 8;
			var.green.length  = 8;
			var.blue.length   = 8;
			var.transp.offset = 24;
			var.red.offset    = 16;
			var.green.offset  = 8;
			var.blue.offset   = 0;
			break;

		case DSPF_LUT8:
		case DSPF_RGB24:
		case DSPF_RGB32:
		case DSPF_RGB332:
#ifdef DFB_YCBCR
        case DSPF_YCbCr24:
#endif // DFB_YCBCR
			break;

		default:
			return DFB_UNSUPPORTED;
	}
	var.activate = surface ? FB_ACTIVATE_NOW : FB_ACTIVATE_TEST;

	var.xres         = mode->xres;
	var.yres         = mode->yres;
	var.xres_virtual = mode->xres;
	var.yres_virtual = vyres;

	var.pixclock     = mode->pixclock;
	var.left_margin  = mode->left_margin;
	var.right_margin = mode->right_margin;
	var.upper_margin = mode->upper_margin;
	var.lower_margin = mode->lower_margin;
	var.hsync_len    = mode->hsync_len;
	var.vsync_len    = mode->vsync_len;

	var.sync = 0;
	if (mode->hsync_high)
		var.sync |= FB_SYNC_HOR_HIGH_ACT;
	if (mode->vsync_high)
		var.sync |= FB_SYNC_VERT_HIGH_ACT;
	if (mode->csync_high)
		var.sync |= FB_SYNC_COMP_HIGH_ACT;
	if (mode->sync_on_green)
		var.sync |= FB_SYNC_ON_GREEN;
	if (mode->external_sync)
		var.sync |= FB_SYNC_EXT;
	if (mode->broadcast)
		var.sync |= FB_SYNC_BROADCAST;

	var.vmode = 0;
	if (mode->laced)
		var.vmode |= FB_VMODE_INTERLACED;
	if (mode->doubled)
		var.vmode |= FB_VMODE_DOUBLE;

	dfb_gfxcard_lock( GDLF_WAIT | GDLF_SYNC | GDLF_RESET | GDLF_INVALIDATE );

	if (FBDEV_IOCTL( FBIOPUT_VSCREENINFO, &var ) < 0) {
		int erno = errno;

		if (surface)
			D_PERROR( "DirectFB/ARIBDev: "
					"Could not set video mode (FBIOPUT_VSCREENINFO)!\n" );

		dfb_gfxcard_unlock();

		return errno2result( erno );
	}

	/*
	* the video mode was set successfully, check if there is enough
	* video ram (for buggy framebuffer drivers)
	*/

	if (shared->fix.smem_len < (var.yres_virtual *
								var.xres_virtual *
								var.bits_per_pixel >> 3))
	{
		if (surface) {
			D_PERROR( "DirectFB/ARIBDev: "
					"Could not set video mode (not enough video ram)!\n" );

			/* restore mode */
			FBDEV_IOCTL( FBIOPUT_VSCREENINFO, &shared->current_var );
		}

		dfb_gfxcard_unlock();

		return DFB_INVARG;
	}
	/* If surface is NULL the mode was only tested, otherwise apply changes. */
	if (surface) {
		struct fb_fix_screeninfo fix;
		DFBSurfacePixelFormat    format;

		FBDEV_IOCTL( FBIOGET_VSCREENINFO, &var );

		format = dfb_fbdev_get_pixelformat( &var );
		if (format == DSPF_UNKNOWN || var.yres_virtual < vyres) {
			D_WARN( "fbdev driver possibly buggy" );

			/* restore mode */
			FBDEV_IOCTL( FBIOPUT_VSCREENINFO, &shared->current_var );

			dfb_gfxcard_unlock();

			return DFB_UNSUPPORTED;
		}

		if (!dfb_config) {
			dfb_gfxcard_unlock();

			return DFB_OK;
		}

		if (dfb_config->mode.format == DSPF_RGB332)
			dfb_fbdev_set_rgb332_palette();
		else
			dfb_fbdev_set_gamma_ramp( dfb_config->mode.format );

		shared->current_var  = var;
		dfb_fbdev_var_to_mode( &var, &shared->current_mode );

		surface->width  = mode->xres;
		surface->height = mode->yres;
		surface->format = dfb_config->mode.format;

		/* To get the new  */
		FBDEV_IOCTL( FBIOGET_FSCREENINFO, &fix );

		/* ++Tony: Other information (such as visual formats) will also change */
		shared->fix = fix;

		dfb_gfxcard_adjust_heap_offset( var.yres_virtual * fix.line_length );

		surface->front_buffer->surface      = surface;
		surface->front_buffer->policy       = CSP_VIDEOONLY;
		surface->front_buffer->format       = format;
		surface->front_buffer->video.health = CSH_STORED;
		surface->front_buffer->video.pitch  = fix.line_length;
		surface->front_buffer->video.offset = 0;

		switch (buffermode) {
		case DLBM_FRONTONLY:
			surface->caps &= ~DSCAPS_FLIPPING;

			if (surface->back_buffer != surface->front_buffer) {
				if (surface->back_buffer->system.addr)
					SHFREE( surface->back_buffer->system.addr );

				SHFREE( surface->back_buffer );

				surface->back_buffer = surface->front_buffer;
			}

			if (surface->idle_buffer != surface->front_buffer) {
				if (surface->idle_buffer->system.addr)
					SHFREE( surface->idle_buffer->system.addr );

				SHFREE( surface->idle_buffer );

				surface->idle_buffer = surface->front_buffer;
			}
			break;

		case DLBM_BACKVIDEO:
			surface->caps |= DSCAPS_DOUBLE;
			surface->caps &= ~DSCAPS_TRIPLE;

			if (surface->back_buffer == surface->front_buffer) {
				surface->back_buffer = SHCALLOC( 1, sizeof(SurfaceBuffer) );
			}
			else {
				if (surface->back_buffer->system.addr) {
					SHFREE( surface->back_buffer->system.addr );
					surface->back_buffer->system.addr = NULL;
				}

				surface->back_buffer->system.health = CSH_INVALID;
			}
			surface->back_buffer->surface      = surface;
			surface->back_buffer->policy       = CSP_VIDEOONLY;
			surface->back_buffer->format       = format;
			surface->back_buffer->video.health = CSH_STORED;
			surface->back_buffer->video.pitch  = fix.line_length;
			surface->back_buffer->video.offset = surface->back_buffer->video.pitch * var.yres;

			if (surface->idle_buffer != surface->front_buffer) {
				if (surface->idle_buffer->system.addr)
					SHFREE( surface->idle_buffer->system.addr );

				SHFREE( surface->idle_buffer );

				surface->idle_buffer = surface->front_buffer;
			}
			break;

		case DLBM_TRIPLE:
			surface->caps |= DSCAPS_TRIPLE;
			surface->caps &= ~DSCAPS_DOUBLE;

			if (surface->back_buffer == surface->front_buffer) {
				surface->back_buffer = SHCALLOC( 1, sizeof(SurfaceBuffer) );
			}
			else {
				if (surface->back_buffer->system.addr) {
					SHFREE( surface->back_buffer->system.addr );
					surface->back_buffer->system.addr = NULL;
				}

				surface->back_buffer->system.health = CSH_INVALID;
			}
			surface->back_buffer->surface      = surface;
			surface->back_buffer->policy       = CSP_VIDEOONLY;
			surface->back_buffer->format       = format;
			surface->back_buffer->video.health = CSH_STORED;
			surface->back_buffer->video.pitch  = fix.line_length;
			surface->back_buffer->video.offset = surface->back_buffer->video.pitch * var.yres;

			if (surface->idle_buffer == surface->front_buffer) {
				surface->idle_buffer = SHCALLOC( 1, sizeof(SurfaceBuffer) );
			}
			else {
				if (surface->idle_buffer->system.addr) {
					SHFREE( surface->idle_buffer->system.addr );
					surface->idle_buffer->system.addr = NULL;
				}

				surface->idle_buffer->system.health = CSH_INVALID;
			}
			surface->idle_buffer->surface      = surface;
			surface->idle_buffer->policy       = CSP_VIDEOONLY;
			surface->idle_buffer->format       = format;
			surface->idle_buffer->video.health = CSH_STORED;
			surface->idle_buffer->video.pitch  = fix.line_length;
			surface->idle_buffer->video.offset = surface->idle_buffer->video.pitch * var.yres * 2;
			break;

		case DLBM_BACKSYSTEM:
			surface->caps |= DSCAPS_DOUBLE;
			surface->caps &= ~DSCAPS_TRIPLE;

			if (surface->back_buffer == surface->front_buffer) {
				surface->back_buffer = SHCALLOC( 1, sizeof(SurfaceBuffer) );
			}
			surface->back_buffer->surface       = surface;
			surface->back_buffer->policy        = CSP_SYSTEMONLY;
			surface->back_buffer->format        = format;
			surface->back_buffer->video.health  = CSH_INVALID;
			surface->back_buffer->system.health = CSH_STORED;
			surface->back_buffer->system.pitch  = (DFB_BYTES_PER_LINE(format, var.xres) + 3) & ~3;

			if (surface->back_buffer->system.addr)
				SHFREE( surface->back_buffer->system.addr );

			surface->back_buffer->system.addr =
				SHMALLOC( surface->back_buffer->system.pitch * var.yres );

			if (surface->idle_buffer != surface->front_buffer) {
				if (surface->idle_buffer->system.addr)
					SHFREE( surface->idle_buffer->system.addr );

				SHFREE( surface->idle_buffer );

				surface->idle_buffer = surface->front_buffer;
			}
			break;

		default:
			D_BUG( "unexpected buffer mode" );
			break;
		}

		dfb_fbdev_pan( 0, false );

		dfb_gfxcard_after_set_var();

		dfb_surface_notify_listeners( surface, CSNF_SIZEFORMAT | CSNF_FLIP | CSNF_VIDEO | CSNF_SYSTEM );

		A_TRACE("%s ***** [surface=%p]*************\n", __FUNCTION__, surface);
		A_TRACE("***** var.xoffset        :[%d]\n", var.xoffset);
		A_TRACE("***** var.yoffset        :[%d]\n", var.yoffset);
		A_TRACE("***** var.xres           :[%d]\n", var.xres);
		A_TRACE("***** var.yres           :[%d]\n", var.yres);
		A_TRACE("***** var.xres_virtual   :[%d]\n", var.xres_virtual);
		A_TRACE("***** var.yres_virtual   :[%d]\n", var.yres_virtual);
		A_TRACE("***** var.bits_per_pixel :[%d]\n", var.bits_per_pixel);
		A_TRACE("***** var.pixclock       :[%d]\n", var.pixclock);
		A_TRACE("***** var.left_margin    :[%d]\n", var.left_margin);
		A_TRACE("***** var.right_margin   :[%d]\n", var.right_margin);
		A_TRACE("***** var.upper_margin   :[%d]\n", var.upper_margin);
		A_TRACE("***** var.lower_margin   :[%d]\n", var.lower_margin);
		A_TRACE("***** var.hsync_len      :[%d]\n", var.hsync_len);
		A_TRACE("***** var.vsync_len      :[%d]\n", var.vsync_len);
		A_TRACE("***** var.transp.length  :[%d]\n", var.transp.length);
		A_TRACE("***** var.red.length     :[%d]\n", var.red.length);
		A_TRACE("***** var.green.length   :[%d]\n", var.green.length);
		A_TRACE("***** var.blue.length    :[%d]\n", var.blue.length);
		A_TRACE("***** var.transp.offset  :[%d]\n", var.transp.offset);
		A_TRACE("***** var.red.offset     :[%d]\n", var.red.offset);
		A_TRACE("***** var.green.offset   :[%d]\n", var.green.offset);
		A_TRACE("***** var.blue.offset    :[%d]\n", var.blue.offset);
		A_TRACE("***** fix.line_length    :[%d]\n", fix.line_length);
		A_TRACE("***** fix.smem_len       :[%d]\n", fix.smem_len);
		A_TRACE("*******************************************************************\n");

	}

	dfb_gfxcard_unlock();

	return DFB_OK;
}

static DFBResult
dfb_fbdev_chg_mode_canvas(
			DFBDimension *video_resolution,
			DFBDimension *canvas_resolution,
			unsigned int aspect_ratio,
			DFBBoolean   progressive )
{
	VideoMode *videomode = NULL;

	A_TRACE("%s: resolution[%d,%d]\n", __FUNCTION__, canvas_resolution->w, canvas_resolution->h);

	videomode = dfb_fbdev->shared->modes;
	while (videomode) {
		if ((videomode->xres == canvas_resolution->w)
		&&	(videomode->yres == canvas_resolution->h))
			break;

		videomode = videomode->next;
	}

	/* buffermode = DLBM_BACKVIDEO */
	if (videomode) {
		dfb_fbdev_set_mode_canvas( dfb_fbdev->shared->canvas_surface, videomode );
	}
	return DFB_OK;
}

/*
 * parses video modes in /etc/fb.modes and stores them in dfb_fbdev->shared->modes
 * (to be replaced by DirectFB's own config system
 */
static DFBResult
dfb_fbdev_read_modes()
{
	FILE *fp;
	char line[80],label[32],value[16];
	int geometry=0, timings=0;
	int dummy;
	VideoMode temp_mode;
	VideoMode *m = dfb_fbdev->shared->modes;

	ARIBDEV_PRINT(("%s%s: \n", SRC_FILE, __FUNCTION__));

	if (!(fp = fopen("/etc/fb.modes","r")))
		return errno2result( errno );

	while (fgets(line,79,fp)) {
		if (sscanf(line, "mode \"%31[^\"]\"",label) == 1) {
			memset( &temp_mode, 0, sizeof(VideoMode) );
			geometry = 0;
			timings = 0;
			while (fgets(line,79,fp) && !(strstr(line,"endmode"))) {
				if (5 == sscanf(line," geometry %d %d %d %d %d", &temp_mode.xres, &temp_mode.yres, &dummy, &dummy, &temp_mode.bpp)) {
					geometry = 1;
				}
				else if (7 == sscanf(line," timings %d %d %d %d %d %d %d", &temp_mode.pixclock, &temp_mode.left_margin,  &temp_mode.right_margin,
									&temp_mode.upper_margin, &temp_mode.lower_margin, &temp_mode.hsync_len,	&temp_mode.vsync_len)) {
					timings = 1;
				}
				else if (1 == sscanf(line, " hsync %15s",value) && 0 == strcasecmp(value,"high")) {
					temp_mode.hsync_high = 1;
				}
				else if (1 == sscanf(line, " vsync %15s",value) && 0 == strcasecmp(value,"high")) {
					temp_mode.vsync_high = 1;
				}
				else if (1 == sscanf(line, " csync %15s",value) && 0 == strcasecmp(value,"high")) {
					temp_mode.csync_high = 1;
				}
				else if (1 == sscanf(line, " laced %15s",value) && 0 == strcasecmp(value,"true")) {
					temp_mode.laced = 1;
				}
				else if (1 == sscanf(line, " double %15s",value) && 0 == strcasecmp(value,"true")) {
					temp_mode.doubled = 1;
				}
				else if (1 == sscanf(line, " gsync %15s",value) && 0 == strcasecmp(value,"true")) {
					temp_mode.sync_on_green = 1;
				}
				else if (1 == sscanf(line, " extsync %15s",value) && 0 == strcasecmp(value,"true")) {
					temp_mode.external_sync = 1;
				}
				else if (1 == sscanf(line, " bcast %15s",value) && 0 == strcasecmp(value,"true")) {
					temp_mode.broadcast = 1;
				}
			}
			if (geometry && timings) {
				if (!m) {
					dfb_fbdev->shared->modes = SHCALLOC(1, sizeof(VideoMode));
					m = dfb_fbdev->shared->modes;
				}
				else {
					m->next = SHCALLOC(1, sizeof(VideoMode));
					m = m->next;
				}
				direct_memcpy (m, &temp_mode, sizeof(VideoMode));
				A_TRACE("%s: %20s %4dx%4d  %s%s\n", __FUNCTION__, label, temp_mode.xres, temp_mode.yres,
						temp_mode.laced ? "interlaced " : "", temp_mode.doubled ? "doublescan" : "");
			}
		}
	}

	fclose (fp);

	return DFB_OK;
}

/*
 * some fbdev drivers use the palette as gamma ramp in >8bpp modes, to have
 * correct colors, the gamme ramp has to be initialized.
 */

static __u16 dfb_fbdev_calc_gamma(
			int n,
			int max)
{
	int ret = 65535.0 * ((float)((float)n/(max)));
	if (ret > 65535) ret = 65535;
	if (ret <     0) ret =    0;

	return ret;
}

static DFBResult
dfb_fbdev_set_gamma_ramp(
			DFBSurfacePixelFormat format )
{
	int i;

	int red_size   = 0;
	int green_size = 0;
	int blue_size  = 0;
	int red_max	   = 0;
	int green_max  = 0;
	int blue_max   = 0;

	struct fb_cmap *cmap;

	ARIBDEV_PRINT(("%s%s: \n", SRC_FILE, __FUNCTION__));

	if (!dfb_fbdev) {
		D_BUG( "dfb_fbdev_set_gamma_ramp() called while dfb_fbdev == NULL!" );

		return DFB_BUG;
	}

	switch (format) {
	case DSPF_ARGB1555:
		red_size   = 32;
		green_size = 32;
		blue_size  = 32;
		break;
	case DSPF_RGB16:
		red_size   = 32;
		green_size = 64;
		blue_size  = 32;
		break;
	case DSPF_RGB24:
	case DSPF_RGB32:
	case DSPF_ARGB:
#ifdef  DFB_YCBCR
    case DSPF_AYCbCr:
    case DSPF_AiYCbCr:
    case DSPF_YCbCr24:
#endif // DFB_YCBCR
		red_size   = 256;
		green_size = 256;
		blue_size  = 256;
		break;
	default:
		return DFB_OK;
	}

	/*
	* ++Tony: The gamma ramp must be set differently if in DirectColor,
	*         ie, to mimic TrueColor, index == color[index].
	*/
	if (dfb_fbdev->shared->fix.visual == FB_VISUAL_DIRECTCOLOR) {
		red_max   = 65536 / (256/red_size);
		green_max = 65536 / (256/green_size);
		blue_max  = 65536 / (256/blue_size);
	}
	else {
		red_max   = red_size;
		green_max = green_size;
		blue_max  = blue_size;
	}

	cmap = &dfb_fbdev->shared->current_cmap;

	/* assume green to have most weight */
	cmap->len = green_size;

	for (i = 0; i < red_size; i++)
		cmap->red[i] = dfb_fbdev_calc_gamma( i, red_max );

	for (i = 0; i < green_size; i++)
		cmap->green[i] = dfb_fbdev_calc_gamma( i, green_max );

	for (i = 0; i < blue_size; i++)
		cmap->blue[i] = dfb_fbdev_calc_gamma( i, blue_max );

	/* ++Tony: Some drivers use the upper byte, some use the lower */
	if (dfb_fbdev->shared->fix.visual == FB_VISUAL_DIRECTCOLOR) {
		for (i = 0; i < red_size; i++)
			cmap->red[i] |= cmap->red[i] << 8;

		for (i = 0; i < green_size; i++)
			cmap->green[i] |= cmap->green[i] << 8;

		for (i = 0; i < blue_size; i++)
			cmap->blue[i] |= cmap->blue[i] << 8;
	}

	if (FBDEV_IOCTL( FBIOPUTCMAP, cmap ) < 0) {
		D_PERROR( "DirectFB/ARIBDev: "
				"Could not set gamma ramp" );

		return errno2result(errno);
	}

	return DFB_OK;
}

static DFBResult
dfb_fbdev_set_rgb332_palette()
{
	int red_val;
	int green_val;
	int blue_val;
	int i = 0;

	struct fb_cmap cmap;

	ARIBDEV_PRINT(("%s%s: \n", SRC_FILE, __FUNCTION__));

	if (!dfb_fbdev) {
		D_BUG( "dfb_fbdev_set_rgb332_palette() called while dfb_fbdev == NULL!" );

		return DFB_BUG;
	}

	cmap.start  = 0;
	cmap.len    = 256;
	cmap.red    = (__u16*)SHMALLOC( 2 * 256 );
	cmap.green  = (__u16*)SHMALLOC( 2 * 256 );
	cmap.blue   = (__u16*)SHMALLOC( 2 * 256 );
	cmap.transp = (__u16*)SHMALLOC( 2 * 256 );


	for (red_val = 0; red_val  < 8 ; red_val++) {
		for (green_val = 0; green_val  < 8 ; green_val++) {
			for (blue_val = 0; blue_val  < 4 ; blue_val++) {
				cmap.red[i]    = dfb_fbdev_calc_gamma( red_val, 7 );
				cmap.green[i]  = dfb_fbdev_calc_gamma( green_val, 7 );
				cmap.blue[i]   = dfb_fbdev_calc_gamma( blue_val, 3 );
				cmap.transp[i] = (i ? 0x2000 : 0xffff);
				i++;
			}
		}
	}

	if (FBDEV_IOCTL( FBIOPUTCMAP, &cmap ) < 0) {
		D_PERROR( "DirectFB/ARIBDev: "
				"Could not set rgb332 palette" );

		SHFREE( cmap.red );
		SHFREE( cmap.green );
		SHFREE( cmap.blue );
		SHFREE( cmap.transp );

		return errno2result(errno);
	}

	SHFREE( cmap.red );
	SHFREE( cmap.green );
	SHFREE( cmap.blue );
	SHFREE( cmap.transp );

	return DFB_OK;
}

static int
fbdev_ioctl_call_handler(
			int  caller,
			int  call_arg,
			void *call_ptr,
			void *ctx )
{
	int        ret;
	const char cursoroff_str[] = "\033[?1;0;0c";
	const char blankoff_str[]  = "\033[9;0]";

	ARIBDEV_PRINT(("%s%s: \n", SRC_FILE, __FUNCTION__));

	if (dfb_config->vt) {
		if (!dfb_config->kd_graphics && call_arg == FBIOPUT_VSCREENINFO)
			ioctl( dfb_fbdev->vt->fd, KDSETMODE, KD_GRAPHICS );
	}

	ret = ioctl( dfb_fbdev->fd, call_arg, call_ptr );

	if (dfb_config->vt) {
		if (call_arg == FBIOPUT_VSCREENINFO) {
			if (!dfb_config->kd_graphics)
				ioctl( dfb_fbdev->vt->fd, KDSETMODE, KD_TEXT );

			write( dfb_fbdev->vt->fd, cursoroff_str, strlen(cursoroff_str) );
			write( dfb_fbdev->vt->fd, blankoff_str, strlen(blankoff_str) );
		}
	}

	return ret;
}

static int
fbdev_ioctl(
			int  request,
			void *arg,
			int  arg_size )
{
	DirectResult ret;
	int          erno;
	void         *tmp_shm = NULL;

	D_ASSERT( dfb_fbdev != NULL );
	D_ASSERT( dfb_fbdev->shared != NULL );

	if (dfb_core_is_master( dfb_fbdev->core ))
		return fbdev_ioctl_call_handler( 1, request, arg, NULL );

	if (arg) {
		if (!fusion_is_shared( arg )) {
			tmp_shm = SHMALLOC( arg_size );
			if (!tmp_shm) {
				errno = ENOMEM;
				return -1;
			}

			direct_memcpy( tmp_shm, arg, arg_size );
		}
	}

	ret = fusion_call_execute( &dfb_fbdev->shared->fbdev_ioctl,
								request, tmp_shm ? tmp_shm : arg, &erno );

	if (tmp_shm) {
		direct_memcpy( arg, tmp_shm, arg_size );
		SHFREE( tmp_shm );
	}

	errno = erno;

	return errno ? -1 : 0;
}

