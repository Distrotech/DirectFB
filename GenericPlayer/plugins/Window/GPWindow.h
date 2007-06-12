/*
 * Copyright (C) 2004, 2006 Apple Computer, Inc.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE COMPUTER, INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE COMPUTER, INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 */

#ifndef GPWindow_h
#define GPWindow_h
#include "GPRuntime.h"
#include "plugins/Window/GPWindowDOM.h"

#ifdef __cplusplus
extern "C" {
#endif

#define GPWindowDef "\
\
gp.GPWindow.Plugin(gp.GPPlugin)\
\
    gp.GPWindow.Plugin.PluginFunc({gp.GPWindow.Plugin}\
            gp.GPPlugin/GPPluginFunc))\
\
    gp.GPWindow.Plugin.DestroyFunc({gp.GPWindow.Plugin}\
            gp.GPWindow.Plugin.DestroyFunc.plugin(gp.GPWindow.Plugin*))\
\
gp.GPWindow.Plugin.RegisterDOMPluginFunc({gp.GPWindow.Plugin}\
    gp.GPWindow.Plugin.RegisterDOMPluginFunc.plugin(gp.GPWindow.Plugin*)\
    gp.GPWindow.Plugin.RegisterDOMPluginFunc.domPlugin(gp.GPPlugin*))\
\
gp.GPWindow.Plugin.BindWindowElementFunc({gp.GPWindow.Plugin}\
    gp.GPWindow.Plugin.BindWindowElementFunc.plugin(gp.GPWindow.Plugin*)\
    gp.GPWindow.Plugin.BindWindowElementFunc.domWindow(gp.GPWindowDOM.Window*))\
\
gp.GPWindow.Plugin.ProcessEventFunc({gp.GPWindow.Plugin}\
    gp.GPWindow.Plugin.ProcessEventFunc.plugin(gp.GPWindow.Plugin*))\
\
"
    
#define GPWINDOW_MIMETYPE "image/window"

typedef struct { 
    GPPlugin super;
} gp_GPWindow_Plugin;


typedef struct { 
    gp_GPWindow_Plugin* plugin;
    GPPlugin* domPlugin;
} gp_GPWindow_RegisterDOMPluginFunc;

typedef struct { 
    gp_GPWindow_Plugin* plugin;
} gp_GPWindow_Plugin_ProcessEventFunc;


typedef struct { 
    gp_GPWindow_Plugin* plugin;
    /*initalize a pre allocated window*/
    gp_GPWindowDOM_Window* domWindow;
} gp_GPWindow_Plugin_BindWindowElementFunc;

static inline void gp_GPWindow_registerDOMPlugin(gp_GPWindow_Plugin* plugin,GPPlugin* domPlugin)
{
    static GPType* func;
    gp_GPWindow_RegisterDOMPluginFunc data = {plugin:plugin,domPlugin:domPlugin};
    if(!func)
        func = GPType_get("gp.GPWindow.Plugin.RegisterDOMPluginFunc");
    func->function(func,&data);
}

static inline void gp_GPWindow_bindWindowElement(gp_GPWindow_Plugin* plugin,gp_GPWindowDOM_Window* domWindow)
{
    static GPType* func;
    gp_GPWindow_Plugin_BindWindowElementFunc data = {plugin:plugin,domWindow:domWindow};
    if(!func)
        func = GPType_get("gp.GPWindow.Plugin.BindWindowElementFunc");
    func->function(func,&data);
}

static inline void gp_GPWindow_processsEvent(gp_GPWindow_Plugin* plugin)
{
    static GPType* func;
    gp_GPWindow_Plugin_ProcessEventFunc data = {plugin:plugin};
    if(!func)
        func = GPType_get("gp.GPWindow.Plugin.ProcessEventFunc");
    func->function(func,&data);
}

#ifdef __cplusplus
}  /* end extern "C" */
#endif

#endif /*GPWindow_h*/
