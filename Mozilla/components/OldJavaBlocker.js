/*
 * esteid-browser-plugin - a browser plugin for Estonian EID card
 *
 * Copyright (C) 2010  Estonian Informatics Centre
 * Copyright (C) 2010  Smartlink OÜ
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

const Ci = Components.interfaces;
const Cu = Components.utils;

Cu.import("resource://gre/modules/XPCOMUtils.jsm");

function esteidBlocker() {
    // dump("esteidBlocker created\n");
}

esteidBlocker.prototype = {
  classDescription:  "EstEID old Java plugin blocker XPCOM component",
  classID:           Components.ID("{299c2970-575f-91d5-8a39-9790297c5a65}"),
  contractID:        "@esteid.googlecode.com/esteid-blocker;1",
  QueryInterface:    XPCOMUtils.generateQI([ Ci.nsIContentPolicy ]),
  _xpcom_categories: [{category: "content-policy"}],

  
  shouldLoad: function(aContentType, aContentLocation, aRequestOrigin,
                       aContext, aMimeTypeGuess, aExtra) {  

    let result = Ci.nsIContentPolicy.ACCEPT;  

    /* Block all objects that have an attribute "code" with the value
     * "SignatureApplet.class". Of-course this COULD hit some other
     * useful applets too, but it's highly unlikely. Why? Because
     * the usage of default package has been discouraged for a while by now
     * and all sane applets have code attributes like this:
     * code="org.opensc.webapplet.WebApplet"
     */
    if ((Ci.nsIContentPolicy.TYPE_OBJECT == aContentType) &&
         aContext.getAttribute("code") == "SignatureApplet.class") {
        return Ci.nsIContentPolicy.REJECT_REQUEST;
    }
  
    return result;  
  }
};

function NSGetModule(compMgr, fileSpec) {
  return XPCOMUtils.generateModule([ esteidBlocker ]);
}
