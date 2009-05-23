/*
   (c) Copyright 2001-2008  The world wide DirectFB Open Source Community (directfb.org)
   (c) Copyright 2000-2004  Convergence (integrated media) GmbH

   All rights reserved.

   Written by Denis Oliver Kropp <dok@directfb.org>,
              Andreas Hundt <andi@fischlustig.de>,
              Sven Neumann <neo@directfb.org>,
              Ville Syrjälä <syrjala@sci.fi> and
              Claudio Ciccani <klan@users.sf.net>.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the
   Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include <config.h>

#include <directfb_util.h>

#include <direct/debug.h>
#include <direct/memcpy.h>
#include <direct/util.h>


D_DEBUG_DOMAIN( DFB_Updates, "DirectFB/Updates", "DirectFB Updates" );

/**********************************************************************************************************************/

const DirectFBPixelFormatNames( dfb_pixelformat_names );

/**********************************************************************************************************************/

bool
dfb_region_rectangle_intersect( DFBRegion          *region,
                                const DFBRectangle *rect )
{
     int x2 = rect->x + rect->w - 1;
     int y2 = rect->y + rect->h - 1;

     if (region->x2 < rect->x ||
         region->y2 < rect->y ||
         region->x1 > x2 ||
         region->y1 > y2)
          return false;

     if (region->x1 < rect->x)
          region->x1 = rect->x;

     if (region->y1 < rect->y)
          region->y1 = rect->y;

     if (region->x2 > x2)
          region->x2 = x2;

     if (region->y2 > y2)
          region->y2 = y2;

     return true;
}

bool
dfb_unsafe_region_intersect( DFBRegion *region,
                             int x1, int y1, int x2, int y2 )
{
     if (region->x1 > region->x2) {
          int temp = region->x1;
          region->x1 = region->x2;
          region->x2 = temp;
     }

     if (region->y1 > region->y2) {
          int temp = region->y1;
          region->y1 = region->y2;
          region->y2 = temp;
     }

     return dfb_region_intersect( region, x1, y1, x2, y2 );
}

bool
dfb_unsafe_region_rectangle_intersect( DFBRegion          *region,
                                       const DFBRectangle *rect )
{
     if (region->x1 > region->x2) {
          int temp = region->x1;
          region->x1 = region->x2;
          region->x2 = temp;
     }

     if (region->y1 > region->y2) {
          int temp = region->y1;
          region->y1 = region->y2;
          region->y2 = temp;
     }

     return dfb_region_rectangle_intersect( region, rect );
}

bool
dfb_rectangle_intersect_by_unsafe_region( DFBRectangle *rectangle,
                                          DFBRegion    *region )
{
     /* validate region */
     if (region->x1 > region->x2) {
          int temp = region->x1;
          region->x1 = region->x2;
          region->x2 = temp;
     }

     if (region->y1 > region->y2) {
          int temp = region->y1;
          region->y1 = region->y2;
          region->y2 = temp;
     }

     /* adjust position */
     if (region->x1 > rectangle->x) {
          rectangle->w -= region->x1 - rectangle->x;
          rectangle->x = region->x1;
     }

     if (region->y1 > rectangle->y) {
          rectangle->h -= region->y1 - rectangle->y;
          rectangle->y = region->y1;
     }

     /* adjust size */
     if (region->x2 < rectangle->x + rectangle->w - 1)
        rectangle->w = region->x2 - rectangle->x + 1;

     if (region->y2 < rectangle->y + rectangle->h - 1)
        rectangle->h = region->y2 - rectangle->y + 1;

     /* set size to zero if there's no intersection */
     if (rectangle->w <= 0 || rectangle->h <= 0) {
          rectangle->w = 0;
          rectangle->h = 0;

          return false;
     }

     return true;
}

bool
dfb_rectangle_intersect_by_region( DFBRectangle    *rectangle,
                                   const DFBRegion *region )
{
     /* adjust position */
     if (region->x1 > rectangle->x) {
          rectangle->w -= region->x1 - rectangle->x;
          rectangle->x = region->x1;
     }

     if (region->y1 > rectangle->y) {
          rectangle->h -= region->y1 - rectangle->y;
          rectangle->y = region->y1;
     }

     /* adjust size */
     if (region->x2 < rectangle->x + rectangle->w - 1)
        rectangle->w = region->x2 - rectangle->x + 1;

     if (region->y2 < rectangle->y + rectangle->h - 1)
        rectangle->h = region->y2 - rectangle->y + 1;

     /* set size to zero if there's no intersection */
     if (rectangle->w <= 0 || rectangle->h <= 0) {
          rectangle->w = 0;
          rectangle->h = 0;

          return false;
     }

     return true;
}

bool dfb_rectangle_intersect( DFBRectangle       *rectangle,
                              const DFBRectangle *clip )
{
     DFBRegion region = { clip->x, clip->y,
                          clip->x + clip->w - 1, clip->y + clip->h - 1 };

     /* adjust position */
     if (region.x1 > rectangle->x) {
          rectangle->w -= region.x1 - rectangle->x;
          rectangle->x = region.x1;
     }

     if (region.y1 > rectangle->y) {
          rectangle->h -= region.y1 - rectangle->y;
          rectangle->y = region.y1;
     }

     /* adjust size */
     if (region.x2 < rectangle->x + rectangle->w - 1)
          rectangle->w = region.x2 - rectangle->x + 1;

     if (region.y2 < rectangle->y + rectangle->h - 1)
          rectangle->h = region.y2 - rectangle->y + 1;

     /* set size to zero if there's no intersection */
     if (rectangle->w <= 0 || rectangle->h <= 0) {
          rectangle->w = 0;
          rectangle->h = 0;

          return false;
     }

     return true;
}

void dfb_rectangle_union ( DFBRectangle       *rect1,
                           const DFBRectangle *rect2 )
{
     if (!rect2->w || !rect2->h)
          return;

     /* FIXME: OPTIMIZE */

     if (rect1->w) {
          int temp = MIN (rect1->x, rect2->x);
          rect1->w = MAX (rect1->x + rect1->w, rect2->x + rect2->w) - temp;
          rect1->x = temp;
     }
     else {
          rect1->x = rect2->x;
          rect1->w = rect2->w;
     }

     if (rect1->h) {
          int temp = MIN (rect1->y, rect2->y);
          rect1->h = MAX (rect1->y + rect1->h, rect2->y + rect2->h) - temp;
          rect1->y = temp;
     }
     else {
          rect1->y = rect2->y;
          rect1->h = rect2->h;
     }
}

/**********************************************************************************************************************/

void
dfb_updates_init( DFBUpdates   *updates,
                  DFBRegion    *regions,
                  unsigned int  max_regions )
{
     D_DEBUG_AT( DFB_Updates, "%s( %p, %p [%d] )\n", __FUNCTION__, updates, regions, max_regions );

     D_ASSERT( updates != NULL );
     D_ASSERT( regions != NULL );
     D_ASSERT( max_regions > 0 );

     updates->regions     = regions;
     updates->max_regions = max_regions;
     updates->num_regions = 0;

     D_MAGIC_SET( updates, DFBUpdates );
}

void
dfb_updates_init_from( DFBUpdates       *updates,
                       DFBRegion        *regions,
                       unsigned int      max_regions,
                       const DFBUpdates *source )
{
     D_DEBUG_AT( DFB_Updates, "%s( %p, %p [%d], %p )\n", __FUNCTION__, updates, regions, max_regions, source );

     D_ASSERT( updates != NULL );
     D_ASSERT( regions != NULL );
     D_ASSERT( max_regions > 0 );
     DFB_UPDATES_ASSERT( source );

     DFB_UPDATES_DEBUG_AT( DFB_Updates, source );

     updates->regions     = regions;
     updates->max_regions = max_regions;

     if (source->num_regions > max_regions) {
          D_LOG( DFB_Updates, VERBOSE, "Number of regions in source (%d) exceeds max. number of regions (%d)\n",
                 source->num_regions, max_regions );

          updates->bounding    = updates->regions[0] = source->bounding;
          updates->num_regions = 1;
     }
     else {
          direct_memcpy( &updates->regions[0], &source->regions[0], sizeof(source->regions[0]) * source->num_regions );

          updates->bounding    = source->bounding;
          updates->num_regions = source->num_regions;
     }

     D_MAGIC_SET( updates, DFBUpdates );
}

DFBRegion *
dfb_updates_add( DFBUpdates      *updates,
                 const DFBRegion *region )
{
     int i;

     DFB_UPDATES_ASSERT( updates );
     DFB_REGION_ASSERT( region );

     D_DEBUG_AT( DFB_Updates, "%s( %p, %4d,%4d-%4dx%4d )\n", __FUNCTION__, updates,
                 DFB_RECTANGLE_VALS_FROM_REGION(region) );

     if (updates->num_regions == 0) {
          D_DEBUG_AT( DFB_Updates, "  -> added as first\n" );

          updates->regions[0]  = updates->bounding = *region;
          updates->num_regions = 1;

          return &updates->regions[0];
     }

     for (i=0; i<updates->num_regions; i++) {
          if (dfb_region_region_extends( &updates->regions[i], region ) ||
              dfb_region_region_intersects( &updates->regions[i], region ))
          {
               D_DEBUG_AT( DFB_Updates, "  -> combined with [%d] %4d,%4d-%4dx%4d\n", i,
                           DFB_RECTANGLE_VALS_FROM_REGION(&updates->regions[i]) );

               dfb_region_region_union( &updates->regions[i], region );

               dfb_region_region_union( &updates->bounding, region );

               D_DEBUG_AT( DFB_Updates, "  -> resulting in  [%d] %4d,%4d-%4dx%4d\n", i,
                           DFB_RECTANGLE_VALS_FROM_REGION(&updates->regions[i]) );

               return &updates->regions[i];
          }
     }

     if (updates->num_regions == updates->max_regions) {
          dfb_region_region_union( &updates->bounding, region );

          updates->regions[0]  = updates->bounding;
          updates->num_regions = 1;

          D_DEBUG_AT( DFB_Updates, "  -> collapsing to [0] %4d,%4d-%4dx%4d\n",
                      DFB_RECTANGLE_VALS_FROM_REGION(&updates->regions[0]) );

          return &updates->regions[0];
     }

     updates->regions[updates->num_regions++] = *region;

     dfb_region_region_union( &updates->bounding, region );

     D_DEBUG_AT( DFB_Updates, "  -> added as      [%d] %4d,%4d-%4dx%4d\n", updates->num_regions - 1,
                 DFB_RECTANGLE_VALS_FROM_REGION(&updates->regions[updates->num_regions - 1]) );

     return &updates->regions[updates->num_regions-1];
}

void
dfb_updates_stat( const DFBUpdates *updates,
                  int              *ret_total,
                  int              *ret_bounding )
{
     int i;

     DFB_UPDATES_ASSERT( updates );

     if (updates->num_regions == 0) {
          if (ret_total)
               *ret_total = 0;

          if (ret_bounding)
               *ret_bounding = 0;

          return;
     }

     if (ret_total) {
          int total = 0;

          for (i=0; i<updates->num_regions; i++) {
               const DFBRegion *r = &updates->regions[i];

               total += (r->x2 - r->x1 + 1) * (r->y2 - r->y1 + 1);
          }

          *ret_total = total;
     }

     if (ret_bounding)
          *ret_bounding = (updates->bounding.x2 - updates->bounding.x1 + 1) *
                          (updates->bounding.y2 - updates->bounding.y1 + 1);
}

void
dfb_updates_get_rectangles( const DFBUpdates *updates,
                            DFBRectangle     *ret_rects,
                            int              *ret_num )
{
     DFB_UPDATES_ASSERT( updates );
     D_ASSERT( ret_rects != NULL );
     D_ASSERT( ret_num != NULL );

     switch (updates->num_regions) {
          case 0:
               *ret_num = 0;
               break;

          default: {
               int n, d, total, bounding;

               dfb_updates_stat( updates, &total, &bounding );

               n = updates->max_regions - updates->num_regions + 1;
               d = n + 1;

               /* Try to optimize updates. Use individual regions only if not too much overhead. */
               if (total < bounding * n / d) {
                    *ret_num = updates->num_regions;

                    for (n=0; n<updates->num_regions; n++)
                         ret_rects[n] = DFB_RECTANGLE_INIT_FROM_REGION( &updates->regions[n] );

                    break;
               }
          }
          /* fall through */

          case 1:
               *ret_num   = 1;
               *ret_rects = DFB_RECTANGLE_INIT_FROM_REGION( &updates->bounding );
               break;
     }
}

void
dfb_updates_get_boxes( const DFBUpdates *updates,
                       DFBBox           *ret_boxes,
                       int              *ret_num )
{
     DFB_UPDATES_ASSERT( updates );
     D_ASSERT( ret_boxes != NULL );
     D_ASSERT( ret_num != NULL );

     switch (updates->num_regions) {
          case 0:
               *ret_num = 0;
               break;

          default: {
               int n, d, total, bounding;

               dfb_updates_stat( updates, &total, &bounding );

               n = updates->max_regions - updates->num_regions + 1;
               d = n + 1;

               /* Try to optimize updates. Use individual regions only if not too much overhead. */
               if (total < bounding * n / d) {
                    *ret_num = updates->num_regions;

                    for (n=0; n<updates->num_regions; n++)
                         ret_boxes[n] = DFB_BOX_INIT_FROM_REGION( &updates->regions[n] );

                    break;
               }
          }
          /* fall through */

          case 1:
               *ret_num   = 1;
               *ret_boxes = DFB_BOX_INIT_FROM_REGION( &updates->bounding );
               break;
     }
}

int
dfb_updates_get_pixels( const DFBUpdates *updates )
{
     int bounding;

     DFB_UPDATES_ASSERT( updates );

     if (!updates->num_regions)
          return 0;

     bounding = (updates->bounding.x2 - updates->bounding.x1 + 1) *
                (updates->bounding.y2 - updates->bounding.y1 + 1);

     if (updates->num_regions > 1) {
          int n, d, pixels = 0;

          for (n=0; n<updates->num_regions; n++)
               pixels += (updates->regions[n].x2 - updates->regions[n].x1 + 1) *
                         (updates->regions[n].y2 - updates->regions[n].y1 + 1);

          n = updates->max_regions - updates->num_regions + 1;
          d = n + 1;

          /* Try to optimize updates. Use individual regions only if not too much overhead. */
          if (pixels < bounding * n / d)
               return pixels;
     }

     return bounding;
}

bool
dfb_updates_intersects( const DFBUpdates *updates,
                        const DFBBox     *box )
{
     int i;

     D_DEBUG_AT( DFB_Updates, "%s( %p, %p )\n", __FUNCTION__, updates, box );

     DFB_UPDATES_ASSERT( updates );

     D_ASSERT( box != NULL );
     D_DEBUG_AT( DFB_Updates, "  -> CHECKING %4d,%4d-%4dx%4d\n", DFB_RECTANGLE_VALS_FROM_BOX( box ) );

     DFB_BOX_ASSERT( box );

     if (!updates->num_regions) {
          D_DEBUG_AT( DFB_Updates, "  -> no updates\n" );
          return false;
     }

     D_DEBUG_AT( DFB_Updates, "  -> BOUNDING %4d,%4d-%4dx%4d\n",
                 DFB_RECTANGLE_VALS_FROM_REGION( &updates->bounding ) );

     if (!dfb_box_region_intersects( box, &updates->bounding )) {
          D_DEBUG_AT( DFB_Updates, "  => NO INTERSECTION with bounding box!\n" );
          return false;
     }

     if (updates->num_regions == 1) {
          D_DEBUG_AT( DFB_Updates, "  => intersects (single box)\n" );
          return true;
     }

     for (i=0; i<updates->num_regions; i++) {
          if (dfb_box_region_intersects( box, &updates->regions[i] )) {
               DFB_REGIONS_DEBUG_AT( DFB_Updates, updates->regions, updates->num_regions );
               D_DEBUG_AT( DFB_Updates, "  => intersects with [%d]\n", i );
               return true;
          }
     }

     D_DEBUG_AT( DFB_Updates, "  => NO INTERSECTION after detailed check!\n" );

     return false;
}

void
dfb_updates_translate( DFBUpdates     *updates,
                       const DFBPoint *offset )
{
     int i;

     D_DEBUG_AT( DFB_Updates, "%s( %p, %p )\n", __FUNCTION__, updates, offset );

     DFB_UPDATES_ASSERT( updates );
     D_ASSERT( offset != NULL );

     D_DEBUG_AT( DFB_Updates, "  +-=> OFFSET %4d,%4d\n", DFB_POINT_VALS( offset ) );

     if (updates->num_regions) {
          dfb_region_translate( &updates->bounding, offset->x, offset->y );

          for (i=0; i<updates->num_regions; i++)
               dfb_region_translate( &updates->regions[i], offset->x, offset->y );
     }
}



/**********************************************************************************************************************/
/**********************************************************************************************************************/

/**********************************************************************************************************************/

void
DFBTachopese_Init( DFBTachopese *thiz )
{
     memset( thiz, 0, sizeof(*thiz) );

     thiz->duration = 1000000;
}

void
DFBTachopese_SetInterval( DFBTachopese *thiz, int millis )
{
     thiz->duration = millis * 1000;
}


void
DFBTachopese_Start( DFBTachopese *thiz )
{
     thiz->pixels    = 0;
     thiz->flips     = 0;
     thiz->flipsels  = 0;
     thiz->chars     = 0;
     thiz->strings   = 0;
     thiz->moves     = 0;
     thiz->cycles    = 3;
     thiz->counter   = 0;
     thiz->last_time = thiz->start_time = direct_clock_get_micros();


     getrusage( RUSAGE_SELF, &thiz->last_usage );
}

bool
DFBTachopese_Continue( DFBTachopese *thiz )
{
     if (thiz->counter < thiz->cycles) {
          thiz->counter++;

          return true;
     }

     long long this_time = direct_clock_get_micros();

     int time_diff  = this_time - thiz->last_time;
     int total_diff = this_time - thiz->start_time;
     int remaining  = thiz->duration  - total_diff;

     if (remaining < 2)
          return false;

     if (time_diff > 0) {
          int goal = (remaining < 200000) ? remaining : 200000;

          thiz->cycles = thiz->cycles * goal / time_diff;
          if (thiz->cycles < 2)
               thiz->cycles = 2;
          else if (thiz->cycles > 100000)
               thiz->cycles = 100000;
     }
     else
          thiz->cycles <<= 2;

     thiz->counter   = 0;
     thiz->last_time = this_time;

     return true;
}

void
DFBTachopese_Stop( DFBTachopese *thiz )
{
     thiz->stop_time = direct_clock_get_micros();

     getrusage( RUSAGE_SELF, &thiz->stop_usage );
}

float
DFBTachopese_GetStringsPerSecond( DFBTachopese *thiz )
{
     return 1000000 * thiz->strings / (float)(thiz->stop_time - thiz->start_time);
}

float
DFBTachopese_GetCharsPerSecond( DFBTachopese *thiz )
{
     return 1000000 * thiz->chars / (float)(thiz->stop_time - thiz->start_time);
}

float
DFBTachopese_GetMovesPerSecond( DFBTachopese *thiz )
{
     return 1000000 * thiz->moves / (float)(thiz->stop_time - thiz->start_time);
}

float
DFBTachopese_GetFlipsPerSecond( DFBTachopese *thiz )
{
     return 1000000 * thiz->flips / (float)(thiz->stop_time - thiz->start_time);
}

float
DFBTachopese_GetFlipselsPerSecond( DFBTachopese *thiz )
{
     return 1000000 * thiz->flipsels / (float)(thiz->stop_time - thiz->start_time);
}

float
DFBTachopese_GetPixelsPerSecond( DFBTachopese *thiz )
{
     return 1000000 * thiz->pixels / (float)(thiz->stop_time - thiz->start_time);
}

float
DFBTachopese_GetSeconds( DFBTachopese *thiz )
{
     return (thiz->stop_time - thiz->start_time) / 1000000.0;
}

float
DFBTachopese_GetUserSeconds( DFBTachopese *thiz )
{
     return (thiz->stop_usage.ru_utime.tv_sec  - thiz->last_usage.ru_utime.tv_sec) +
            (thiz->stop_usage.ru_utime.tv_usec - thiz->last_usage.ru_utime.tv_usec) / 1000000.0;
}

float
DFBTachopese_GetSystemSeconds( DFBTachopese *thiz )
{
     return (thiz->stop_usage.ru_stime.tv_sec  - thiz->last_usage.ru_stime.tv_sec) +
            (thiz->stop_usage.ru_stime.tv_usec - thiz->last_usage.ru_stime.tv_usec) / 1000000.0;
}

float
DFBTachopese_GetCPUSeconds( DFBTachopese *thiz )
{
     return DFBTachopese_GetUserSeconds( thiz ) + DFBTachopese_GetSystemSeconds( thiz );
}

float
DFBTachopese_GetUserUsage( DFBTachopese *thiz )
{
     float usage = DFBTachopese_GetUserSeconds( thiz ) / DFBTachopese_GetSeconds( thiz );

     return usage < 1.0 ? usage : 1.0;
}

float
DFBTachopese_GetSystemUsage( DFBTachopese *thiz )
{
     float usage = DFBTachopese_GetSystemSeconds( thiz ) / DFBTachopese_GetSeconds( thiz );

     return usage < 1.0 ? usage : 1.0;
}

float
DFBTachopese_GetCPUUsage( DFBTachopese *thiz )
{
     float usage = DFBTachopese_GetCPUSeconds( thiz ) / DFBTachopese_GetSeconds( thiz );

     return usage < 1.0 ? usage : 1.0;
}

unsigned long long
DFBTachopese_GetFlips( DFBTachopese *thiz )
{
     return thiz->flips;
}

unsigned long long
DFBTachopese_GetFlipsels( DFBTachopese *thiz )
{
     return thiz->flips;
}

unsigned long long
DFBTachopese_GetPixels( DFBTachopese *thiz )
{
     return thiz->pixels;
}


void
DFBTachopese_Move( DFBTachopese *thiz, int dx, int dy )
{
     thiz->moves++;
}

void
DFBTachopese_Flip( DFBTachopese *thiz, int width, int height )
{
     D_ASSERT( width  >= 0 );
     D_ASSERT( height >= 0 );

     thiz->flipsels += width * height;
     thiz->flips++;
}

void
DFBTachopese_DrawString( DFBTachopese *thiz, int length )
{
     D_ASSERT( length >= 0 );

     thiz->strings++;
     thiz->chars += length;
}

void
DFBTachopese_FillRectangle( DFBTachopese *thiz, int width, int height )
{
     D_ASSERT( width  >= 0 );
     D_ASSERT( height >= 0 );

     thiz->pixels += width * height;
}

void
DFBTachopese_FillRectangles( DFBTachopese *thiz, const DFBRectangle *rects, int num )
{
     int i;

     for (i=0; i<num; i++) {
          D_ASSERT( rects[i].w >= 0 );
          D_ASSERT( rects[i].h >= 0 );

          thiz->pixels += rects[i].w * rects[i].h;
     }
}

void
DFBTachopese_DrawRectangle( DFBTachopese *thiz, int width, int height )
{
     D_ASSERT( width  >= 0 );
     D_ASSERT( height >= 0 );

     thiz->pixels += width * 2 + (height - 2) * 2;
}

void
DFBTachopese_DrawLine( DFBTachopese *thiz, int x1, int y1, int x2, int y2 )
{
     int dx = x2 - x1;
     int dy = y2 - y1;

     D_ASSERT( dx >= 0 );
     D_ASSERT( dy >= 0 );

     thiz->pixels += MAX( dx, dy );
}

