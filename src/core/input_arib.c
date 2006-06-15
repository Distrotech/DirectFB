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

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <fusion/shmalloc.h>
#include <fusion/reactor.h>
#include <fusion/arena.h>
#include <direct/list.h>

#include <directfb_arib.h>

#include <core/core.h>
#include <core/coredefs.h>
#include <core/coretypes.h>

#include <core/core_parts.h>

#include <core/input.h>
#include <direct/modules.h>

#include <fusion/build.h>


void
dfb_input_device_set_usedkeylist( CoreInputDevice            *device,
                                  DFBARIBInputDeviceKeyGroup key_groups )
{
     D_MAGIC_ASSERT( device, CoreInputDevice );

     D_ASSERT( device != NULL );
     D_ASSERT( device->shared != NULL );

     device->shared->key_groups = key_groups;
}

void
dfb_input_device_get_usedkeylist( CoreInputDevice            *device,
                                  DFBARIBInputDeviceKeyGroup *key_groups )
{
     D_MAGIC_ASSERT( device, CoreInputDevice );

     D_ASSERT( device != NULL );
     D_ASSERT( device->shared != NULL );
     D_ASSERT( key_groups != NULL );

     *key_groups = device->shared->key_groups;
}

void
dfb_input_device_set_key_suppress_state( CoreInputDevice *device,
                                         DFBBoolean      suppress )
{
     D_MAGIC_ASSERT( device, CoreInputDevice );

     D_ASSERT( device != NULL );
     D_ASSERT( device->shared != NULL );

     device->shared->suppress = suppress;
}

void
dfb_input_device_get_key_suppress_state( CoreInputDevice *device,
                                         DFBBoolean      *suppress )
{
     D_MAGIC_ASSERT( device, CoreInputDevice );

     D_ASSERT( device != NULL );
     D_ASSERT( device->shared != NULL );

     *suppress = device->shared->suppress;
}

void
dfb_input_device_query_key_group( CoreInputDevice *device,
                                  DFBARIBInputDeviceKeyGroup *key_groups,
                                  DFBInputDeviceKeySymbol    key_symbol)
{
     D_MAGIC_ASSERT( device, CoreInputDevice );

     D_ASSERT( device != NULL );
     D_ASSERT( device->shared != NULL );
     D_ASSERT( key_groups != NULL );

     switch(key_symbol)
     {
     /* BASIC */
     case DIKS_BACKSPACE:
     case DIKS_RETURN:
     case DIKS_CURSOR_LEFT:
     case DIKS_CURSOR_RIGHT:
     case DIKS_CURSOR_UP:
     case DIKS_CURSOR_DOWN:
         *key_groups = DARIBIKG_BASIC;
         break;

     /* DATA */
     case DIKS_RED:
     case DIKS_GREEN:
     case DIKS_YELLOW:
     case DIKS_BLUE:
         *key_groups = DARIBIKG_DATA_BUTTON;
         break;

     /* NUMERIC */
     case DIKS_0:
     case DIKS_1:
     case DIKS_2:
     case DIKS_3:
     case DIKS_4:
     case DIKS_5:
     case DIKS_6:
     case DIKS_7:
     case DIKS_8:
     case DIKS_9:
     case DIKS_ASTERISK:
     case DIKS_NUMBER_SIGN:
         *key_groups = DARIBIKG_NUMERIC_TUNING;
         break;

     /* OTHER */
     case DIKS_VIDEO:
     case DIKS_CHANNEL_UP:
     case DIKS_CHANNEL_DOWN:
     case DIKS_SERVICE_SWITCH:
         *key_groups = DARIBIKG_OTHER_TUNING;
         break;

     /* MISC */
     case DIKS_DATA:
     /* fallthru */
     default:
         *key_groups = DARIBIKG_MISC;
         break;
     }
}
