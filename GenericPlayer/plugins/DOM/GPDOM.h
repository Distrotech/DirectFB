/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 2; tab-width: 2 -*- */
/* gdome.c
 *
 * Copyright (C) 1999 Raph Levien <raph@acm.org>
 * Copyright (C) 2000 Mathieu Lacage <mathieu@gnu.org>
 * CopyRight (C) 2001 Paolo Casarini <paolo@casarini.org>
 *
 * This file is generated automatically.  To make changes, edit
 * test/apigen/core.xml
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef GPdom_h
#define GPdom_h
#include <GPRuntime.h>
#include <GPPlugin.h>

/*Based on DOM events*/

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Based on DOM Event
 * http://www.w3.org/TR/DOM-Level-2-Events/events.html
 * http://www.w3.org/TR/2003/NOTE-DOM-Level-3-Events-20031107/events.html
 */

#define GPDOMDef "\
\
dom.Plugin(\
    dom.Plugin.function(function)\
    dom.Plugin.impl(void*))\
\
    dom.Plugin.PluginFunc({dom.Plugin}\
            gp.GPPlugin/GPPluginFunc))\
\
    dom.Plugin.DestroyFunc({dom.Plugin}\
            dom.Plugin.DestroyFunc.plugin(dom.Plugin*))\
\
dom.Plugin.ProcessEventFunc({dom.Plugin})\
\
dom.Plugin.SetupInheritanceFunc({dom.Plugin}\
    dom.Plugin.SetupInheritanceFunc.typeName(uchar*))\
\
dom.Plugin.GetDOMImplementationRegisteryFunc({dom.Plugin}\
    dom.Plugin.GetDOMImplementationRegisteryFunc.result(dom.DOMImplementationRegistery*)\
    dom.Plugin.GetDOMImplementationRegisteryFunc.plugin(dom.GPPlugin*))\
\
dom.DOMTimeStamp(ullong)\
\
dom.utf8(uchar*)\
\
dom.DOMString(\
    dom.DOMString.data(ushort*)\
    dom.DOMString.length(uint))\
\
dom.DOMStringList((dom.Object)\
dom.DOMStringList.length(ulong))\
\
dom.DOMStringList.ItemFunc({dom.DOMStringList}\
    dom.DOMStringList.ItemFunc.result(dom.utf8*)\
    dom.DOMStringList.ItemFunc.stringList(dom.DOMStringList*)\
    dom.DOMStringList.ItemFunc.index(ulong))\
\
dom.DOMStringList.ContainsFunc({dom.DOMStringList}\
    dom.DOMStringList.ContainsFunc.result(bool)\
    dom.DOMStringList.ContainsFunc.stringList(dom.DOMStringList*)\
    dom.DOMStringList.ContainsFunc.str(dom.utf8*))\
\
dom.Object(\
    dom.Object.gpType(dom.GPType*))\
    dom.Object.impl(void*))\
\
dom.Object.GetMethodFunc({dom.Object}\
dom.Object.GetMethodFunc.result(gp.GPType*)\
dom.Object.GetMethodFunc.owner(gp.GPType*)\
dom.Object.GetMethodFunc.signature(uchar*))\
\
dom.Object.GetMemberFunc({dom.Object}\
dom.Object.GetMemberFunc.result(gp.GPType*)\
dom.Object.GetMemberFunc.owner(gp.GPType*)\
dom.Object.GetMemberFunc.signature(uchar*))\
\
dom.DOMObject(\
    dom.DOMObject.super(dom.Object))\
dom.DOMObject.GetMethodFunc({dom.DOMObject}\
    dom.Object.GetMethodFunc)\
\
dom.DOMObject.GetMemberFunc({dom.DOMObject}\
    dom.Object.GetMemberFunc)\
\
dom.DOMUserData(void*)\
\
dom.DOMUserData.UserDataHandlerFunc({dom.DOMUserData}\
    dom.DOMUserData.UserDataHandlerFunc.operation(ushort)\
    dom.DOMUserData.UserDataHandlerFunc.key(uchar*)\
    dom.DOMUserData.UserDataHandlerFunc.data(dom.DOMUserData*)\
    dom.DOMUserData.UserDataHandlerFunc.src(dom.Node*)\
    dom.DOMUserData.UserDataHandlerFunc.dst(dom.Node*))\
\
dom.Node(\
    dom.Node.super(dom.Object))\
\
dom.DefaultNode(\
    dom.DefaultNode.super(dom.Node)\
    dom.DefaultNode.tree(gp.GPTreeNode*)\
    dom.DefaultNode.localName(dom.utf8*))\
\
dom.Node.GetMethodFunc({dom.Node}\
    dom.Object.GetMethodFunc)\
\
dom.Node.GetMemberFunc({dom.Node}\
    dom.Object.GetMemberFunc)\
\
dom.Node.GetNodeNameFunc({dom.Node}\
        dom.Node.GetNodeNameFunc.result(dom.utf8*)\
        dom.Node.GetNodeNameFunc.node(dom.Node*))\
\
dom.Node.GetNodeValueFunc({dom.Node}\
        dom.Node.GetNodeValueFunc.result(dom.utf8*)\
        dom.Node.GetNodeValueFunc.exception(dom.DOMException*)\
        dom.Node.GetNodeValueFunc.node(dom.Node*))\
\
dom.Node.SetNodeValueFunc({dom.Node}\
        dom.Node.SetNodeValueFunc.exception(dom.DOMException*)\
        dom.Node.SetNodeValueFunc.node(dom.Node*)\
        dom.Node.SetNodeValueFunc.name(dom.utf8*))\
\
dom.Node.GetNodeTypeFunc({dom.Node}\
        dom.Node.GetNodeTypeFunc.result(ushort)\
        dom.Node.GetNodeTypeFunc.node(dom.Node*))\
\
dom.Node.GetParentNodeFunc({dom.Node}\
        dom.Node.GetParentNodeFunc.result(dom.Node*)\
        dom.Node.GetParentNodeFunc.node(dom.Node*))\
\
dom.Node.GetChildNodesFunc({dom.Node}\
        dom.Node.GetChildNodesFunc.result(dom.NodeList*)\
        dom.Node.GetChildNodesFunc.node(dom.Node*))\
\
dom.Node.GetFirstChildFunc({dom.Node}\
        dom.Node.FirstChildFunc.result(dom.Node*)\
        dom.Node.FirstChildFunc.node(dom.Node*))\
\
dom.Node.GetLastChildFunc({dom.Node}\
        dom.Node.GetLastChildFunc.result(dom.Node*)\
        dom.Node.GetLastChildFunc.node(dom.Node*))\
\
dom.Node.GetPreviousSiblingFunc({dom.Node}\
        dom.Node.GetPreviousSiblingFunc.result(dom.Node*)\
        dom.Node.GetPreviousSiblingFunc.node(dom.Node*))\
\
dom.Node.GetNextSiblingFunc({dom.Node}\
        dom.Node.GetNextSiblingFunc.result(dom.Node*)\
        dom.Node.GetNextSiblingFunc.node(dom.Node*))\
\
dom.Node.GetAttributesFunc({dom.Node}\
        dom.Node.GetAttributesFunc.result(dom.NamedNodeMap*)\
        dom.Node.GetAttributesFunc.node(dom.Node*))\
\
dom.Node.GetOwnerDocumentFunc({dom.Node}\
        dom.Node.GetOwnerDocumentFunc.result(dom.Node*)\
        dom.Node.GetOwnerDocumentFunc.node(dom.Node*))\
\
dom.Node.InsertBeforeFunc({dom.Node}\
        dom.Node.InsertBeforeFunc.result(dom.Node*)\
        dom.Node.InsertBeforeFunc.exception(dom.DOMException*)\
        dom.Node.InsertBeforeFunc.node(dom.Node*)\
        dom.Node.InsertBeforeFunc.newChild(dom.Node*)\
        dom.Node.InsertBeforeFunc.refChild(dom.Node*))\
\
dom.Node.ReplaceChildFunc({dom.Node}\
        dom.Node.ReplaceChildFunc.result(dom.Node*)\
        dom.Node.ReplaceChildFunc.exception(dom.DOMException*)\
        dom.Node.ReplaceChildFunc.node(dom.Node*)\
        dom.Node.ReplaceChildFunc.newChild(dom.Node*)\
        dom.Node.ReplaceChildFunc.oldChild(dom.Node*))\
\
dom.Node.RemoveChildFunc({dom.Node}\
        dom.Node.RemoveChildFunc.result(dom.Node*)\
        dom.Node.RemoveChildFunc.exception(dom.DOMException*)\
        dom.Node.RemoveChildFunc.node(dom.Node*)\
        dom.Node.RemoveChildFunc.oldChild(dom.Node*))\
\
dom.Node.AppendChildFunc({dom.Node}\
        dom.Node.AppendChildFunc.result(dom.Node*)\
        dom.Node.AppendChildFunc.exception(dom.DOMException*)\
        dom.Node.AppendChildFunc.node(dom.Node*)\
        dom.Node.AppendChildFunc.newChild(dom.Node*))\
\
dom.Node.HasChildNodesFunc({dom.Node}\
        dom.Node.HasChildNodesFunc.result(bool)\
        dom.Node.HasChildNodesFunc.node(dom.Node*))\
\
dom.Node.CloneNodeFunc({dom.Node}\
        dom.Node.CloneNodeFunc.result(dom.Node*)\
        dom.Node.CloneNodeFunc.node(dom.Node*)\
        dom.Node.CloneNodeFunc.deep(bool))\
\
dom.Node.NormalizeFunc({dom.Node}\
        dom.Node.NormalizeFunc.node(dom.Node*))\
\
dom.Node.IsSupportedFunc({dom.Node}\
        dom.Node.IsSupportedFunc.result(bool)\
        dom.Node.IsSupportedFunc.node(dom.Node*)\
        dom.Node.IsSupportedFunc.feature(dom.utf8*)\
        dom.Node.IsSupportedFunc.version(dom.utf8*))\
\
dom.Node.GetNamespaceURIFunc({dom.Node}\
        dom.Node.GetNamespaceURIFunc.result(dom.utf8*)\
        dom.Node.GetNamespaceURIFunc.node(dom.Node*))\
\
dom.Node.GetPrefixFunc({dom.Node}\
        dom.Node.GetPrefixFunc.result(dom.utf8*)\
        dom.Node.GetPrefixFunc.node(dom.Node*))\
\
dom.Node.SetPrefixFunc({dom.Node}\
        dom.Node.SetPrefixFunc.exception(dom.DOMException*)\
        dom.Node.SetPrefixFunc.node(dom.Node*)\
        dom.Node.SetPrefixFunc.prefix(dom.utf8*))\
\
dom.Node.GetLocalNameFunc({dom.Node}\
        dom.Node.GetLocalNameFunc.result(dom.utf8*)\
        dom.Node.GetLocalNameFunc.node(dom.Node*))\
\
dom.Node.HasAttributesFunc({dom.Node}\
        dom.Node.HasAttributesFunc.result(bool)\
        dom.Node.GetLocalNameFunc.node(dom.Node*))\
\
dom.Node.GetBaseURIFunc({dom.Node}\
        dom.Node.GetBaseURIFunc.result(dom.utf8*)\
        dom.Node.GetBaseURIFunc.node(dom.Node*))\
\
dom.Node.CompareDocumentPositionFunc({dom.Node}\
        dom.Node.CompareDocumentPositionFunc.result(ushort)\
        dom.Node.CompareDocumentPositionFunc.node(dom.Node*))\
\
dom.Node.GetTextContentFunc({dom.Node}\
        dom.Node.GetTextContentFunc.result(dom.utf8*)\
        dom.Node.GetTextContentFunc.exception(dom.DOMException*)\
        dom.Node.GetTextContentFunc.node(dom.Node*))\
\
dom.Node.SetTextContentFunc({dom.Node}\
        dom.Node.SetTextContentFunc.textContent(dom.utf8*)\
        dom.Node.GetTextContentFunc.exception(dom.DOMException*)\
        dom.Node.SetTextContentFunc.node(dom.Node*))\
\
dom.Node.IsSameNodeFunc({dom.Node}\
        dom.Node.IsSameNodeFunc.result(bool)\
        dom.Node.IsSameNodeFunc.node(dom.Node*)\
        dom.Node.IsSameNodeFunc.other(dom.Node*))\
\
dom.Node.LookupPrefixFunc({dom.Node}\
        dom.Node.LookupPrefixFunc.result(dom.utf8*)\
        dom.Node.LookupPrefixFunc.node(dom.Node*)\
        dom.Node.LookupPrefixFunc.namespaceURI(dom.utf8*))\
\
dom.Node.IsDefaultNamespaceFunc({dom.Node}\
        dom.Node.IsDefaultNamespaceFunc.result(bool)\
        dom.Node.IsDefaultNamespaceFunc.node(dom.Node*)\
        dom.Node.IsDefaultNamespaceFunc.namespaceURI(dom.utf8*))\
\
dom.Node.LookupNamespaceURIFunc({dom.Node}\
        dom.Node.IsDefaultNamespaceFunc.result(dom.utf8*)\
        dom.Node.IsDefaultNamespaceFunc.node(dom.Node*)\
        dom.Node.IsDefaultNamespaceFunc.prefix(dom.utf8*))\
\
dom.Node.IsEqualNodeFunc({dom.Node}\
        dom.Node.IsEqualNodeFunc.result(bool)\
        dom.Node.IsEqualNodeFunc.node(dom.Node*)\
        dom.Node.IsEqualNodeFunc.arg(Node*))\
\
dom.Node.GetFeatureFunc({dom.Node}\
        dom.Node.GetFeatureFunc.result(dom.DOMObject*)\
        dom.Node.GetFeatureFunc.node(dom.Node*)\
        dom.Node.GetFeatureFunc.feature(dom.utf8*)\
        dom.Node.GetFeatureFunc.version(dom.utf8*))\
\
dom.Node.SetUserDataFunc({dom.Node}\
        dom.Node.SetUserDataFunc.result(dom.DOMUserData*)\
        dom.Node.SetUserDataFunc.node(dom.Node*)\
        dom.Node.SetUserDataFunc.key(dom.utf8*)\
        dom.Node.SetUserDataFunc.data(dom.DOMUserData*)\
        dom.Node.SetUserDataFunc.handler(dom.UserDataHandler*))\
\
dom.Node.GetUserDataFunc({dom.Node}\
        dom.Node.GetUserDataFunc.result(dom.DOMUserData*)\
        dom.Node.GetUserDataFunc.node(dom.Node*)\
        dom.Node.GetUserDataFunc.key(dom.utf8*))\
\
dom.NodeList(\
    dom.NodeList.impl(void*))\
\
dom.NodeList.GetLengthFunc({dom.NodeList}\
    dom.NodeList.GetLengthFunc.result(ulong)\
    dom.NodeList.GetLengthFunc.characterData(dom.NodeList*))\
\
dom.NodeList.ItemFunc({dom.NodeList}\
        dom.NodeList.ItemFunc.result(dom.Node)\
        dom.NodeList.ItemFunc.list(dom.NodeList*)\
        dom.NodeList.ItemFunc.index(ulong))\
\
dom.NamedNodeMap(\
  (dom.Object))\
\
dom.NamedNodeMap.GetLengthFunc({dom.NamedNodeMap}\
    dom.NamedNodeMap.GetLengthFunc.result(ulong)\
    dom.NamedNodeMap.GetLengthFunc.characterData(dom.NamedNodeMap*))\
\
dom.NamedNodeMap.GetNamedItemFunc({dom.NamedNodeMap}\
        dom.NamedNodeMap.GetNamedItemFunc.result(dom.Node*)\
        dom.NamedNodeMap.GetNamedItemFunc.map(dom.NamedNodeMap*)\
        dom.NamedNodeMap.GetNamedItemFunc.name(dom.utf8*))\
\
dom.NamedNodeMap.SetNamedItemFunc({dom.NamedNodeMap}\
        dom.NamedNodeMap.SetNamedItemFunc.result(dom.Node*)\
        dom.NamedNodeMap.SetNamedItemFunc.exception(dom.DOMException*)\
        dom.NamedNodeMap.SetNamedItemFunc.map(dom.NamedNodeMap*)\
        dom.NamedNodeMap.SetNamedItemFunc.arg(dom.Node*))\
\
dom.NamedNodeMap.RemoveNamedItemFunc({dom.NamedNodeMap}\
        dom.NamedNodeMap.RemoveNamedItemFunc.result(dom.Node*)\
        dom.NamedNodeMap.RemoveNamedItemFunc.exception(dom.DOMException*)\
        dom.NamedNodeMap.RemoveNamedItemFunc.map(dom.NamedNodeMap*)\
        dom.NamedNodeMap.RemoveNamedItemFunc.name(dom.utf8*))\
\
dom.NamedNodeMap.ItemFunc({dom.NamedNodeMap}\
        dom.NamedNodeMap.ItemFunc.result(dom.Node*)\
        dom.NamedNodeMap.ItemFunc.map(dom.NamedNodeMap*)\
        dom.NamedNodeMap.ItemFunc.index(ulong))\
\
dom.NamedNodeMap.GetNamedItemNSFunc({dom.NamedNodeMap}\
        dom.NamedNodeMap.GetNamedItemNSFunc.result(dom.Node*)\
        dom.NamedNodeMap.GetNamedItemNSFunc.exception(dom.DOMException*)\
        dom.NamedNodeMap.GetNamedItemFunc.map(dom.NamedNodeMap*)\
        dom.NamedNodeMap.GetNamedItemNSFunc.namespaceURI(dom.utf8*)\
        dom.NamedNodeMap.GetNamedItemNSFunc.localName(dom.utf8*))\
\
dom.NamedNodeMap.SetNamedItemNSFunc({dom.NamedNodeMap}\
        dom.NamedNodeMap.SetNamedItemNSFunc.result(dom.Node*)\
        dom.NamedNodeMap.SetNamedItemNSFunc.exception(dom.DOMException*)\
        dom.NamedNodeMap.SetNamedItemFunc.map(dom.NamedNodeMap*)\
        dom.NamedNodeMap.SetNamedItemNSFunc.arg(dom.Node*))\
\
dom.NamedNodeMap.RemoveNamedItemNSFunc({dom.NamedNodeMap}\
        dom.NamedNodeMap.RemoveNamedItemNSFunc.result(dom.Node*)\
        dom.NamedNodeMap.RemoveNamedItemNSFunc.exception(dom.DOMException*)\
        dom.NamedNodeMap.RemoveNamedItemFunc.map(dom.NamedNodeMap*)\
        dom.NamedNodeMap.RemoveNamedItemNSFunc.namespaceURI(dom.utf8*)\
        dom.NamedNodeMap.RemoveNamedItemNSFunc.localName(dom.utf8*))\
\
dom.CharacterData(dom.Node)\
\
dom.CharacterData.GetDataFunc({dom.CharacterData}\
    dom.CharacterData.GetDataFunc.result(dom.utf8*)\
    dom.CharacterData.GetDataFunc.exception(dom.DOMException*)\
    dom.CharacterData.GetDataFunc.characterData(dom.CharacterData*))\
\
dom.CharacterData.SetDataFunc({dom.CharacterData}\
    dom.CharacterData.SetDataFunc.exception(dom.DOMException*)\
    dom.CharacterData.SetDataFunc.characterData(dom.CharacterData*)\
    dom.CharacterData.SetDataFunc.data(dom.utf8*))\
\
dom.CharacterData.GetLengthFunc({dom.CharacterData}\
    dom.CharacterData.GetLengthFunc.result(ulong)\
    dom.CharacterData.GetLengthFunc.characterData(dom.CharacterData*))\
\
dom.CharacterData.SubstringDataFunc({dom.CharacterData}\
    dom.CharacterData.SubstringDataFunc.result(dom.utf8*)\
    dom.CharacterData.SubstringDataFunc.exception(dom.DOMException*)\
    dom.CharacterData.SubstringDataFunc.characterData(dom.CharacterData*)\
    dom.CharacterData.SubstringDataFunc.offset(ulong)\
    dom.CharacterData.SubstringDataFunc.count(ulong))\
\
dom.CharacterData.AppendDataFunc({dom.CharacterData}\
    dom.CharacterData.AppendDataFunc.exception(dom.DOMException*)\
    dom.CharacterData.AppendDataFunc.characterData(dom.CharacterData*)\
    dom.CharacterData.AppendDataFunc.arg(dom.utf8*))\
\
dom.CharacterData.InsertDataFunc({dom.CharacterData}\
    dom.CharacterData.InsertDataFunc.exception(dom.DOMException*)\
    dom.CharacterData.InsertDataFunc.characterData(dom.CharacterData*)\
    dom.CharacterData.InsertDataFunc.offset(ulong))\
    dom.CharacterData.InsertDataFunc.arg(dom.utf8*))\
\
dom.CharacterData.DeleteDataFunc({dom.CharacterData}\
    dom.CharacterData.DeleteDataFunc.exception(dom.DOMException*)\
    dom.CharacterData.DeleteDataFunc.characterData(dom.CharacterData*)\
    dom.CharacterData.DeleteDataFunc.offset(ulong)\
    dom.CharacterData.DeleteDataFunc.count(ulong))\
\
dom.CharacterData.ReplaceDataFunc({dom.CharacterData}\
    dom.CharacterData.ReplaceDataFunc.exception(dom.DOMException*)\
    dom.CharacterData.ReplaceDataFunc.characterData(dom.CharacterData*)\
    dom.CharacterData.ReplaceDataFunc.offset(ulong)\
    dom.CharacterData.ReplaceDataFunc.count(ulong)\
    dom.CharacterData.ReplaceDataFunc.arg(dom.utf8*))\
\
dom.Text(dom.CharacterData)\
\
dom.Text.SplitTextFunc({dom.Text}\
    dom.Text.SplitTextFunc.result(dom.Text*)\
    dom.Text.SplitTextFunc.exception(dom.DOMException*)\
    dom.Text.SplitTextFunc.text(dom.Text*)\
    dom.Text.SplitTextFunc.offset(ulong))\
\
dom.Text.GetIsElementContentWhitespaceFunc({dom.Text}\
    dom.Text.GetIsElementContentWhitespaceFunc.result(bool)\
    dom.Text.GetIsElementContentWhitespaceFunc.text(dom.Text*))\
\
dom.Text.GetWholeTextFunc({dom.Text}\
    dom.Text.GetWholeTextFunc.result(dom.utf8*)\
    dom.Text.GetWholeTextFunc.text(dom.Text*))\
\
dom.Text.ReplaceWholeTextFunc({dom.Text}\
    dom.Text.ReplaceWholeTextFunc.result(dom.utf8*)\
    dom.Text.ReplaceWholeTextFunc.exception(dom.DOMException*)\
    dom.Text.ReplaceWholeTextFunc.text(dom.Text*)\
    dom.Text.ReplaceWholeTextFunc.content(dom.utf8*))\
\
dom.Comment(dom.Text)\
\
dom.CDATASection(dom.Text)\
\
dom.Attr(Node))\
\
dom.Attr.GetNameFunc({dom.Attr}\
    dom.Attr.GetNameFunc.result(dom.utf8*)\
    dom.Attr.GetNameFunc.attr(dom.Attr*))\
\
dom.Attr.GetSpecifiedFunc({dom.Attr}\
    dom.Attr.GetSpecifiedFunc.result(bool)\
    dom.Attr.GetSpecifiedFunc.attr(dom.Attr*))\
\
dom.Attr.GetValueFunc({dom.Attr}\
    dom.Attr.GetValueFunc.result(dom.utf8*)\
    dom.Attr.GetValueFunc.attr(dom.Attr*))\
\
dom.Attr.SetValueFunc({dom.Attr}\
    dom.Attr.SetValueFunc.exception(dom.DOMException*)\
    dom.Attr.SetValueFunc.attr(dom.Attr*)\
    dom.Attr.SetValueFunc.value(dom.utf8*))\
\
dom.Attr.GetTypedValueFunc({dom.Attr}\
    dom.Attr.GetTypeValueFunc.result(vod*)\
    dom.Attr.GetTypeValueFunc.resultType(gp.GPType*)\
    dom.Attr.GetTypeValueFunc.attr(dom.Attr*))\
    dom.Attr.GetTypeValueFunc.inType(gp.GPType*))\
\
dom.Attr.SetTypedValueFunc({dom.Attr}\
    dom.Attr.SetTypedValueFunc.exception(dom.DOMException*)\
    dom.Attr.SetTypedValueFunc.attr(dom.Attr*)\
    dom.Attr.SetTypedValueFunc.inType(gp.GPType*)\
    dom.Attr.SetTypedValueFunc.value(void*))\
\
dom.Attr.GetOwnerElementFunc({dom.Attr}\
    dom.Attr.GetOwnerElementFunc.result(dom.Element*)\
    dom.Attr.GetOwnerElementFunc.attr(dom.Attr*))\
\
dom.Attr.GetSchemaTypeInfoFunc({dom.Attr}\
    dom.Attr.GetSchemaTypeInfoFunc.result(dom.TypeInfo*)\
    dom.Attr.GetSchemaTypeInfoFunc.attr(dom.Attr*))\
\
dom.Attr.IsIdFunc({dom.Attr}\
    dom.Attr.IsIdFunc.result(bool)\
    dom.Attr.IsIdFunc.attr(dom.Attr*))\
\
dom.TypeInfo(\
  dom.TypeInfo.typeName(dom.utf8*)\
  dom.TypeInfo.typeNamespace(dom.utf8*))\
dom.TypeInfo.IsDerivedFromFunc({dom.TypeInfo}\
    dom.TypeInfo.IsDerivedFromFunc.result(bool)\
    dom.TypeInfo.IsDerivedFromFunc.type(dom.TypeInfo*)\
    dom.TypeInfo.IsDerivedFromFunc.typeNamespaceArg(dom.utf8*)\
    dom.TypeInfo.IsDerivedFromFunc.typeNameArg(dom.utf8*)\
    dom.TypeInfo.IsDerivedFromFunc.derivationMethod(ulong))\
\
dom.Element(dom.Element.super(dom.Node))\
\
dom.DefaultElement(dom.DefaultElement.super(dom.DefaultNode))\
\
dom.Element.GetTagNameFunc({dom.Element}\
    dom.Element.GetTagNameFunc.result(dom.utf8*)\
    dom.Element.GetTagNameFunc.element(dom.Element*))\
\
dom.Element.GetAttributeFunc({dom.Element}\
    dom.Element.GetAttributeFunc.result(utf8*)\
    dom.Element.GetAttributeFunc.element(dom.Element*)\
    dom.Element.GetAttributeFunc.name(dom.utf8*))\
\
dom.Element.SetAttributeFunc({dom.Element}\
    dom.Element.SetAttributeFunc.exception(dom.DOMException*)\
    dom.Element.SetAttributeFunc.element(dom.Element*)\
    dom.Element.SetAttributeFunc.name(dom.utf8*)\
    dom.Element.SetAttributeFunc.value(dom.utf8*))\
\
dom.Element.GetTypedAttributeFunc({dom.Element}\
    dom.Element.GetTypedAttributeFunc.result(void*)\
    dom.Element.GetTypedAttributeFunc.resultType(gp.GPType*)\
    dom.Element.GetTypedAttributeFunc.exception(dom.DOMException*)\
    dom.Element.GetTypedAttributeFunc.element(dom.Element*)\
    dom.Element.GetTypedAttributeFunc.name(dom.utf8*)\
    dom.Element.GetTypedAttributeFunc.inType(gp.GPType*))\
\
dom.Element.SetTypedAttributeFunc({dom.Element}\
    dom.Element.SetTypedAttributeFunc.exception(dom.DOMException*)\
    dom.Element.SetTypedAttributeFunc.element(dom.Element*)\
    dom.Element.SetTypedAttributeFunc.name(dom.utf8*)\
    dom.Element.SetTypedAttributeFunc.inType(gp.GPType*))\
    dom.Element.SetTypedAttributeFunc.value(void*))\
\
dom.Element.RemoveAttributeFunc({dom.Element}\
    dom.Element.RemoveAttributeFunc.exception(dom.DOMException*)\
    dom.Element.RemoveAttributeFunc.element(dom.Element*)\
    dom.Element.RemoveAttributeFunc.name(dom.utf8*))\
\
dom.Element.GetAttributeNodeFunc({dom.Element}\
    dom.Element.GetAttributeNodeFunc.result(dom.Attr*)\
    dom.Element.GetAttributeNodeFunc.element(dom.Element*)\
    dom.Element.GetAttributeNodeFunc.name(dom.utf8*))\
\
dom.Element.SetAttributeNodeFunc({dom.Element}\
    dom.Element.SetAttributeNodeFunc.exception(dom.DOMException*)\
    dom.Element.SetAttributeNodeFunc.element(dom.Element*)\
    dom.Element.SetAttributeNodeFunc.newAttr(dom.Attr*))\
\
dom.Element.RemoveAttributeNodeFunc({dom.Element}\
    dom.Element.RemoveAttributeNodeFunc.exception(dom.DOMException*)\
    dom.Element.RemoveAttributeNodeFunc.element(dom.Element*)\
    dom.Element.RemoveAttributeNodeFunc.oldAttr(dom.Attr*))\
\
dom.Element.GetElementsByTagNameFunc({dom.Element}\
    dom.Element.GetElementsByTagNameFunc.result(dom.NodeList*)\
    dom.Element.GetElementsByTagNameFunc.element(dom.Element*)\
    dom.Element.GetElementsByTagNameFunc.name(dom.utf8*))\
\
dom.Element.GetAttributeNSFunc({dom.Element}\
    dom.Element.GetAttributeNSFunc.result(utf8*)\
    dom.Element.GetAttributeNSFunc.element(dom.Element*)\
    dom.Element.GetAttributeNSFunc.namespaceURI(dom.utf8*)\
    dom.Element.GetAttributeNSFunc.localName(dom.utf8*))\
\
dom.Element.GetTypedAttributeNSFunc({dom.Element}\
    dom.Element.GetTypedAttributeNSFunc.result(void*)\
    dom.Element.GetTypedAttributeNSFunc.resultType(gp.GPType*)\
    dom.Element.GetTypedAttributeNSFunc.exception(dom.DOMException*)\
    dom.Element.GetTypedAttributeNSFunc.element(dom.Element*)\
    dom.Element.GetAttributeNSFunc.namespaceURI(dom.utf8*)\
    dom.Element.GetTypedAttributeNSFunc.localName(dom.utf8*)\
    dom.Element.GetTypedAttributeNSFunc.inType(gp.GPType*))\
\
dom.Element.SetAttributeNSFunc({dom.Element}\
    dom.Element.SetAttributeNSFunc.exception(dom.DOMException*)\
    dom.Element.SetAttributeNSFunc.element(dom.Element*)\
    dom.Element.SetAttributeNSFunc.namespaceURI(dom.utf8*)\
    dom.Element.SetAttributeNSFunc.qualifiedName(dom.utf8*)\
    dom.Element.SetAttributeNSFunc.value(dom.utf8*))\
\
dom.Element.SetTypedAttributeNSFunc({dom.Element}\
    dom.Element.SetTypedAttributeNSFunc.exception(dom.DOMException*)\
    dom.Element.SetTypedAttributeNSFunc.element(dom.Element*)\
    dom.Element.SetAttributeNSFunc.namespaceURI(dom.utf8*)\
    dom.Element.SetTypedAttributeNSFunc.qualifiedName(dom.utf8*)\
    dom.Element.SetTypedAttributeNSFunc.inType(gp.GPType*))\
    dom.Element.SetTypedAttributeNSFunc.value(void*))\
\
dom.Element.RemoveAttributeNSFunc({dom.Element}\
    dom.Element.RemoveAttributeNSFunc.exception(dom.DOMException*)\
    dom.Element.RemoveAttributeNSFunc.element(dom.Element*)\
    dom.Element.RemoveAttributeNSFunc.namespaceURI(dom.utf8*)\
    dom.Element.RemoveAttributeNSFunc.localName(dom.utf8*))\
\
dom.Element.GetAttributeNodeNS({dom.Element}\
    dom.Element.GetAttributeNodeNS.result(dom.Attr*)\
    dom.Element.GetAttributeNodeNS.exception(dom.DOMException*)\
    dom.Element.GetAttributeNodeNS.element(dom.Element*)\
    dom.Element.GetAttributeNodeNS.namespaceURI(dom.utf8*)\
    dom.Element.GetAttributeNodeNS.localName(dom.utf8*))\
\
dom.Element.SetAttributeNodeNS({dom.Element}\
    dom.Element.SetAttributeNodeNS.result(dom.Attr*)\
    dom.Element.SetAttributeNodeNS.exception(dom.DOMException*)\
    dom.Element.SetAttributeNodeNS.element(dom.Element*)\
    dom.Element.SetAttributeNodeNS.newAttr(dom.Attr*))\
\
dom.Element.GetElementsByTagNameNS({dom.Element}\
    dom.Element.GetElementsByTagNameNS.result(dom.NodeList*)\
    dom.Element.GetElementsByTagNameNS.exception(dom.DOMException*)\
    dom.Element.GetElementsByTagNameNS.element(dom.Element*)\
    dom.Element.GetElementsByTagNameNS.namespaceURI(dom.utf8*)\
    dom.Element.GetElementsByTagNameNS.localName(dom.utf8*))\
\
dom.Element.HasAttributeFunc({dom.Element}\
    dom.Element.HasAttributeFunc.result(bool)\
    dom.Element.HasAttributeFunc.element(dom.Element*)\
    dom.Element.HasAttributeFunc.name(dom.utf8*))\
\
dom.Element.HasAttributeNSFunc({dom.Element}\
    dom.Element.HasAttributeNSFunc.result(bool)\
    dom.Element.HasAttributeNSFunc.element(dom.Element*)\
    dom.Element.HasAttributeNSFunc.namespaceURI(dom.utf8*)\
    dom.Element.HasAttributeNSFunc.localName(dom.utf8*))\
\
dom.Element.GetSchemaTypeInfoFunc({dom.Element}\
    dom.Element.GetSchemaTypeInfoFunc.result(dom.TypeInfo*)\
    dom.Element.GetSchemaTypeInfoFunc.element(dom.Element*))\
\
dom.Element.SetIdAttribute({dom.Element}\
    dom.Element.SetIdAttributeFunc.exception(dom.DOMException*))\
    dom.Element.SetIdAttributeFunc.element(dom.Element*))\
    dom.Element.SetIdAttributeFunc.name(dom.utf8*)\
    dom.Element.SetIdAttributeFunc.isId(bool))\
\
dom.Element.SetIdAttributeNSFunc({dom.Element}\
    dom.Element.SetIdAttributeNSFunc.exception(dom.DOMException*))\
    dom.Element.SetIdAttributeNSFunc.element(dom.Element*))\
    dom.Element.SetIdAttributeNSFunc.namespaceURI(dom.utf8*)\
    dom.Element.SetIdAttributeNSFunc.localName(dom.utf8*)\
    dom.Element.SetIdAttributeNSFunc.isId(bool))\
\
dom.Element.SetIdAttributeNodeFunc({dom.Element}\
    dom.Element.SetIdAttributeNodeFunc.exception(dom.DOMException*))\
    dom.Element.SetIdAttributeNodeFunc.element(dom.Element*))\
    dom.Element.SetIdAttributeNodeFunc.idAttr(dom.Attr*)\
    dom.Element.SetIdAttributeNodeFunc.isId(bool))\
\
dom.Document(\
    dom.Document.super(dom.Node))\
\
dom.DefaultDocument(\
    dom.DefaultDocument.super(dom.DefaultNode))\
\
dom.Document.GetMethodFunc({dom.Document}\
    dom.Object.GetMethodFunc)\
\
dom.Document.GetMemberFunc({dom.Document}\
    dom.Object.GetMemberFunc)\
\
dom.Document.GetDocumentViewFunc({dom.Document}\
    dom.Document.GetDocumentViewFunc.result(dom.views.DocumentView*)\
    dom.Document.GetDocumentViewFunc.document(dom.Document*))\
\
dom.Document.GetDocumentTypeFunc({dom.Document}\
    dom.Document.GetDocumentTypeFunc.result(dom.DocumentType*)\
    dom.Document.GetDocumentTypeFunc.document(dom.Document*))\
\
dom.Document.GetDOMImplementationFunc({dom.Document}\
    dom.Document.GetDOMImplementationFunc.result(dom.DOMImplementation*)\
    dom.Document.GetDOMImplementationFunc.document(dom.Document*))\
\
dom.Document.GetDocumentElementFunc({dom.Document}\
    dom.Document.GetDocumentElementFunc.result(dom.Element*)\
    dom.Document.GetDocumentElementFunc.document(dom.Document*))\
\
dom.Document.CreateElementFunc({dom.Document}\
    dom.Document.CreateElementFunc.result(dom.Element*)\
    dom.Document.CreateElementFunc.exception(dom.DOMException*)\
    dom.Document.CreateElementFunc.document(dom.Document*)\
    dom.Document.CreateElementFunc.tagName(dom.utf8*))\
\
dom.Document.CreateDocumentFragmentFunc({dom.Document}\
    dom.Document.CreateDocumentFragmentFunc.result(dom.DocumentFragment*)\
    dom.Document.CreateDocumentFragmentFunc.document(dom.Document*))\
\
dom.Document.CreateTextNodeFunc({dom.Document}\
    dom.Document.CreateTextNodeFunc.result(dom.Text*)\
    dom.Document.CreateTextNodeFunc.document(dom.Document*)\
    dom.Document.CreateTextNodeFunc.data(dom.utf8*))\
\
dom.Document.CreateCommentFunc({dom.Document}\
    dom.Document.CreateCommentFunc.result(dom.Comment*)\
    dom.Document.CreateCommentFunc.document(dom.Document*)\
    dom.Document.CreateCommentFunc.data(dom.utf8*))\
\
dom.Document.CreateCDATASectionFunc({dom.Document}\
    dom.Document.CreateCDATASectionFunc.result(dom.CDATASection*)\
    dom.Document.CreateCDATASectionFunc.exception(dom.DOMException*)\
    dom.Document.CreateCDATASectionFunc.document(dom.Document*)\
    dom.Document.CreateCDATASectionFunc.data(dom.utf8*))\
\
dom.Document.CreateProcessingInstructionFunc({dom.Document}\
    dom.Document.CreateProcessingInstructionFunc.result(dom.Comment*)\
    dom.Document.CreateProcessingInstructionFunc.exception(dom.DOMException*)\
    dom.Document.CreateProcessingInstructionFunc.document(dom.Document*)\
    dom.Document.CreateProcessingInstructionFunc.target(dom.utf8*)\
    dom.Document.CreateProcessingInstructionFunc.data(dom.utf8*))\
\
dom.Document.CreateAttributeFunc({dom.Document}\
    dom.Document.CreateAttributeFunc.result(dom.Attr*)\
    dom.Document.CreateAttributeFunc.exception(dom.DOMException*)\
    dom.Document.CreateAttributeFunc.document(dom.Document*)\
    dom.Document.CreateAttributeFunc.name(dom.utf8*))\
\
dom.Document.CreateEntityReferenceFunc({dom.Document}\
    dom.Document.CreateEntityReferenceFunc.result(dom.Attr*)\
    dom.Document.CreateEntityReferenceFunc.exception(dom.DOMException*)\
    dom.Document.CreateEntityReferenceFunc.document(dom.Document*)\
    dom.Document.CreateEntityReferenceFunc.name(dom.utf8*))\
\
dom.Document.GetElementsByTagNameFunc({dom.Document}\
    dom.Document.GetElementsByTagNameFunc.result(dom.NodeList*)\
    dom.Document.GetElementsByTagNameFunc.document(dom.Document*)\
    dom.Document.GetElementsByTagNameFunc.tagname(dom.utf8*))\
\
dom.Document.ImportNodeFunc({dom.Document}\
    dom.Document.ImportNodeFunc.result(dom.Node*)\
    dom.Document.ImportNodeFunc.exception(dom.DOMException*)\
    dom.Document.ImportNodeFunc.document(dom.Document*)\
    dom.Document.ImportNodeFunc.importedNode(dom.Node*)\
    dom.Document.ImportNodeFunc.deep(bool))\
\
dom.Document.CreateElementNSFunc({dom.Document}\
    dom.Document.CreateElementNSFunc.result(dom.Element*)\
    dom.Document.CreateElementNSFunc.exception(dom.DOMException*)\
    dom.Document.CreateElementNSFunc.document(dom.Document*)\
    dom.Document.CreateElementNSFunc.namespaceURI(dom.utf8*)\
    dom.Document.CreateElementNSFunc.qualifiedName(dom.utf8*))\
\
dom.Document.CreateAttributeNSFunc({dom.Document}\
    dom.Document.CreateAttributeNSFunc.result(dom.Attr*)\
    dom.Document.CreateAttributeNSFunc.exception(dom.DOMException*)\
    dom.Document.CreateAttributeNSFunc.document(dom.Document*)\
    dom.Document.CreateAttributeNSFunc.namespaceURI(dom.utf8*)\
    dom.Document.CreateAttributeNSFunc.qualifiedName(dom.utf8*))\
\
dom.Document.GetElementsByTagNameNSFunc({dom.Document}\
    dom.Document.GetElementsByTagNameNSFunc.result(dom.NodeList*)\
    dom.Document.GetElementsByTagNameNSFunc.document(dom.Document*)\
    dom.Document.GetElementsByTagNameNSFunc.namespaceURI(dom.utf8*)\
    dom.Document.GetElementsByTagNameNSFunc.localName(dom.utf8*))\
\
dom.Document.GetElementByIdFunc({dom.Document}\
    dom.Document.GetElementByIdFunc.result(dom.Element*)\
    dom.Document.GetElementByIdFunc.document(dom.Document*)\
    dom.Document.GetElementByIdFunc.elementId(dom.utf8*))\
\
dom.Document.GetInputEncodingFunc({dom.Document}\
    dom.Document.GetInputEncodingFunc.result(dom.utf8**)\
    dom.Document.GetInputEncodingFunc.document(dom.Document*))\
\
dom.Document.GetXMLEncodingFunc({dom.Document}\
    dom.Document.GetXMLEncodingFunc.result(dom.utf8**)\
    dom.Document.GetXMLEncodingFunc.document(dom.Document*))\
\
dom.Document.GetXMLStandaloneFunc({dom.Document}\
    dom.Document.GetXMLStandaloneFunc.result(bool)\
    dom.Document.GetXMLStandaloneFunc.document(dom.Document*))\
\
dom.Document.SetXMLStandaloneFunc({dom.Document}\
    dom.Document.SetXMLStandaloneFunc.exception(dom.DOMException)\
    dom.Document.SetXMLStandaloneFunc.document(dom.Document*)\
    dom.Document.SetXMLStandaloneFunc.xmlStandalone(bool))\
\
dom.Document.GetXMLVersionFunc({dom.Document}\
    dom.Document.GetXMLVersionFunc.result(dom.utf8*)\
    dom.Document.GetXMLVersionFunc.document(dom.Document*))\
\
dom.Document.SetXMLVersionFunc({dom.Document}\
    dom.Document.SetXMLVersionFunc.exception(dom.DOMException)\
    dom.Document.SetXMLVersionFunc.document(dom.Document*)\
    dom.Document.SetXMLVersionFunc.xmlVersion(dom.utf8*))\
\
dom.Document.GetStrictErrorCheckingFunc({dom.Document}\
    dom.Document.GetStrictErrorCheckingFunc.result(bool)\
    dom.Document.GetStrictErrorCheckingFunc.document(dom.Document*))\
\
dom.Document.SetStrictErrorCheckingFunc({dom.Document}\
    dom.Document.SetStrictErrorCheckingFunc.document(dom.Document*)\
    dom.Document.SetStrictErrorCheckingFunc.strictErrorChecking(bool))\
\
dom.Document.GetDocumentURIFunc({dom.Document}\
    dom.Document.GetDocumentURIFunc.result(dom.utf8*)\
    dom.Document.GetDocumentURIFunc.document(dom.Document*))\
\
dom.Document.SetDocumentURIFunc({dom.Document}\
    dom.Document.SetDocumentURIFunc.document(dom.Document*)\
    dom.Document.SetDocumentURIFunc.documentURI(dom.utf8*))\
\
dom.Document.AdoptNodeFunc({dom.Document}\
    dom.Document.AdoptNodeFunc.result(dom.Node*)\
    dom.Document.AdoptNodeFunc.exception(dom.DOMException*)\
    dom.Document.AdoptNodeFunc.document(dom.Document*)\
    dom.Document.AdoptNodeFunc.source(dom.Node*))\
\
dom.Document.GetDOMConfigurationFunc({dom.Document}\
    dom.Document.GetDOMConfigurationFunc.result(dom.DOMConfiguration*)\
    dom.Document.GetDOMConfigurationFunc.document(dom.Document*))\
\
dom.Document.NormalizeDocumentFunc({dom.Document}\
    dom.Document.NormalizeDocumentFunc.document(dom.Document*))\
\
dom.Document.RenameNodeFunc({dom.Document}\
    dom.Document.RenameNodeFunc.result(dom.Node*))\
    dom.Document.RenameNodeFunc.exception(dom.DOMException*))\
    dom.Document.RenameNodeFunc.document(dom.Document*)\
    dom.Document.RenameNodeFunc.document(dom.Node*)\
    dom.Document.RenameNodeFunc.namespaceURI(dom.utf8*)\
    dom.Document.RenameNodeFunc.qualifiedName(dom.utf8*))\
\
dom.views.AbstractView(\
    (dom.Object)\
    dom.views.AbstractView.document(dom.views.DocumentView*)\
\
dom.views.DocumentView(\
    (dom.Object)\
    dom.views.DocumentView.defaultView(dom.views.AbstractView*))\
\
dom.DocumentFragment(dom.Node)\
\
dom.DOMConfiguration(dom.Object)\
\
dom.DOMConfiguration.SetParameterFunc({dom.DOMConfiguration}\
    dom.DOMConfiguration.SetParameterFunc.exception(dom.DOMException*)\
    dom.DOMConfiguration.SetParameterFunc.domConfiguration(dom.DOMConfiguration*)\
    dom.DOMConfiguration.SetParameterFunc.name(dom.utf8*)\
    dom.DOMConfiguration.SetParameterFunc.value(dom.DOMUserData*))\
\
dom.DOMConfiguration.GetParameterFunc({dom.DOMConfiguration}\
    dom.DOMConfiguration.GetParameterFunc.result(dom.DOMUserData*)\
    dom.DOMConfiguration.GetParameterFunc.exception(dom.DOMException*)\
    dom.DOMConfiguration.GetParameterFunc.domConfiguration(dom.DOMConfiguration*)\
    dom.DOMConfiguration.GetParameterFunc.name(dom.utf8*))\
\
dom.DOMConfiguration.CanSetParameterFunc({dom.DOMConfiguration}\
    dom.DOMConfiguration.CanSetParameterFunc.result(bool)\
    dom.DOMConfiguration.CanSetParameterFunc.domConfiguration(dom.DOMConfiguration*)\
    dom.DOMConfiguration.CanSetParameterFunc.name(dom.utf8*)\
    dom.DOMConfiguration.CanSetParameterFunc.value(dom.utf8*))\
\
dom.DOMConfiguration.GetParameterNamesFunc({dom.DOMConfiguration}\
    dom.DOMConfiguration.GetParameterNamesFunc.result(dom.DOMStringList*)\
    dom.DOMConfiguration.GetParameterNamesFunc.domConfiguration(dom.DOMConfiguration*))\
\
dom.DocumentType(\
    (dom.Node)\
    dom.DocumentType.name(dom.utf8*)\
    dom.DocumentType.entities(dom.NamedNodeMap*)\
    dom.DocumentType.notations(dom.NamedNodeMap*)\
    dom.DocumentType.publicId(dom.utf8*)\
    dom.DocumentType.systemId(dom.utf8*)\
    dom.DocumentType.internalSubset(dom.utf8*))\
\
dom.Notation((dom.Node)\
    dom.Notation.publicId(dom.utf8*)\
    dom.Notation.systemId(dom.utf8*))\
\
dom.Entity((dom.Node)\
    dom.Entity.publicId(dom.utf8*)\
    dom.Entity.systemId(dom.utf8*)\
    dom.Entity.notationName(dom.utf8*)\
    dom.Entity.inputEncoding(dom.utf8*)\
    dom.Entity.xmlEncoding(dom.utf8*)\
    dom.Entity.xmlVersion(dom.utf8*))\
\
dom.EntityReference((dom.Node))\
\
dom.ProcessingInstruction(dom.Node)\
\
dom.ProcessingInstruction.GetTargetFunc({dom.ProcessingInstruction}\
    dom.ProcessingInstruction.GetTargetFunc.result(utf8*)\
    dom.ProcessingInstruction.GetTargetFunc.processingInstruction(dom.ProcessingInstruction*))\
\
dom.ProcessingInstruction.GetDataFunc({dom.ProcessingInstruction}\
    dom.ProcessingInstruction.GetDataFunc.result(utf8*)\
    dom.ProcessingInstruction.GetDataFunc.exception(dom.DOMException*)\
    dom.ProcessingInstruction.GetDataFunc.processingInstruction(dom.ProcessingInstruction*))\
\
dom.ProcessingInstruction.SetDataFunc({dom.ProcessingInstruction}\
    dom.ProcessingInstruction.SetDataFunc.exception(dom.DOMException*)\
    dom.ProcessingInstruction.SetDataFunc.processingInstruction(dom.ProcessingInstruction*)\
    dom.ProcessingInstruction.SetDataFunc.data(utf8*))\
\
dom.DOMException(\
    dom.DOMException.value(uint))\
    dom.DOMException.impl(void*))\
\
dom.DOMImplementationRegistery(\
(dom.Object)\
dom.DOMImplementationRegistery.sources(gp.GPList*)\
dom.DOMImplementationRegistery.eventSources(gp.GPList*))\
\
dom.DOMImplementationRegistery.GetDOMImplementationFunc({dom.DOMImplementationRegistery}\
    dom.DOMImplementationRegistery.GetDOMImplementationFunc.result()\
    dom.DOMImplementationRegistery.GetDOMImplementationFunc.registery(dom.DOMImplementationRegistery*)\
    dom.DOMImplementationRegistery.GetDOMImplementationFunc.mimeType(dom.utf8*)\
    dom.DOMImplementationRegistery.GetDOMImplementationFunc.features(dom.utf8*))\
\
dom.DOMImplementationRegistery.GetDOMImplementationListFunc({dom.DOMImplementationRegistery}\
    dom.DOMImplementationRegistery.GetDOMImplementationListFunc.result(\
        dom.DOMImplementationList*)\
    dom.DOMImplementationRegistery.GetDOMImplementationListFunc.registery(\
        dom.DOMImplementationRegistery*)\
    dom.DOMImplementationRegistery.GetDOMImplementationListFunc.mimeType(dom.utf8*)\
    dom.DOMImplementationRegistery.GetDOMImplementationListFunc.features(dom.utf8*))\
\
dom.DOMImplementationRegistery.RegisterDOMImplementationSourceFunc({dom.DOMImplementationRegistery}\
    dom.DOMImplementationRegistery.RegisterDOMImplementationSourceFunc.registery(\
        dom.DOMImplementationRegistery*)\
    dom.DOMImplementationRegistery.RegisterDOMImplementationSourceFunc.source(gp.GPType*))\
\
dom.DOMImplementationRegistery.RegisterDOMEventSourceFunc({dom.DOMImplementationRegistery}\
    dom.DOMImplementationRegistery.RegisterDOMEventSourceFunc.registery(\
        dom.DOMImplementationRegistery*)\
    dom.DOMImplementationRegistery.RegisterDOMEventSourceFunc.eventSource(gp.GPCallback*))\
\
dom.DOMImplementationRegistery.RemoveDOMEventSourceFunc({dom.DOMImplementationRegistery}\
    dom.DOMImplementationRegistery.RemoveDOMEventSourceFunc.registery(\
        dom.DOMImplementationRegistery*)\
    dom.DOMImplementationRegistery.RemoveDOMEventSourceFunc.eventSource(gp.GPCallback*))\
\
dom.DOMImplementationSource()\
\
dom.DOMImplementationSource.GetDOMImplementationFunc({dom.DOMImplementationSource}\
    dom.DOMImplementationSource.GetDOMImplementationFunc.result(dom.DOMImplementation*)\
    dom.DOMImplementationSource.GetDOMImplementationFunc.mimeType(dom.utf8*)\
    dom.DOMImplementationSource.GetDOMImplementationFunc.features(dom.utf8*))\
\
dom.DOMImplementationSource.GetDOMImplementationListFunc({dom.DOMImplementationSource}\
    dom.DOMImplementationSource.GetDOMImplementationListFunc.result(\
        dom.DOMImplementationList*)\
    dom.DOMImplementationSource.GetDOMImplementationListFunc.mimeType(dom.utf8*)\
    dom.DOMImplementationSource.GetDOMImplementationListFunc.features(dom.utf8*))\
\
dom.DOMImplementationList(\
     dom.DOMImplementationList.impl(void*))\
\
dom.DOMImplementationList.GetLength({dom.DOMImplementationList}\
        dom.DOMImplementationList.GetLength.result(ulong)\
        dom.DOMImplementationList.GetLength.list(dom.DOMImplementationList*))\
\
dom.DOMImplementationList.ItemFunc({dom.DOMImplementationList}\
        dom.DOMImplementationList.ItemFunc.result(dom.Node)\
        dom.DOMImplementationList.ItemFunc.list(dom.DOMImplementationList*)\
        dom.DOMImplementationList.ItemFunc.index(ulong))\
\
dom.DOMImplementation(\
    dom.DOMImplementation.super(dom.Object)\
    dom.DOMImplementation.plugin(gp.GPPlugin))\
\
dom.DOMImplementation.HasFeatureFunc({dom.DOMImplementation}\
    dom.DOMImplementation.HasFeatureFunc.result(bool)\
    dom.DOMImplementation.HasFeatureFunc.feature(dom.utf8*)\
    dom.DOMImplementation.HasFeatureFunc.version(dom.utf8*))\
\
dom.DOMImplementation.CreateDocumentTypeFunc({dom.DOMImplementation}\
    dom.DOMImplementation.CreateDocumentTypeFunc.result(dom.DocumentType*)\
    dom.DOMImplementation.CreateDocumentTypeFunc.exception(dom.DOMException*)\
    dom.DOMImplementation.CreateDocumentTypeFunc.impl(dom.DOMImplementation*)\
    dom.DOMImplementation.CreateDocumentTypeFunc.qualifiedName(dom.utf8*)\
    dom.DOMImplementation.CreateDocumentTypeFunc.publicId(dom.utf8*)\
    dom.DOMImplementation.CreateDocumentTypeFunc.systemId(dom.utf8*))\
\
dom.DOMImplementation.CreateDocumentFunc({dom.DOMImplementation}\
    dom.DOMImplementation.CreateDocumentFunc.result(dom.Document*)\
    dom.DOMImplementation.CreateDocumentFunc.exception(dom.DOMException*)\
    dom.DOMImplementation.CreateDocumentFunc.impl(dom.DOMImplementation*)\
    dom.DOMImplementation.CreateDocumentFunc.namespaceURI(dom.utf8*)\
    dom.DOMImplementation.CreateDocumentFunc.qualifiedName(dom.utf8*)\
    dom.DOMImplementation.CreateDocumentFunc.docType(dom.DocumentType*))\
\
dom.DOMImplementation.GetFeatureFunc({dom.DOMImplementation}\
    dom.DOMImplementation.GetFeatureFunc.result(dom.DOMObject*)\
    dom.DOMImplementation.GetFeatureFunc.feature(dom.utf8*)\
    dom.DOMImplementation.GetFeatureFunc.version(dom.utf8*))"

#define GP_DOM_MIMETYPE "dom/dom"

#define  GP_ELEMENT_NODE 1
#define  GP_ATTRIBUTE_NODE 2
#define  GP_TEXT_NODE 3
#define  GP_CDATA_SECTION_NODE 4
#define  GP_ENTITY_REFERENCE_NODE 5
#define  GP_ENTITY_NODE 6
#define  GP_PROCESSING_INSTRUCTION_NODE 7
#define  GP_COMMENT_NODE 8
#define  GP_DOCUMENT_NODE 9
#define  GP_DOCUMENT_TYPE_NODE 10
#define  GP_DOCUMENT_FRAGMENT_NODE 11
#define  GP_NOTATION_NODE 12
#define  GP_RESERVED_NODE 200 

/*DocumentPosition*/
#define GP_DOCUMENT_POSITION_DISCONNECTED  0x01
#define GP_DOCUMENT_POSITION_PRECEDING     0x02
#define GP_DOCUMENT_POSITION_FOLLOWING     0x04
#define GP_DOCUMENT_POSITION_CONTAINS      0x08
#define GP_DOCUMENT_POSITION_CONTAINED_BY  0x10
#define GP_DOCUMENT_POSITION_IMPLEMENTATION_SPECIFIC  0x20

/*OperationType*/
#define GP_NODE_CLONED 1
#define GP_NODE_IMPORTED 2
#define GP_NODE_DELETED 3
#define GP_NODE_RENAMED 4
#define GP_NODE_ADOPTED 5

/*DerivationMethods*/
#define GP_DERIVATION_RESTRICTION          0x1
#define GP_DERIVATION_EXTENSION            0x2
#define GP_DERIVATION_UNION                0x4
#define GP_DERIVATION_LIST                 0x8

/*ExceptionCode*/
#define GP_INDEX_SIZE_ERR                 1
#define GP_DOMSTRING_SIZE_ERR             2
#define GP_HIERARCHY_REQUEST_ERR          3
#define GP_WRONG_DOCUMENT_ERR             4
#define GP_INVALID_CHARACTER_ERR          5
#define GP_NO_DATA_ALLOWED_ERR            6
#define GP_NO_MODIFICATION_ALLOWED_ERR    7
#define GP_NOT_FOUND_ERR                  8
#define GP_NOT_SUPPORTED_ERR              9
#define GP_INUSE_ATTRIBUTE_ERR            10
/*Introduced in DOM Level 2*/
#define GP_INVALID_STATE_ERR              11
/*Introduced in DOM Level 2*/
#define GP_SYNTAX_ERR                     12
/*Introduced in DOM Level 2*/
#define GP_INVALID_MODIFICATION_ERR       13
/*Introduced in DOM Level 2*/
#define GP_NAMESPACE_ERR                  14
/*Introduced in DOM Level 2*/
#define GP_INVALID_ACCESS_ERR             15
/*Introduced in DOM Level 3*/
#define GP_VALIDATION_ERR                 16
/*Introduced in DOM Level 3*/
#define GP_TYPE_MISMATCH_ERR              17
#define GP_RESERVED_ERR                   200 /*Extension*/ 
#define GP_OUT_OF_MEMORY_ERR              201 /*Extension*/ 
#define GP_INVALID_ARG_ERR                202 /*Extension*/ 

typedef struct {
    unsigned int value;
    void* impl;
} dom_DOMException;

typedef struct {
    GPPlugin* plugin;
} dom_Plugin_ProcessEventFunc;

typedef struct {
    const char* typeName;
    GPPlugin* plugin;
} dom_Plugin_SetupInheritanceFunc;

typedef char dom_utf8;

typedef struct {
  unsigned short code;
} DOMException;

typedef struct {
    GPType* gpType;
    void* impl;
} dom_Object;

typedef struct {
    dom_Object super;
} dom_DOMObject;

typedef struct {
    dom_Object super;
} dom_Node;

typedef struct {
    dom_Node super;
    GPTreeNode* tree;
    const dom_utf8* localName; 
} dom_DefaultNode;

typedef struct {
    dom_Node* result;
    dom_DOMException* exception;
    dom_Node* node;
    dom_Node* newChild;
} dom_Node_AppendChildFunc;

typedef struct {
  dom_Object super;
  unsigned long length;
} dom_NamedNodeMap;

typedef struct {
  dom_Object super;
  GPList* sources;
  GPList* eventSources;
} dom_DOMImplementationRegistery;


typedef struct {
    dom_DOMImplementationRegistery* result;
    GPPlugin* plugin;
} dom_Plugin_GetDOMImplementationRegisteryFunc;


typedef struct {
   dom_DOMImplementationRegistery* registery;
   GPType* source;
} dom_DOMImplementationRegistery_RegisterDOMImplementationSourceFunc;

typedef struct {
   dom_DOMImplementationRegistery* registery;
   GPCallback* eventSource;
} dom_DOMImplementationRegistery_RegisterDOMEventSourceFunc;

typedef struct {
   dom_DOMImplementationRegistery* registery;
   GPCallback* eventSource;
} dom_DOMImplementationRegistery_RemoveDOMEventSourceFunc;

typedef struct {
    dom_Object super;
    GPPlugin* plugin;
} dom_DOMImplementation;

typedef struct {
    bool result;
    dom_utf8* feature;
    dom_utf8* version;
}dom_DOMImplementation_HasFeatureFunc;

typedef struct {
    dom_DOMImplementation* result;
    dom_DOMImplementationRegistery* registery;
    const dom_utf8* mimeType;
    const dom_utf8* features;
} dom_DOMImplementationRegistery_GetDOMImplementationFunc;

typedef struct {
     unsigned long   length;
     void* impl;
} dom_DOMImplementationList; 

typedef struct {
     dom_DOMImplementation* result;
     dom_DOMImplementationList* list;
     unsigned long   index;
} dom_DOMImplementationListItemFunc; 


typedef struct {
    dom_DOMImplementationList* result;
    dom_DOMImplementationRegistery* registery;
    const dom_utf8* mimeType;
    const dom_utf8* features;
} dom_DOMImplementationRegistery_GetDOMImplementationListFunc;

typedef struct {
    dom_DOMImplementation* result;
    const dom_utf8* mimeType;
    const dom_utf8* features;
} dom_DOMImplementationSource_GetDOMImplementationFunc;

typedef struct {
    dom_DOMImplementationList* result;
    const dom_utf8* mimeType;
    const dom_utf8* features;
} dom_DOMImplementationSource_GetDOMImplementationListFunc;



typedef struct {
    dom_Node super;
    dom_utf8* name;
    dom_NamedNodeMap* entities;
    dom_NamedNodeMap* notations;
    dom_utf8* publicId;
    dom_utf8* systemId;
    dom_utf8* internalSubset;
} dom_DocumentType;

typedef struct _dom_views_DocumentView dom_views_DocumentView;

typedef struct {
    dom_Object super;
    dom_views_DocumentView* document;
} dom_views_AbstractView;

struct _dom_views_DocumentView {
    dom_Object super;
    dom_views_AbstractView* defaultView;
};

typedef struct {
    dom_Node super;
} dom_Document;

typedef struct {
    dom_DefaultNode super;
} dom_DefaultDocument;

typedef struct {
    dom_Node super;
} dom_Element;

typedef struct {
    dom_DefaultNode super;
} dom_DefaultElement;

typedef struct {
    dom_DocumentType* result;
    dom_DOMException* exception;
    dom_DOMImplementation* impl;
    dom_utf8* qualifiedName;
    dom_utf8* publicId;
    dom_utf8* systemId;
}dom_DOMImplementation_CreateDocumentTypeFunc;

typedef struct {
    dom_Document* result;
    dom_DOMException* exception;
    dom_DOMImplementation* impl;
    dom_utf8* namespaceURI;
    dom_utf8* qualifiedName;
    dom_DocumentType* docType;
}dom_DOMImplementation_CreateDocumentFunc;

typedef struct {
    dom_DOMObject* result;
    dom_utf8* feature;
    dom_utf8* version;
} dom_DOMImplementation_GetFeatureFunc;

typedef struct {
    dom_Element* result;
    dom_Document* document;
} dom_Document_GetDocumentElementFunc;

typedef struct {
    dom_Element* result;
    dom_DOMException* exception;
    dom_Document* document;
    const dom_utf8* tagName;
} dom_Document_CreateElementFunc;

/**
 * Check for events and run until no more are ready 
 */
void dom_processEvent() {
    static GPType* func;
    if(!func)
        func = GPType_get("dom.Plugin.ProcessEventFunc");
    func->function(func,NULL);
}

void dom_setupInheritance(const char* typeName) {
    static GPType* func;
    dom_Plugin_SetupInheritanceFunc data = {typeName:typeName};
    if(!func)
        func = GPType_get("dom.Plugin.SetupInheritanceFunc");
    func->function(func,&data);
}

dom_DOMImplementationRegistery* dom_getDOMImplementationRegistery() {
    static GPType* func;
    static GPPlugin* plugin;
    if(!plugin)
        plugin = GPPlugin_plugin(GP_DOM_MIMETYPE,0,NULL,NULL);
    dom_Plugin_GetDOMImplementationRegisteryFunc data = { result:NULL,plugin:plugin};
    if(!func)
        func = GPType_get("dom.Plugin.GetDOMImplementationRegisteryFunc");
    func->function(func,&data);
    return data.result;
}

void dom_registerDOMImplementationSource( GPType* source ) 
{
    static GPType* func;
    dom_DOMImplementationRegistery_RegisterDOMImplementationSourceFunc data = {
            registery:dom_getDOMImplementationRegistery(),source:source };
    if(!func)
        func = GPType_get("dom.DOMImplementationRegistery.RegisterDOMImplementationSourceFunc");
    func->function(func,&data);
}

void dom_registerDOMEventSource( GPCallback* eventSource ) 
{
    static GPType* func;
    dom_DOMImplementationRegistery_RegisterDOMEventSourceFunc data = {
            registery:dom_getDOMImplementationRegistery(),eventSource:eventSource };
    if(!func)
        func = GPType_get("dom.DOMImplementationRegistery.RegisterDOMEventSourceFunc");
    func->function(func,&data);
}

void dom_removeDOMEventSource( GPCallback* eventSource ) 
{
    static GPType* func;
    dom_DOMImplementationRegistery_RemoveDOMEventSourceFunc data = {
            registery:dom_getDOMImplementationRegistery(),eventSource:eventSource };
    if(!func)
        func = GPType_get("dom.DOMImplementationRegistery.RemoveDOMEventSourceFunc");
    func->function(func,&data);
}

dom_DOMImplementation* dom_getDOMImplementation(const dom_utf8* mimeType,const dom_utf8* features ) 
{
    static GPType* func;
    dom_DOMImplementationRegistery_GetDOMImplementationFunc data = {
            result:NULL,registery:dom_getDOMImplementationRegistery(),
            mimeType:mimeType,features:features};
    if(!func)
        func = GPType_get("dom.DOMImplementationRegistery.GetDOMImplementationFunc");
    func->function(func,&data);
    return data.result;
}


dom_DocumentType* dom_createDocumentType( dom_DOMException** exception,
                                                            dom_DOMImplementation* impl,
                                                            dom_utf8* qualifiedName,
                                                            dom_utf8* publicId )

{
    static GPType* func;
    dom_DOMImplementation_CreateDocumentTypeFunc data = {
                result:NULL,
                exception:NULL,
                impl:impl,
                qualifiedName:qualifiedName,
                publicId:publicId };
    if(!func)
        func = GPType_get("dom.DOMImplementation.CreateDocumentTypeFunc");
    func->function(func,&data);
    if(exception)
        *exception = data.exception;
    return data.result;
}

dom_Element* dom_getDocumentElement( dom_Document* document )
{
    static GPType* func;
    dom_Document_GetDocumentElementFunc data = {
                    result:NULL,
                    document:document};
    if(!func)
        func = GPType_get("dom.Document.GetDocumentElementFunc");
    func->function(func,&data);
    return data.result;
}

dom_Document* dom_createDocument( dom_DOMException** exception,
                                                            dom_DOMImplementation* impl,
                                                            dom_utf8* namespaceURI,
                                                            dom_utf8* qualifiedName,
                                                            dom_DocumentType* docType)
{
static GPType* func;
    dom_DOMImplementation_CreateDocumentFunc data = {
                    result:NULL,
                    exception:NULL,
                    impl:impl,
                    namespaceURI:namespaceURI,
                    qualifiedName:qualifiedName,
                    docType:docType };
    if(!func)
        func = GPType_get("dom.DOMImplementation.CreateDocumentFunc");
    func->function(func,&data);
    if(exception)
        *exception = data.exception;
    return data.result;
}


dom_Element* dom_createElement(dom_DOMException** exception,
                               dom_Document* document,
                               const dom_utf8* tagName)
{
    static GPType* func;
    dom_Document_CreateElementFunc data = {
                    result:NULL,
                    exception:NULL,
                    document:document,
                    tagName:tagName };
    if(!func)
        func = GPType_get("dom.Document.CreateElementFunc");
    func->function(func,&data);
    if(exception)
        *exception = data.exception;
    return data.result;
}
dom_Node* dom_Node_appendChild(dom_DOMException** exception, dom_Node* node,dom_Node* newChild ) {
    static GPType* func;
    dom_Node_AppendChildFunc data = { result:NULL,exception:NULL,node:node,newChild:newChild};
    if(!func)
        func = GPType_get("dom.Node.AppendChildFunc");
    func->function(func,&data);
    if(exception)
        *exception = data.exception;
    return data.result;
}

#ifdef __cplusplus
}  /* end extern "C" */
#endif 

#endif /*GPdom_h*/
