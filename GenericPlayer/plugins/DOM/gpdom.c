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
#include <plugins/DOM/GPDOM.h>
#include <plugins/DOM/GPDOMDefaultImpl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/**
 * DOM plugin is a singleton
 */
static GPPlugin* plugin;

typedef struct {
    GPType*   function;
    GPPlugin* plugin;
} dom_Plugin_EventSource;

static inline GPType* registerObject(const char* typeName, unsigned int firstMethod) {
    int i =0;
    GPList* methods;
    GPType* type;
    type = GPType_get(typeName);
    assert(type);
    i = firstMethod; 
    /*this trick depends on DOMFuncs being in same order as in header*/
    for(methods = type->methods; methods; methods = methods->next ) {
        GPType* method=(GPType*)methods->data;
        method->function = GPLibraryFunction;
        method->impl = (void*)i;
        i++;
    }
    return type;
}

void GPLibraryFunction(GPType* type, void* data)
{
    DOMFuncs func = (DOMFuncs)type->impl;
    //printf(" DOM CALLED GPLibraryFunction %d %s %p\n",func,type->name,GPLibraryFunction);
    switch(func)
    {
        case GPDOM_LIBRARY_OPEN:
        {
            static int initialized;
            GPType* type;
            GPList* methods;
            int i =0;
            if(initialized)
                return;
            initialized = TRUE;
            GPPlugin_register(GP_DOM_MIMETYPE,"dom.Plugin",GPDOMDef);

            registerObject("dom.Plugin",GPDOM_PLUGIN_CREATE);
            registerObject("dom.Object",GPDOM_OBJECT_GET_METHOD);

            registerObject("dom.DOMImplementationRegistery",
                        GPDOM_DOM_IMPLEMENTATION_REGISTERY_GET_DOM_IMPLEMENTATION);
            registerObject("dom.DOMImplementation",GPDOM_DOM_IMPLEMENTATION_HAS_FEATURE);
            registerObject("dom.Node",GPDOM_NODE_GET_METHOD);
            registerObject("dom.Document",GPDOM_DOCUMENT_GET_METHOD);

        /*FIXME: maybe we should do this all internally not in the headers
         * Check if we have super if so then fixup GetMethod etc
         * also recurse etc
         **/
            dom_setupInheritance("dom.DOMObject");
            dom_setupInheritance("dom.Node");
            dom_setupInheritance("dom.Document");
        }
        break;
        case GPDOM_PLUGIN_CREATE:
        {
            GPPluginFunc* args = (GPPluginFunc*)data;
            args->result = NULL;
            /*singleton*/
            if(!plugin) {
                if(!(plugin = calloc(1,sizeof(GPPlugin))))
                    return;
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
        case GPDOM_PLUGIN_PROCESS_EVENT:
        {
            dom_DOMImplementationRegistery* registery;
            GPList* sources;
            if(!plugin || !plugin->impl)
                return;
            registery = (dom_DOMImplementationRegistery*)plugin->impl;
            for( sources = registery->eventSources; sources; sources = sources->next)
                GPCallback_call((GPCallback*)sources->data);
        }
        break;
        case GPDOM_PLUGIN_INHERIT:
        {
            GPType* type;
            GPType* superType;
            dom_Plugin_SetupInheritanceFunc* args = (dom_Plugin_SetupInheritanceFunc*)data;
            type = GPType_get(args->typeName);
            assert(type);
            superType = GPType_getMember(type,"super");
            if(superType && superType->node && superType->node->children) 
                superType = superType->node->children->data;
            if(superType) {
                GPType* superMethod;
                GPType* method;
                superMethod = GPType_getMethod(superType,"GetMethodFunc");
                assert(superMethod);
                method = GPType_getMethod(type,"GetMethodFunc");
                assert(method);
                method->function = superMethod->function;
                method->impl = superMethod->impl;
                superMethod = GPType_getMethod(superType,"GetMemberFunc");
                method = GPType_getMethod(type,"GetMemberFunc");
                method->function = superMethod->function;
                method->impl = superMethod->impl;
            }
        }
        break;
        case GPDOM_PLUGIN_GET_DOM_IMPLEMENTATION_REGISTERY:
        {
            dom_DOMImplementationRegistery* registery;
            dom_Plugin_GetDOMImplementationRegisteryFunc* args = (dom_Plugin_GetDOMImplementationRegisteryFunc*)data;
            args->result = NULL;
           /*singleton*/
            if(!args->plugin->impl) {
                if(!(registery = calloc(1,sizeof(dom_DOMImplementationRegistery))))
                    return;
                args->plugin->impl = registery;
            }else {
                registery = (dom_DOMImplementationRegistery*)args->plugin->impl;
            }
            args->result = registery;
        }
        break;
        case GPDOM_OBJECT_GET_METHOD:
        {
            static GPType* func;
            GPType_GetMethodFunc* args = (GPType_GetMethodFunc*)data;
            GPType* superclass = GPType_getMember(args->owner,"super");
            if(!func)
                func = GPType_get("gp.GPType.GetMethodFunc");
            if(superclass && superclass->node->children) 
                superclass = (GPType*)superclass->node->children->data;
            if(superclass) {
                args->owner = superclass;
                func->function(func,data);
            }

        }
        break;
        case GPDOM_OBJECT_GET_MEMBER:
        {
            static GPType* func;
            GPType_GetMemberFunc* args = (GPType_GetMemberFunc*)data;
            GPType* superclass = GPType_getMember(args->owner,"super");
            if(!func)
                func = GPType_get("gp.GPType.GetMemberFunc");
            if(superclass && superclass->node->children) 
                superclass = (GPType*)superclass->node->children->data;
            if(superclass) {
                args->owner = superclass;
                func->function(func,data);
            }

        }
        break;
        case GPDOM_DOM_IMPLEMENTATION_REGISTERY_GET_DOM_IMPLEMENTATION:
        {
            GPList* sources;
            const char * err;
            dom_DOMImplementationRegistery_GetDOMImplementationFunc* args = 
                    (dom_DOMImplementationRegistery_GetDOMImplementationFunc*)data;

            args->result = NULL;
            /*FIXME:should throw exception*/
            if(err = GPLibrary_open(args->mimeType)) {
                printf("Load Library for mimeType %s failed:%s \n",args->mimeType,err);
                return;
            }
            dom_DOMImplementationSource_GetDOMImplementationFunc sourceArgs = 
                    {result:NULL,mimeType:args->mimeType,features:args->features}; 
            for( sources = args->registery->sources; sources; sources = sources->next) {
                    GPType* source = (GPType*)sources->data;
                    GPType* func = GPType_getMethod(source,"GetDOMImplementationFunc");
                    if(func) {
                        func->function(func,&sourceArgs);
                        if(sourceArgs.result) {
                            args->result = sourceArgs.result;
                            break;
                        }
                    }
            }
        }
        break;
        case GPDOM_DOM_IMPLEMENTATION_REGISTERY_GET_DOM_IMPLEMENTATION_LIST:
        {
            printf("GPDOM_DOM_IMPLEMENTATION_REGISTERY_GET_DOM_IMPLEMENTATION_LIST:\n");
        }
        break;
        case GPDOM_DOM_IMPLEMENTATION_REGISTERY_REGISTER_DOM_IMPLEMENTATION_SOURCE:
        {
            dom_DOMImplementationRegistery_RegisterDOMImplementationSourceFunc* args = 
                    (dom_DOMImplementationRegistery_RegisterDOMImplementationSourceFunc*)data;
            args->registery->sources = GPList_append(args->registery->sources,args->source);
        }
        break;
        case GPDOM_DOM_IMPLEMENTATION_REGISTERY_REGISTER_DOM_EVENT_SOURCE:
        {
            dom_DOMImplementationRegistery_RegisterDOMEventSourceFunc* args = 
                    (dom_DOMImplementationRegistery_RegisterDOMEventSourceFunc*)data;
            GPType* func;
            if(!args || !args->registery || !args->eventSource)
                return;
            args->registery->eventSources = GPList_append(args->registery->eventSources,args->eventSource);
        }
        break;
        case GPDOM_DOM_IMPLEMENTATION_REGISTERY_REMOVE_DOM_EVENT_SOURCE:
        {
            GPList* sources;
            dom_DOMImplementationRegistery_RemoveDOMEventSourceFunc* args = 
                    (dom_DOMImplementationRegistery_RemoveDOMEventSourceFunc*)data;
            for( sources = args->registery->eventSources; sources; sources = sources->next) {
                    GPCallback* eventSource = (GPCallback*)sources->data;
                    if(sources->data == args->eventSource) {
                        args->registery->eventSources= GPList_remove_link(
                                                            args->registery->eventSources,sources); 
                        free(sources);
                        return;
                    }
                    
            }
        }
        break;
        case GPDOM_DOM_IMPLEMENTATION_SOURCE_GET_DOM_IMPLEMENTATION:
        break;
        case GPDOM_DOM_IMPLEMENTATION_SOURCE_GET_DOM_IMPLEMENTATION_LIST:
        break;
        case GPDOM_DOM_IMPLEMENTATION_LIST_GET_LENGTH:
        break;
        case GPDOM_DOM_IMPLEMENTATION_LIST_GET_LENGTH_ITEM:
        break;
        case GPDOM_DOM_IMPLEMENTATION_HAS_FEATURE:
        {
            printf("GPDOM_DOM_IMPLEMENTATION_HAS_FEATURE:\n");
        }
        break;
        case GPDOM_DOM_IMPLEMENTATION_CREATE_DOCUMENT_TYPE:
        {
            printf("GPDOM_DOM_IMPLEMENTATION_CREATE_DOCUMENT_TYPE:\n");
        }
        break;
        case GPDOM_DOM_IMPLEMENTATION_CREATE_DOCUMENT:
        {
            dom_DOMImplementation_CreateDocumentFunc* args = (dom_DOMImplementation_CreateDocumentFunc*)data;
           GPType* func = GPType_getMethod(args->impl->super.gpType,"CreateDocumentFunc");
           assert(func);
           func->function(func,args);
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
        {
            dom_Node_AppendChildFunc* args = (dom_Node_AppendChildFunc*)data;
            dom_DefaultNode* node = (dom_DefaultNode*)args->node;
            dom_DefaultNode* newChild = (dom_DefaultNode*)args->newChild;

            if(!args->node) 
                goto INVALID_ARG;
            if(!node->tree && !(node->tree = GPTree_new(node)))
                goto  OUT_OF_MEMORY;

            if(!(newChild->tree = GPTree_new(newChild)))
                goto  OUT_OF_MEMORY;
            GPTree_append(node->tree,newChild->tree);
            return;

            INVALID_ARG:
            {
                if(args->exception)
                    args->exception->value = GP_INVALID_ARG_ERR;
                return;
            }
            OUT_OF_MEMORY:
            {
                if(args->exception)
                    args->exception->value = GP_OUT_OF_MEMORY_ERR;
                return;
            }
        }
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
        {
            dom_Document_GetDocumentElementFunc*args = 
                    (dom_Document_GetDocumentElementFunc*)data;
            args->result = NULL;
            dom_DefaultNode* doc =(dom_DefaultNode*)args->document;
            if(doc->tree && doc->tree->children)
                args->result = (dom_Element*)doc->tree->children->data;
        }
        break;
        case GPDOM_DOCUMENT_CREATE_ELEMENT:
        {

            dom_Document_CreateElementFunc* args = (dom_Document_CreateElementFunc*)data;
            dom_DefaultElement* element;
            args->result = NULL;

            if(!args->document || !args->tagName)
                goto INVALID_ARG; 

            if(!(element = calloc(1,sizeof(dom_DefaultElement))))
                goto OUT_OF_MEMORY;

            element->super.super.super.gpType = GPType_get("dom.DefaultElement");
            ((dom_DefaultNode*)element)->localName = args->tagName;
            args->result= (dom_Element*)element;
            return;

            GPDOM_DOCUMENT_CREATE_ELEMENT_INVALID_ARG:
            {
                if(args->exception)
                    args->exception->value = GP_INVALID_ARG_ERR;
                return;
            }
            GPDOM_DOCUMENT_CREATE_ELEMENT_OUT_OF_MEMORY:
            {
                if(args->exception)
                    args->exception->value = GP_OUT_OF_MEMORY_ERR;
                return;
            }
        }
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
        default:
        {
            printf("---------->GPDOM UNKOWN FUNC:%d inherit=%d",func,GPDOM_PLUGIN_INHERIT);
            assert(0);
        }
        break;
    }
}

