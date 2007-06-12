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
 
#ifndef GPHashtable_h
#define GPHashtable_h

#ifdef __cplusplus
extern "C" {
#endif

/*FIXME: finish definitions and internal registery*/

#ifndef GP_HASH_MIN_SIZE
#define GP_HASH_MIN_SIZE 11
#endif
#ifndef GP_HASH_MAX_SIZE
#define GB_HASH_MAX_SIZE 13845163
#endif

#define GPHashNodeDef "gp.GPHashNode(\
    gp.GPHashNode.next (gp.GPHashNode*))" 


typedef struct _GPHashNode GPHashNode; 
struct _GPHashNode {
    GPHashNode* next;
    void* key;
    void* value;
};


#define GPHashtableDef "gp.GPHashtable(\
    gp.GPHashtable.int (uint) \
    gp.GPHashtable.nnode (uint) \
    gp.GPHashtable.equalsFunc (gp.GPType*) \
    gp.GPHashtable.hashFunc (gp.GPType*) \
    gp.GPHashtable.keyOffset (uint) \
    gp.GPHashtable.nodes (gp.GPHashNode**))" 
    
typedef struct 
{
  unsigned int      size;
  unsigned int      nnodes;
  GPHashNode**     nodes;
  GPType*          equalsFunc;
  GPType*          hashFunc;
} GPHashtable;

typedef struct {
    void* result;
    const char* key;
} GPHashGetStringFunc;

static inline void* GPHash_getString(GPHashtable* table,const char* key)
{
    static GPType* get;
    GPHashGetStringFunc args ={result:NULL,key:key};
    if(!get)
        get = GPType_get("gp.GPHashGetStringFunc");
    /**
     * get type is a special case
     */
    get->function(get,&args);
}

#define GPHashHashStringFuncDef "gp.GPHashHashStringFunc({gp.GPHashtable}\
    gp.GPHashHashStringFunc.result (uint) \
    gp.GPHashHashStringFunc.key (uchar*))" 

typedef struct {
    unsigned int result;
    const char* key;
} GPHashHashStringFunc;

#define GPHashHashCharArrayFuncDef "gp.GPHashHashCharArrayFunc({gp.GPHashtable}\
    gp.GPHashHashCharArrayFunc.result (uint) \
    gp.GPHashHashCharArrayFunc.key (uchar*) \
    gp.GPHashHashCharArrayFunc.length (int))" 

typedef struct {
    unsigned int result;
    const char* key;
    unsigned int length;
} GPHashHashCharArrayFunc;

#define GPHashHashPtrFuncDef "gp.GPHashHashPtrFunc({gp.GPHashtable}\
    gp.GPHashHashPtrFunc.result (uint) \
    gp.GPHashHashPtrFunc.key (void*))"

typedef struct {
    unsigned int result;
    void* key;
} GPHashHashPtrFunc;

#define GPHashHashIntFuncDef "gp.GPHashHashIntFunc({gp.GPHashtable}\
    gp.GPHashHashIntFunc.result (uint) \
    gp.GPHashHashIntFunc.key (int))"

typedef struct {
    unsigned int result;
    int key;
} GPHashHashIntFunc;

#define GPHashHashFuncDef "gp.GPHashHashFunc({gp.GPHashtable}\
    gp.GPHashHashFunc.result (uint) \
    gp.GPHashHashFunc.table (gp.GPHashtable*) \
    gp.GPHashHashFunc.key (void*))"

typedef struct {
    unsigned int result;
    GPHashtable* table;
    void* key;
} GPHashHashFunc;

#define GPHashStringEqFuncDef "gp.GPHashStringEqFunc({gp.GPHashtable}\
    gp.GPHashStringEqFunc.result (uint) \
    gp.GPHashStringEqFunc.key1 (uchar*) \
    gp.GPHashStringEqFunc.key2 (uchar*))"

typedef struct {
    unsigned int result;
    const char* key1;
    const char* key2;
} GPHashStringEqFunc;

#define GPHashCharArrayEqFuncDef "gp.GPHashCharArrayEqFunc({gp.GPHashtable}\
    gp.GPHashCharArrayEqFunc.result (uint) \
    gp.GPHashCharArrayEqFunc.arrayKey (uchar*) \
    gp.GPHashCharArrayEqFunc.length (uint) \
    gp.GPHashCharArrayEqFunc.key2 (uchar*))"

typedef struct {
    unsigned int result;
    const char* arrayKey;
    unsigned int length;
    const char* key2;
} GPHashCharArrayEqFunc;

#define GPHashPtrEqFuncDef "gp.GPHashPtrEqFunc({gp.GPHashtable}\
    gp.GPHashPtrEqFunc.result (uint) \
    gp.GPHashPtrEqFunc.key1 (void*) \
    gp.GPHashPtrEqFunc.key2 (void*))"

typedef struct {
    unsigned int result;
    void* key1;
    void* key2;
} GPHashPtrEqFunc;

#define GPHashIntEqFuncDef "gp.GPHashIntEqFunc({gp.GPHashtable}\
    gp.GPHashIntEqFunc.result (uint) \
    gp.GPHashIntEqFunc.key1 (int) \
    gp.GPHashIntEqFunc.key2 (int))"

typedef struct {
    unsigned int result;
    int key1;
    int key2;
} GPHashIntEqFunc;

#define GPHashEqFuncDef "gp.GPHashEqFunc({gp.GPHashtable}\
    gp.GPHashEqFunc.result (uint) \
    gp.GPHashEqFunc.table (gp.GPHashtable*) \
    gp.GPHashEqFunc.key1 (void*) \
    gp.GPHashEqFunc.key2 (void*))"

typedef struct {
    unsigned int result;
    GPHashtable* table;
    void* key1;
    void* key2;
} GPHashEqFunc;


#define GPHashGetFuncDef "gp.GPHashGetFunc({gp.GPHashtable}\
    gp.GPHashGetFunc.result (void*) \
    gp.GPHashGetFunc.table (gp.GPHashtable*) \
    gp.GPHashGetFunc.key (void*))"

typedef struct {
    void* result;
    GPHashtable* table;
    void* key;
} GPHashGetFunc;

#define GPHashGetCharArrayFuncDef "gp.GPHashGetCharArrayFunc({gp.GPHashtable}\
    gp.GPHashGetCharArrayFunc.result (void*) \
    gp.GPHashGetCharArrayFunc.table (gp.GPHashtable*) \
    gp.GPHashGetCharArrayFunc.key (int))"

typedef struct {
    void* result;
    GPHashtable* table;
    const char* arrayKey;
    unsigned int length;
} GPHashGetCharArrayFunc;

#define GPHashGetNodeFuncDef "gp.GPHashGetNodeFunc({gp.GPHashtable}\
    gp.GPHashGetNodeFunc.result (void*) \
    gp.GPHashGetNodeFunc.table (gp.GPHashtable*) \
    gp.GPHashGetNodeFunc.key (void*))"

typedef struct {
    GPHashNode** result;
    GPHashtable* table;
    void* key;
} GPHashGetNodeFunc;

#define GPHashInsertFuncDef "gp.GPHashInsertFunc({gp.GPHashtable}\
    gp.GPHashInsertFunc.table (gp.GPHashtable*) \
    gp.GPHashInsertFunc.key (void*) \
    gp.GPHashInsertFunc.value (void*))"

typedef struct {
    GPHashtable* table;
    void* key;
    void* value;
} GPHashInsertFunc;

#define GPHashReplaceFuncDef "gp.GPHashReplaceFunc({gp.GPHashtable}\
    gp.GPHashReplaceFunc.result (void*) \
    gp.GPHashReplaceFunc.table (gp.GPHashtable*) \
    gp.GPHashReplaceFunc.key (void*) \
    gp.GPHashInsertFunc.value (void*))"

typedef struct {
    void* result;
    GPHashtable* table;
    void* key;
    void* value;
} GPHashReplaceFunc;

#define GPHashRemoveFuncDef "gp.GPHashRemoveFunc({gp.GPHashtable}\
    gp.GPHashRemoveFunc.result (void*) \
    gp.GPHashRemoveFunc.table (gp.GPHashtable*) \
    gp.GPHashRemoveFunc.key (void*))"

typedef struct {
    void* result;
    GPHashtable* table;
    void* key;
} GPHashRemoveFunc;

#define GPHashResizeFuncDef "gp.GPHashResizeFunc({gp.GPHashtable}\
    gp.GPHashResizeFunc.table (gp.GPHashtable*) \
    gp.GPHashResizeFunc.size (uint))"

typedef struct {
    GPHashtable* table;
    unsigned int size;
} GPHashResizeFunc;

#ifdef __cplusplus
}  /* end extern "C" */
#endif

#endif /*GPHashtable_h*/
