/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_UNOXML_SOURCE_DOM_CDATASECTION_HXX
#define INCLUDED_UNOXML_SOURCE_DOM_CDATASECTION_HXX

#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/xml/dom/XCDATASection.hpp>

#include <cppuhelper/implbase.hxx>
#include <text.hxx>

namespace DOM
{
    typedef ::cppu::ImplInheritanceHelper< CText, css::xml::dom::XCDATASection >
        CCDATASection_Base;

    class CCDATASection
        : public CCDATASection_Base
    {
        friend class CDocument;

    protected:
        CCDATASection(CDocument const& rDocument, ::osl::Mutex const& rMutex,
                xmlNodePtr const pNode);

    public:

        virtual void saxify(const css::uno::Reference< css::xml::sax::XDocumentHandler >& i_xHandler) override;

        virtual css::uno::Reference< css::xml::dom::XText > SAL_CALL splitText(sal_Int32 offset) override
        {
            return CText::splitText(offset);
        }

         // --- delegations for XCharacterData
        virtual void SAL_CALL appendData(const OUString& arg) override
        {
            CCharacterData::appendData(arg);
        }
        virtual void SAL_CALL deleteData(sal_Int32 offset, sal_Int32 count) override
        {
            CCharacterData::deleteData(offset, count);
        }
        virtual OUString SAL_CALL getData() override
        {
            return CCharacterData::getData();
        }
        virtual sal_Int32 SAL_CALL getLength() override
        {
            return CCharacterData::getLength();
        }
        virtual void SAL_CALL insertData(sal_Int32 offset, const OUString& arg) override
        {
            CCharacterData::insertData(offset, arg);
        }
        virtual void SAL_CALL replaceData(sal_Int32 offset, sal_Int32 count, const OUString& arg) override
        {
            CCharacterData::replaceData(offset, count, arg);
        }
        virtual void SAL_CALL setData(const OUString& data) override
        {
            CCharacterData::setData(data);
        }
        virtual OUString SAL_CALL subStringData(sal_Int32 offset, sal_Int32 count) override
        {
            return CCharacterData::subStringData(offset, count);
        }


         // --- overrides for XNode base
        virtual OUString SAL_CALL getNodeName() override;
        virtual OUString SAL_CALL getNodeValue() override;

    // --- delegation for XNode base.
    virtual css::uno::Reference< css::xml::dom::XNode > SAL_CALL appendChild(const css::uno::Reference< css::xml::dom::XNode >& newChild) override
    {
        return CNode::appendChild(newChild);
    }
    virtual css::uno::Reference< css::xml::dom::XNode > SAL_CALL cloneNode(sal_Bool deep) override
    {
        return CNode::cloneNode(deep);
    }
    virtual css::uno::Reference< css::xml::dom::XNamedNodeMap > SAL_CALL getAttributes() override
    {
        return CNode::getAttributes();
    }
    virtual css::uno::Reference< css::xml::dom::XNodeList > SAL_CALL getChildNodes() override
    {
        return CNode::getChildNodes();
    }
    virtual css::uno::Reference< css::xml::dom::XNode > SAL_CALL getFirstChild() override
    {
        return CNode::getFirstChild();
    }
    virtual css::uno::Reference< css::xml::dom::XNode > SAL_CALL getLastChild() override
    {
        return CNode::getLastChild();
    }
    virtual OUString SAL_CALL getLocalName() override
    {
        return CNode::getLocalName();
    }
    virtual OUString SAL_CALL getNamespaceURI() override
    {
        return CNode::getNamespaceURI();
    }
    virtual css::uno::Reference< css::xml::dom::XNode > SAL_CALL getNextSibling() override
    {
        return CNode::getNextSibling();
    }
    virtual css::xml::dom::NodeType SAL_CALL getNodeType() override
    {
        return CNode::getNodeType();
    }
    virtual css::uno::Reference< css::xml::dom::XDocument > SAL_CALL getOwnerDocument() override
    {
        return CNode::getOwnerDocument();
    }
    virtual css::uno::Reference< css::xml::dom::XNode > SAL_CALL getParentNode() override
    {
        return CNode::getParentNode();
    }
    virtual OUString SAL_CALL getPrefix() override
    {
        return CNode::getPrefix();
    }
    virtual css::uno::Reference< css::xml::dom::XNode > SAL_CALL getPreviousSibling() override
    {
        return CNode::getPreviousSibling();
    }
    virtual sal_Bool SAL_CALL hasAttributes() override
    {
        return CNode::hasAttributes();
    }
    virtual sal_Bool SAL_CALL hasChildNodes() override
    {
        return CNode::hasChildNodes();
    }
    virtual css::uno::Reference< css::xml::dom::XNode > SAL_CALL insertBefore(
            const css::uno::Reference< css::xml::dom::XNode >& newChild, const css::uno::Reference< css::xml::dom::XNode >& refChild) override
    {
        return CNode::insertBefore(newChild, refChild);
    }
    virtual sal_Bool SAL_CALL isSupported(const OUString& feature, const OUString& ver) override
    {
        return CNode::isSupported(feature, ver);
    }
    virtual void SAL_CALL normalize() override
    {
        CNode::normalize();
    }
    virtual css::uno::Reference< css::xml::dom::XNode > SAL_CALL removeChild(const css::uno::Reference< css::xml::dom::XNode >& oldChild) override
    {
        return CNode::removeChild(oldChild);
    }
    virtual css::uno::Reference< css::xml::dom::XNode > SAL_CALL replaceChild(
            const css::uno::Reference< css::xml::dom::XNode >& newChild, const css::uno::Reference< css::xml::dom::XNode >& oldChild) override
    {
        return CNode::replaceChild(newChild, oldChild);
    }
    virtual void SAL_CALL setNodeValue(const OUString& nodeValue) override
    {
        return CText::setNodeValue(nodeValue);
    }
    virtual void SAL_CALL setPrefix(const OUString& prefix) override
    {
        return CNode::setPrefix(prefix);
    }

    };

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
