/*
   (c) Copyright 2001-2008  The world wide DirectFB Open Source Community (directfb.org)
   (c) Copyright 2000-2004  Convergence (integrated media) GmbH

   All rights reserved.

   Written by Denis Oliver Kropp <dok@directfb.org>,
              Andreas Hundt <andi@fischlustig.de>,
              Sven Neumann <neo@directfb.org>,
              Ville Syrjälä <syrjala@sci.fi> and
              Claudio Ciccani <klan@users.sf.net>.

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

#include <directfb.h>
#include <directfb_strings.h>

#include <direct/clock.h>
#include <direct/log.h>

#include <core/core.h>

#include <dfbdump/dfbdump.h>


static bool do_loop;
static bool show_shm;
static bool show_pools;
static bool show_allocs;
static int  dump_layer;       /* ref or -1 (all) or 0 (none) */
static int  dump_surface;     /* ref or -1 (all) or 0 (none) */

/**********************************************************************************************************************/

static DFBBoolean parse_command_line( int argc, char *argv[] );

/**********************************************************************************************************************/

static void
run_dump( CoreDFB *core )
{
     long long millis;
     long int  seconds, minutes, hours, days;

     millis = direct_clock_get_millis();

     seconds  = millis / 1000;
     millis  %= 1000;

     minutes  = seconds / 60;
     seconds %= 60;

     hours    = minutes / 60;
     minutes %= 60;

     days     = hours / 24;
     hours   %= 24;

     switch (days) {
          case 0:
               direct_log_printf( NULL, "\nDirectFB uptime: %02ld:%02ld:%02ld\n",
                                  hours, minutes, seconds );
               break;

          case 1:
               direct_log_printf( NULL, "\nDirectFB uptime: %ld day, %02ld:%02ld:%02ld\n",
                                  days, hours, minutes, seconds );
               break;

          default:
               direct_log_printf( NULL, "\nDirectFB uptime: %ld days, %02ld:%02ld:%02ld\n",
                                  days, hours, minutes, seconds );
               break;
     }

     dfbdump_show_surfaces( core, dump_surface );
     direct_log_flush( NULL, false );

     dfbdump_show_layers( core, dump_layer );
     direct_log_flush( NULL, false );

     if (show_shm) {
          direct_log_printf( NULL, "\n" );
          dfbdump_show_shmpools( dfb_core_world(core) );
          direct_log_flush( NULL, false );
     }

     if (show_pools) {
          direct_log_printf( NULL, "\n" );
          dfbdump_show_surface_pool_info( core );
          direct_log_flush( NULL, false );
     }

     if (show_allocs) {
          direct_log_printf( NULL, "\n" );
          dfbdump_show_surface_pools( core );
          direct_log_flush( NULL, false );
     }

     direct_log_printf( NULL, "\n" );
}

int
main( int argc, char *argv[] )
{
     DFBResult  ret;
     CoreDFB   *core;

     direct_log_set_buffer( NULL, NULL, 0x100000 );

     /* Initialize DirectFB. */
     ret = DirectFBInit( &argc, &argv );
     if (ret) {
          D_DERROR( ret, "Tools/Dump: DirectFBInit() failed!\n" );
          return -1;
     }

     /* Parse the command line. */
     if (!parse_command_line( argc, argv ))
          return -2;

     /* Create the core. */
     ret = dfb_core_create( &core );
     if (ret) {
          D_DERROR( ret, "Tools/Dump: dfb_core_create() failed!\n" );
          return -3;
     }

     dfb_core_activate( core );


     do {
          run_dump( core );
     } while (do_loop && !sleep( 1 ));


     /* Destroy the core. */
     dfb_core_destroy( core, false );

     return ret;
}

/**********************************************************************************************************************/

static void
print_usage (const char *prg_name)
{
     direct_log_printf( NULL, "\nDirectFB Dump (version %s)\n\n", DIRECTFB_VERSION );
     direct_log_printf( NULL, "Usage: %s [options]\n\n", prg_name );
     direct_log_printf( NULL, "Options:\n" );
     direct_log_printf( NULL, "   -s,  --shm          Show shared memory pool content (if debug enabled)\n" );
     direct_log_printf( NULL, "   -p,  --pools        Show information about surface pools\n" );
     direct_log_printf( NULL, "   -a,  --allocs       Show surface buffer allocations in surface pools\n" );
     direct_log_printf( NULL, "   -dl, --dumplayer    Dump surfaces of layer contexts into files (dfb_layer_context_REFID...)\n" );
     direct_log_printf( NULL, "   -ds, --dumpsurface  Dump surfaces (front buffers) into files (dfb_surface_REFID...)\n" );
     direct_log_printf( NULL, "   -l,  --loop         Loop mode, one second between each dump\n" );
     direct_log_printf( NULL, "   -h,  --help         Show this help message\n" );
     direct_log_printf( NULL, "   -v,  --version      Print version information\n" );
     direct_log_printf( NULL, "\n");
}

static DFBBoolean
parse_command_line( int argc, char *argv[] )
{
     int n;

     for (n = 1; n < argc; n++) {
          const char *arg = argv[n];

          if (strcmp (arg, "-h") == 0 || strcmp (arg, "--help") == 0) {
               print_usage (argv[0]);
               return DFB_FALSE;
          }

          if (strcmp (arg, "-v") == 0 || strcmp (arg, "--version") == 0) {
               direct_log_printf( NULL, "dfbdump version %s\n", DIRECTFB_VERSION );
               return DFB_FALSE;
          }

          if (strcmp (arg, "-s") == 0 || strcmp (arg, "--shm") == 0) {
               show_shm = true;
               continue;
          }

          if (strcmp (arg, "-p") == 0 || strcmp (arg, "--pools") == 0) {
               show_pools = true;
               continue;
          }

          if (strcmp (arg, "-a") == 0 || strcmp (arg, "--allocs") == 0) {
               show_allocs = true;
               continue;
          }

          if (strcmp (arg, "-dl") == 0 || strcmp (arg, "--dumplayer") == 0) {
               dump_layer = -1;
               continue;
          }

          if (strcmp (arg, "-ds") == 0 || strcmp (arg, "--dumpsurface") == 0) {
               dump_surface = -1;
               continue;
          }

          if (strcmp (arg, "-l") == 0 || strcmp (arg, "--loop") == 0) {
               do_loop = true;
               continue;
          }

          print_usage (argv[0]);

          return DFB_FALSE;
     }

     return DFB_TRUE;
}

