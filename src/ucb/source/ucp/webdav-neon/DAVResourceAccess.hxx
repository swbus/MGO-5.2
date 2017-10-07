/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef INCLUDED_UCB_SOURCE_UCP_WEBDAV_NEON_DAVRESOURCEACCESS_HXX
#define INCLUDED_UCB_SOURCE_UCP_WEBDAV_NEON_DAVRESOURCEACCESS_HXX

#include <config_lgpl.h>
#include <vector>
#include <rtl/ustring.hxx>
#include <rtl/ref.hxx>
#include <osl/mutex.hxx>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/ucb/Lock.hpp>
#include <com/sun/star/ucb/WebDAVHTTPMethod.hpp>
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#include "DAVAuthListener.hxx"
#include "DAVException.hxx"
#include "DAVSession.hxx"
#include "DAVResource.hxx"
#include "DAVTypes.hxx"
#include "NeonUri.hxx"

namespace webdav_ucp
{

class DAVSessionFactory;

class DAVResourceAccess
{
    osl::Mutex    m_aMutex;
    OUString m_aURL;
    OUString m_aPath;
    css::uno::Sequence< css::beans::NamedValue > m_aFlags;
    rtl::Reference< DAVSession > m_xSession;
    rtl::Reference< DAVSessionFactory > m_xSessionFactory;
    css::uno::Reference< css::uno::XComponentContext > m_xContext;
    std::vector< NeonUri > m_aRedirectURIs;
    sal_uInt32   m_nRedirectLimit;

public:
    DAVResourceAccess( const css::uno::Reference< css::uno::XComponentContext > & rxContext,
                       rtl::Reference< DAVSessionFactory > const & rSessionFactory,
                       const OUString & rURL );
    DAVResourceAccess( const DAVResourceAccess & rOther );

    DAVResourceAccess & operator=( const DAVResourceAccess & rOther );

    /// @throws DAVException
    void setFlags( const css::uno::Sequence< css::beans::NamedValue >& rFlags );

    /// @throws DAVException
    void setURL( const OUString & rNewURL );

    void resetUri();

    const OUString & getURL() const { return m_aURL; }

    const rtl::Reference< DAVSessionFactory >& getSessionFactory() const
    { return m_xSessionFactory; }

    // DAV methods

    /// @throws DAVException
    void
    OPTIONS( DAVOptions & rOptions,
             const css::uno::Reference<
             css::ucb::XCommandEnvironment > & xEnv );

    // allprop & named
    /// @throws DAVException
    void
    PROPFIND( const Depth nDepth,
              const std::vector< OUString > & rPropertyNames,
              std::vector< DAVResource > & rResources,
              const css::uno::Reference< css::ucb::XCommandEnvironment > & xEnv );

    // propnames
    /// @throws DAVException
    void
    PROPFIND( const Depth nDepth,
              std::vector< DAVResourceInfo > & rResInfo,
              const css::uno::Reference< css::ucb::XCommandEnvironment > & xEnv );

    /// @throws DAVException
    void
    PROPPATCH( const std::vector< ProppatchValue > & rValues,
               const css::uno::Reference< css::ucb::XCommandEnvironment >& xEnv );

    /// @throws DAVException
    void
    HEAD( const std::vector< OUString > & rHeaderNames, // empty == 'all'
          DAVResource & rResource,
          const css::uno::Reference< css::ucb::XCommandEnvironment >& xEnv );

    /// @throws DAVException
    css::uno::Reference< css::io::XInputStream >
    GET( const css::uno::Reference< css::ucb::XCommandEnvironment > & xEnv );

    /// @throws DAVException
    void
    GET( css::uno::Reference< css::io::XOutputStream > & rStream,
         const css::uno::Reference< css::ucb::XCommandEnvironment > & xEnv );

    /// @throws css::uno::RuntimeException
    /// @throws DAVException
    css::uno::Reference< css::io::XInputStream >
    GET( const std::vector< OUString > & rHeaderNames, // empty == 'all'
         DAVResource & rResource,
         const css::uno::Reference< css::ucb::XCommandEnvironment > & xEnv );

    // used as HEAD substitute when HEAD is not implemented on server
    /// @throws DAVException
    void
    GET0( DAVRequestHeaders & rRequestHeaders,
          const std::vector< rtl::OUString > & rHeaderNames, // empty == 'all'
          DAVResource & rResource,
          const css::uno::Reference< css::ucb::XCommandEnvironment > & xEnv );

    /// @throws DAVException
    void
    GET( css::uno::Reference< css::io::XOutputStream > & rStream,
         const std::vector< OUString > & rHeaderNames, // empty == 'all'
         DAVResource & rResource,
         const css::uno::Reference< css::ucb::XCommandEnvironment > & xEnv );

    /// @throws css::uno::RuntimeException
    /// @throws DAVException
    void
    PUT( const css::uno::Reference< css::io::XInputStream > & rStream,
         const css::uno::Reference< css::ucb::XCommandEnvironment > & xEnv );

    /// @throws css::uno::RuntimeException
    /// @throws DAVException
    css::uno::Reference< css::io::XInputStream >
    POST( const OUString & rContentType,
          const OUString & rReferer,
          const css::uno::Reference< css::io::XInputStream > & rInputStream,
          const css::uno::Reference< css::ucb::XCommandEnvironment >& xEnv );

    /// @throws css::uno::RuntimeException
    /// @throws DAVException
    void
    POST( const OUString & rContentType,
          const OUString & rReferer,
          const css::uno::Reference< css::io::XInputStream > & rInputStream,
          css::uno::Reference< css::io::XOutputStream > & rOutputStream,
          const css::uno::Reference< css::ucb::XCommandEnvironment >& xEnv );

    /// @throws DAVException
    void
    MKCOL( const css::uno::Reference< css::ucb::XCommandEnvironment > & xEnv );

    /// @throws DAVException
    void
    COPY( const OUString & rSourcePath,
          const OUString & rDestinationURI,
          bool bOverwrite,
          const css::uno::Reference< css::ucb::XCommandEnvironment > & xEnv );

    /// @throws DAVException
    void
    MOVE( const OUString & rSourcePath,
          const OUString & rDestinationURI,
          bool bOverwrite,
          const css::uno::Reference< css::ucb::XCommandEnvironment > & xEnv );

    /// @throws DAVException
    void
    DESTROY( const css::uno::Reference< css::ucb::XCommandEnvironment > & xEnv );

    // set new lock.
    /// @throws DAVException
    void
    LOCK( css::ucb::Lock & inLock,
          const css::uno::Reference< css::ucb::XCommandEnvironment > & xEnv );

    /// @throws DAVException
    void
    UNLOCK( const css::uno::Reference< css::ucb::XCommandEnvironment > & xEnv );

    /// @throws DAVException
    void
    abort();

    // helper
    static void
    getUserRequestHeaders(
        const css::uno::Reference< css::ucb::XCommandEnvironment > & xEnv,
        const OUString & rURI,
        css::ucb::WebDAVHTTPMethod eMethod,
        DAVRequestHeaders & rRequestHeaders );

        /// @throws DAVException
        bool handleException( const DAVException & e, int errorCount );

private:
    const OUString & getRequestURI() const;
    /// @throws DAVException
    bool detectRedirectCycle( const OUString& rRedirectURL );
    /// @throws DAVException
    void initialize();
};

} // namespace webdav_ucp

#endif // INCLUDED_UCB_SOURCE_UCP_WEBDAV_NEON_DAVRESOURCEACCESS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
