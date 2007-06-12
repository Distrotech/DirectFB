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

#ifndef GPDOMWindow_h
#define GPDOMWindow_h
#include "GPRuntime.h"
#include <plugins/DOM/GPDOM.h>

#ifdef __cplusplus
extern "C" {
#endif

#define GPWindowDOMDef "\
\
gp.GPWindowDOM.Plugin(\
\
gp.GPWindowDOM.Plugin.function(function)\
    gp.GPWindowDOM.Plugin.impl(void*))\
\
gp.GPWindowDOM.Plugin.PluginFunc({gp.GPWindowDOM.Plugin}\
            gp.GPPlugin/GPPluginFunc))\
\
gp.GPWindowDOM.Plugin.DestroyFunc({gp.GPWindowDOM.Plugin}\
            gp.GPWindowDOM.Plugin.DestroyFunc.plugin(gp.GPWindowDOM.Plugin*))\
\
gp.GPWindowDOM.Plugin.ProcessEventFunc({gp.GPWindowDOM.Plugin}\
    dom.Plugin.ProcessEventFunc)\
\
gp.GPWindowDOM.Plugin.GetWindowPluginFunc({gp.GPWindowDOM.Plugin}\
            gp.GPWindowDOM.Plugin.GetWindowPluginFunc.result(gp.GPWindow.Plugin*))\
            gp.GPWindowDOM.Plugin.GetWindowPluginFunc.plugin(gp.GPWindowDOM.Plugin*))\
\
gp.GPWindowDOM.DOMImplementationSource( )\
\
gp.GPWindowDOM.DOMImplementationSource.GetDOMImplementationFunc({gp.GPWindowDOM.DOMImplementationSource}\
    gp.GPWindowDOM.DOMImplementationSource.GetDOMImplementationFunc.result(dom.DOMImplementation*)\
    gp.GPWindowDOM.DOMImplementationSource.GetDOMImplementationFunc.source(dom.DOMImplementationSource*)\
    gp.GPWindowDOM.DOMImplementationSource.GetDOMImplementationFunc.features(dom.utf8*))\
\
gp.GPWindowDOM.DOMImplementationSource.GetDOMImplementationListFunc({gp.GPWindowDOM.DOMImplementationSource}\
    gp.GPWindowDOM.DOMImplementationSource.GetDOMImplementationListFunc.result(\
        dom.DOMImplementationList*)\
    gp.GPWindowDOM.DOMImplementationSource.GetDOMImplementationListFunc.features(dom.utf8*))\
\
gp.GPWindowDOM.DOMImplementation( \
(dom.DOMImplementation)) \
gp.GPWindowDOM.DOMImplementation.HasFeatureFunc({gp.GPWindowDOM.DOMImplementation}\
    gp.GPWindowDOM.DOMImplementation.HasFeatureFunc.result(bool)\
    gp.GPWindowDOM.DOMImplementation.HasFeatureFunc.feature(dom.utf8*)\
    gp.GPWindowDOM.DOMImplementation.HasFeatureFunc.version(dom.utf8*))\
\
gp.GPWindowDOM.DOMImplementation.CreateDocumentTypeFunc({gp.GPWindowDOM.DOMImplementation}\
    gp.GPWindowDOM.DOMImplementation.CreateDocumentTypeFunc.result(dom.DocumentType*)\
    gp.GPWindowDOM.DOMImplementation.CreateDocumentTypeFunc.exception(dom.DOMException*)\
    gp.GPWindowDOM.DOMImplementation.CreateDocumentTypeFunc.impl(dom.DOMImplementation*)\
    gp.GPWindowDOM.DOMImplementation.CreateDocumentTypeFunc.qualifiedName(dom.utf8*)\
    gp.GPWindowDOM.DOMImplementation.CreateDocumentTypeFunc.publicId(dom.utf8*)\
    gp.GPWindowDOM.DOMImplementation.CreateDocumentTypeFunc.systemId(dom.utf8*))\
\
gp.GPWindowDOM.DOMImplementation.CreateDocumentFunc({gp.GPWindowDOM.DOMImplementation}\
    gp.GPWindowDOM.DOMImplementation.CreateDocumentFunc.result(dom.Document*)\
    gp.GPWindowDOM.DOMImplementation.CreateDocumentFunc.exception(dom.DOMException*)\
    gp.GPWindowDOM.DOMImplementation.CreateDocumentFunc.impl(dom.DOMImplementation*)\
    gp.GPWindowDOM.DOMImplementation.CreateDocumentFunc.namespaceURI(dom.utf8*)\
    gp.GPWindowDOM.DOMImplementation.CreateDocumentFunc.qualifiedName(dom.utf8*)\
    gp.GPWindowDOM.DOMImplementation.CreateDocumentFunc.docType(dom.DocumentType*))\
\
gp.GPWindowDOM.DOMImplementation.GetFeatureFunc({gp.GPWindowDOM.DOMImplementation}\
    gp.GPWindowDOM.DOMImplementation.GetFeatureFunc.result(dom.DOMObject*)\
    gp.GPWindowDOM.DOMImplementation.GetFeatureFunc.feature(dom.utf8*)\
    gp.GPWindowDOM.DOMImplementation.GetFeatureFunc.version(dom.utf8*))\
\
gp.GPWindowDOM.Document( \
    gp.GPWindowDOM.Document.super(dom.Document)\
    gp.GPWindowDOM.Document.tree(gp.GPTree*)) \
\
gp.GPWindowDOM.Document.GetMethodFunc({gp.GPWindowDOM.Document}\
    dom.Object.GetMethodFunc)\
\
gp.GPWindowDOM.Document.GetMemberFunc({gp.GPWindowDOM.Document}\
    dom.Object.GetMemberFunc)\
\
gp.GPWindowDOM.Document.CreateElementFunc({dom.Document}\
    gp.GPWindowDOM.Document.CreateElementFunc)\
\
gp.GPWindowDOM.Window( \
    gp.GPWindowDOM.Window.super(dom.DefaultElement)\
    gp.GPWindowDOM.Window.x(int)\
    gp.GPWindowDOM.Window.y(int)\
    gp.GPWindowDOM.Window.width(int)\
    gp.GPWindowDOM.Window.heigth(int)\
    gp.GPWindowDOM.Window.windowImpl(void*)\
    )\
\
gp.GPWindowDOM.Window.GetMethodFunc({gp.GPWindowDOM.Window}\
    dom.Object.GetMethodFunc)\
\
gp.GPWindowDOM.Window.GetMemberFunc({gp.GPWindowDOM.Window}\
    dom.Object.GetMemberFunc)\
\
gp.GPWindowDOM.Window.DestroyFunc({gp.GPWindowDOM.Window}\
    gp.GPWindowDOM.Window.DestroyFunc.window(gp.GPWindowDOM.Window*))\
\
gp.GPWindowDOM.Window.SetBoundsFunc({gp.GPWindowDOM.Window}\
    gp.GPWindowDOM.Window.SetBoundsFunc.window(gp.GPWindowDOM.Window*)\
    gp.GPWindowDOM.Window.SetBoundsFunc.x(int)\
    gp.GPWindowDOM.Window.SetBoundsFunc.y(int)\
    gp.GPWindowDOM.Window.SetBoundsFunc.width(int)\
    gp.GPWindowDOM.Window.SetBoundsFunc.height(int))\
\
gp.GPWindowDOM.Window.SetVisibleFunc({gp.GPWindowDOM.Window}\
    gp.GPWindowDOM.Window.SetVisibleFunc.window(gp.GPWindowDOM.Window*)\
    gp.GPWindowDOM.Window.SetVisibleFunc.visible(bool)\
    gp.GPWindowDOM.Window.SetVisibleFunc.toFront(bool))\
\
gp.GPWindowDOM.Window.ToFrontFunc({gp.GPWindowDOM.Window}\
    gp.GPWindowDOM.Window.ToFrontFunc.window(gp.GPWindowDOM.Window*))\
\
gp.GPWindowDOM.Window.ToBackFunc({gp.GPWindowDOM.Window}\
    gp.GPWindowDOM.Window.ToBackFunc.window(gp.GPWindowDOM.Window*))\
\
gp.GPWindowDOM.Window.SetPositionFunc({gp.GPWindowDOM.Window}\
    gp.GPWindowDOM.Window.SetPositionFunc.window(gp.GPWindowDOM.Window*)\
    gp.GPWindowDOM.Window.SetPositionFunc.x(int)\
    gp.GPWindowDOM.Window.SetPositionFunc.y(int))\
\
gp.GPWindowDOM.Window.SetSizeFunc({gp.GPWindowDOM.Window}\
    gp.GPWindowDOM.Window.SetSizeFunc.window(gp.GPWindowDOM.Window*)\
    gp.GPWindowDOM.Window.SetSizeFunc.width(int)\
    gp.GPWindowDOM.Window.SetSizeFunc.height(int))\
\
gp.GPWindowDOM.Window.SetBorderFunc({gp.GPWindowDOM.Window}\
    gp.GPWindowDOM.Window.SetBorderFunc.window(gp.GPWindowDOM.Window*)\
    gp.GPWindowDOM.Window.SetBorderFunc.border(uchar*))\
\
gp.GPWindowDOM.Window.SetTitleFunc({gp.GPWindowDOM.Window}\
    gp.GPWindowDOM.Window.SetTitleFunc.window(gp.GPWindowDOM.Window*)\
    gp.GPWindowDOM.Window.SetTitleFunc.title(uchar*))\
\
gp.GPWindowDOM.Window.ClearFunc({gp.GPWindowDOM.Window}\
    gp.GPWindowDOM.Window.ClearFunc.window(gp.GPWindowDOM.Window*) \
    gp.GPWindowDOM.Window.ClearFunc.x(int) \
    gp.GPWindowDOM.Window.ClearFunc.y(int) \
    gp.GPWindowDOM.Window.ClearFunc.width(int) \
    gp.GPWindowDOM.Window.ClearFunc.height(int))\
\
gp.GPWindowDOM.Window.RedrawFunc({gp.GPWindowDOM.Window}\
    gp.GPWindowDOM.Window.RedrawFunc.window(gp.GPWindowDOM.Window*) \
    gp.GPWindowDOM.Window.RedrawFunc.x(int) \
    gp.GPWindowDOM.Window.RedrawFunc.y(int) \
    gp.GPWindowDOM.Window.RedrawFunc.width(int) \
    gp.GPWindowDOM.Window.RedrawFunc.height(int))\
\
"

#define GPWINDOW_DOM_MIMETYPE "dom/window"

typedef struct {
    GPPlugin* result;
    GPPlugin* plugin;
} gp_GPWindowDOM_Plugin_GetWindowPluginFunc;

typedef struct {
    dom_DOMImplementation super;
} gp_GPWindowDOM_DOMImplementation;

typedef struct {
    dom_DefaultDocument super;
} gp_GPWindowDOM_Document;

typedef struct {
    dom_DefaultElement super;
    int x;
    int y;
    int width;
    int height;
    void* windowImpl; /*Reserved for use by windowing system*/
} gp_GPWindowDOM_Window;

typedef struct { 
    gp_GPWindowDOM_Window* window;
    int x;
    int y;
    int width;
    int height;
} gp_GPWindowDOM_Window_SetBoundsFunc;


typedef struct { 
    gp_GPWindowDOM_Window* window;
} gp_GPWindowDOM_Window_DestroyFunc;

typedef struct { 
    gp_GPWindowDOM_Window* window;
    bool visible;
    bool toFront;
} gp_GPWindowDOM_Window_SetVisibleFunc;

typedef struct { 
    gp_GPWindowDOM_Window* window;
} gp_GPWindowDOM_Window_ToFrontFunc;

typedef struct { 
    gp_GPWindowDOM_Window* window;
} gp_GPWindowDOM_Window_ToBackFunc;

typedef struct { 
    gp_GPWindowDOM_Window* window;
    int x;
    int y;
} gp_GPWindowDOM_Window_SetPositionFunc;

typedef struct { 
    gp_GPWindowDOM_Window* window;
    int width;
    int height;
} gp_GPWindowDOM_Window_SetSizeFunc;

typedef struct { 
    gp_GPWindowDOM_Window* window;
    char* border;
} gp_GPWindowDOM_Window_SetBorderFunc;

typedef struct { 
    gp_GPWindowDOM_Window* window;
    char* title;
} gp_GPWindowDOM_Window_SetTitleFunc;

typedef struct { 
    gp_GPWindowDOM_Window* window;
    int x;
    int y;
    int width;
    int height;
} gp_GPWindowDOM_Window_ClearFunc;

typedef struct { 
    gp_GPWindowDOM_Window* window;
    int x;
    int y;
    int width;
    int height;
} gp_GPWindowDOM_Window_RedrawFunc;


GPPlugin* gp_GPWindowDOM_GetWindowPlugin( GPPlugin* plugin ) {
    static GPType* func;
    gp_GPWindowDOM_Plugin_GetWindowPluginFunc data = {
                result:NULL,
                plugin:plugin };
    if(!func)
        func = GPType_get("gp.GPWindowDOM.Plugin.GetWindowPlugin");
    func->function(func,&data);
    return data.result;
}

static inline void gp_GPWindowDOM_setBounds(gp_GPWindowDOM_Window* window,int x,int y, int width, int height)
{
    static GPType* func;
    gp_GPWindowDOM_Window_SetBoundsFunc data = {window:window,x:x,y:y,width:width,height:height};
    if(!func)
        func = GPType_get("gp.GPWindowDOM.Window.SetBoundsFunc");
    func->function(func,&data);
}


static inline void gp_GPWindowDOM_destroy(gp_GPWindowDOM_Window* window)
{
    static GPType* func;
    gp_GPWindowDOM_Window_DestroyFunc data = {window:window};
    if(!func)
        func = GPType_get("gp.GPWindowDOM.Window.DestroyFunc");
    func->function(func,&data);
}

static inline void gp_GPWindowDOM_setVisible(gp_GPWindowDOM_Window* window,bool visible, bool toFront)
{
    static GPType* func;
    gp_GPWindowDOM_Window_SetVisibleFunc data = {window:window,visible:visible,toFront:toFront};
    if(!func)
        func = GPType_get("gp.GPWindowDOM.Window.SetVisibleFunc");
    func->function(func,&data);
}


static inline void gp_GPWindowDOM_clear(gp_GPWindowDOM_Window* window,int x,int y, int width, int height)
{
    static GPType* func;
    gp_GPWindowDOM_Window_ClearFunc data = {window:window,x:x,y:y,width:width,height:height};
    if(!func)
        func = GPType_get("gp.GPWindowDOM.Window.ClearFunc");
    func->function(func,&data);
}

static inline void gp_GPWindowDOM_redraw(gp_GPWindowDOM_Window* window,int x,int y, int width, int height)
{
    static GPType* func;
    gp_GPWindowDOM_Window_RedrawFunc data = {window:window,x:x,y:y,width:width,height:height};
    if(!func)
        func = GPType_get("gp.GPWindowDOM.Window.RedrawFunc");
    func->function(func,&data);
}


#ifdef __cplusplus
}  /* end extern "C" */
#endif

#endif /*GPDOMWindow_h*/
