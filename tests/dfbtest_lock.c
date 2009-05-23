/*
   (c) Copyright 2008  Denis Oliver Kropp

   All rights reserved.

   This file is subject to the terms and conditions of the MIT License:

   Permission is hereby granted, free of charge, to any person
   obtaining a copy of this software and associated documentation
   files (the "Software"), to deal in the Software without restriction,
   including without limitation the rights to use, copy, modify, merge,
   publish, distribute, sublicense, and/or sell copies of the Software,
   and to permit persons to whom the Software is furnished to do so,
   subject to the following conditions:

   The above copyright notice and this permission notice shall be
   included in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
   EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
   IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
   CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
   TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
   SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <config.h>

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <direct/messages.h>

#include <directfb.h>
#include <directfb_strings.h>
#include <directfb_util.h>
#include <directfb_version.h>

#include <idirectfb.h>

#include <dfbdump/dfbdump.h>

/**********************************************************************************************************************/

static int
print_usage( const char *prg )
{
     fprintf (stderr, "\n");
     fprintf (stderr, "== DirectFB Surface Locking Test (version %s) ==\n", DIRECTFB_VERSION);
     fprintf (stderr, "\n");
     fprintf (stderr, "\n");
     fprintf (stderr, "Usage: %s [options]\n", prg);
     fprintf (stderr, "\n");
     fprintf (stderr, "Options:\n");
     fprintf (stderr, "  -h, --help                        Show this help message\n");
     fprintf (stderr, "  -v, --version                     Print version information\n");
     fprintf (stderr, "  -r, --read                        Set read mode\n");
     fprintf (stderr, "  -w, --write                       Set write mode\n");
#if DIRECTFB_VERSION_CHECK( 1, 5, 0 )
     fprintf (stderr, "  -R, --rectangle  <x,y,w,h>        Set rectangle to be locked\n");
#endif
     fprintf (stderr, "  -b, --benchmark                   Enable benchmarking mode\n");

     return -1;
}

static bool
parse_rectangle( const char   *arg,
                 DFBRectangle *ret_rect )
{
     return sscanf( arg, "%d,%d,%d,%d", &ret_rect->x, &ret_rect->y, &ret_rect->w, &ret_rect->h ) == 4;
}

/**********************************************************************************************************************/

static void
test_lock( IDirectFBSurface    *surface,
           DFBSurfaceLockFlags  flags )
{
     DFBResult  ret;
     int        width, height;
     void      *data;
     int        pitch;

     surface->GetSize( surface, &width, &height );

     ret = surface->Lock( surface, flags, &data, &pitch );
     if (ret) {
          D_DERROR( ret, "DFBTest/Lock: IDirectFBSurface::Lock( 0x%x ) failed!\n", flags );
          return;
     }

     if (flags & DSLF_WRITE) {
          memset( data, 0xff, width * 4 );

          memset( data + pitch * (height - 1), 0xff, width * 4 );
     }

     surface->Unlock( surface );
}

#if DIRECTFB_VERSION_CHECK( 1, 5, 0 )
static void
test_lock_rect( IDirectFBSurface    *surface,
                DFBSurfaceLockFlags  flags,
                const DFBRectangle  *rect )
{
     DFBResult  ret;
     void      *data;
     int        pitch;

     ret = surface->LockRectangle( surface, flags, rect, &data, &pitch );
     if (ret) {
          D_DERROR( ret, "DFBTest/Lock: IDirectFBSurface::LockRectangle( 0x%x, %d,%d - %dx%d ) failed!\n",
                    flags, DFB_RECTANGLE_VALS( rect ) );
          return;
     }

     if (flags & DSLF_WRITE) {
          memset( data, 0xff, rect->w * 4 );

          memset( data + pitch * (rect->h - 1), 0xff, rect->w * 4 );
     }

     surface->Unlock( surface );
}
#endif

int
main( int argc, char *argv[] )
{
     int                     i;
     DFBResult               ret;
     DFBSurfaceDescription   desc;
     DFBRectangle            rect;
     IDirectFB              *dfb;
     IDirectFBSurface       *surface   = NULL;
     IDirectFBSurface       *dest      = NULL;
     DFBSurfaceLockFlags     lock      = 0;
     bool                    lock_rect = false;
     bool                    benchmark = false;

     /* Initialize DirectFB. */
     ret = DirectFBInit( &argc, &argv );
     if (ret) {
          D_DERROR( ret, "DFBTest/Lock: DirectFBInit() failed!\n" );
          return ret;
     }

     /* Parse arguments. */
     for (i=1; i<argc; i++) {
          const char *arg = argv[i];

          if (strcmp( arg, "-h" ) == 0 || strcmp (arg, "--help") == 0)
               return print_usage( argv[0] );
          else if (strcmp (arg, "-v") == 0 || strcmp (arg, "--version") == 0) {
               fprintf (stderr, "dfbtest_blit version %s\n", DIRECTFB_VERSION);
               return 0;
          }
          else if (strcmp (arg, "-r") == 0 || strcmp (arg, "--read") == 0)
               lock |= DSLF_READ;
          else if (strcmp (arg, "-w") == 0 || strcmp (arg, "--write") == 0)
               lock |= DSLF_WRITE;
          else if (strcmp (arg, "-R") == 0 || strcmp (arg, "--rectangle") == 0) {
               if (++i == argc)
                    return print_usage (argv[0]);

               if (!parse_rectangle( argv[i], &rect ))
                    return print_usage (argv[0]);

               lock_rect = true;
          }
          else if (strcmp (arg, "-b") == 0 || strcmp (arg, "--benchmark") == 0)
               benchmark = true;
          else
               return print_usage( argv[0] );
     }

     if (!lock)
          lock = DSLF_READ | DSLF_WRITE;

     /* Create super interface. */
     ret = DirectFBCreate( &dfb );
     if (ret) {
          D_DERROR( ret, "DFBTest/Lock: DirectFBCreate() failed!\n" );
          return ret;
     }

     /* Fill description for a primary surface. */
     desc.flags = DSDESC_CAPS;
     desc.caps  = DSCAPS_PRIMARY | DSCAPS_FLIPPING;

     dfb->SetCooperativeLevel( dfb, DFSCL_FULLSCREEN );

     /* Create a primary surface. */
     ret = dfb->CreateSurface( dfb, &desc, &dest );
     if (ret) {
          D_DERROR( ret, "DFBTest/Lock: IDirectFB::CreateSurface() failed!\n" );
          goto out;
     }

     dest->GetSize( dest, &desc.width, &desc.height );
     dest->GetPixelFormat( dest, &desc.pixelformat );

     D_INFO( "DFBTest/Lock: Destination is %dx%d using %s\n",
             desc.width, desc.height, dfb_pixelformat_name(desc.pixelformat) );



     /* Create the test surface. */
     desc.flags       = DSDESC_WIDTH | DSDESC_HEIGHT | DSDESC_PIXELFORMAT;
     desc.pixelformat = DSPF_RGB32;

     ret = dfb->CreateSurface( dfb, &desc, &surface );
     if (ret) {
          D_DERROR( ret, "DFBTest/Lock: IDirectFB::CreateSurface() failed!\n" );
          goto out;
     }

     D_INFO( "DFBTest/Lock: Surface is %dx%d using %s\n",
             desc.width, desc.height, dfb_pixelformat_name(desc.pixelformat) );


     surface->Clear( surface, 0, 0, 0, 0 );

#if DIRECTFB_VERSION_CHECK( 1, 5, 0 )
     if (lock_rect)
          test_lock_rect( surface, lock, &rect );
     else
#endif
          test_lock( surface, lock );

     dest->StretchBlit( dest, surface, NULL, NULL );
     dest->Flip( dest, NULL, DSFLIP_NONE );


     dfbdump_show_surface_pools( ((IDirectFB_data*) dfb)->core );

     if (benchmark) {
          int       num = 0;
          long long start, diff = 0, speed;

          sync();

          sleep( 1 );

          dest->StretchBlit( dest, surface, NULL, NULL );

          D_INFO( "DFBTest/Lock: Benchmarking...\n" );

          dfb->WaitIdle( dfb );

          start = direct_clock_get_millis();

          do {
#if DIRECTFB_VERSION_CHECK( 1, 5, 0 )
               if (lock_rect)
                    test_lock_rect( surface, lock, &rect );
               else
#endif
                    test_lock( surface, lock );

               dest->StretchBlit( dest, surface, NULL, NULL );

               if ((num & 7) == 7)
                    diff = direct_clock_get_millis() - start;

               num++;
          } while (diff < 2300);

          dfb->WaitIdle( dfb );

          diff = direct_clock_get_millis() - start;

          speed = (long long) num * desc.width * desc.height / diff;

          D_INFO( "DFBTest/Lock: Speed is %lld.%03lld MPixel/sec (%dx%d x %d in %lld.%03lld sec)\n",
                  speed / 1000LL, speed % 1000LL, desc.width, desc.height, num, diff / 1000LL, diff % 1000LL );
     }
     else
          sleep( 2 );


out:
     if (surface)
          surface->Release( surface );

     if (dest)
          dest->Release( dest );

     /* Shutdown DirectFB. */
     dfb->Release( dfb );

     return ret;
}

