/*
   GLIB - Library of useful routines for C programming
   Copyright (C) 1995-1997  Peter Mattis, Spencer Kimball and Josh MacDonald
 
   (c) Copyright 2001-2007  The DirectFB Organization (directfb.org)
   (c) Copyright 2000-2004  Convergence (integrated media) GmbH

   All rights reserved.

   Written by Denis Oliver Kropp <dok@directfb.org>,
            Andreas Hundt <andi@fischlustig.de>,
            Sven Neumann <neo@directfb.org>,
            Ville Syrjälä <syrjala@sci.fi>,
            Claudio Ciccani <klan@users.sf.net> and
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

/*
 * Modified by the GLib Team and others 1997-2000.  See the AUTHORS
 * file for a list of people on the GLib Team.  See the ChangeLog
 * files for a list of changes.  These files are distributed with
 * GLib at ftp://ftp.gtk.org/pub/gtk/. 
 *
 * GMimeType is similar too the Objective-C runtime in gcc but not compatible
 * with bits of glibc
 * directfb gtk glib sprinkled in 
 *
 */

#include "GPRuntime.h"
#include "GPAtomicString.h"
#include "GPType.h"
#include "GPHashtable.h"
#include "GPLibrary.h"
#include "GPPlugin.h"
#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h> /*for offsetof*/
#include <stdarg.h>
#include <dlfcn.h>


typedef struct _GP_AllTypes {
  unsigned char C;
  char c;
  unsigned short S;
  short s;
  unsigned int I;
  int i;
  unsigned long L;
  long l;
  unsigned long long Q;
  long long q;
  float f;
  double d;
  long double D;
  void *ptr;
  void (*Fptr)();
#ifdef __cplusplus
  bool b;
  int AllTypes::*mptr;
  int (AllTypes::*mFptr)();
#endif
} GP_AllTypes;


#undef MAX
#define MAX(a,b) ({ typeof (a) _a = (a); typeof (b) _b = (b);  _a > _b ? _a : _b; })
#undef MIN
#define MIN(a,b) ({ typeof (a) _a = (a); typeof (b) _b = (b);  _a < _b ? _a : _b; })

/* Determine default alignment.  */

union fooround
{
  uintmax_t i;
  long double d;
  void *p;
};

struct fooalign
{
  char c;
  union fooround u;
};


/*from glibc If malloc were really smart, it would round addresses to DEFAULT_ALIGNMENT.
   But in fact it might be less smart and round addresses to as much as
   DEFAULT_ROUNDING.  So we prepare for it to do that. */
enum
{
    GP_DEFAULT_ALIGNMENT = offsetof (struct fooalign, u),
    GP_DEFUALT_ROUNDING = sizeof (union fooround),
    GP_MIN_STRUCT_ALIGNMENT = sizeof(struct{char a;}),
    GP_MAX_STRUCT_ALIGNMENT = __alignof__(GP_AllTypes),
};

typedef enum {
    GPLIBRARY_OPEN = 0, /*magic impl used by libs and runtime do not change*/
    GPTYPE_GET,
    GPTYPE_REGISTER,
    GPTYPE_GET_METHOD,
    GPTYPE_GET_MEMBER,
    GPTYPE_PRINT,
    GPATOM_ATOM,
    GPATOM_IS_ATOM,
    GPLIBRARY_REGISTER,
    GPLIBRARY_SYMBOL,
    GPLIBRARY_CLOSE,
    GPPLUGIN_REGISTER,
    GPPLUGIN_GET_PLUGIN_TYPE,
    GPPLUGIN_GET_ATTRIBUTE,
    GPPLUGIN_SET_ATTRIBUTE,
    GPPLUGIN_NEW,
    GPPLUGIN_DESTROY
} GPFuncs;

const char* GP_TYPE_CALL_NAMES[] = {
    "GPLIBRARY_OPEN",
    "GPTYPE_GET",
    "GPTYPE_REGISTER",
    "GPTYPE_GET_METHOD",
    "GPTYPE_GET_MEMBER",
    "GPTYPE_PRINT",
    "GPATOM_ATOM",
    "GPATOM_IS_ATOM",
    "GPLIBRARY_REGISTER",
    "GPLIBRARY_SYMBOL",
    "GPLIBRARY_CLOSE",
    "GPPLUGIN_REGISTER",
    "GPPLUGIN_GET_PLUGIN_TYPE",
    "GPPLUGIN_GET_ATTRIBUTE",
    "GPPLUGIN_SET_ATTRIBUTE",
    "GPPLUGIN_NEW",
    "GPPLUGIN_DESTROY"
};

typedef enum {
    GPHASH_INIT=0,
    GPHASH_REMOVE,
    GPHASH_REPLACE,
    GPHASH_GET,
    GPHASH_GET_NODE,
    GPHASH_GET_CHAR_ARRAY, /*5*/
    GPHASH_INSERT,
    GPHASH_RESIZE,
    GPHASH_EQUALS,
    GPHASH_STRING_EQUALS,
    GPHASH_CHAR_ARRAY_EQUALS, /*10*/
    GPHASH_PTR_EQUALS,
    GPHASH_INT_EQUALS,
    GPHASH_KEY,
    GPHASH_HASH_FUNC,
    GPHASH_HASH_STRING, /*15*/
    GPHASH_HASH_CHAR_ARRAY,
    GPHASH_HASH_PTR,
    GPHASH_HASH_INT,
    GPHASH_GET_KEY
} GPHashFuncs;


#define GPLibraryDef "gp(gp.GPLibrary( \
    gp.GPLibrary.hashNext(gp.GPLibrary*)\
    gp.GPLibrary.ref(uint)\
    gp.GPLibrary.mimeType(uchar*)\
    gp.GPLibrary.url(uchar*)\
    gp.GPLibrary.urlResolved(uchar*)\
    gp.GPLibrary.filename(uchar*)\
    gp.GPLibrary.handle(void*)\
    gp.GPLibrary.function(function) \
    gp.GPLibrary.error(char*) \
    gp.GPLibrary.plugins(GPList*)"


typedef struct _GPLibrary {
    struct _GPLibrary* hashNext;
    unsigned int ref;
    const char* mimeType;
    const char* filename;
    void* handle;
    GPFunction function;
    char* error;
    GPList* plugins;
    GPHashtable pluginTypes;
} GPLibrary;

static GPHashtable type_table;
static GPHashtable atomic_string_table;
static GPHashtable library_table;


static inline void gp_type_register(const char* inSignature);
static inline GPType* gp_type_lookup(const char* keyStart, unsigned int length);
static inline void gp_type_insert(GPType* type);

static inline const char* gp_atom(const char* string, int length);
static void gp_atom_function(GPType* type,void *data);
static inline GPLibrary* gp_library_get(const char* mimeType);
static inline void gp_library_replace(GPLibrary* lib); 
static inline void gp_library_open(const char*mimeType);

static void gp_hash_call(GPHashFuncs func, void* data ); 
static inline void* gp_hash_get(GPHashtable* table, void* key);
static inline void  gp_hash_insert(GPHashtable* table,void* key, void* value);
static inline void* gp_hash_replace(GPHashtable* table,void* key, void* value);
static inline void* gp_hash_remove(GPHashtable* table,void* key);
static inline unsigned int gp_hash_equals(GPHashtable* table,void* key1,void* key2);
static inline unsigned int gp_hash_equals_char_array(const char* arrayKey,int length,void* key2);
static inline void* gp_hash_get_char_array(GPHashtable* table,const char* arrayKey,int length);

static inline GPType* gp_plugin_get_type(const char* mimeType);
static inline GPType* GPType_initialize();

/**
 * aligns value to next highest address
 * FIXME: if the gcc version defines
 * ROUND_TYPE_ALIGN or ROUND_TYPE_SIZE
 * see the objc runtime for usage
 * for the processor then this is not right.
 * see the gcc porting docs 
 * ROUND_TYPE_ALIGN 
 * known to be defined for powerpc64 and sparc
 * ROUND_TYPE_SIZE not used
 * macros defined in the dir gcc/config
 * in the gcc source
 * Other important macro's.
 * BIGGEST_FIELD_ALIGNMENT defined on x86
 * ADJUST_FIELD_ALIGN 
 * FIXME: I think the libc malloc align macros
 * included should be sufficient
 * see __PTR_ALIGN
 *
 */

/*FIXME: IMPLEMENT*/
static void gp_type_destroy(GPType* type)
{
    if(!type) 
        return;
    if(type->node)
        GPTree_destroy(type->node);
}

void GPRuntimeFunction(GPType* arg, void* data)
{
    /*FIXME: add turning on type checking option and code*/
    static GPType* pluginType;
    GPFuncs func = (GPFuncs)arg->impl;
    //printf("GPRuntimeFunction:%s,%s\n",arg->name,GP_TYPE_CALL_NAMES[func]);

    switch(func) {
        
        case GPLIBRARY_OPEN:
        {
            gp_GPLibrary_OpenFunc* args;
            GPLibrary* lib;

            if(args && !data) {
                GPType** type = (GPType**)arg;
                *type = GPType_initialize();
                assert(*type);
                return;
            }
            if(!data)
                return;

            args = (gp_GPLibrary_OpenFunc*)data;
            args->result = NULL;
            lib = gp_library_get(args->mimeType);
            if(!lib) {
                static char estring[256];
                snprintf(estring,255,"Library not registered %s",args->mimeType);
                args->result = estring;
                return;
            }
            if(!lib->handle)
                lib->handle = dlopen(lib->filename,RTLD_LAZY);
            if(args->result = lib->error = dlerror())
                return;
            lib->ref++;
            lib->function = (GPFunction)dlsym(lib->handle,"GPLibraryFunction"); 
            if(args->result = lib->error = dlerror())
                return;
            if(lib->function) {
                GPType* openType = gp_type_lookup("gp.GPLibrary.OpenFunc",0);
                assert(openType);
                lib->function(openType,data);
            }
        }
        break;
        case GPTYPE_GET:
        {
            GPTypeGetFunc* args = (GPTypeGetFunc*)data;
            args->result = gp_type_lookup(args->signature,0);
        }
        break;
        /*FIXME: hack!!! this should be very fast*/
        case GPTYPE_GET_METHOD:
        {
            GPType_GetMethodFunc* args = (GPType_GetMethodFunc*)data;
            char* sig = (char*)args->signature;
            int doFree = FALSE;
            int len;
            int nameLen;
            char *dir;
            args->result = NULL;
            if(!args->signature || !args->owner )
                return;
            len = strlen(args->signature);
            nameLen = strlen(args->owner->name);
            dir = ((char*)args->owner->name)+nameLen;
            if(nameLen) {
                doFree = TRUE;
                sig = calloc(nameLen+len+2,sizeof(char));
                strncat(sig,args->owner->name,nameLen+1);
                strcat(sig,".");
                strcat(sig,args->signature);
            }
            args->result = gp_type_lookup(sig,0);
            if(doFree)
                free(sig);
            /*Allow Some sort of inheritance*/
            if(!args->result) {
                if(strcmp("GetMethodFunc",args->signature) != 0 ) {
                    printf("args->owner %s.%s\n",args->owner->name,args->signature);
                    GPType* func = GPType_getMethod(args->owner,"GetMethodFunc");
                    if(func && func->function != GPRuntimeFunction)
                        func->function(func,data);
                }
            }
        }
        break;
        /*FIXME: hack!!! this should be very fast*/
        case GPTYPE_GET_MEMBER:
        {
            GPType* type;
            GPType_GetMemberFunc* args = (GPType_GetMemberFunc*)data;
            char* sig = (char*)args->signature;
            int doFree = FALSE;
            int len;
            int nameLen;
            char *dir;
            if(!args->signature || !args->owner )
                return;
            len = strlen(args->signature);
            nameLen = strlen(args->owner->name);
            dir = ((char*)args->owner->name)+nameLen;
            if(nameLen) {
                doFree = TRUE;
                sig = calloc(nameLen+len+2,sizeof(char));
                strncat(sig,args->owner->name,nameLen);
                strcat(sig,".");
                strcat(sig,args->signature);
            }
            args->result = gp_type_lookup(sig,0);
            if(doFree)
                free(sig);
            if(!args->result) {
                if(strcmp("GetMemberFunc",args->signature) != 0 ) {
                    GPType* func = GPType_getMethod(args->owner,"GetMemberFunc");
                    if(func && func->node->children){
                        func = (GPType*)func->node->children->data;
                    }
                    if(func && func->function != GPRuntimeFunction)
                        func->function(func,data);
                }
            }
        }
        break;
        case GPTYPE_REGISTER:
        {
            GPTypeRegisterFunc* args = (GPTypeRegisterFunc*)data;
            gp_type_register(args->signature);
        }
        break;
        case GPTYPE_PRINT:
        {
            GPTreeNode* member;
            GPList* methods;
            GPTypePrintFunc* args = (GPTypePrintFunc*)data;
            GPTreeNode* node = args->node;
            GPType* type = args->type;
            GPBool deep = args->deep;
            if(!type) {
                printf("\n----Bad GPType: NULL-----------\n");
                return;
            }
            if(!node && type->node)
                node = type->node->parent;

            printf("\n----Start GPType:'%s'-----------\n",type->name);
            if(node) {
                printf("parent:%s\n",(node ? ((GPType*)node->data)->name:NULL));
                printf("prev:%s\n",(node->prev ? ((GPType*)node->prev->data)->name:NULL));
                printf("next:%s\n",(node->next ? ((GPType*)node->next->data)->name:NULL));
            }
            if(type->node)
                printf("children:%p\n",type->node->children);
            printf("align:%d\n",type->align);
            printf("size:%d\n",type->size);
            printf("offset:%d\n",type->offset);
            printf("pad:%d\n",type->pad);
            if(!deep) return;
            if(type->node->children) {
                printf("---------------Members-------------\n");
                for(member = type->node->children; member; member = member->next ) {
                    GPType* child=(GPType*)member->data;
                    GPType_print(type->node,child,TRUE);
                }
                printf("---------------End Members-------------\n");
            }
            if(type->methods) {
                printf("---------------Methods-------------\n");
                for(methods = type->methods; methods; methods = methods->next ) {
                    GPType* method=(GPType*)methods->data;
                    GPType_print(NULL,method,TRUE);
                }
                printf("---------------End Methods-------------\n");
            }
        }
        break;
        case GPLIBRARY_REGISTER:
        {
            GPTreeNode* member;
            GPType* type;
            gp_GPLibrary_RegisterFunc* args = (gp_GPLibrary_RegisterFunc*)data;
            GPLibrary* lib =  calloc(1,sizeof(GPLibrary));
            if(!lib)
                return;
            type =  gp_type_lookup("gp.GPHashHashStringFunc",0);
            lib->pluginTypes.hashFunc = type;
            type =  gp_type_lookup("gp.GPHashStringEqFunc",0);
            lib->pluginTypes.equalsFunc = type;

            if(args->filename)
                lib->filename = strdup(args->filename);
            if(args->mimeType) {
                lib->mimeType = strdup(args->mimeType);
                gp_hash_replace(&library_table,(void*)lib->mimeType,lib);
                assert(gp_library_get(args->mimeType));
            }
        }
        break;
        case GPLIBRARY_SYMBOL:
        {
            gp_GPLibrary_SymbolFunc* args;
            GPLibrary* lib;
            if(!data)
                return;
            args = (gp_GPLibrary_SymbolFunc*)data;
            lib = gp_library_get(args->mimeType);
            if(!lib)
                return;
            if(!lib->handle) 
                gp_library_open(args->mimeType);
            if(lib->handle) 
                args->result = dlsym(lib->handle,args->symbol);
        }
        break;
        case GPLIBRARY_CLOSE:
        {
            gp_GPLibrary_CloseFunc* args;
            GPLibrary* lib;
            if(!data)
                return;
            args = (gp_GPLibrary_CloseFunc*)data;
            lib = gp_library_get(args->mimeType);
            if(!lib)
                return;
            if(!lib->handle) 
                gp_library_open(args->mimeType);
            if(lib->handle) {
                GPType* closeType = gp_type_lookup("gp.GPLibrary.CloseFunc",0);
                lib->function(closeType,data);
                dlclose(lib->handle);
                lib->ref--;
                if(lib->ref < 0)
                    lib->ref = 0;
            }
        }
        break;
        case GPPLUGIN_REGISTER:
        {
            gp_GPPlugin_RegisterFunc* args = (gp_GPPlugin_RegisterFunc*)data;
            GPLibrary* lib = gp_library_get(args->mimeType);
            if(lib) {
                gp_type_register(args->defs);
                GPType* pluginType = gp_type_lookup(args->typeName,0);
                gp_hash_replace(&lib->pluginTypes,(void*)args->mimeType,pluginType);
                assert(gp_hash_get(&lib->pluginTypes,(void*)args->mimeType));
            }
        }
        break;
        case GPPLUGIN_GET_PLUGIN_TYPE:
        {
            GPPluginGetPluginTypeFunc* args = (GPPluginGetPluginTypeFunc*)data;
            GPLibrary* lib = gp_library_get(args->mimeType);
            if(lib) {
                if(!lib->handle)
                    gp_library_open(args->mimeType);
                args->result = (GPType*)gp_hash_get(&lib->pluginTypes,(void*)args->mimeType);
            }
        }
        break;
        case GPPLUGIN_GET_ATTRIBUTE:
        {

        }
        break;
        case GPPLUGIN_SET_ATTRIBUTE:
        {

        }
        break;
        case GPPLUGIN_NEW:
        {
            GPType* func;
            GPPluginFunc* args = (GPPluginFunc*)data;
            GPType* type =  gp_plugin_get_type(args->mimeType);
            args->result = NULL;
            if(type) {
                func =  GPType_getMethod(type,"PluginFunc");
                if(func) 
                    func->function(func,data);
            }
        }
        break;
        case GPPLUGIN_DESTROY:
        {
            static GPType* func;
            GPPluginDestroyFunc* args = (GPPluginDestroyFunc*)data;
            if(!func)
                func =  gp_type_lookup("gp.GPPluginDestroyFunc",0);
            args->plugin->type->function(func,data);

        }
        break;
        default:
            printf("GPRuntimeFunction: UNKNOWN %s\n",GP_TYPE_CALL_NAMES[func]);
            assert(0);
        break;
    }

}

static inline GPType* gp_create_type(GPType* parent, const char* start, unsigned int length) 
{
    GPType* type;
    char key[length+1];

    if(length == 0)
        return NULL;
    if(length) {
        strncpy(key,start,length);
        key[length] = '\0';
    }else { 
        const char* tmp = (char*)key;
        tmp = start;
    }

    type = gp_type_lookup(start,length);

    if(!type) {
        if(!(type = calloc(1,sizeof(GPType))))
            return NULL;
/*printf("-------> CREATING TYPE %.*s \n",length,start);*/
        if(!(type->name=(char*)gp_atom((const char*)start,length))) {
            assert(0);
            free(type);
            return NULL;
        } 
        if(!(type->node = GPTree_new(type))){
            assert(0);
            free(type);
            return NULL;
        } 
        type->node->data = type;
        /* a pointer*/
        if(type->name[length-1] == '*') {
            type->align = __alignof__(void*);
            type->size = sizeof(void*);
        }
        type->function = GPRuntimeFunction;
        if(parent) { 
            GPTree_append(parent->node,type->node);
        }
        gp_type_insert(type);
    } else if(parent) {
        GPTreeNode* node;
        if(!(node = GPTree_new(type))){
            assert(0);
            free(type);
            return NULL;
        } 
        GPTree_append(parent->node,node);
    }
    return type;
}

static inline void gp_type_register(const char* inSignature)
{
    const char* start;
    char* cur = NULL;
    unsigned int sz;
    GPType* parent = NULL;
    GPBool isSigned = FALSE; /*signed explictly set*/
    GPType* type;

    int i;
    int k=0;
    int len = strlen(inSignature)+1;
    char signature[len];
    memset(signature,0,len);
    /*remove whitespace*/
    for(i=0; i < len;i++) 
        if(!isspace(inSignature[i])){
            signature[k] = inSignature[i];
            k++;
        }
    start = signature;
    cur = signature;

    while(cur[0] != '\0'){
        /*reset temp vars*/
        while( cur[0] != '(' && cur[0] != '\0' && cur[0] != ')' && cur[0] != '{' && cur[0] !='}')
            cur++;
        switch(cur[0]) {
            case'\0':
                return;
            case '{':
                cur++;
                start = cur;
            break;
            case '}':
            {
                GPType* methodParent = gp_create_type(NULL,start,(cur-start));
                if(methodParent && parent)
                    methodParent->methods = GPList_append(methodParent->methods,parent);
                cur++;
                start = cur;
            }
            break;
            case '(':
            {
                GPType* prevParent= parent;
                parent = gp_create_type(parent,start,(cur-start));
                cur++;
                start = cur;
            }
            break;
            case ')':
            {

                GPTreeNode* member;
                type = gp_create_type(parent,start,(cur-start));
                if(parent) {
                    for(member = parent->node->children; member; member = member->next ) {
                        type = member->data;
                        type->align = MAX(GP_MIN_STRUCT_ALIGNMENT,type->align);
                        type->offset = parent->size;
                        parent->size += type->size;  
                        parent->align = MAX(parent->align,type->align);
                        if(parent->size%type->align) {
                            int pad = (((parent->size + (type->align - 1))/type->align) * type->align ) - parent->size; 
                            type->pad = pad;
                            parent->size += pad;
                            type->offset +=pad;
                        }
                    }
                    parent = parent->node->parent ? parent->node->parent->data : NULL;
                } 
                cur++;
                start = cur;
            }
            break;
        } 
    }
}

static GPType* GPType_initialize()
{
    static GPType* typeFunction;
    GPType* type;
    GPType tmpHashFunc;
    GPType tmpEqFunc;
    if(typeFunction)
        return typeFunction;

    /*hack the hash method for bootstrap*/
    memset(&tmpHashFunc,0,sizeof(GPType));
    memset(&tmpEqFunc,0,sizeof(GPType));
    tmpHashFunc.impl = (void*)GPHASH_HASH_STRING;
    tmpEqFunc.impl = (void*)GPHASH_STRING_EQUALS;

    type_table.hashFunc = &tmpHashFunc;
    atomic_string_table.hashFunc = &tmpHashFunc;

    type_table.equalsFunc = &tmpEqFunc;
    atomic_string_table.equalsFunc = &tmpEqFunc;

    gp_type_register(GPRootTypesDef);

    /**
     * GP char is by default signed !!!
     */
    type =  gp_type_lookup("char",0);
    type->align = __alignof__(signed char);
    type->size = sizeof(signed char);

    type =  gp_type_lookup("uchar",0);
    type->align = __alignof__(unsigned char);
    type->size = sizeof(unsigned char);
    type->function = gp_atom_function;

    type =  gp_type_lookup("bool",0);
    type->align = __alignof__(unsigned char);
    type->size = sizeof(unsigned char);
    type->function = gp_atom_function;

    type =  gp_type_lookup("short",0);
    type->align = __alignof__(short);
    type->size = sizeof(short);

    type =  gp_type_lookup("ushort",0);
    type->align = __alignof__(unsigned short);
    type->size = sizeof(unsigned short);

    type =  gp_type_lookup("int",0);
    type->align = __alignof__(int);
    type->size = sizeof(int);

    type =  gp_type_lookup("uint",0);
    type->align = __alignof__(uint);
    type->size = sizeof(uint);

    type =  gp_type_lookup("long",0);
    type->align = __alignof__(long);
    type->size = sizeof(long);

    type =  gp_type_lookup("ulong",0);
    type->align = __alignof__(unsigned long);
    type->size = sizeof(unsigned long);

    /**
     * Differ agian from C here to make parsing easier
     * llong == long long
     */
    type =  gp_type_lookup("llong",0);
   /*FIXME: GCC 4.1.2 is broken for
    * __alignof__(struct{long long a;}) 
    * it gives 4 not 8 in a struct
    */
    type->align = __alignof__(struct{long long a;});
    type->size = sizeof(long long);

    type =  gp_type_lookup("ullong",0);
   /*FIXME: GCC 4.1.2 is broken for
    * __alignof__(struct{unsigned long long a;}) 
    * it gives 4 not 8 in a struct
    */
    type->align = __alignof__(struct{unsigned long long a;});
    type->size = sizeof(unsigned long long);

    type =  gp_type_lookup("float",0);
    type->align = __alignof__(float);
    type->size = sizeof(float);

    type =  gp_type_lookup("double",0);
    type->align = __alignof__(double);
    type->size = sizeof(double);

    type =  gp_type_lookup("ldouble",0);
    type->align = __alignof__(long double);
    type->size = sizeof(long double);

    type =  gp_type_lookup("function",0);
    {
        void (*FunctionPtr)();
        type->align = __alignof(FunctionPtr);
        type->size = sizeof(FunctionPtr);
    }

    gp_type_register(GPTypeDef);
    gp_type_register(GPListDef);
    gp_type_register(GPTreeNodeDef);

    type =  gp_type_lookup("gp.GPType.GetFunc",0);
    type->impl = (void*)GPTYPE_GET;

    type =  gp_type_lookup("gp.GPType.GetMethodFunc",0);
    type->impl = (void*)GPTYPE_GET_METHOD;

    type =  gp_type_lookup("gp.GPType.GetMemberFunc",0);
    type->impl = (void*)GPTYPE_GET_MEMBER;

    type =  gp_type_lookup("gp.GPType.RegisterFunc",0);
    type->impl = (void*)GPTYPE_REGISTER;

    type =  gp_type_lookup("gp.GPType.PrintFunc",0);
    type->impl = (void*)GPTYPE_PRINT;

    gp_type_register(GPAtomDef);
    type =  gp_type_lookup("gp.GPAtomFunc",0);
    type->impl = (void*)GPATOM_ATOM;
    type->function = gp_atom_function;

    type =  gp_type_lookup("gp.GPIsAtomFunc",0);
    type->impl = (void*)GPATOM_IS_ATOM;
    type->function = gp_atom_function;


    /**FIXME: not fully resolved for hash table*/
    gp_type_register(GPHashHashFuncDef);
    gp_type_register(GPHashHashStringFuncDef);

    gp_type_register(GPHashEqFuncDef);
    gp_type_register(GPHashStringEqFuncDef);

    gp_type_register(GPLibraryRegisterFuncDef);
    type = gp_type_lookup("gp.GPLibrary.RegisterFunc",0);
    type->impl = (void*)GPLIBRARY_REGISTER;

    gp_type_register(GPLibraryOpenFuncDef);
    type =  gp_type_lookup("gp.GPLibrary.OpenFunc",0);
    type->impl = (void*)GPLIBRARY_OPEN;

    gp_type_register(GPLibraryCloseFuncDef);
    type =  gp_type_lookup("gp.GPLibrary.CloseFunc",0);
    type->impl = (void*)GPLIBRARY_CLOSE;

    gp_type_register(GPLibrarySymbolFuncDef);
    type =  gp_type_lookup("gp.GPLibrary.SymbolFunc",0);
    type->impl = (void*)GPLIBRARY_SYMBOL;

    gp_type_register(GPPluginDef);
    type =  gp_type_lookup("gp.GPPlugin.RegisterFunc",0);
    type->impl = (void*)GPPLUGIN_REGISTER;

    type =  gp_type_lookup("gp.GPPlugin.GetPluginTypeFunc",0);
    type->impl = (void*)GPPLUGIN_GET_PLUGIN_TYPE;

    type =  gp_type_lookup("gp.GPPlugin.GetAttributeFunc",0);
    type->impl = (void*)GPPLUGIN_GET_ATTRIBUTE;

    type =  gp_type_lookup("gp.GPPlugin.SetAttributeFunc",0);
    type->impl = (void*)GPPLUGIN_SET_ATTRIBUTE;

    type =  gp_type_lookup("gp.GPPlugin.GPPluginFunc",0);
    type->impl = (void*)GPPLUGIN_NEW;

    type =  gp_type_lookup("gp.GPPlugin.DestroyFunc",0);
    type->impl = (void*)GPPLUGIN_DESTROY;

    type =  gp_type_lookup("gp.GPHashHashStringFunc",0);
    type->impl = (void*)GPHASH_HASH_STRING;

    type_table.hashFunc = type;
    atomic_string_table.hashFunc = type;
    library_table.hashFunc = type;

    type =  gp_type_lookup("gp.GPHashStringEqFunc",0);
    type->impl = (void*)GPHASH_STRING_EQUALS;

    type_table.equalsFunc = type;
    atomic_string_table.equalsFunc = type;
    library_table.equalsFunc = type;

    typeFunction = gp_type_lookup("gp.GPType.GetFunc",0);
    assert(typeFunction);
    return typeFunction;
}

static inline GPType* gp_type_lookup(const char* key, unsigned int length) 
{
    if( length == 0 ) {
        return( (GPType*)gp_hash_get(&type_table,(void*)key));
    } else {
        GPHashGetCharArrayFunc args = {result:NULL,table:&type_table,arrayKey:key,length:length};
        gp_hash_call(GPHASH_GET_CHAR_ARRAY,&args);
        return((GPType*)args.result);
    }
}


static void gp_type_insert(GPType* type) {
    gp_hash_insert(&type_table,(void*)type->name,type);
}

static void gp_atom_call(GPFuncs func,void *data)
{
    GPAtomFunc* args =  (GPAtomFunc*)data; 

    args->result = NULL;
    if(!args->atomName)
        return;

    switch(func) {
        case GPATOM_ATOM:
        {
            char* atomicString = NULL;
            if(args->length) 
                atomicString = (char*)gp_hash_get_char_array(&atomic_string_table,args->atomName,args->length);
             else 
                atomicString = (char*)gp_hash_get(&atomic_string_table,(void*)args->atomName);
            
            if(!atomicString) {
                if(args->length) {
                    if(!(atomicString = calloc(1,args->length+1))) {
                        args->result = NULL;
                        return;
                    }
                    strncpy(atomicString,args->atomName,args->length);
                } else
                   atomicString = strdup(args->atomName);
                gp_hash_insert(&atomic_string_table,atomicString,atomicString);
            }
            args->result = atomicString;
        }
        break;
        case GPATOM_IS_ATOM:
        {
            args->result =  gp_hash_get(&atomic_string_table,(void*)args->atomName);
        }
        break;
        default:
        break;
    }
}


/*internal use for bootsrapping*/
static inline const char*  gp_atom(const char* string, int length)
{
   GPAtomFunc args ={result:NULL,atomName:string,length:length};
   gp_atom_call(GPATOM_ATOM,&args);
   return args.result;
}

static void gp_atom_function(GPType* type,void *data)
{
    GPFuncs func;
    if(type->function != gp_atom_function){
        type->function(type,data);
        return;
    }
    func =(GPFuncs)type->impl;
    gp_atom_call(func,data);
}


static inline GPLibrary* gp_library_get(const char* mimeType) {
    GPLibrary* result =  (GPLibrary*)gp_hash_get(&library_table,(void*)mimeType);
    return result;
}

static inline void gp_library_open(const char*mimeType)
{
   GPType func = {impl:(void*)GPLIBRARY_OPEN};
   gp_GPLibrary_OpenFunc args ={mimeType:mimeType};
   GPRuntimeFunction(&func,&args);
}

static inline GPType* gp_plugin_get_type(const char* mimeType)
{
   GPType func = {impl:(void*)GPPLUGIN_GET_PLUGIN_TYPE};
   GPPluginGetPluginTypeFunc args ={result:NULL,mimeType:mimeType};
   GPRuntimeFunction(&func,&args);
   return args.result;
}

static inline unsigned int gp_hash_hash(GPHashtable* table, void* key)
{
   GPHashHashFunc args = { result:0,table:table,key:key};
   gp_hash_call(GPHASH_HASH_FUNC,&args);
   return args.result;
}

static inline unsigned int gp_hash_hash_char_array(GPHashtable*table,const char* key, unsigned int length)
{
   GPHashHashCharArrayFunc args = { result:0,key:key,length:length};
   gp_hash_call(GPHASH_HASH_CHAR_ARRAY,&args);
   return args.result;
}

static inline void* gp_hash_get(GPHashtable* table, void* key)
{
   GPHashGetFunc args ={result:NULL,table:table,key:key};
   gp_hash_call(GPHASH_GET,&args);
   return args.result;
}

static inline void* gp_hash_get_char_array(GPHashtable* table,const char* arrayKey,int length)
{
    GPHashGetCharArrayFunc args = {result:NULL,table:table,arrayKey:arrayKey,length:length};
    gp_hash_call(GPHASH_GET_CHAR_ARRAY,&args);
    return args.result;
}

static inline GPHashNode** gp_hash_get_node(GPHashtable* table, void* key)
{
    GPHashGetNodeFunc args ={result:NULL,table:table,key:key};
    gp_hash_call(GPHASH_GET_NODE,&args);
    return args.result;
}

static inline unsigned int gp_hash_equals(GPHashtable* table, void* key1,void* key2)
{
    GPHashEqFunc args = { result:0,table:table,key1:key1,key2:key2};
    gp_hash_call(GPHASH_EQUALS,&args);
    return args.result;
}

static inline unsigned int gp_hash_equals_char_array(const char* arrayKey,int length,void* key2)
{
    GPHashCharArrayEqFunc args = {arrayKey:arrayKey,length:length,key2:key2};
    gp_hash_call(GPHASH_CHAR_ARRAY_EQUALS,&args);
    return args.result;
}

static inline void* gp_hash_remove(GPHashtable* table, void* key)
{
   GPHashRemoveFunc args ={result:NULL,table:table,key:key};
   gp_hash_call(GPHASH_REMOVE,&args);
   return args.result;
}

static inline void* gp_hash_replace(GPHashtable* table, void* key,void* value)
{
   GPHashReplaceFunc args ={result:NULL,table:table,key:key,value:value};
   gp_hash_call(GPHASH_REPLACE,&args);
   return args.result;
}

static inline void gp_hash_insert(GPHashtable* table, void* key, void* value)
{
   GPHashInsertFunc args ={table:table,key:key,value:value};
   gp_hash_call(GPHASH_INSERT,&args);
}

static inline void gp_hash_resize_table(GPHashtable* table,unsigned int size)
{
   GPHashResizeFunc args ={table:table,size:size};
   gp_hash_call(GPHASH_RESIZE,&args);
}

static void gp_hash_call(GPHashFuncs func, void* data ) 
{ 
    switch(func) {
        case GPHASH_INIT:
        break;
        case GPHASH_EQUALS:
        {
            GPHashEqFunc* args =(GPHashEqFunc*)data;
            switch((unsigned int)(args->table->equalsFunc->impl))
            {
                case GPHASH_STRING_EQUALS:
                {
                    GPHashStringEqFunc sargs = { result:0,key1:((const char*)args->key1),key2:((const char*)args->key2)};
                    gp_hash_call(GPHASH_STRING_EQUALS,&sargs);
                    args->result = sargs.result;
                }
                break;
                case GPHASH_PTR_EQUALS:
                {
                    GPHashPtrEqFunc pargs = {result:0,key1:args->key1,key2:args->key2};
                    gp_hash_call(GPHASH_PTR_EQUALS,&pargs);
                    args->result = pargs.result;
                }
                break;
                case GPHASH_INT_EQUALS:
                {
                    GPHashIntEqFunc iargs = { result:0,key1:((int)args->key1),key2:((int)args->key2)};
                    gp_hash_call(GPHASH_INT_EQUALS,&iargs);
                    args->result = iargs.result;
                }
                break;
                default:
                    printf(" BAD EQUALS CALL %d \n",args->table->equalsFunc->impl);
                    assert(0);
            }
        }
        break;
        case GPHASH_STRING_EQUALS:
        {
            GPHashStringEqFunc* args = (GPHashStringEqFunc*)data;
            if(!args->key1 || !args->key2)
                args->result = FALSE;
            else
                args->result=(strcmp(args->key1,args->key2) == 0 );
        }
        break;
        case GPHASH_CHAR_ARRAY_EQUALS:
        {
            GPHashCharArrayEqFunc* args = (GPHashCharArrayEqFunc*)data;
            if( strlen(args->key2) != args->length )
                args->result = FALSE;
            else
                args->result=(strncmp(args->arrayKey,args->key2,args->length) == 0 );
        }
        break;
        case GPHASH_PTR_EQUALS:
        {
            GPHashPtrEqFunc* args = (GPHashPtrEqFunc*)data;
            args->result = ( args->key1 ==  args->key2);
        }
        break;
        case GPHASH_INT_EQUALS:
        {
            GPHashIntEqFunc* args = (GPHashIntEqFunc*)data;
            args->result = ( args->key1 ==  args->key2);
        }
        break;
        case GPHASH_HASH_FUNC:
        {
            GPHashHashFunc* args =(GPHashHashFunc*)data;
            switch((unsigned int)(args->table->hashFunc->impl))
            {
                case GPHASH_HASH_STRING:
                {
                    GPHashHashStringFunc sargs = { result:0,key:((char*)args->key)};
                    gp_hash_call(GPHASH_HASH_STRING,&sargs);
                    args->result = sargs.result;
                }
                break;
                case GPHASH_HASH_PTR:
                {
                    GPHashHashPtrFunc pargs = {result:0,key:args->key};
                    gp_hash_call(GPHASH_HASH_PTR,&pargs);
                    args->result = pargs.result;
                }
                break;
                case GPHASH_HASH_INT:
                {
                    GPHashHashIntFunc iargs = { result:0,key:((int)args->key)};
                    gp_hash_call(GPHASH_HASH_INT,&iargs);
                    args->result = iargs.result;
                }
                break;
            }
        }
        break;
        case GPHASH_HASH_STRING:
        {
            GPHashHashStringFunc* args = (GPHashHashStringFunc*)data;
            const signed char* p = (const signed char*)args->key; 
            unsigned int h; 
            if( !p || *p == '\0') 
                args->result = 0;
            else {
                h = *p; 
                for (p += 1; *p != '\0'; p++) 
                    h = (h << 5) - h + *p; 
            }
            args->result = h;
        }
        break;
        case GPHASH_HASH_CHAR_ARRAY:
        {


            GPHashHashCharArrayFunc* args = (GPHashHashCharArrayFunc*)data;
            const signed char* p = (const signed char*)args->key; 
            unsigned int length = args->length;
            unsigned int h; 
            unsigned int k =0;
            args->result=0;
            if(p) { 
                h = p[k]; 
                for (k = 1; k < length; k++) 
                    h = (h << 5) - h + p[k]; 
            }
            args->result=h; 
        }
        break;
        case GPHASH_HASH_PTR:
        {
            GPHashHashPtrFunc* args = (GPHashHashPtrFunc*)data;
            args->result = ((unsigned int)args->key); 
        }
        break;
        case GPHASH_HASH_INT:
        {
            /*Robert Jenkins' 32 bit integer hash function
              constants could be varied for a perfect hash
            */
            GPHashHashIntFunc* args = (GPHashHashIntFunc*)data;
            unsigned int a = (unsigned int)args->key;
            a = (a+0x7ed55d16) + (a<<12);
            a = (a^0xc761c23c) ^ (a>>19);
            a = (a+0x165667b1) + (a<<5);
            a = (a+0xd3a2646c) ^ (a<<9);
            a = (a+0xfd7046c5) + (a<<3);
            a = (a^0xb55a4f09) ^ (a>>16);
            args->result = a;
        }
        break;
        case GPHASH_GET:
        {
            GPHashGetFunc* args = (GPHashGetFunc*)data;
            GPHashNode** node = gp_hash_get_node(args->table,args->key);
            if(node && *node) 
                args->result = (*node)->value;
            else
                args->result = NULL;
        }
        break;
        case GPHASH_GET_NODE:
        { 
            GPHashNode** node = NULL;
            GPHashGetNodeFunc* args = (GPHashGetNodeFunc*)data;
            unsigned int hash;
            unsigned int position;
            args->result = NULL;
            if(!args->key)
                break;
            if(!args->table->size ) 
                gp_hash_resize_table(args->table,1);
            hash = gp_hash_hash(args->table,args->key);
            position =  hash % args->table->size; 
            node = &(args->table->nodes[position]); 
            while(*node && !gp_hash_equals(args->table,(*node)->key,args->key)) {
                node = &((*node)->next);  
            }
            args->result = node;
        } 
        break;
        case GPHASH_GET_CHAR_ARRAY:
        {
            GPHashGetCharArrayFunc* args = (GPHashGetCharArrayFunc*)data;
            GPHashNode** node = NULL;
            unsigned int hash;
            unsigned int position;
            args->result = NULL;
            if(!args->table->size)
                gp_hash_resize_table(args->table,1);

            hash = gp_hash_hash_char_array(args->table,args->arrayKey,args->length);
            position =  hash % args->table->size; 
            node = &(args->table->nodes[position]); 
            while(*node && !gp_hash_equals_char_array(args->arrayKey,args->length,(*node)->key)) 
                node = &((*node)->next);  

            if(*node)
                args->result = (*node)->value;
            else
                args->result = NULL;
        }
        break;
        case GPHASH_REPLACE:
        {
            GPHashReplaceFunc* args = (GPHashReplaceFunc*)data;
            args->result = gp_hash_remove(args->table,args->key);
            gp_hash_insert(args->table,args->key,args->value);
        }
        break;
        case GPHASH_INSERT:
        {
            GPHashInsertFunc* args = (GPHashInsertFunc*)data;
            GPHashNode** node = gp_hash_get_node(args->table,args->key);
            if(*node)
                return;
            *node = (GPHashNode*)calloc(1,sizeof(GPHashNode));
            /*FIXME: error here*/
            if(!*node) 
                return;
            (*node)->key = args->key; 
            (*node)->value = args->value; 
            args->table->nnodes++;
            gp_hash_resize_table(args->table,0);
        }
        break;
        case GPHASH_REMOVE:
        {
            GPHashRemoveFunc* args = (GPHashRemoveFunc*)data;
            GPHashNode** node = NULL;
            GPHashNode** tmp;
            unsigned int hash;
            unsigned int position;
            if(!args->table->nnodes)
                return;

            hash = gp_hash_hash(args->table,args->key);
            position = hash % args->table->size; 
            args->result = NULL;
            node = &(args->table->nodes[position]); 
            tmp = node;
            while(*node && !gp_hash_equals(args->table,(*node)->key,args->key)) {
                tmp = node;
                node = &((*node)->next);  
            }
            if(tmp != node && *node)  
                (*tmp)->next = (*node)->next;
            
            if (*node) { 
                args->result = (*node)->value;
                free(*node);
                args->table->nnodes--; 
            } 
            gp_hash_resize_table(args->table,0);
        } 
        break;
        case GPHASH_RESIZE:
        {
            GPHashResizeFunc* args = (GPHashResizeFunc*)data;
            GPHashNode** new_nodes; 
            GPHashNode* tmpnode; 
            GPHashNode* next; 
            unsigned int hash_val; 
            int new_size = args->size; 
            int i; 
            GPHashtable* table = args->table;
            static const unsigned int primes[] = {
                11, 
                19, 
                37, 
                73, 
                109, 
                163, 
                251, 
                367, 
                557, 
                823, 
                1237, 
                1861, 
                2777, 
                4177, 
                6247, 
                9371, 
                14057, 
                21089, 
                31627, 
                47431, 
                71143, 
                106721, 
                160073, 
                240101, 
                360163, 
                540217, 
                810343, 
                1215497, 
                1823231, 
                2734867, 
                4102283, 
                6153409, 
                9230113, 
                13845163 
            }; 
            static const unsigned int nprimes = sizeof (primes) / sizeof (primes[0]); 
 
            if (!new_size && ((table->size >= 3 * table->nnodes && 
                table->size > GP_HASH_MIN_SIZE) || 
                (3 * table->size <= table->nnodes && table->size < GB_HASH_MAX_SIZE))) 
                    for (i = 0; i < nprimes; i++) 
                        if (primes[i] > table->nnodes ) { 
                            new_size = primes[i]; 
                            break; 
                        } 
            if (new_size > GB_HASH_MAX_SIZE ) 
                new_size = GB_HASH_MAX_SIZE; 
            if (new_size && (new_size <  GP_HASH_MIN_SIZE && table->nnodes > GP_HASH_MIN_SIZE)) 
                new_size = table->nnodes; 

            if( !new_size ||  new_size == table->size) {
                return; 
            }
 
            new_nodes = calloc(new_size, sizeof(GPHashNode*)); 
            if (!new_nodes) 
                return; 
            /* empty table pointer smash*/ 
            for (i = 0; i < table->size; i++)
                for (tmpnode = table->nodes[i]; tmpnode; tmpnode = next) { 
                    hash_val = gp_hash_hash(table,tmpnode->key) % new_size; 
                    next = tmpnode->next; 
                    tmpnode->next = new_nodes[hash_val]; 
                    new_nodes[hash_val] = tmpnode; 
                } 
            if(table->nodes)
                free(table->nodes); 
            table->nodes = new_nodes; 
            table->size = new_size; 
        }
        break;
        default:
        {
            fprintf(stderr,"Unimplemented case %d CHAR_ARRAY=%d\n",func,GPHASH_HASH_CHAR_ARRAY);
            assert(0);
        }
        break;
    }/*end switch*/
}


