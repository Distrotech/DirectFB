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
 
#ifndef GPAtomicString_h
#define GPAtomicString_h
#include "GPType.h"

#ifdef __cplusplus
extern "C" {
#endif

#define GPAtomDef "gp.GPAtomFunc({uchar} \
    gp.GPAtomFunc.result(uchar*)      \
    gp.GPAtomFunc.atomName(uchar*)    \
    gp.GPAtomFunc.length(uint))       \
                                        \
    gp.GPIsAtomFunc({uchar}           \
    gp.GPIsAtomFunc.result(uchar*)    \
    gp.GPIsAtomFunc.atomName(uchar*))"


typedef struct {
    const char* result;
    const char* atomName;
    int length;
} GPAtomFunc;

/**
 * Make a string atomic is unique or atomic allowing ptr compares 
 * by interning in a string table a atomic string is never freed
 * if length is not zero only n chars will be interned.
 * A zero terminated string is returned.
 */
static inline const char* GP_atom(const char* string, int length)
{
    static GPType* atom;
    GPAtomFunc args ={result:NULL,atomName:string,length:length};
    if(!atom)
        atom = GPType_get("/gp/GPAtomFunc");
    /**
     * get type is a special case
     */
    atom->function(atom,&args);
    return args.result;
}


typedef struct {
    const char* result;
    const char* atomName;
} GPIsAtomFunc;

/**
 * Check if a string is atomic
 * returns NULL if not or the atomic
 * string
 */
static inline const char* GP_isAtom(const char* string)
{
    static GPType* isAtom;
    GPIsAtomFunc args ={result:NULL,atomName:string};
    if(!isAtom)
        isAtom = GPType_get("/gp/GPIsAtomFunc");
    /**
     * get type is a special case
     */
    isAtom->function(isAtom,&args);
    return args.result;
}

#ifdef __cplusplus
}  /* end extern "C" */
#endif

#endif /*GPAtomicString_h*/
