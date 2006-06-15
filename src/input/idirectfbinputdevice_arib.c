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

#include <string.h>

#include <fusion/reactor.h>

#include <directfb_arib.h>

#include <core/coredefs.h>
#include <core/coretypes.h>

#include <core/input.h>
#include <core/input_arib.h>

#include <misc/util.h>
#include <direct/interface.h>
#include <direct/interface_arib.h>

#include <direct/mem.h>

#include "idirectfbinputdevice.h"
#include "idirectfbinputdevice_arib.h"

#include "idirectfbinputbuffer.h"


/*
 * private data struct of IDirectFBARIBInputDevice
 */
typedef struct {
	IDirectFBInputDevice_data     parent;          /* IDirectFBInputDevice_data parent */
} IDirectFBARIBInputDevice_data;



static void
IDirectFBARIBInputDevice_Destruct( IDirectFBARIBInputDevice *thiz )
{
	IDirectFBInputDevice_data *parent_data = (IDirectFBInputDevice_data *)thiz->parent.priv;

	dfb_input_detach( parent_data->device, &parent_data->reaction );

	DIRECT_DEALLOCATE_CHILD_INTERFACE( thiz );
}

static DFBResult
IDirectFBARIBInputDevice_AddRef( IDirectFBARIBInputDevice *thiz )
{
	IDirectFBInputDevice_data  *parent_data;

	DIRECT_CHILD_INTERFACE_GET_DATA(IDirectFBARIBInputDevice);

	parent_data = (IDirectFBInputDevice_data *)data;

	parent_data->ref++;

	return DFB_OK;
}

static DFBResult
IDirectFBARIBInputDevice_Release( IDirectFBARIBInputDevice *thiz )
{
	IDirectFBInputDevice_data  *parent_data;

	DIRECT_CHILD_INTERFACE_GET_DATA(IDirectFBARIBInputDevice);

	parent_data = (IDirectFBInputDevice_data *)data;

	if (--parent_data->ref == 0)
		IDirectFBARIBInputDevice_Destruct( thiz );

	return DFB_OK;
}

static DFBResult
IDirectFBARIBInputDevice_SetARIBUsedKeyList( IDirectFBARIBInputDevice   *thiz,
                                             DFBARIBInputDeviceKeyGroup  key_groups)
{
	IDirectFBInputDevice_data  *parent_data;

	DIRECT_CHILD_INTERFACE_GET_DATA(IDirectFBARIBInputDevice);

	parent_data = (IDirectFBInputDevice_data *)data;

	dfb_input_device_set_usedkeylist(parent_data->device, key_groups);

	return  DFB_OK;
}

static DFBResult
IDirectFBARIBInputDevice_GetARIBUsedKeyList( IDirectFBARIBInputDevice    *thiz,
                                             DFBARIBInputDeviceKeyGroup  *key_groups)
{
	IDirectFBInputDevice_data  *parent_data;

	DIRECT_CHILD_INTERFACE_GET_DATA(IDirectFBARIBInputDevice);

	parent_data = (IDirectFBInputDevice_data *)data;

	dfb_input_device_get_usedkeylist(parent_data->device, key_groups);

	return  DFB_OK;
}

static DFBResult
IDirectFBARIBInputDevice_SetARIBKeySuppressState( IDirectFBARIBInputDevice  *thiz,
                                                  DFBBoolean                suppress)
{
	IDirectFBInputDevice_data  *parent_data;

	DIRECT_CHILD_INTERFACE_GET_DATA(IDirectFBARIBInputDevice);

	parent_data = (IDirectFBInputDevice_data *)data;

	dfb_input_device_set_key_suppress_state(parent_data->device, suppress);

	return  DFB_OK;
}

static DFBResult
IDirectFBARIBInputDevice_GetARIBKeySuppressState( IDirectFBARIBInputDevice  *thiz,
                                                  DFBBoolean                *suppress)
{
	IDirectFBInputDevice_data  *parent_data;

	DIRECT_CHILD_INTERFACE_GET_DATA(IDirectFBARIBInputDevice);

	parent_data = (IDirectFBInputDevice_data *)data;

	dfb_input_device_get_key_suppress_state(parent_data->device, suppress);

	return  DFB_OK;
}

static DFBResult
IDirectFBARIBInputDevice_QueryARIBKeyGroup( IDirectFBARIBInputDevice   *thiz,
                                            DFBARIBInputDeviceKeyGroup *key_groups,
                                            DFBInputDeviceKeySymbol    key_symbol)
{
	IDirectFBInputDevice_data  *parent_data;

	DIRECT_CHILD_INTERFACE_GET_DATA(IDirectFBARIBInputDevice);

	parent_data = (IDirectFBInputDevice_data *)data;

	dfb_input_device_query_key_group(parent_data->device, key_groups, key_symbol);

	return DFB_OK;
}


DFBResult
IDirectFBARIBInputDevice_Construct( IDirectFBARIBInputDevice  *thiz,
                                    CoreInputDevice           *device )
{
	DFBResult  ret;

	DIRECT_ALLOCATE_CHILD_INTERFACE_DATA(thiz, IDirectFBARIBInputDevice);

	ret = IDirectFBInputDevice_Construct((IDirectFBInputDevice *)thiz, device);
	if (ret)
		return ret;

	thiz->AddRef                  = IDirectFBARIBInputDevice_AddRef;
	thiz->Release                 = IDirectFBARIBInputDevice_Release;
	thiz->SetARIBUsedKeyList      = IDirectFBARIBInputDevice_SetARIBUsedKeyList;
	thiz->GetARIBUsedKeyList      = IDirectFBARIBInputDevice_GetARIBUsedKeyList;
	thiz->SetARIBKeySuppressState = IDirectFBARIBInputDevice_SetARIBKeySuppressState;
	thiz->GetARIBKeySuppressState = IDirectFBARIBInputDevice_GetARIBKeySuppressState;
	thiz->GetARIBKeySuppressState = IDirectFBARIBInputDevice_GetARIBKeySuppressState;
	thiz->QueryARIBKeyGroup       = IDirectFBARIBInputDevice_QueryARIBKeyGroup;

	return DFB_OK;
}


