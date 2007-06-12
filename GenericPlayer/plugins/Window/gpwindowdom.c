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
#include <plugins/DOM/GPDOMDefaultImpl.h>
#include <plugins/Window/GPWindowDOM.h>
#include <plugins/Window/GPWindow.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


typedef enum {
    GPWINDOWDOM_GET_WINDOW_PLUGIN = GPDOM_DOM_EXTENSION
} GPWindowDOMFuncs;

typedef struct  {
    gp_GPWindow_Plugin* windowPlugin;
} WindowDomImpl;

static inline dom_Element* gpwindowdom_create_element(dom_Document* document,const char* tagName )
{
    GPType type = {impl:(void*)GPDOM_DOCUMENT_CREATE_ELEMENT};
    dom_Document_CreateElementFunc data = {result:NULL,exception:NULL,
                    document:document,tagName:tagName };
    GPLibraryFunction(&type,&data);
    return data.result;
}

void GPLibraryFunction(GPType* type, void* data)
{
    DOMFuncs func;

    func = (DOMFuncs)type->impl;
    //printf("WindowDOM CALLED GPLibraryFunction %d %s %p\n",func,type->name,GPLibraryFunction);
    switch(func)
    {
        case GPDOM_LIBRARY_OPEN: 
        {
            static int initialized;
            GPType* type;
            GPList* methods;
            GPType* extension;
            int i =0;
            if(initialized) 
                return;
            initialized = true;
            /*FIXME: handle depenencies ?*/
            GPPlugin_register(GPWINDOW_DOM_MIMETYPE,"gp.GPWindowDOM.Plugin",GPWindowDOMDef);
            type = GPType_get("gp.GPWindowDOM.Plugin");
            assert(type);
            /*this trick depends on DOMFuncs being in same order as in header*/
            i = GPDOM_PLUGIN_CREATE; 
            extension = GPType_getMethod(type,"GetWindowPluginFunc");
            for(methods = type->methods; methods; methods = methods->next ) {
                GPType* method=(GPType*)methods->data;
                /*skip to extensions*/
                if(method == extension)
                    i = GPWINDOWDOM_GET_WINDOW_PLUGIN;
                method->function = GPLibraryFunction;
                method->impl = (void*)i;
                i++;
            }

            type = GPType_get("gp.GPWindowDOM.DOMImplementationSource");
            assert(type);
            i = GPDOM_DOM_IMPLEMENTATION_SOURCE_GET_DOM_IMPLEMENTATION; 
            for(methods = type->methods; methods; methods = methods->next ) {
                GPType* method=(GPType*)methods->data;
                method->function = GPLibraryFunction;
                method->impl = (void*)i;
                i++;
            }
            dom_registerDOMImplementationSource(type);

            type = GPType_get("gp.GPWindowDOM.DOMImplementation");
            assert(type);
            i =  GPDOM_DOM_IMPLEMENTATION_HAS_FEATURE;
            for(methods = type->methods; methods; methods = methods->next ) {
                GPType* method=(GPType*)methods->data;
                method->function = GPLibraryFunction;
                method->impl = (void*)i;
                i++;
            }
            type = GPType_get("gp.GPWindowDOM.Document");
            assert(type);
            i =  GPDOM_DOCUMENT_GET_METHOD;
            for(methods = type->methods; methods; methods = methods->next ) {
                GPType* method=(GPType*)methods->data;
                method->function = GPLibraryFunction;
                method->impl = (void*)i;
                i++;
            }
            dom_setupInheritance("gp.GPWindowDOM.Document");
            return;
        }
        case GPDOM_PLUGIN_CREATE:
        {
            static GPPlugin* plugin;
            WindowDomImpl* impl;
            GPPluginFunc* args = (GPPluginFunc*)data;
            args->result = NULL;
            /*singleton*/
            if(!plugin) {
                if(!(plugin = calloc(1,sizeof(GPPlugin))))
                    return;
                if(!(impl = calloc(1,sizeof(WindowDomImpl)))) {
                    free(plugin);
                    return;
                }
                plugin->type = GPType_get("gp.GPWindowDOM.Plugin");
                impl->windowPlugin = (gp_GPWindow_Plugin*)GPPlugin_plugin(GPWINDOW_MIMETYPE,0,NULL,NULL);
                plugin->impl = impl;
                gp_GPWindow_registerDOMPlugin(impl->windowPlugin,plugin);
            }else {
                //FIXME: refcount
            }
            args->result = plugin;
        }
        break;
        case GPDOM_PLUGIN_DESTROY:
        {
            GPPluginDestroyFunc* args = (GPPluginDestroyFunc*)data;
            //FIXME: refcount
            free(args->plugin);
        }
        break;
        case GPDOM_PLUGIN_INHERIT:
        break;
        case GPDOM_PLUGIN_PROCESS_EVENT:
        break;
        case GPDOM_PLUGIN_GET_DOM_IMPLEMENTATION_REGISTERY:
        break;
        case GPDOM_OBJECT_GET_METHOD:
        break;
        case GPDOM_OBJECT_GET_MEMBER:
        break;
        case GPDOM_DOM_IMPLEMENTATION_REGISTERY_GET_DOM_IMPLEMENTATION:
        break;
        case GPDOM_DOM_IMPLEMENTATION_REGISTERY_GET_DOM_IMPLEMENTATION_LIST:
        break;
        case GPDOM_DOM_IMPLEMENTATION_REGISTERY_REGISTER_DOM_IMPLEMENTATION_SOURCE:
        break;
        case GPDOM_DOM_IMPLEMENTATION_REGISTERY_REGISTER_DOM_EVENT_SOURCE:
        break;
        case GPDOM_DOM_IMPLEMENTATION_REGISTERY_REMOVE_DOM_EVENT_SOURCE:
        break;
        case GPDOM_DOM_IMPLEMENTATION_SOURCE_GET_DOM_IMPLEMENTATION:
        {
            unsigned int argCount = 0;
            const char* argsName = NULL;
            dom_DOMImplementationSource_GetDOMImplementationFunc* args = 
                    (dom_DOMImplementationSource_GetDOMImplementationFunc*)data;
            args->result = NULL;
            if(!args->mimeType)
                return;
            /*FIXME PARSE ARGS FOR REAL*/
            if( strstr(args->mimeType,GPWINDOW_DOM_MIMETYPE) != NULL) {
                if(!(args->result = calloc(1,sizeof(gp_GPWindowDOM_DOMImplementation))))
                    return;
                args->result->super.gpType = GPType_get("gp.GPWindowDOM.DOMImplementation");
                if(args->features) {
                    argCount =1;
                    argsName = "DOMFeatures";
                }
                args->result->plugin = GPPlugin_plugin(GPWINDOW_DOM_MIMETYPE,
                                                    argCount,&argsName,&args->features); 
            }
        }
        break;
        case GPDOM_DOM_IMPLEMENTATION_SOURCE_GET_DOM_IMPLEMENTATION_LIST:
        break;
        case GPDOM_DOM_IMPLEMENTATION_LIST_GET_LENGTH:
        break;
        case GPDOM_DOM_IMPLEMENTATION_LIST_GET_LENGTH_ITEM:
        break;
        case GPDOM_DOM_IMPLEMENTATION_HAS_FEATURE:
        break;
        case GPDOM_DOM_IMPLEMENTATION_CREATE_DOCUMENT_TYPE:
        break;
        case GPDOM_DOM_IMPLEMENTATION_CREATE_DOCUMENT:
        {
            dom_DOMImplementation_CreateDocumentFunc* args = 
                    (dom_DOMImplementation_CreateDocumentFunc*)data;
            dom_Document* doc;
            gp_GPWindowDOM_Window* element;
            if(!(doc = calloc(1,sizeof(gp_GPWindowDOM_Document))))
                return;
            doc->super.super.gpType = GPType_get("gp.GPWindowDOM.Document");
            args->result=doc;
            element = (gp_GPWindowDOM_Window*)dom_createElement(NULL,doc,"Window");
            if(element) {
                GPPlugin* plugin = args->impl->plugin;
                dom_Object* obj = (dom_Object*)element;
                assert(plugin);
                obj->gpType = GPType_get("gp.GPWindowDOM.Window");
                WindowDomImpl* impl = (WindowDomImpl*)plugin->impl;
                dom_Node_appendChild(NULL,(dom_Node*)doc,(dom_Node*)element);
                gp_GPWindow_bindWindowElement((gp_GPWindow_Plugin*)impl->windowPlugin,element); 
            }
        }
        break;
        case GPDOM_DOM_IMPLEMENTATION_GET_FEATURE:
        break;
        case GPDOM_NODE_GET_METHOD:
        break;
        case GPDOM_NODE_GET_MEMBER:
        break;
        case GPDOM_NODE_GET_NODE_NAME:
        break;
        case GPDOM_NODE_GET_NODE_VALUE:
        break;
        case GPDOM_NODE_SET_NODE_VALUE:
        break;
        case GPDOM_NODE_GET_NODE_TYPE:
        break;
        case GPDOM_NODE_GET_PARENT_NODE:
        break;
        case GPDOM_NODE_GET_CHILD_NODES:
        break;
        case GPDOM_NODE_GET_FIRST_CHILD:
        break;
        case GPDOM_NODE_GET_LAST_CHILD:
        break;
        case GPDOM_NODE_GET_PREVIOUS_SIBLING:
        break;
        case GPDOM_NODE_GET_NEXT_SIBLING:
        break;
        case GPDOM_NODE_GET_ATTRIBUTES:
        break;
        case GPDOM_NODE_GET_OWNER_DOCUMENT:
        break;
        case GPDOM_NODE_INSERT_BEFORE:
        break;
        case GPDOM_NODE_REPLACE_CHILD:
        break;
        case GPDOM_NODE_REMOVE_CHILD:
        break;
        case GPDOM_NODE_APPEND_CHILD:
        break;
        case GPDOM_NODE_HAS_CHILD_NODES:
        break;
        case GPDOM_NODE_CLONE_NODE:
        break;
        case GPDOM_NODE_NORMALIZE:
        break;
        case GPDOM_NODE_IS_SUPPORTED:
        break;
        case GPDOM_NODE_GET_NAMESPACE_URI:
        break;
        case GPDOM_NODE_GET_PREFIX:
        break;
        case GPDOM_NODE_SET_PREFIX:
        break;
        case GPDOM_NODE_GET_LOCAL_NAME:
        break;
        case GPDOM_NODE_HAS_ATTRIBUTES:
        break;
        case GPDOM_NODE_GET_BASE_URI:
        break;
        case GPDOM_NODE_COMPARE_DOCUMENT_POSITION:
        break;
        case GPDOM_NODE_GET_TEXT_CONTENT:
        break;
        case GPDOM_NODE_SET_TEXT_CONTENT:
        break;
        case GPDOM_NODE_IS_SAME_NODE:
        break;
        case GPDOM_NODE_LOOKUP_PREFIX:
        break;
        case GPDOM_NODE_IS_DEFAULT_NAMESPACE:
        break;
        case GPDOM_NODE_LOOKUP_NAMESPACE_URI:
        break;
        case GPDOM_NODE_IS_EQUAL_NODE:
        break;
        case GPDOM_NODE_GET_FEATURE:
        break;
        case GPDOM_NODE_SET_USER_DATA:
        break;
        case GPDOM_NODE_GET_USER_DATA:
        break;
        case GPDOM_NODE_GET_LENGTH:
        break;
        case GPDOM_NODE_LIST_ITEM:
        break;
        case GPDOM_NAMED_NODE_MAP_GET_LENGTH:
        break;
        case GPDOM_NAMED_NODE_MAP_GET_NAMED_ITEM:
        break;
        case GPDOM_NAMED_NODE_MAP_SET_NAMED_ITEM:
        break;
        case GPDOM_NAMED_NODE_MAP_REMOVE_NAMED_ITEM:
        break;
        case GPDOM_NAMED_NODE_MAP_ITEM:
        break;
        case GPDOM_NAMED_NODE_MAP_GET_ITEM_NS:
        break;
        case GPDOM_NAMED_NODE_MAP_SET_ITEM_NS:
        break;
        case GPDOM_NAMED_NODE_MAP_REMOVE_ITEM_NS:
        break;
        case GPDOM_CHARACTER_DATA_GET_DATA:
        break;
        case GPDOM_CHARACTER_DATA_SET_DATA:
        break;
        case GPDOM_CHARACTER_DATA_GET_LENGTH:
        break;
        case GPDOM_CHARACTER_DATA_SUBSTRING_DATA:
        break;
        case GPDOM_CHARACTER_DATA_APPEND_DATA:
        break;
        case GPDOM_CHARACTER_DATA_INSERT_DATA:
        break;
        case GPDOM_CHARACTER_DATA_DELETE_DATA:
        break;
        case GPDOM_CHARACTER_DATA_REPLACE_DATA:
        break;
        case GPDOM_TEXT_SPLIT_TEXT:
        break;
        case GPDOM_TEXT_GET_IS_ELEMENT_CONTENT_WHITESPACE:
        break;
        case GPDOM_TEXT_GET_WHOLE_TEXT:
        break;
        case GPDOM_TEXT_REPLACE_WHOLE_TEXT:
        break;
        case GPDOM_ELEMENT_GET_TAG_NAME:
        break;
        case GPDOM_ELEMENT_GET_ATTRIBUTE:
        break;
        case GPDOM_ELEMENT_SET_ATTRIBUTE:
        break;
        case GPDOM_ELEMENT_GET_TYPED_ATTRIBUTE:
        break;
        case GPDOM_ELEMENT_SET_TYPED_ATTRIBUTE:
        break;
        case GPDOM_ELEMENT_REMOVE_ATTRIBUTE:
        break;
        case GPDOM_ELEMENT_GET_ATTRIBUTE_NODE:
        break;
        case GPDOM_ELEMENT_SET_ATTRIBUTE_NODE:
        break;
        case GPDOM_ELEMENT_GET_ELEMENTS_BY_TAG_NAME:
        break;
        case GPDOM_ELEMENT_GET_ATTRIBUTE_NS:
        break;
        case GPDOM_ELEMENT_GET_TYPED_ATTRIBUTE_NS:
        break;
        case GPDOM_ELEMENT_SET_ATTRIBUTE_NS:
        break;
        case GPDOM_ELEMENT_SET_TYPE_ATTRIBUTE_NS:
        break;
        case GPDOM_ELEMENT_REMOVE_ATTRIBUTE_NS:
        break;
        case GPDOM_ELEMENT_GET_ATTRIBUTE_NODE_NS:
        break;
        case GPDOM_ELEMENT_SET_ATTRIBUTE_NODE_NS:
        break;
        case GPDOM_ELEMENT_GET_ELEMENTS_BY_TAG_NAME_NS:
        break;
        case GPDOM_ELEMENT_HAS_ATTRIBUTE:
        break;
        case GPDOM_ELEMENT_HAS_ATTRIBUTE_NS:
        break;
        case GPDOM_ELEMENT_GET_SCHEMA_TYPE_INFO:
        break;
        case GPDOM_ELEMENT_SET_ID_ATTRIBUTE:
        break;
        case GPDOM_ELEMENT_SET_ID_ATTRIBUTE_NS:
        break;
        case GPDOM_ELEMENT_SET_ID_ATTRIBUTE_NODE:
        break;
        case GPDOM_ATTR_GET_NAME:
        break;
        case GPDOM_ATTR_GET_SPECIFIED:
        break;
        case GPDOM_ATTR_GET_VALUE:
        break;
        case GPDOM_ATTR_SET_VALUE:
        break;
        case GPDOM_ATTR_GET_OWNER_ELEMENT:
        break;
        case GPDOM_ATTR_GET_SCHEMA_TYPE_INFO:
        break;
        case GPDOM_ATTR_IS_ID:
        break;
        case GPDOM_DOCUMENT_GET_METHOD:
        break;
        case GPDOM_DOCUMENT_GET_MEMBER:
        break;
        case GPDOM_DOCUMENT_GET_DOCUMENT_VIEW:
        break;
        case GPDOM_DOCUMENT_GET_DOCUMENT_TYPE:
        break;
        case GPDOM_DOCUMENT_GET_DOM_IMPLEMENTATION:
        break;
        case GPDOM_DOCUMENT_GET_DOCUMENT_ELEMENT:
        break;
        case GPDOM_DOCUMENT_CREATE_ELEMENT:
        break;
        case GPDOM_DOCUMENT_CREATE_DOCUMENT_FRAGMENT:
        break;
        case GPDOM_DOCUMENT_CREATE_TEXT_NODE:
        break;
        case GPDOM_DOCUMENT_CREATE_COMMENT:
        break;
        case GPDOM_DOCUMENT_CREATE_CDATA_SECTION:
        break;
        case GPDOM_DOCUMENT_CREATE_PROCESSING_INSTRUCTION:
        break;
        case GPDOM_DOCUMENT_CREATE_ATTRIBUTE:
        break;
        case GPDOM_DOCUMENT_CREATE_ENTITY_REFERENCE:
        break;
        case GPDOM_DOCUMENT_GET_ELEMENTS_BY_TAG_NAME:
        break;
        case GPDOM_DOCUMENT_IMPORT_NODE:
        break;
        case GPDOM_DOCUMENT_CREATE_ELEMENT_NS:
        break;
        case GPDOM_DOCUMENT_CREATE_ATTRIBUTE_NS:
        break;
        case GPDOM_DOCUMENT_GET_ELEMENTS_BY_TAG_NAME_NS:
        break;
        case GPDOM_DOCUMENT_GET_ELEMENTS_BY_ID:
        break;
        case GPDOM_DOCUMENT_GET_INPUT_ENCODING:
        break;
        case GPDOM_DOCUMENT_GET_XML_ENCODING:
        break;
        case GPDOM_DOCUMENT_GET_XML_STANDALONE:
        break;
        case GPDOM_DOCUMENT_SET_XML_STANDALONE:
        break;
        case GPDOM_DOCUMENT_GET_XML_VERSION:
        break;
        case GPDOM_DOCUMENT_SET_XML_VERSION:
        break;
        case GPDOM_DOCUMENT_GET_STRICT_ERROR_CHECKING:
        break;
        case GPDOM_DOCUMENT_SET_STRICT_ERROR_CHECKING:
        break;
        case GPDOM_DOCUMENT_GET_DOCUMENT_URI:
        break;
        case GPDOM_DOCUMENT_SET_DOCUMENT_URI:
        break;
        case GPDOM_DOCUMENT_ADOPT_NODE:
        break;
        case GPDOM_DOCUMENT_GET_DOM_CONFIGURATION:
        break;
        case GPDOM_DOCUMENT_NORMALIZE_DOCUMENT:
        break;
        case GPDOM_DOCUMENT_RENAME_NODE:
        break;
        case GPDOM_DOM_CONFIGURATION_SET_PARAMETER:
        break;
        case GPDOM_DOM_CONFIGURATION_GET_PARAMETER:
        break;
        case GPDOM_DOM_CONFIGURATION_CAN_SET_PARAMETER:
        break;
        case GPDOM_DOM_CONFIGURATION_GET_PARAMETER_NAMES:
        break;
        case GPDOM_DOM_CONFIGURATION_GET_DOCUMENT_VIEW:
        break;
        case GPWINDOWDOM_GET_WINDOW_PLUGIN:
        {
            gp_GPWindowDOM_Plugin_GetWindowPluginFunc* args = 
                    (gp_GPWindowDOM_Plugin_GetWindowPluginFunc*)data;
            WindowDomImpl* impl = (WindowDomImpl*)args->plugin->impl;
            args->result = (GPPlugin*)impl->windowPlugin;
        }
        break;
        default:
        {
            printf("GPDOM UNKOWN FUNC:%d %s\n",func,type->name);
        }
        break;
    }
}


