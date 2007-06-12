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
 
#ifndef GPType_h
#define GPType_h
#include "GPTree.h"
#include "GPList.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Names must be fully qualified for now
 */

#define GPRootTypesDef  "                   \
        (char) (char*) (uchar) (uchar*)  (bool) \
        (short) (short*) (ushort) (ushort*) \
        (int) (int*) (uint) (uint*)         \
        (long) (long*) (ulong) (ulong*)     \
        (llong) (llong*) (ullong) (ullong*) \
        (float) (float*) (double) (double*) \
        (ldouble) (ldouble*) (void) (void*) \
        (function)                          \
        gp.GPType.GetFunc(                   \
        gp.GPType.GetFunc.result(gp.GPType*) \
        gp.GPType.GetFunc.signature(uchar*))" 

#define GPTypeDef "gp.GPType(                   \
    gp.GPType.impl (void*)                      \
    gp.GPType.node (gp.GPTreeNode*)             \
    gp.GPType.name (uchar*)                     \
    gp.GPType.methods (gp.GPList*)              \
    gp.GPType.ptrCount (uint)                   \
    gp.GPType.offset (uint)                     \
    gp.GPType.pad (uint)                        \
    gp.GPType.size (uint)                       \
    gp.GPType.align(uint)                       \
    gp.GPType.GPFunction (function))            \
                                                \
    gp.GPType.GetMethodFunc({gp.GPType}         \
    gp.GPType.GetMethodFunc.result(gp.GPType*)  \
    gp.GPType.GetMethodFunc.owner(gp.GPType*)   \
    gp.GPType.GetMethodFunc.signature(uchar*)) \
                                                  \
    gp.GPType.GetMemberFunc({gp.GPType}         \
    gp.GPType.GetMemberFunc.result(gp.GPType*)  \
    gp.GPType.GetMemberFunc.owner(gp.GPType*)   \
    gp.GPType.GetMemberFunc.signature(uchar*)) \
                                                  \
    gp.GPType.PrintFunc({gp.GPType}             \
    gp.GPType.PrintFunc.node(gp.GPTreeNode*)    \
    gp.GPType.PrintFunc.type(gp.GPType*)        \
    gp.GPType.PrintFunc.deep(uint))             \
                                                \
    gp.GPType.RegisterFunc({gp.GPType}          \
    gp.GPType.RegisterFunc.signature(uchar*))\
\
gp.Callback(\
    gp.Callback.type(gp.GPType*)\
    gp.Callback.data(void*)\
\
"


typedef void (*GPFunction)(GPType* type, void* arg);

struct  _GPType {
    /**
     * Implementation private data
     */
    void* impl;

    GPTreeNode  *node;
    const char* name; 
    /**
     * FIXME: move to array later
     * for now don't assume its a traditional vtable
     */
    GPList*  methods;
    /**
     * offset from begining of container
     */
    unsigned int offset;
    /**
     * Pad bytes before next sibling
     */
    unsigned int pad;
    /**
     * type size in bytes
     * initialized with GP_sizeof
     */ 
    unsigned int size; 
    /**
     * offset in bytes if embedded in larger type 0 if not
     */ 
    unsigned int align; 
    /**
     * Function call interface for type 
     */
    GPFunction function;
};

/**
 * Sig for generic function callback closure
 */
typedef struct {
    GPType* type;
    void* data;
} GPCallback;

typedef struct {
    GPType* result;
    const char* signature;
} GPTypeGetFunc; 

typedef struct {
    GPType* result;
    GPType* owner;
    const char* signature;
} GPType_GetMethodFunc; 

typedef struct {
    GPType* result;
    GPType* owner;
    const char* signature;
} GPType_GetMemberFunc; 

typedef struct {
    GPTreeNode* node;
    GPType* type;
    GPBool deep;
} GPTypePrintFunc; 

typedef struct {
    const char* signature;
} GPTypeRegisterFunc; 

/**
 * This is the main bootstrap function
 * it plays some tricks.
 */
static inline GPType* GPType_get(const char* sig)
{
    static GPType* func;
    GPTypeGetFunc args ={result:NULL,signature:sig};
    /*this bootstrap hack assumes func->impl == 0*/
    if(!func) 
        GPRuntimeFunction((GPType*)&func,NULL);
    func->function(func,&args);
    return args.result;
}

static inline GPType* GPType_getMethod(GPType* owner,const char* sig)
{
    static GPType* func;
    GPType_GetMethodFunc args ={result:NULL,owner:owner,signature:sig};
    /*this must be true to bootstrap*/
    if(!func) 
        func = GPType_get("gp.GPType.GetMethodFunc");
    func->function(func,&args);
    return args.result;
}

static inline GPType* GPType_getMember(GPType* owner,const char* sig)
{
    static GPType* func;
    GPType_GetMemberFunc args ={result:NULL,owner:owner,signature:sig};
    /*this must be true to bootstrap*/
    if(!func) 
        func = GPType_get("gp.GPType.GetMemberFunc");
    func->function(func,&args);
    return args.result;
}


static inline void GPType_print(GPTreeNode* node, GPType* type, GPBool deep)
{
    static GPType* print;
    GPTypePrintFunc args ={node:node,type:type,deep:deep};
    if(!print)
        print = GPType_get("gp.GPType.PrintFunc");
    /**
     * get type is a special case
     */
    print->function(print,&args);
}

static inline void GPType_register(const char* signature)
{
    static GPType* registerfunc;
    GPTypeRegisterFunc args = {signature:signature};
    if(!registerfunc)
        registerfunc = GPType_get("gp.GPType.RegisterFunc");
    /**
     * get type is a special case
     */
    registerfunc->function(registerfunc,&args);
}

static inline void GPCallback_call(GPCallback* call)
{
   if(!call || !call->type || !call->type->function )
    return;
   call->type->function(call->type,call->data);
}

#ifdef __cplusplus
}  /* end extern "C" */
#endif

#endif /*GPType_h*/
