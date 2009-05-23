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

#include <direct/debug.h>
#include <direct/messages.h>
#include <direct/util.h>

#include <fusion/build.h>
#include <fusion/types.h>
#include <fusion/lock.h>
#include <fusion/shmalloc.h>

#include "fusion_internal.h"


#if FUSION_BUILD_MULTI

D_LOG_DOMAIN( Fusion_Skirmish, "Fusion/Skirmish", "Fusion's Skirmish (Mutex)" );


#if FUSION_BUILD_KERNEL

DirectResult
fusion_skirmish_init( FusionSkirmish    *skirmish,
                      const char        *name,
                      const FusionWorld *world )
{
     FusionEntryInfo info;

     D_ASSERT( skirmish != NULL );
     D_ASSERT( name != NULL );
     D_MAGIC_ASSERT( world, FusionWorld );

     D_DEBUG_AT( Fusion_Skirmish, "fusion_skirmish_init( %p, '%s' )\n", skirmish, name ? : "" );

//     direct_trace_print_stack( NULL );

     while (ioctl( world->fusion_fd, FUSION_SKIRMISH_NEW, &skirmish->multi.id )) {
          if (errno == EINTR)
               continue;

          D_PERROR( "FUSION_SKIRMISH_NEW" );
          return DR_FUSION;
     }

     D_DEBUG_AT( Fusion_Skirmish, "  -> new skirmish %p [%d]\n", skirmish, skirmish->multi.id );

     info.type = FT_SKIRMISH;
     info.id   = skirmish->multi.id;

     direct_snputs( info.name, name, sizeof(info.name) );

     ioctl( world->fusion_fd, FUSION_ENTRY_SET_INFO, &info );

     /* Keep back pointer to shared world data. */
     skirmish->multi.shared = world->shared;

     D_MAGIC_SET( &skirmish->multi, FusionSkirmish );

     return DR_OK;
}

DirectResult
fusion_skirmish_prevail( FusionSkirmish *skirmish )
{
     D_MAGIC_ASSERT( &skirmish->multi, FusionSkirmish );

     while (ioctl (_fusion_fd( skirmish->multi.shared ), FUSION_SKIRMISH_PREVAIL, &skirmish->multi.id)) {
          switch (errno) {
               case EINTR:
                    continue;

               case EINVAL:
                    D_ERROR ("Fusion/Lock: invalid skirmish\n");
                    return DR_DESTROYED;
          }

          D_PERROR ("FUSION_SKIRMISH_PREVAIL");
          return DR_FUSION;
     }

     return DR_OK;
}

DirectResult
fusion_skirmish_swoop( FusionSkirmish *skirmish )
{
     D_MAGIC_ASSERT( &skirmish->multi, FusionSkirmish );

     while (ioctl (_fusion_fd( skirmish->multi.shared ), FUSION_SKIRMISH_SWOOP, &skirmish->multi.id)) {
          switch (errno) {
               case EINTR:
                    continue;

               case EAGAIN:
                    return DR_BUSY;

               case EINVAL:
                    D_ERROR ("Fusion/Lock: invalid skirmish\n");
                    return DR_DESTROYED;
          }

          D_PERROR ("FUSION_SKIRMISH_SWOOP");
          return DR_FUSION;
     }

     return DR_OK;
}

DirectResult
fusion_skirmish_lock_count( FusionSkirmish *skirmish, int *lock_count )
{
     int data[2];

     D_MAGIC_ASSERT( &skirmish->multi, FusionSkirmish );

     data[0] = skirmish->multi.id;
     data[1] = 0;

     while (ioctl (_fusion_fd( skirmish->multi.shared ), FUSION_SKIRMISH_LOCK_COUNT, data)) {
           switch (errno) {
               case EINTR:
                    continue;

               case EINVAL:
                    D_ERROR ("Fusion/Lock: invalid skirmish\n");
                    return DR_DESTROYED;
           }

          D_PERROR ("FUSION_SKIRMISH_LOCK_COUNT");
          return DR_FUSION;
     }

     *lock_count = data[1];
     return DR_OK;
}

DirectResult
fusion_skirmish_dismiss (FusionSkirmish *skirmish)
{
     D_MAGIC_ASSERT( &skirmish->multi, FusionSkirmish );

     while (ioctl (_fusion_fd( skirmish->multi.shared ), FUSION_SKIRMISH_DISMISS, &skirmish->multi.id)) {
          switch (errno) {
               case EINTR:
                    continue;

               case EINVAL:
                    D_ERROR ("Fusion/Lock: invalid skirmish\n");
                    return DR_DESTROYED;
          }

          D_PERROR ("FUSION_SKIRMISH_DISMISS");
          return DR_FUSION;
     }

     return DR_OK;
}

DirectResult
fusion_skirmish_destroy (FusionSkirmish *skirmish)
{
     D_MAGIC_ASSERT( &skirmish->multi, FusionSkirmish );

     D_DEBUG_AT( Fusion_Skirmish, "fusion_skirmish_destroy( %p [%d] )\n", skirmish, skirmish->multi.id );

     while (ioctl( _fusion_fd( skirmish->multi.shared ), FUSION_SKIRMISH_DESTROY, &skirmish->multi.id )) {
          switch (errno) {
               case EINTR:
                    continue;

               case EINVAL:
                    D_ERROR ("Fusion/Lock: invalid skirmish\n");
                    return DR_DESTROYED;
          }

          D_PERROR ("FUSION_SKIRMISH_DESTROY");
          return DR_FUSION;
     }

     D_MAGIC_CLEAR( &skirmish->multi );

     return DR_OK;
}

DirectResult
fusion_skirmish_wait( FusionSkirmish *skirmish, unsigned int timeout )
{
     FusionSkirmishWait wait;

     D_MAGIC_ASSERT( &skirmish->multi, FusionSkirmish );

     wait.id         = skirmish->multi.id;
     wait.timeout    = timeout;
     wait.lock_count = 0;

     while (ioctl (_fusion_fd( skirmish->multi.shared ), FUSION_SKIRMISH_WAIT, &wait)) {
          switch (errno) {
               case EINTR:
                    continue;

               case ETIMEDOUT:
                    return DR_TIMEOUT;

               case EINVAL:
                    D_ERROR ("Fusion/Lock: invalid skirmish\n");
                    return DR_DESTROYED;
          }

          D_PERROR ("FUSION_SKIRMISH_WAIT");
          return DR_FUSION;
     }

     return DR_OK;
}

DirectResult
fusion_skirmish_notify( FusionSkirmish *skirmish )
{
     D_MAGIC_ASSERT( &skirmish->multi, FusionSkirmish );

     while (ioctl (_fusion_fd( skirmish->multi.shared ), FUSION_SKIRMISH_NOTIFY, &skirmish->multi.id)) {
          switch (errno) {
               case EINTR:
                    continue;

               case EINVAL:
                    D_ERROR ("Fusion/Lock: invalid skirmish\n");
                    return DR_DESTROYED;
          }

          D_PERROR ("FUSION_SKIRMISH_NOTIFY");
          return DR_FUSION;
     }

     return DR_OK;
}

DirectResult
fusion_skirmish_set_name( FusionSkirmish *skirmish,
                          const char     *name )
{
     FusionEntryInfo info;

     D_MAGIC_ASSERT( &skirmish->multi, FusionSkirmish );
     D_ASSERT( name != NULL );

     D_DEBUG_AT( Fusion_Skirmish, "%s( %p, '%s' )\n", __FUNCTION__, skirmish, name );

     /* Initialize reactor info. */
     info.type = FT_SKIRMISH;
     info.id   = skirmish->multi.id;

     /* Put reactor name into info. */
     direct_snputs( info.name, name, sizeof(info.name) );

     /* Set the reactor info. */
     while (ioctl( _fusion_fd( skirmish->multi.shared ), FUSION_ENTRY_SET_INFO, &info )) {
          switch (errno) {
               case EINTR:
                    continue;

               case EINVAL:
                    D_ERROR( "Fusion/Skirmish: invalid skirmish\n" );
                    return DR_IDNOTFOUND;
          }

          D_PERROR( "FUSION_ENTRY_SET_INFO( skirmish 0x%08x, '%s' )\n", skirmish->multi.id, name );
          return DR_FUSION;
     }

     return DR_OK;
}

#else /* FUSION_BUILD_KERNEL */

#include <direct/clock.h>
#include <direct/list.h>
#include <direct/system.h>

typedef struct {
     DirectLink  link;

     pid_t       pid;
     bool        notified;
} WaitNode;


DirectResult
fusion_skirmish_init( FusionSkirmish    *skirmish,
                      const char        *name,
                      const FusionWorld *world )
{
     D_ASSERT( skirmish != NULL );
     //D_ASSERT( name != NULL );
     D_MAGIC_ASSERT( world, FusionWorld );

     D_DEBUG_AT( Fusion_Skirmish, "fusion_skirmish_init( %p, '%s' )\n",
                 skirmish, name ? : "" );

     skirmish->multi.id = ++world->shared->lock_ids;

     /* Set state to unlocked. */
     skirmish->multi.builtin.locked = 0;
     skirmish->multi.builtin.owner  = 0;

     skirmish->multi.builtin.waiting = NULL;

     skirmish->multi.builtin.requested = false;
     skirmish->multi.builtin.destroyed = false;

     /* Keep back pointer to shared world data. */
     skirmish->multi.shared = world->shared;

     return DR_OK;
}

DirectResult
fusion_skirmish_prevail( FusionSkirmish *skirmish )
{
     D_ASSERT( skirmish != NULL );

     if (skirmish->multi.builtin.destroyed)
          return DR_DESTROYED;

     asm( "" ::: "memory" );

     if (skirmish->multi.builtin.locked &&
         skirmish->multi.builtin.owner != direct_gettid())
     {
          int count = 0;

          while (skirmish->multi.builtin.locked) {
               /* Check whether owner exited without unlocking. */
               if (kill( skirmish->multi.builtin.owner, 0 ) < 0 && errno == ESRCH) {
                    skirmish->multi.builtin.locked = 0;
                    skirmish->multi.builtin.requested = false;
                    break;
               }

               skirmish->multi.builtin.requested = true;

               asm( "" ::: "memory" );

               if (++count > 1000) {
                    usleep( 10000 );
                    count = 0;
               }
               else {
                    direct_sched_yield();
               }

               if (skirmish->multi.builtin.destroyed)
                    return DR_DESTROYED;
          }
     }

     skirmish->multi.builtin.locked++;
     skirmish->multi.builtin.owner = direct_gettid();

     asm( "" ::: "memory" );

     return DR_OK;
}

DirectResult
fusion_skirmish_swoop( FusionSkirmish *skirmish )
{
     D_ASSERT( skirmish != NULL );

     if (skirmish->multi.builtin.destroyed)
          return DR_DESTROYED;

     asm( "" ::: "memory" );

     if (skirmish->multi.builtin.locked &&
         skirmish->multi.builtin.owner != direct_gettid()) {
          /* Check whether owner exited without unlocking. */
          if (kill( skirmish->multi.builtin.owner, 0 ) < 0 && errno == ESRCH) {
               skirmish->multi.builtin.locked = 0;
               skirmish->multi.builtin.requested = false;
          }
          else
               return DR_BUSY;
     }

     skirmish->multi.builtin.locked++;
     skirmish->multi.builtin.owner = direct_gettid();

     asm( "" ::: "memory" );

     return DR_OK;
}

DirectResult
fusion_skirmish_lock_count( FusionSkirmish *skirmish, int *lock_count )
{
     D_ASSERT( skirmish != NULL );

     if (skirmish->multi.builtin.destroyed) {
          *lock_count = 0;
          return DR_DESTROYED;
     }

     *lock_count = skirmish->multi.builtin.locked;

     return DR_OK;
}

DirectResult
fusion_skirmish_dismiss (FusionSkirmish *skirmish)
{
     D_ASSERT( skirmish != NULL );

     if (skirmish->multi.builtin.destroyed)
          return DR_DESTROYED;

     asm( "" ::: "memory" );

     if (skirmish->multi.builtin.locked) {
          if (skirmish->multi.builtin.owner != direct_gettid()) {
               D_ERROR( "Fusion/Skirmish: "
                        "Tried to dismiss a skirmish not owned by current process!\n" );
               return DR_ACCESSDENIED;
          }

          if (--skirmish->multi.builtin.locked == 0) {
               skirmish->multi.builtin.owner = 0;

               if (skirmish->multi.builtin.requested) {
                    skirmish->multi.builtin.requested = false;
                    direct_sched_yield();
               }
          }
     }

     asm( "" ::: "memory" );

     return DR_OK;
}

DirectResult
fusion_skirmish_destroy (FusionSkirmish *skirmish)
{
     D_ASSERT( skirmish != NULL );

     D_DEBUG_AT( Fusion_Skirmish, "fusion_skirmish_destroy( %p )\n", skirmish );

     if (skirmish->multi.builtin.destroyed)
          return DR_DESTROYED;

     if (skirmish->multi.builtin.waiting)
          fusion_skirmish_notify( skirmish );

     skirmish->multi.builtin.destroyed = true;

     return DR_OK;
}

#ifdef SIGRTMAX
# define SIGRESTART  SIGRTMAX
#else
# define SIGRESTART  SIGCONT
#endif

static void restart_handler( int s ) {}

DirectResult
fusion_skirmish_wait( FusionSkirmish *skirmish, unsigned int timeout )
{
     WaitNode         *node;
     long long         stop;
     struct sigaction  act, oldact;
     sigset_t          mask, set;
     DirectResult      ret = DR_OK;

     D_ASSERT( skirmish != NULL );

     if (skirmish->multi.builtin.destroyed)
          return DR_DESTROYED;

     /* Set timeout. */
     stop = direct_clock_get_micros() + timeout * 1000ll;

     /* Add ourself to the list of waiting processes. */
     node = SHMALLOC( skirmish->multi.shared->main_pool, sizeof(WaitNode) );
     if (!node)
          return D_OOSHM();

     node->pid      = direct_gettid();
     node->notified = false;

     direct_list_append( &skirmish->multi.builtin.waiting, &node->link );

     /* Install a (fake) signal handler for SIGRESTART. */
     act.sa_handler = restart_handler;
     act.sa_flags   = SA_RESETHAND | SA_RESTART | SA_NOMASK;

     sigaction( SIGRESTART, &act, &oldact );

     /* Unblock SIGRESTART. */
     sigprocmask( SIG_SETMASK, NULL, &mask );
     sigdelset( &mask, SIGRESTART );

     fusion_skirmish_dismiss( skirmish );

     while (!node->notified) {
          if (timeout) {
               long long now = direct_clock_get_micros();

               if (now >= stop) {
                    /* Stop notifying us. */
                    node->notified = true;
                    ret = DR_TIMEOUT;
                    break;
               }

               sigprocmask( SIG_SETMASK, &mask, &set );
               usleep( stop - now );
               sigprocmask( SIG_SETMASK, &set, NULL );
          }
          else {
               sigsuspend( &mask );
          }
     }

     /* Flush pending signals. */
     if (!sigpending( &set ) && sigismember( &set, SIGRESTART ) > 0)
          sigsuspend( &mask );

     if (fusion_skirmish_prevail( skirmish ))
          ret = DR_DESTROYED;

     direct_list_remove( &skirmish->multi.builtin.waiting, &node->link );

     SHFREE( skirmish->multi.shared->main_pool, node );

     sigaction( SIGRESTART, &oldact, NULL );

     return ret;
}

DirectResult
fusion_skirmish_notify( FusionSkirmish *skirmish )
{
     WaitNode *node, *temp;

     D_ASSERT( skirmish != NULL );

     if (skirmish->multi.builtin.destroyed)
          return DR_DESTROYED;

     direct_list_foreach_safe (node, temp, skirmish->multi.builtin.waiting) {
          if (node->notified)
               continue;

          node->notified = true;

          if (kill( node->pid, SIGRESTART ) < 0) {
               if (errno == ESRCH) {
                    /* Remove dead process. */
                    direct_list_remove( &skirmish->multi.builtin.waiting, &node->link );
                    SHFREE( skirmish->multi.shared->main_pool, node );
               }
               else {
                    D_PERROR( "Fusion/Skirmish: Couldn't send notification signal!\n" );
               }
          }
     }

     return DR_OK;
}

DirectResult
fusion_skirmish_set_name( FusionSkirmish *skirmish,
                          const char     *name )
{
     return DR_UNIMPLEMENTED;
}

#endif /* FUSION_BUILD_KERNEL */

#else  /* FUSION_BUILD_MULTI */

DirectResult
fusion_skirmish_init( FusionSkirmish    *skirmish,
                      const char        *name,
                      const FusionWorld *world )
{
     D_ASSERT( skirmish != NULL );

     direct_recursive_mutex_init( &skirmish->single.lock );
     direct_waitqueue_init( &skirmish->single.cond );

     return DR_OK;
}

DirectResult
fusion_skirmish_prevail (FusionSkirmish *skirmish)
{
     DirectResult ret;

     D_ASSERT( skirmish != NULL );

     ret = direct_mutex_lock( &skirmish->single.lock );
     if (ret)
          return ret;

     skirmish->single.count++;

     return DR_OK;
}

DirectResult
fusion_skirmish_swoop (FusionSkirmish *skirmish)
{
     DirectResult ret;

     D_ASSERT( skirmish != NULL );

     ret = direct_mutex_trylock( &skirmish->single.lock );
     if (ret)
          return ret;

     skirmish->single.count++;

     return DR_OK;
}

DirectResult
fusion_skirmish_lock_count( FusionSkirmish *skirmish, int *lock_count )
{
     DirectResult ret;

     D_ASSERT( skirmish != NULL );
     D_ASSERT( lock_count != NULL );

     ret = direct_mutex_trylock( &skirmish->single.lock );
     if (ret) {
          *lock_count = 0;
          return ret;
     }

     *lock_count = skirmish->single.count;

     direct_mutex_unlock( &skirmish->single.lock );

     return DR_OK;
}

DirectResult
fusion_skirmish_dismiss (FusionSkirmish *skirmish)
{
     D_ASSERT( skirmish != NULL );

     skirmish->single.count--;

     return direct_mutex_unlock( &skirmish->single.lock );
}

DirectResult
fusion_skirmish_destroy (FusionSkirmish *skirmish)
{
     D_ASSERT( skirmish != NULL );

     direct_waitqueue_broadcast( &skirmish->single.cond );
     direct_waitqueue_deinit( &skirmish->single.cond );

     return direct_mutex_deinit( &skirmish->single.lock );
}

DirectResult
fusion_skirmish_wait( FusionSkirmish *skirmish, unsigned int timeout )
{
     D_ASSERT( skirmish != NULL );

     if (timeout)
          return direct_waitqueue_wait_timeout( &skirmish->single.cond, &skirmish->single.lock, timeout * 1000 );

     return direct_waitqueue_wait( &skirmish->single.cond, &skirmish->single.lock );
}

DirectResult
fusion_skirmish_notify( FusionSkirmish *skirmish )
{
     D_ASSERT( skirmish != NULL );

     return direct_waitqueue_broadcast( &skirmish->single.cond );
}

DirectResult
fusion_skirmish_set_name( FusionSkirmish *skirmish,
                          const char     *name )
{
     return DR_UNIMPLEMENTED;
}

#endif

