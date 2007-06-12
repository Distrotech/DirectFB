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
#ifndef GPPlugin_h
#define GPPlugin_h
#include "GPType.h"
#include "GPPlugin.h"

#ifdef __cplusplus
extern "C" {
#endif

#define GPPluginDef "gp.GPPlugin( \
    gp.GPPlugin.mimeType(uchar*) \
    gp.GPPlugin.type(gp.GPType*) \
    gp.GPPlugin.function(function) \
    gp.GPPlugin.impl(void*)) \
    \
    gp.GPPlugin.RegisterFunc({gp.GPPlugin} \
    gp.GPPlugin.RegisterFunc.mimeType(uchar*) \
    gp.GPPlugin.RegisterFunc.typName(uchar*) \
    gp.GPPlugin.RegisterFunc.defs(uchar*)) \
    \
    gp.GPPlugin.GetPluginTypeFunc({gp.GPPlugin} \
    gp.GPPlugin.GetPluginTypeFunc.result(gp.GPType*) \
    gp.GPPlugin.GetPluginTypeFunc.mimeType(uchar*))\
    \
    gp.GPPlugin.GetAttributeFunc({gp.GPPlugin} \
    gp.GPPlugin.GetAttributeFunc.result(uchar*) \
    gp.GPPlugin.GetAttributeFunc.mimeType(uchar*) \
    gp.GPPlugin.GetAttributeFunc.name(uchar*)) \
    \
    gp.GPPlugin.SetAttributeFunc({gp.GPPlugin} \
    gp.GPPlugin.SetAttributeFunc.name(uchar*) \
    gp.GPPlugin.SetAttributeFunc.value(uchar*))\
    \
    gp.GPPlugin.DestroyFunc({gp.GPPlugin} \
    gp.GPPlugin.DestroyFunc.plugin(gp.GPPlugin*))\
    \
    gp.GPPlugin.GetValueFunc({gp.GPPlugin} \
    gp.GPPlugin.GetValueFunc.result(void*)\
    gp.GPPlugin.GetValueFunc.plugin(gp.GPPlugin*)\
    gp.GPPlugin.GetValueFunc.key(uchar*) \
    gp.GPPlugin.GetValueFunc.args(void*)) \
    \
    gp.GPPlugin.SetValueFunc({gp.GPPlugin} \
    gp.GPPlugin.SetValueFunc.result(void*)\
    gp.GPPlugin.SetValueFunc.plugin(gp.GPPlugin*)\
    gp.GPPlugin.SetValueFunc.key(uchar*) \
    gp.GPPlugin.SetValueFunc.value(void*)) \
    \
    gp.GPPlugin.SupportsValueFunc({gp.GPPlugin} \
    gp.GPPlugin.SupportsValueFunc.result(uint)\
    gp.GPPlugin.SupportsValueFunc.plugin(gp.GPPlugin*)\
    gp.GPPlugin.SupportsValueFunc.key(uchar*)) \
    \
    gp.GPPlugin.GPPluginFunc({gp.GPPlugin} \
    gp.GPPlugin.GPPluginFunc.result(gp.GPPlugin*) \
    gp.GPPlugin.GPPluginFunc.mimeType(uchar*) \
    gp.GPPlugin.GPPluginFunc.argCount(uint) \
    gp.GPPlugin.GPPluginFunc.argNames(uchar**) \
    gp.GPPlugin.GPPluginFunc.argValues(uchar**))"

typedef struct { 
    GPType* type;
    const char* mimeType;
    GPFunction function;
    void* impl;
} GPPlugin;


typedef struct { 
  const char* mimeType;
  const char* typeName;
  const char* defs;
} gp_GPPlugin_RegisterFunc;

static inline void GPPlugin_register(const char* mimeType,const char* typeName, const char* defs)
{
    static GPType* func;
    gp_GPPlugin_RegisterFunc data = {mimeType:mimeType,typeName:typeName,defs:defs};
    if(!func)
        func = GPType_get("gp.GPPlugin.RegisterFunc");
    func->function(func,&data);
}

typedef struct { 
  GPType* result;
  const char* mimeType;
} GPPluginGetPluginTypeFunc;


static inline GPType* GPPlugin_getPluginType(const char* mimeType)
{
    static GPType* func;
    GPPluginGetPluginTypeFunc data = {result:NULL,mimeType:mimeType};
    if(!func)
        func = GPType_get("gp.GPPlugin.GetPluginTypeFunc");
    func->function(func,&data);
    return data.result;
}

typedef struct { 
    const char* result;
    const char* mimeType;
    const char* name;
} GPPluginGetAttributeFunc;

static inline const char* GPPlugin_getAttribute(const char* mimeType, const char* name)
{
    static GPType* func;
    GPPluginGetAttributeFunc data = {result:NULL,mimeType:mimeType,name:name};
    if(!func) {
        func = GPType_get("gp.GPPlugin.GetAttributeFunc");
    }
    return data.result;
}

typedef struct { 
    const char* mimeType;
    const char* name;
    const char* value;
} GPPluginSetAttributeFunc;

static inline void GPPlugin_setAttribute(const char* mimeType, const char* name, const char* value)
{
    static GPType* func;
    GPPluginSetAttributeFunc data = {mimeType:mimeType,name:name,value:value};
    if(!func) {
        func = GPType_get("gp.GPPlugin.SetAttributeFunc");
    }
    func->function(func,&data);
}

typedef struct { 
    GPPlugin* result;
    const char* mimeType;
    unsigned int argCount;
    const char** argNames;
    const char** argValues;
} GPPluginFunc;

static inline GPPlugin* GPPlugin_plugin(const char* mimeType, unsigned int argCount, const char** argNames, const char** argValues )
{
    static GPType* func;
    GPPluginFunc data = { result:NULL,
            mimeType:mimeType,
            argCount:argCount,
            argNames:argNames,
            argValues:argValues};
    if(!func)
        func = GPType_get("gp.GPPlugin.GPPluginFunc");
    func->function(func,&data);
    return data.result;
}

typedef struct { 
    GPPlugin* plugin;
} GPPluginDestroyFunc;

static inline void GPPlugin_destroy(GPPlugin* plugin)
{
    static GPType* func;
    GPPluginDestroyFunc data = {plugin:plugin};
    if(!func)
        func = GPType_get("gp.GPPlugin.DestroyFunc");
    func->function(func,&data);
}

typedef struct { 
    void* result;
    GPPlugin* plugin;
    const char* key;
    void* args;
} GPPluginGetValueFunc;

static inline void* GPPlugin_getValue(GPPlugin* plugin,const char* key, void* args)
{
    static GPType* func;
    GPPluginGetValueFunc data = {result:NULL,plugin:plugin,key:key,args:args};
    if(!func)
        func = GPType_get("gp.GPPlugin.GetValueFunc");
    func->function(func,&data);
    return data.result;
}

typedef struct { 
    void* result;
    GPPlugin* plugin;
    const char* key;
    void* value;
} GPPluginSetValueFunc;

static inline void* GPPlugin_setValue(GPPlugin* plugin,const char* key, void* value)
{
    static GPType* func;
    GPPluginSetValueFunc data = {result:NULL, plugin:plugin, key:key, value:value};
    if(!func)
        func = GPType_get("gp.GPPlugin.SetValueFunc");
    func->function(func,&data);
    return data.result;
}

typedef struct { 
    unsigned int result;
    GPPlugin* plugin;
    const char* key;
} GPPluginSupportsValueFunc;

static inline unsigned int GPPlugin_supportsValue(GPPlugin* plugin,const char* key)
{
    static GPType* func;
    GPPluginSupportsValueFunc data = {result:0, plugin:plugin, key:key};
    if(!func)
        func = GPType_get("gp.GPPlugin.SupportsValueFunc");
    func->function(func,&data);
    return data.result;
}

#ifdef __cplusplus
}  /* end extern "C" */
#endif

#endif //GPPlugin_h
