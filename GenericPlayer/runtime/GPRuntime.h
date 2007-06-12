/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is mozilla.org code.
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 1998
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */
 
#ifndef GPRuntime_h
#define GPRuntime_h

#ifdef __cplusplus
extern "C" {
#endif

#ifndef FALSE
#define FALSE (0)
#endif
#ifndef TRUE
#define TRUE (1)
#endif
#ifndef NULL
#define NULL (0L)
#endif

typedef unsigned char    GPBool;
typedef short            GPError;
typedef short            GPReason;

#if !defined(__cplusplus) && !defined(__bool_true_false_are_defined)
#ifndef false
#define false (0)
#endif
#ifndef true
#define true (!false)
#endif
typedef unsigned char bool;
#endif

typedef struct _GPType  GPType;
/*
 * A function of this name with the signature GPLibraryFunction(GPType*, void* data)
 * must be implemented in the library to register and handle runtime calles
 */
extern void GPLibraryFunction(GPType*, void* data);
/**
 * Bootstrap function
 */
void GPRuntimeFunction(GPType*, void* data);

#ifdef __cplusplus
}  /* end extern "C" */
#endif

#include "GPType.h"
#include "GPLibrary.h"
#include "GPPlugin.h"

#endif /*GPRuntime_h*/
