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
 
 
 /*
  *  Generic plugin sample plugin 
  */
#ifndef GPLibrary_h
#define GPLibrary_h
#include <assert.h>
#include <stdio.h>
#include "GPType.h"

#ifdef __cplusplus
extern "C" {
#endif


#define GPLibraryRegisterFuncDef "gp.GPLibrary.RegisterFunc( \
    gp.GPLibrary.RegisterFunc.result(gp.GPLibrary*)\
    gp.GPLibrary.RegisterFunc.mimeType(uchar*) \
    gp.GPLibrary.RegisterFunc.filename(uchar*))"

typedef struct { 
    const char* mimeType;
    const char* filename;
} gp_GPLibrary_RegisterFunc;

static inline void GPLibrary_register(const char* mimeType, const char* filename)
{
    static GPType* func;
    gp_GPLibrary_RegisterFunc data = { mimeType:mimeType, filename:filename};
    if(!func) 
        func = GPType_get("gp.GPLibrary.RegisterFunc");
    func->function(func,&data);
}

#define GPLibraryOpenFuncDef "gp.GPLibrary.OpenFunc( \
    gp.GPLibrary.OpenFunc.result(uchar*) \
    gp.GPLibrary.OpenFunc.mimeType(uchar*))"

typedef struct { 
    const char* result;
    const char* mimeType;
} gp_GPLibrary_OpenFunc;

static inline const char* GPLibrary_open(const char* mimeType)
{
    static GPType* open;
    gp_GPLibrary_OpenFunc data = { result:NULL,mimeType:mimeType};
    if(!open)
        open = GPType_get("gp.GPLibrary.OpenFunc");
    open->function(open,&data);
    return data.result;
}

#define GPLibraryCloseFuncDef "gp.GPLibrary.CloseFunc( \
    gp.GPLibrary.CloseFunc.mimeType(uchar*))"

typedef struct { 
    const char* mimeType;
} gp_GPLibrary_CloseFunc;

static inline void GPLibrary_close(char* mimeType)
{
    static GPType* close;
    gp_GPLibrary_CloseFunc data = { mimeType:mimeType};
    if(!close)
        close = GPType_get("gp.GPLibrary.CloseFunc");
    close->function(close,&data);
}

#define GPLibrarySymbolFuncDef "gp.GPLibrary.SymbolFunc( \
    gp.GPLibrary.SymbolFunc.result(void*)\
    gp.GPLibrary.SymbolFunc.mimeType(uchar*)\
    gp.GPLibrary.SymbolFunc.symbol(uchar*))"

typedef struct { 
    void* result;
    const char* mimeType;
    const char* symbol;
} gp_GPLibrary_SymbolFunc;

static inline void* gp_GPLibrary_symbol(char* mimeType,const char* symbol)
{
    static GPType* sym;
    gp_GPLibrary_SymbolFunc data = {result:NULL, mimeType:mimeType, symbol:symbol};
    if(!sym)
        sym = GPType_get("gp.GPLibrary.SymbolFunc");
    sym->function(sym,&data);
}


#ifdef __cplusplus
}  /* end extern "C" */
#endif

#endif //GPLibrary_h
