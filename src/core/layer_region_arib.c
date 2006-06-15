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

#include <directfb_arib.h>

#include <core/coredefs.h>
#include <core/coretypes.h>

#include <direct/messages.h>

#include <fusion/shmalloc.h>

#include <core/core.h>
#include <core/gfxcard.h>
#include <core/layer_context.h>
#include <core/layer_control.h>
#include <core/layer_region_arib.h>
#include <core/layers_internal.h>
#include <core/surfaces.h>

#include <gfx/util.h>
#include <misc/util.h>
#include <direct/util.h>


DFBResult
dfb_arib_layer_region_flip_update( CoreLayerRegion     *region,
                                   const DFBRegion     *update,
                                   DFBSurfaceFlipFlags  flags )
{
	DFBResult         ret = DFB_OK;
	CoreLayer         *layer;
	CoreLayerContext  *context;
	CoreSurface       *surface;
	DisplayLayerFuncs *funcs;
	DFBRegion         updateRegion;

	D_ASSERT( region != NULL );

	context = region->context;
	D_ASSERT( context != NULL );

	layer = dfb_layer_at( context->layer_id );
	D_ASSERT( layer != NULL );

	funcs = layer->funcs;
	D_ASSERT( funcs != NULL );

	/* Lock the region. */
	if (dfb_layer_region_lock( region ))
		return DFB_FUSION;

	D_ASSUME( region->surface != NULL );

	/* Check for NULL surface. */
	if (!region->surface) {
		dfb_layer_region_unlock( region );
		return DFB_UNSUPPORTED;
	}

	surface = region->surface;

	DFB_REGION_SET( &updateRegion, 0, 0, surface->width-1, surface->height-1 );
	if (update)
		updateRegion = *update;

	A_TRACE("%s: updateRegion(%d,%d,%d,%d) flags(0x%x) \n", __FUNCTION__,
			updateRegion.x1, updateRegion.y1, updateRegion.x2, updateRegion.y2, flags );

	/* primaryFlipRegionMulti */
	if (funcs->FlipRegionMulti) {
		ret = funcs->FlipRegionMulti( layer,
									  layer->driver_data,
									  layer->layer_data,
									  region->region_data,
									  &updateRegion, flags );
	}
	else {
		D_BUG("unknown funcs FlipRegionMulti ");
		ret = DFB_BUG;
	}
	/* Unlock the region. */
	dfb_layer_region_unlock( region );

	return ret;
}

DFBResult
dfb_arib_layer_region_temporary_to_layer_surface( CoreLayerRegion *region,
                                                  DFBRectangle    *src_full_rect,
                                                  DFBRectangle    *dst_full_rect)
{
	DFBResult         ret = DFB_OK;
	CoreLayer         *layer;
	CoreLayerContext  *context;
	DisplayLayerFuncs *funcs;

	D_ASSERT( region != NULL );

	context = region->context;
	D_ASSERT( context != NULL );

	layer = dfb_layer_at( context->layer_id );
	D_ASSERT( layer != NULL );

	funcs = layer->funcs;
	D_ASSERT( funcs != NULL );

	/* Lock the region. */
	if (dfb_layer_region_lock( region ))
		return DFB_FUSION;

	D_ASSUME( region->surface != NULL );

	/* Check for NULL surface. */
	if (!region->surface) {
		dfb_layer_region_unlock( region );
		return DFB_UNSUPPORTED;
	}

	A_TRACE("%s: full_rect(%d,%d,%d,%d)->(%d,%d,%d,%d)\n",__FUNCTION__,
			src_full_rect->x, src_full_rect->y, src_full_rect->w, src_full_rect->h,
			dst_full_rect->x, dst_full_rect->y, dst_full_rect->w, dst_full_rect->h);

	/* primaryBlitLayerSurface/aribBlitLayerSurface */
	if (funcs->BlitLayerSurface) {
		ret = funcs->BlitLayerSurface( layer, layer->driver_data,
			  						   layer->layer_data,
									   src_full_rect,
									   dst_full_rect);
	}
	else {
		D_BUG("unknown funcs BlitLayerSurface ");
		ret = DFB_BUG;
	}
	/* Unlock the region. */
	dfb_layer_region_unlock( region );

	return ret;
}

DFBResult
dfb_arib_layer_region_clear_temporary_surface( CoreLayerRegion *region,
                                               DFBRectangle    *clear_rect )
{
	DFBResult         ret = DFB_OK;
	CoreLayer         *layer;
	CoreLayerContext  *context;
	DisplayLayerFuncs *funcs;

	D_ASSERT( region != NULL );

	context = region->context;
	D_ASSERT( context != NULL );

	layer = dfb_layer_at( context->layer_id );
	D_ASSERT( layer != NULL );

	funcs = layer->funcs;
	D_ASSERT( funcs != NULL );

	/* Lock the region. */
	if (dfb_layer_region_lock( region ))
		return DFB_FUSION;

	A_TRACE("%s: clear_rect(%d,%d,%d,%d)\n",__FUNCTION__,
			clear_rect->x, clear_rect->y, clear_rect->w, clear_rect->h);

	/* primaryClearTmpSurface/aribClearTmpSurface */
	if (funcs->ClearTmpSurface) {
		ret = funcs->ClearTmpSurface( layer, layer->driver_data,
			  			       	      layer->layer_data,
							          clear_rect );
	}
	else {
		D_BUG("unknown funcs ClearTmpSurface ");
		ret = DFB_BUG;
	}
	/* Unlock the region. */
	dfb_layer_region_unlock( region );

	return ret;
}

DFBResult
dfb_arib_layer_region_get_temporary_surface( CoreLayerRegion *region,
                                             CoreSurface     **surface )
{
	DFBResult         ret = DFB_OK;
	CoreLayer         *layer;
	CoreLayerContext  *context;
	DisplayLayerFuncs *funcs;

	D_ASSERT( region != NULL );

	context = region->context;
	D_ASSERT( context != NULL );

	layer = dfb_layer_at( context->layer_id );
	D_ASSERT( layer != NULL );

	funcs = layer->funcs;
	D_ASSERT( funcs != NULL );

	/* Lock the region. */
	if (dfb_layer_region_lock( region ))
		return DFB_FUSION;

	A_TRACE("%s: \n",__FUNCTION__);

	/* primaryGetTmpSurface/aribGetTmpSurface */
	if (funcs->GetTmpSurface) {
		funcs->GetTmpSurface( layer, layer->driver_data,
		                      layer->layer_data, surface );
	}
	else {
		D_BUG("unknown funcs GetTmpSurface ");
		ret = DFB_BUG;
	}
	/* Unlock the region. */
	dfb_layer_region_unlock( region );

	return ret;
}

DFBResult
dfb_arib_layer_region_get_destination_rectangle( CoreLayerRegion *region,
                                                 DFBRectangle    *src_rect,
                                                 DFBRectangle    *src_full_rect,
                                                 DFBRectangle    *dst_rect,
                                                 DFBRectangle    *dst_full_rect,
                                                 DFBRectangle    *modify_rect )
{
	DFBResult         ret = DFB_OK;
	CoreLayer         *layer;
	CoreLayerContext  *context;
	DisplayLayerFuncs *funcs;

	D_ASSERT( region != NULL );

	context = region->context;
	D_ASSERT( context != NULL );

	layer = dfb_layer_at( context->layer_id );
	D_ASSERT( layer != NULL );

	funcs = layer->funcs;
	D_ASSERT( funcs != NULL );

	/* Lock the region. */
	if (dfb_layer_region_lock( region ))
		return DFB_FUSION;

	/* primaryGetDstRefRect/aribGetDstRefRect */
	if (funcs->GetDstRefRect) {
		ret = funcs->GetDstRefRect( layer, layer->driver_data, layer->layer_data,
		                            src_rect, src_full_rect,
		                            dst_rect, dst_full_rect,
		                            modify_rect );

		A_TRACE("%s: src[%d,%d,%d,%d] src_full[%d,%d,%d,%d]\n",
				__FUNCTION__,
				src_rect->x, src_rect->y, src_rect->w, src_rect->h,
				src_full_rect->x, src_full_rect->y, src_full_rect->w, src_full_rect->h);
	}
	else {
		D_BUG("unknown funcs GetDstRefRect ");
		ret = DFB_BUG;
	}
	/* Unlock the region. */
	dfb_layer_region_unlock( region );

	return ret;
}

