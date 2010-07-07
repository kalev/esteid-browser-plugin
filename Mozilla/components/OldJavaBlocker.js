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
