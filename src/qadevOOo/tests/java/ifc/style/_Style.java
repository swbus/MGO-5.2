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

package ifc.style;

import lib.MultiPropertyTest;

/**
* Testing <code>com.sun.star.style.Style</code>
* service properties :
* <ul>
*  <li><code> IsPhysical</code></li>
*  <li><code> FollowStyle</code></li>
*  <li><code> DisplayName</code></li>
*  <li><code> IsAutoUpdate</code></li>
* </ul> <p>
* Properties testing is automated by <code>lib.MultiPropertyTest</code>.
* @see com.sun.star.style.Style
*/
public class _Style extends MultiPropertyTest {

    public void _FollowStyle() {
        String style = (String)tEnv.getObjRelation("FollowStyle");
        if (style == null) style = "Heading 1";
        testProperty("FollowStyle", style, "Heading 2");
    }

}  // finish class _Style


