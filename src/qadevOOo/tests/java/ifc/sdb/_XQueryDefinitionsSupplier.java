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

package ifc.sdb;

import lib.MultiMethodTest;

import com.sun.star.container.XNameAccess;
import com.sun.star.sdb.XQueryDefinitionsSupplier;

/**
 * Testing <code>com.sun.star.sdb.XQueryDefinitionsSupplier</code>
 * interface methods :
 * <ul>
 *  <li><code> getQueryDefinitions()</code></li>
 * </ul> <p>
 * Test is <b> NOT </b> multithread compliant. <p>
 * @see com.sun.star.sdb.XQueryDefinitionsSupplier
 */
public class _XQueryDefinitionsSupplier extends MultiMethodTest {

    // oObj filled by MultiMethodTest
    public XQueryDefinitionsSupplier oObj = null ;

    /**
     * Test calls the method. <p>
     * Has <b> OK </b> status if the method returns not
     * <code>null</code> value.
     */
    public void _getQueryDefinitions() {

        XNameAccess docs = oObj.getQueryDefinitions() ;

        String[] docNames = docs.getElementNames() ;
        if (docNames != null) {
            log.println("Totally " + docNames.length + " queries :") ;
            for (int i = 0; i < docNames.length; i++)
                log.println("  " + docNames[i]) ;
        }

        tRes.tested("getQueryDefinitions()", docNames != null) ;
    }

}  // finish class _XQueryDefinitionsSupplier

