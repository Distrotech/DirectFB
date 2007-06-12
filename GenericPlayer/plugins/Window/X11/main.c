/*
   (c) Copyright 2001-2007  The DirectFB Organization (directfb.org)
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



#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "xwindow.h"


int main(int argc, char* argv)
{
    XInitThreads();
    Display *display =XOpenDisplay(NULL);
    XWindow* xw; 
    int width = 200;
    int height = 200;
    int depth=DefaultDepth(display,DefaultScreen(display));
	switch(depth) {
	    case 16:
	    break;
		case 24:
		break;
		case 32:
		break;
		default:
		printf(" Unsupported X11 screen depth %d \n",depth);
		exit(-1);
		break;
    }
    xw_openWindow(&xw,display,0, 0,width,height,depth);
    //XCloseDisplay(display);
    //xw_closeWindow(&xw);

	//XShmPutImage(xw->display, xw->window, xw->gc, xw->ximage,
    //				 0, 0, 0, 0, xw->width, xw->height, False);
	XFlush(xw->display);
    while(1)
    {
        XEvent xevent;
        XNextEvent(display,&xevent);
        switch (xevent.type) {
            case Expose:
                if(xevent.xexpose.count == 0){
                    XShmPutImage(xw->display, xw->window, xw->gc, xw->ximage,
                            0, 0, 0, 0, xw->width, xw->height, False);
                    XFlush(xw->display);
                }
            break;
        }

    }
}	


