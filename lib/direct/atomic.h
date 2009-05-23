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

#ifndef __DIRECT__ATOMIC_H__
#define __DIRECT__ATOMIC_H__

#include <direct/types.h>

#if defined (__SH4__) || defined (__SH4A__)

/*
 * SH4 Atomic Operations
 */

#define D_SYNC_BOOL_COMPARE_AND_SWAP( ptr, old_value, new_value )               \
     ({                                                                         \
          typeof(*(ptr)) __temp;                                                \
          typeof(*(ptr)) __result;                                              \
                                                                                \
          __asm__ __volatile__ (                                                \
               "1:                                                    \n"       \
               "    movli.l        @%2, %0                            \n"       \
               "    mov            %0, %1                             \n"       \
               "    cmp/eq         %1, %3                             \n"       \
               "    bf             2f                                 \n"       \
               "    mov            %4, %0                             \n"       \
               "2:                                                    \n"       \
               "    movco.l        %0, @%2                            \n"       \
               "    bf             1b                                 \n"       \
               "    synco                                             \n"       \
               : "=&z" (__temp), "=&r" (__result)                               \
               : "r" (ptr), "r" (old_value), "r" (new_value)                    \
               : "t"                                                            \
          );                                                                    \
                                                                                \
          __result == (old_value);                                              \
     })


#define D_SYNC_ADD_AND_FETCH( ptr, value )                                      \
     ({                                                                         \
          typeof(*(ptr)) __result;                                              \
                                                                                \
          __asm__ __volatile__ (                                                \
               "1:                                                    \n"       \
               "    movli.l        @%1, %0                            \n"       \
               "    add            %2, %0                             \n"       \
               "    movco.l        %0, @%1                            \n"       \
               "    bf             1b                                 \n"       \
               "    synco                                             \n"       \
               : "=&z" (__result)                                               \
               : "r" (ptr), "r" (value)                                         \
               : "t"                                                            \
          );                                                                    \
                                                                                \
          __result;                                                             \
     })


#define D_SYNC_FETCH_AND_CLEAR( ptr )                                           \
     ({                                                                         \
          typeof(*(ptr)) __temp;                                                \
          typeof(*(ptr)) __result;                                              \
                                                                                \
          __asm__ __volatile__ (                                                \
               "1:                                                    \n"       \
               "    movli.l        @%2, %0                            \n"       \
               "    mov            %0, %1                             \n"       \
               "    xor            %0, %0                             \n"       \
               "    movco.l        %0, @%2                            \n"       \
               "    bf             1b                                 \n"       \
               "    synco                                             \n"       \
               : "=&z" (__temp), "=&r" (__result)                               \
               : "r" (ptr)                                                      \
               : "t"                                                            \
          );                                                                    \
                                                                                \
          __result;                                                             \
     })


#define D_SYNC_ADD( ptr, value )                                                \
     do {                                                                       \
          typeof(*(ptr)) __temp;                                                \
                                                                                \
          __asm__ __volatile__ (                                                \
               "1:                                                    \n"       \
               "    movli.l        @%1, %0                            \n"       \
               "    add            %2, %0                             \n"       \
               "    movco.l        %0, @%1                            \n"       \
               "    bf             1b                                 \n"       \
               : "=&z" (__temp)                                                 \
               : "r" (ptr), "r" (value)                                         \
               : "t"                                                            \
          );                                                                    \
     } while (0)

/*
 * FIFO Push
 *
 *   *iptr = *fptr
 *   *fptr =  iptr
 */
#define D_SYNC_PUSH_SINGLE( fptr, iptr )                                        \
     do {                                                                       \
          unsigned long **__fptr = (void*)(fptr);                               \
          unsigned long **__iptr = (void*)(iptr);                               \
          unsigned long  *__temp;                                               \
                                                                                \
          __asm__ __volatile__ (                                                \
               "1:                                                    \n"       \
               "    movli.l        @%1, %0                            \n"       \
               "    mov.l          %0, @%2                            \n"       \
               "    mov            %2, %0                             \n"       \
               "    movco.l        %0, @%1                            \n"       \
               "    bf             1b                                 \n"       \
               "    synco                                             \n"       \
               : "=&z" (__temp)                                                 \
               : "r" (__fptr), "r" (__iptr)                                     \
               : "t"                                                            \
          );                                                                    \
     } while (0)

/*
 * FIFO Pop
 *
 *    iptr = *fptr
 *   *fptr = *iptr  <- if iptr != NULL
 *
 *   return iptr
 */
#define D_SYNC_POP_SINGLE( fptr )                                               \
     ({                                                                         \
          unsigned long **__fptr = (void*)(fptr);                               \
          unsigned long **__iptr;                                               \
          unsigned long  *__temp;                                               \
                                                                                \
          __asm__ __volatile__ (                                                \
               "1:                                                    \n"       \
               "    movli.l        @%2, %0                            \n"       \
               "    mov            %0, %1                             \n"       \
               "    cmp/eq         #0, %0                             \n"       \
               "    bt             2f                                 \n"       \
               "    mov.l          @%1, %0                            \n"       \
               "2:                                                    \n"       \
               "    movco.l        %0, @%2                            \n"       \
               "    bf             1b                                 \n"       \
               "    synco                                             \n"       \
               : "=&z" (__temp), "=&r" (__iptr)                                 \
               : "r" (__fptr)                                                   \
               : "t"                                                            \
          );                                                                    \
                                                                                \
          (typeof(*(fptr))) __iptr;                                             \
     })


#endif




/*
 * GCC Atomic Builtins
 */

#ifndef D_SYNC_BOOL_COMPARE_AND_SWAP
#define D_SYNC_BOOL_COMPARE_AND_SWAP( ptr, old_value, new_value )     \
     __sync_bool_compare_and_swap( ptr, old_value, new_value )
#endif

#ifndef D_SYNC_FETCH_AND_CLEAR
#define D_SYNC_FETCH_AND_CLEAR( ptr )                                 \
     __sync_fetch_and_and( ptr, 0 )
#endif

#ifndef D_SYNC_ADD_AND_FETCH
#define D_SYNC_ADD_AND_FETCH( ptr, value )                            \
     __sync_add_and_fetch( ptr, value )
#endif

#ifndef D_SYNC_ADD
#define D_SYNC_ADD( ptr, value )                                      \
     do { (void) D_SYNC_ADD_AND_FETCH( ptr, value ); } while (0)
#endif


/*
 * FIFO Push
 *
 *   *iptr = *fptr
 *   *fptr =  iptr
 */

#ifndef D_SYNC_PUSH_SINGLE
#define D_SYNC_PUSH_SINGLE( fptr, iptr )                                        \
     do {                                                                       \
          unsigned long **__fptr = (unsigned long **)(void*)(fptr);             \
          unsigned long **__iptr = (unsigned long **)(void*)(iptr);             \
                                                                                \
          do {                                                                  \
               *__iptr = *__fptr;                                               \
          } while (!D_SYNC_BOOL_COMPARE_AND_SWAP( __fptr, *__iptr, __iptr ));   \
     } while (0)
#endif

#ifndef D_SYNC_PUSH_MULTI
#define D_SYNC_PUSH_MULTI( fptr, iptr )                                         \
     do {                                                                       \
          unsigned long **__fptr = (unsigned long **)(void*)(fptr);             \
          unsigned long **__iptr = (unsigned long **)(void*)(iptr);             \
          volatile unsigned int   n = 1;                                        \
          unsigned int            r = 0;                                        \
                                                                                \
          while (true) {                                                        \
               *__iptr = *__fptr;                                               \
                                                                                \
               if (D_SYNC_BOOL_COMPARE_AND_SWAP( __fptr, *__iptr, __iptr ))     \
                    break;                                                      \
                                                                                \
               r = ((r + n) & 0x7f);                                            \
                                                                                \
               for (n=0; n<r; n++);                                             \
          }                                                                     \
     } while (0)
#endif


/*
 * FIFO Pop
 *
 *    iptr = *fptr
 *   *fptr = *iptr
 *
 *   return iptr
 */

#ifndef D_SYNC_POP_SINGLE
#define D_SYNC_POP_SINGLE( fptr )                                                         \
     ({                                                                                   \
          unsigned long **__fptr = (unsigned long**)(void*)(fptr);                        \
          unsigned long  *__iptr;                                                         \
                                                                                          \
          do {                                                                            \
               __iptr = *__fptr;                                                          \
          } while (__iptr && !D_SYNC_BOOL_COMPARE_AND_SWAP( __fptr, __iptr, *__iptr ));   \
                                                                                          \
          (typeof(*(fptr))) __iptr;                                                       \
     })
#endif

#ifndef D_SYNC_POP_MULTI
#define D_SYNC_POP_MULTI( fptr )                                                \
     ({                                                                         \
          unsigned long         **__fptr = (unsigned long**)(void*)(fptr);      \
          unsigned long          *__iptr;                                       \
          volatile unsigned int  n = 1;                                         \
          unsigned int           r = 0;                                         \
                                                                                \
          while (true) {                                                        \
               __iptr = *__fptr;                                                \
                                                                                \
               if (D_SYNC_BOOL_COMPARE_AND_SWAP( __fptr, __iptr, *__iptr ))     \
                    break;                                                      \
                                                                                \
               r = ((r + n) & 0x7f);                                            \
                                                                                \
               for (n=0; n<r; n++);                                             \
          }                                                                     \
                                                                                \
          (typeof(*(fptr))) __iptr;                                             \
     })
#endif


#ifndef D_SYNC_PUSH
#ifdef DIRECT_BUILD_MULTICORE
#define D_SYNC_PUSH D_SYNC_PUSH_MULTI
#else
#define D_SYNC_PUSH D_SYNC_PUSH_SINGLE
#endif
#endif

#ifndef D_SYNC_POP
#ifdef DIRECT_BUILD_MULTICORE
#define D_SYNC_POP D_SYNC_POP_MULTI
#else
#define D_SYNC_POP D_SYNC_POP_SINGLE
#endif
#endif


#endif

