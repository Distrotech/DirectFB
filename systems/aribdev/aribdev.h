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

#ifndef __CORE__ARIBDEV_H__
#define __CORE__ARIBDEV_H__
#include <linux/fb.h>
#include <core/coretypes.h>
#include <core/coredefs.h>

#include <core/system.h>
#include <directfb_arib.h>

#include <fusion/call.h>
#include <fusion/reactor.h>

#include <pthread.h>

#include "vt.h"

#ifndef FBIO_WAITFORVSYNC
#define FBIO_WAITFORVSYNC	_IOW('F', 0x20, u_int32_t)
#endif


typedef struct {
    int x;
    int y;
} POINT;

typedef struct {
	DFBDisplayLayerTypeFlags layer_type;
 	DFBDisplayLayerID id;
	CoreLayer         *layer;
	CoreSurface       *surface;
} Layers_Context;

typedef struct {
	/* fbdev fixed screeninfo, contains infos about memory and type of card */
	struct fb_fix_screeninfo  fix;
	VideoMode                 *modes;					/* linked list of valid video modes */
	VideoMode                 current_mode;				/* current video mode */
	struct fb_var_screeninfo  current_var;				/* fbdev variable screeninfo set by DirectFB */
	struct fb_var_screeninfo  orig_var;					/* fbdev variable screeninfo before DirectFB was started */
	struct fb_cmap            orig_cmap;				/* original palette */
	struct fb_cmap            current_cmap;				/* our copy of the cmap */
	struct fb_cmap            temp_cmap;				/* scratch */
	FusionCall                fbdev_ioctl;				/* ioctl rpc */
	unsigned long             page_mask;				/* PAGE_SIZE - 1 */
	struct {
		int                 bus;
		int                 dev;
		int                 func;
	} pci;                                  /* PCI Bus ID of graphics device */

	struct {
		unsigned short      vendor;        /* Graphics device vendor id */
		unsigned short      model;         /* Graphics device model id */
	} device;

	/* DFB_ARIB Add Start ---------------------------> */
	DFB_RESOLUTION_TABLE      resolution_table;			/* DFB_ARIB: ARIB Resolution Table */
	FusionCall                fusion_call[CB_REG_MAX];	/* DFB_ARIB: call back function */
	pthread_t                 cb_thread[CB_REG_MAX];	/* DFB_ARIB: call back thread */
	DFBBoolean                resolution_change;		/* DFB_ARIB: resolution change flag */
	DFBBoolean                location_change;			/* DFB_ARIB: location change flag */
	DFBLocation               aribLocation;				/* DFB_ARIB: arib position to the screen */
	DFBLocation               guiLocation;				/* DFB_ARIB: gui position to the screen */
	unsigned long             x_offset;					/* DFB_ARIB: canvas X offset */
	unsigned long             y_offset;					/* DFB_ARIB: canvas Y offset */
	DFBBoolean                progressive;				/* DFB_ARIB: progressive */
	DFBDimension              canvasRes;				/* DFB_ARIB: canvas resolution */
	DFBBoolean                canvasAspect;				/* DFB_ARIB: canvas aspect Ratio */
	DFBRectangle              canvasRefRect;			/* DFB_ARIB: canvas rectangle */
	DFBDimension              displayRes;				/* DFB_ARIB: display resolution */
	DFBBoolean                displayAspect;			/* DFB_ARIB: display aspect Ratio */
	DFBRectangle              displayRefRect;			/* DFB_ARIB: display rectangle */
	DFBDimension              guiGfxRes;				/* DFB_ARIB: gui lesolution */
	DFBBoolean                guiGfxAspect;				/* DFB_ARIB: gui aspect Ratio */
	DFBRectangle              guiRefRect;				/* DFB_ARIB: gui rectangle */
	DFBDimension              aribGfxRes;				/* DFB_ARIB: arib lesolution */
	DFBBoolean                aribGfxAspect;			/* DFB_ARIB: arib aspect Ratio */
	DFBRectangle              aribRefRect;				/* DFB_ARIB: arib rectangle */
	DFBDimension              mpegRes;					/* DFB_ARIB: mpeg resolution */
	DFBBoolean                mpegAspect;				/* DFB_ARIB: mpeg aspect Ratio */
	DFBDimension              dbcastRes;				/* DFB_ARIB: dbacst resolution */
	DFBBoolean                dbcastAspect;				/* DFB_ARIB: dbcast aspect Ratio */
	DFBRectangle              videoPosRect;				/* DFB_ARIB: video rectangle */
	DFBRectangle              videoPosition;			/* DFB_ARIB: dbcast video position */
	DFBAribBmlInfo            aribBmlInfo;				/* DFB_ARIB: bml information */
	CoreSurface               *canvas_surface;			/* DFB_ARIB: canvas surface */
	CoreSurface               *arib_tmp_surface;		/* DFB_ARIB: arib window tmp surface */
	CoreSurface               *gui_tmp_surface;			/* DFB_ARIB: gui window tmp surface */
	DFBDisplayLayerBufferMode canvas_buffermode;		/* DFB_ARIB: canvas surface buffermode */
	/* DFB_ARIB Add End   <--------------------------- */
} FBDevShared;

typedef struct {
     FBDevShared     *shared;
     CoreDFB         *core;
     void            *framebuffer_base;	/* virtual framebuffer address */
     int             fd;				/* file descriptor for /dev/fb */
     VirtualTerminal *vt;
} FBDev;

/*
 * core init function, opens /dev/fb, get fbdev screeninfo
 * disables font acceleration, reads mode list
 */
DFBResult dfb_fbdev_initialize();
DFBResult dfb_fbdev_join();

/*
 * deinitializes DirectFB fbdev stuff and restores fbdev settings
 */
DFBResult dfb_fbdev_shutdown( bool emergency );
DFBResult dfb_fbdev_leave( bool emergency );


#endif
