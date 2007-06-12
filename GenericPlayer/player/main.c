#include <GPRuntime.h>
#include <plugins/DOM/GPDOM.h>
#include <plugins/Window/GPWindowDOM.h>
#include <plugins/Window/GPWindow.h>
#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h> /*for offsetof*/

/**
 * create a window as if dynamically
 * bound from a interpeter
 * in general the interpeter would cache
 * the resolved type info
 */
static void callWindowSetBounds(gp_GPWindowDOM_Window* window, int x, int y, int w, int h) {
    char* addr;
    GPType* type = GPType_get("gp.GPWindowDOM.Window");
    GPType* method = GPType_getMethod(type,"SetBoundsFunc");
    GPType* windowArg = GPType_getMember(method,"window");
    GPType* xArg = GPType_getMember(method,"x");
    GPType* yArg = GPType_getMember(method,"y");
    GPType* widthArg = GPType_getMember(method,"width");
    GPType* heightArg = GPType_getMember(method,"height");

    char data[method->size];
    memset(data,-1,method->size);

    /*GPType_print(NULL,method,TRUE);*/

    addr =(char *)&data;  
    *((void**)addr) = (void*)window;
    addr = data+xArg->offset;
    *((int*)addr) = x;
    addr = data+yArg->offset;
    *((int*)addr) = y;
    addr = data+widthArg->offset;
    *((int*)addr) = w;
    addr = data+heightArg->offset;
    *((int*)addr) = h;
    {
        gp_GPWindowDOM_Window_SetBoundsFunc* args = (gp_GPWindowDOM_Window_SetBoundsFunc*)data;
        printf("CHECK INPUT:%p(%d,%d,%d,%d)\n",window,x,y,w,h);
        printf("CHECK:%p(%d,%d,%d,%d)\n",args->window,args->x,args->y,args->width,args->height);
        printf("CHECK OFFSETS:size=%d %d,%d,%d,%d,%d)\n",method->size,windowArg->offset,xArg->offset,yArg->offset,widthArg->offset,heightArg->offset);
        printf("CHECK C OFFSETS:size=%d %d,%d,%d,%d,%d)\n",
            sizeof(gp_GPWindowDOM_Window_SetBoundsFunc),
            offsetof(gp_GPWindowDOM_Window_SetBoundsFunc,window),
            offsetof(gp_GPWindowDOM_Window_SetBoundsFunc,x),
            offsetof(gp_GPWindowDOM_Window_SetBoundsFunc,y),
            offsetof(gp_GPWindowDOM_Window_SetBoundsFunc,width),
            offsetof(gp_GPWindowDOM_Window_SetBoundsFunc,height));

    }
    method->function(method,data);
}

int main(int argc, char *argv[],char *envp[])
{
    dom_DOMImplementation* impl=NULL;
    dom_Document* doc=NULL;
    gp_GPWindowDOM_Window* window=NULL;

    GPLibrary_register(GPWINDOW_MIMETYPE,"libgpwindow_x11.so");
    GPLibrary_register(GP_DOM_MIMETYPE,"libgpdom.so");
    GPLibrary_register(GPWINDOW_DOM_MIMETYPE,"libgpwindowdom.so");

    impl = dom_getDOMImplementation("dom/window",NULL);

    if(impl)
        doc = dom_createDocument(NULL,impl,"http://localhost/system/Window","Window",NULL);
    if(doc)
        window = (gp_GPWindowDOM_Window*)dom_getDocumentElement(doc);
    printf("GOT WindowDOM Impl = %p \n",impl);
    printf("GOT Window DOCUMENT  = %p \n",doc);
    printf("GOT Window Element  = %p \n",window);
    if(window) {
        /*gp_GPWindowDOM_setBounds(window,20,20,200,200);*/
        /*do a dynamic call*/
        callWindowSetBounds(window,20,20,200,200);
        gp_GPWindowDOM_setVisible(window,true,true);
        while(1)
            dom_processEvent();
    }
    return 0;
}

