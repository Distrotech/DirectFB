/*
   (c) Copyright 2001-2007  The DirectFB Organization (directfb.org)
   (c) Copyright 2000-2004  Convergence (integrated media) GmbH

   All rights reserved.

   Written by Denis Oliver Kropp <dok@directfb.org>,
              Andreas Hundt <andi@fischlustig.de>,
              Sven Neumann <neo@directfb.org>,
              Ville Syrjälä <syrjala@sci.fi>, 
              Claudio Ciccani <klan@users.sf.net>,
              Michael Emmel <mike.emmel@gmail.com>.

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

#include <GPRuntime.h>
#include <plugins/Window/GPWindow.h>
#include <plugins/Window/GPWindowDOM.h>
#include <stdio.h>
#include <X11/Xlib.h>    /* fundamentals X datas structures */
#include <X11/Xutil.h>   /* datas definitions for various functions */
#include <X11/keysym.h>  /* for a perfect use of keyboard events */
#include <X11/extensions/XShm.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <getopt.h> 
#include <string.h>
#include <stdlib.h>

typedef enum {
    GPX11_PLUGIN_CREATE,
    GPX11_PLUGIN_DESTROY,
    GPX11_PLUGIN_REGISTER_DOM_PLUGIN,
    GPX11_PLUGIN_BIND_WINDOW,
    GPX11_PLUGIN_PROCESS_EVENT,
    GPX11_WINDOW_DESTROY,
    GPX11_WINDOW_SET_BOUNDS,
    GPX11_WINDOW_SET_VISIBLE,
    GPX11_WINDOW_TO_FRONT,
    GPX11_WINDOW_TO_BACK,
    GPX11_WINDOW_SET_POSITION,
    GPX11_WINDOW_SET_SIZE,
    GPX11_WINDOW_SET_BORDER,
    GPX11_WINDOW_SET_TITLE,
    GPX11_WINDOW_CLEAR,
    GPX11_WINDOW_REDRAW
} X11Funcs;

typedef struct {
    Display* display;
    /*Default display info*/
	Screen*				screenptr;
	int 				screennum;
	Window 				window;
	Visual*				visual;
    int                 depth;
	/* (Null) cursor stuff*/
	Pixmap  			pixmp1;
	Pixmap  			pixmp2;
	Cursor 				NullCursor;
} X11Plugin;

typedef struct {
    X11Plugin*          plugin;
	Window 				window;
	GC 					gc;
	XImage*				ximage;
	Colormap 			colormap;
	Pixmap 				pixmap;
	XShmSegmentInfo*	shmseginfo;
	unsigned char*		videomemory;

	unsigned char*		virtualscreen;
	int 				videoaccesstype;
    Bool               override_redirect;
} XWindow;

//
// Data to create an invisible cursor
//
static const char null_cursor_bits[] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

void GPLibraryFunction(GPType* type, void* data)
{
    X11Funcs func;
    static int initialized;
    if(!initialized) {
        GPType* type;
        GPList* methods;
        int i =0;
        initialized = TRUE;
        GPPlugin_register("image/window","gp.GPWindow.Plugin",GPWindowDef);
        //GPPlugin_register("image/window/X11","gp.GPWindow.X11Plugin",GPWindowDef);
        type = GPType_get("gp.GPWindow.Plugin");
        /*this trick depends on X11Funcs being in same order as in header*/
        i = GPX11_PLUGIN_CREATE; 
        for(methods = type->methods; methods; methods = methods->next ) {
            GPType* method=(GPType*)methods->data;
            method->function = GPLibraryFunction;
            method->impl = (void*)i;
            i++;
        }
        return;
    }
    func = (X11Funcs)type->impl;
    //printf("X11Window CALLED GPLibraryFunction %d %s\n",func,type->name);
    switch(func)
    {
        case GPX11_PLUGIN_CREATE:
        {
            GPPlugin* plugin;
            X11Plugin* x11Plugin;
            char* displayName = NULL;
            GPPluginFunc* args = (GPPluginFunc*)data;
            args->result = NULL;
            if(args->argCount) {
                int i;
                if(args->argNames){
                    for(i = 0; i < args->argCount; i++) {
                        if(strncmp("display",args->argNames[i],strlen("display")) == 0)
                            displayName = (char*)args->argValues[i];
                    }
                }else {
                    /*act like a command line*/
                    static struct option long_options[] =
                    {
                        {"display",  required_argument, 0, 'd'},
                        {0, 0, 0, 0}
                    };
                    int option_index = 0;
                    for(i = 0; i < args->argCount; i++) {
                        char  c = getopt_long (args->argCount,(char** const)args->argValues, "d:",
                            long_options, &option_index);
                        if(c == -1)
                            break;
                        switch(c) {
                            case 'd':
                                displayName = optarg;
                        }
                    }
     
                }

            }
            if(!(plugin = calloc(1,sizeof(GPPlugin))))
                return;
            if(!(x11Plugin = calloc(1,sizeof(X11Plugin)))){
                if(plugin)
                    free(plugin);
                return;
            }
            XInitThreads();
            if( !(x11Plugin->display =XOpenDisplay(displayName))) {
                free(plugin);
                free(x11Plugin);
                return;
            }

	        x11Plugin->screenptr = DefaultScreenOfDisplay(x11Plugin->display);
	        x11Plugin->screennum = DefaultScreen(x11Plugin->display);
	        x11Plugin->visual= DefaultVisualOfScreen(x11Plugin->screenptr);
            x11Plugin->depth = DefaultDepth(x11Plugin->display,x11Plugin->screennum); 
            x11Plugin->window = RootWindowOfScreen(x11Plugin->screenptr);
	        /*Create a null cursor*/
            {
	            XColor  fore;
	            XColor  back;
	            x11Plugin->pixmp1 = XCreateBitmapFromData(x11Plugin->display, x11Plugin->window,
                                        null_cursor_bits, 16, 16);
	            x11Plugin->pixmp2 = XCreateBitmapFromData(x11Plugin->display,x11Plugin->window,
                                        null_cursor_bits, 16, 16);
	            x11Plugin->NullCursor = XCreatePixmapCursor(x11Plugin->display,
                                            x11Plugin->pixmp1,x11Plugin->pixmp2,&fore,&back,0,0);
            }
            plugin->type = GPType_get("gp.GPWindow.Plugin");
            plugin->mimeType = args->mimeType;
            plugin->function = GPLibraryFunction;
            plugin->impl = x11Plugin;
            args->result = plugin;
        }
        break;
        case GPX11_PLUGIN_DESTROY:
        {
            GPPluginDestroyFunc* args = (GPPluginDestroyFunc*)data;
            X11Plugin* x11Plugin = (X11Plugin*)args->plugin->impl;
            XCloseDisplay(x11Plugin->display);
            //FIXME: free window list
            free(x11Plugin);
            free(args->plugin);
        }
        break;
        case GPX11_PLUGIN_REGISTER_DOM_PLUGIN:
        {
            GPType* type;
            GPList* methods;
            GPCallback* callback;
            dom_Plugin_ProcessEventFunc* call;
            int override = false;
            int i = 0;
            gp_GPWindow_RegisterDOMPluginFunc* args = (gp_GPWindow_RegisterDOMPluginFunc*)data;

            type = GPType_get("gp.GPWindowDOM.Window");
            i = 0; 
            for(methods = type->methods; methods; methods = methods->next ) {
                GPType* method=(GPType*)methods->data;
                if(strcmp("gp.GPWindowDOM.Window.DestroyFunc",method->name) ==0 ) { 
                    override = true;
                    i = GPX11_WINDOW_DESTROY;
                }
                if(override) {
                    method->function = GPLibraryFunction;
                    method->impl = (void*)i;
                }
                i++;
            }

            type = GPType_getMethod(args->domPlugin->type,"ProcessEventFunc");
            assert(type);
            type->impl = (void*) GPX11_PLUGIN_PROCESS_EVENT;
            type->function = GPLibraryFunction; 

            if(!(callback = calloc(1,sizeof(GPCallback))))
                return;
            if(!(call = calloc(1,sizeof(dom_Plugin_ProcessEventFunc)))) {
                free(callback);
                return;
            }
            callback->type = GPType_getMethod(args->plugin->super.type,"ProcessEventFunc"); 
            assert(callback->type);
            call->plugin = (GPPlugin*)args->plugin;
            callback->data = call;
            dom_registerDOMEventSource(callback);
        }
        break;
        case GPX11_PLUGIN_PROCESS_EVENT:
        {

            dom_Plugin_ProcessEventFunc* args = (dom_Plugin_ProcessEventFunc*)data;
            X11Plugin* x11Plugin = (X11Plugin*)args->plugin->impl;
            while(XPending(x11Plugin->display)) {
                XEvent xevent;
                XNextEvent(x11Plugin->display,&xevent);
                switch (xevent.type) {
                    case Expose:
                        if(xevent.xexpose.count == 0){
#if 0
                            XShmPutImage(xw->display, xw->window, xw->gc, xw->ximage,
                            0, 0, 0, 0, xw->width, xw->height, False);
                            XFlush(xw->display);
#endif
                        }
                    break;
                }

            }
        }
        break;
        case GPX11_PLUGIN_BIND_WINDOW:
        {
            gp_GPWindow_Plugin_BindWindowElementFunc* args = (gp_GPWindow_Plugin_BindWindowElementFunc*)data;
            X11Plugin* x11Plugin = (X11Plugin*)args->plugin->super.impl;
            XWindow* xw;
            /*type check plugin*/
            if(strcmp(args->plugin->super.mimeType,"image/window") != 0 )
                return;
            
            if(!(xw = calloc(1,sizeof(XWindow))))
                return;
            xw->plugin = x11Plugin;

	        //xw->window=XCreateWindow(x11Plugin->display,
		//					         x11Plugin->window,
		//					 0,0,0,0, 0, x11Plugin->depth, InputOutput,
		//					 x11Plugin->visual, 0, NULL); 
	        xw->window=XCreateSimpleWindow(x11Plugin->display,
							         x11Plugin->window, 0,0,1,1,0,0,0);
	        if(!xw->window) {
                free(xw);
                return;
            }

	        XSelectInput(x11Plugin->display, xw->window,
				 ExposureMask|KeyPressMask|KeyReleaseMask|PointerMotionMask|ButtonPressMask|ButtonReleaseMask);


	        XDefineCursor(x11Plugin->display, xw->window, x11Plugin->NullCursor);
	        xw->gc=XCreateGC(x11Plugin->display, xw->window, 0, NULL);
            args->domWindow->windowImpl = xw; 
        }
        break;
        case GPX11_WINDOW_DESTROY:
        {
            gp_GPWindowDOM_Window_DestroyFunc* args = (gp_GPWindowDOM_Window_DestroyFunc*)data;
            XWindow* xw = (XWindow*)args->window->windowImpl;
            X11Plugin* x11Plugin = xw->plugin;
		    XFreeGC(x11Plugin->display,xw->gc);
		    XDestroyWindow(x11Plugin->display,xw->window);
#if 0
		    XShmDetach(xw->display, xw->shmseginfo);
		    if ( xw->ximage ) XDestroyImage(xw->ximage);
		        shmdt(xw->shmseginfo->shmaddr);
		        shmctl(xw->shmseginfo->shmid,IPC_RMID,NULL);
		        free(xw->shmseginfo);
#endif
		    free(xw);
        }
        break;
        case GPX11_WINDOW_SET_VISIBLE:
        {
            gp_GPWindowDOM_Window_SetVisibleFunc* args = (gp_GPWindowDOM_Window_SetVisibleFunc*)data;
            XWindow* xw = (XWindow*)args->window->windowImpl;
            X11Plugin* x11Plugin = xw->plugin;
            if(args->visible) {
                if(args->toFront)
                    XMapRaised(x11Plugin->display,xw->window);
                else 
                    XMapWindow(x11Plugin->display,xw->window);
            } else {
                XUnmapWindow(x11Plugin->display,xw->window);
                if(args->toFront)
                    XRaiseWindow(x11Plugin->display,xw->window);
                else
                    XLowerWindow(x11Plugin->display,xw->window);
            }
        }
        break;
        case GPX11_WINDOW_TO_FRONT:
        {
            gp_GPWindowDOM_Window_ToFrontFunc* args = (gp_GPWindowDOM_Window_ToFrontFunc*)data;
            XWindow* xw = (XWindow*)args->window->windowImpl;
            X11Plugin* x11Plugin = xw->plugin;
            XRaiseWindow(x11Plugin->display,xw->window);
        }
        break;
        case GPX11_WINDOW_TO_BACK:
        {
            gp_GPWindowDOM_Window_ToBackFunc* args = (gp_GPWindowDOM_Window_ToBackFunc*)data;
            XWindow* xw = (XWindow*)args->window->windowImpl;
            X11Plugin* x11Plugin = xw->plugin;
            XLowerWindow(x11Plugin->display,xw->window);
        }
        break;
        case GPX11_WINDOW_SET_POSITION:
        {
            gp_GPWindowDOM_Window_SetPositionFunc* args = (gp_GPWindowDOM_Window_SetPositionFunc*)data;
            XWindow* xw = (XWindow*)args->window->windowImpl;
            X11Plugin* x11Plugin = xw->plugin;
            XMoveWindow(x11Plugin->display,xw->window,args->x,args->y);
        }
        break;
        case GPX11_WINDOW_SET_SIZE:
        {
            gp_GPWindowDOM_Window_SetSizeFunc* args = (gp_GPWindowDOM_Window_SetSizeFunc*)data;
            XWindow* xw = (XWindow*)args->window->windowImpl;
            X11Plugin* x11Plugin = xw->plugin;
            XResizeWindow(x11Plugin->display,xw->window,args->width,args->height);

        }
        break;
        case GPX11_WINDOW_SET_BOUNDS:
        {
            gp_GPWindowDOM_Window_SetBoundsFunc* args = (gp_GPWindowDOM_Window_SetBoundsFunc*)data;
            XWindow* xw = (XWindow*)args->window->windowImpl;
            X11Plugin* x11Plugin = xw->plugin;
            args->window->x = args->x;
            args->window->y = args->y;
            args->window->width = args->width;
            args->window->height = args->height;
            XMoveResizeWindow(x11Plugin->display,xw->window,args->x,args->y,args->width,args->height);
        }
        break;
        case GPX11_WINDOW_SET_BORDER:
        {
            gp_GPWindowDOM_Window_SetBorderFunc* args = (gp_GPWindowDOM_Window_SetBorderFunc*)data;
            XWindow* xw = (XWindow*)args->window->windowImpl;
            X11Plugin* x11Plugin = xw->plugin;
            XSetWindowAttributes attr;
            if( !args->border || strcmp(args->border,"None") == 0 )
               attr.override_redirect = True;
            else
                if(xw->override_redirect)
                    attr.override_redirect = False;
            XChangeWindowAttributes (x11Plugin->display,xw->window, CWOverrideRedirect, &attr);
            xw->override_redirect = attr.override_redirect;
        }
        break;
        case GPX11_WINDOW_SET_TITLE:
        {

        }
        break;
        case GPX11_WINDOW_CLEAR:
        {
	        //memset(xw->virtualscreen, 127, xw->width*xw->height*xw->pixelsize);
        }
        break;
        case GPX11_WINDOW_REDRAW:
        {
            /*X11 does not have a concept of repaint/redraw so just send the event later
             * When we have  events.
             **/
        }
        break;
        default:
        {
            printf("GPX11 UNKOWN FUNC:%d %s\n",func,type->name);

        }
        break;
        
    }
}

