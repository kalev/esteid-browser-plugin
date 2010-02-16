var params = window.arguments[0];
var sb = null;

function onLoad() {
  var sbs = Components.classes["@mozilla.org/intl/stringbundle;1"]
                      .getService(Components.interfaces.nsIStringBundleService);
  sb = sbs.createBundle("chrome://esteid/locale/esteid.properties");

  try {
    Components.utils.import("resource://esteid/global.jsm");
    logmsg = esteidglobal.log;
  } catch(e) {
    if(params.log) logmsg = params.log;
    else logmsg = "";
    logmsg += e + "\n";
  }

  var elt = document.getElementById('log');
  elt.value = logmsg;
}

function doOK() {
}
