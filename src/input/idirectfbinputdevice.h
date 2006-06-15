/*
   (c) Copyright 2000-2002  convergence integrated media GmbH.
   (c) Copyright 2002       convergence GmbH.

   All rights reserved.

   Written by Denis Oliver Kropp <dok@directfb.org>,
              Andreas Hundt <andi@fischlustig.de> and
              Sven Neumann <sven@convergence.de>.

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
/*
 * (c) Copyright 2004-2006 Mitsubishi Electric Corp.
 *
 * All rights reserved.
 *
 * Written by Koichi Hiramatsu,
 *            Seishi Takahashi,
 *            Atsushi Hori
 */

#ifndef __IDIRECTFBINPUTDEVICE_H__
#define __IDIRECTFBINPUTDEVICE_H__

#include <core/input.h>

#ifdef DFB_ARIB	/* idirectfbinputdevice.c -> idirectfbinputdevice.h */
#include <fusion/reactor.h>
#include <directfb.h>
#include <core/coredefs.h>
#include <core/coretypes.h>
#include <core/input.h>

/*
 * private data struct of IDirectFBInputDevice
 */
typedef struct {
     int                         ref;               /* reference counter */
     CoreInputDevice            *device;            /* pointer to input core
                                                       device struct*/

     int                         axis[DIAI_LAST+1]; /* position of all axes */
     DFBInputDeviceKeyState      keystates[DIKI_NUMBER_OF_KEYS];
                                                    /* state of all keys */
     DFBInputDeviceModifierMask  modifiers;         /* bitmask reflecting the
                                                       state of the modifier
                                                       keys */
     DFBInputDeviceLockState     locks;             /* bitmask reflecting the
						       state of the key locks */
     DFBInputDeviceButtonMask    buttonmask;        /* bitmask reflecting the
                                                       state of the buttons */

     DFBInputDeviceDescription   desc;              /* device description */

     Reaction                    reaction;
} IDirectFBInputDevice_data;
#endif

/*
 * initializes input device, adds it to input listeners and initializes mutexes
 */
DFBResult IDirectFBInputDevice_Construct( IDirectFBInputDevice *thiz,
                                          CoreInputDevice      *device );

#endif
