var logmsg = "";
var errflag = false;

function esteid_log(a) {
  try {
    eidui.logMessage(a);
  } catch(e) {
    logmsg += a + "\n";
  }
}

function esteid_debug(a) {
  try {
    eidui.debugMessage(a);
  } catch(e) {
    logmsg += "DEBUG: " + a + "\n";
  }
}

function esteid_error(a) {
  try {
    eidui.errorMessage(a);
  } catch(e) {
    logmsg += "ERROR: " + a + "\n";
  }
  errflag = true;
}

function showNormalIcon() {
  var elt = document.getElementById('eidicon');
  elt.src = "chrome://esteid/skin/id-16.png";
}

function showErrorIcon() {
  var elt = document.getElementById('eidicon');
  elt.src = "chrome://esteid/skin/id-error-16.png";
}

function showWarnIcon() {
  var elt = document.getElementById('eidicon');
  elt.src = "chrome://esteid/skin/id-warning-16.png";
}

function isBrowser() {
    var com = document.getElementById('esteidbrowserpanel');
    return (com != null);
}

function LoadEstEID() {
    if(isBrowser()) {
        try {
            var com = document.getElementById('eidplugin');

            if(!eidui.isConfigured)
                esteid_log("Plugin Version: " + com.getVersion() + "\n");

            var elt = document.getElementById('esteidbrowserpanel');
	    elt.setAttribute("tooltiptext", "EstEID plugin " + com.getVersion());
        } catch (anError) {
            esteid_error("Can't find signing plugin: " + anError);
            // FIXME: Remove as soon as plugin is moved to NPRuntime
            try {
                var com = Components.classes["@id.eesti.ee/esteid;1"]
                                    .createInstance(Components.interfaces.nsIEstEID);
                esteid_log("XPCOM component version: " + com.getVersion() + "\n");
            } catch (e) {
                esteid_error(e);
            }
        }
    }

    try {
        if(!eidui.isConfigured)
            ConfigureEstEID();
    } catch (anError) {
        esteid_error(anError);
    }

    if(errflag) showErrorIcon();
    else        showNormalIcon();

    errflag = false;
    eidui.isConfigured = true;
}

var eidui = Components.classes["@id.eesti.ee/esteid-private;1"]
                      .getService(Components.interfaces.nsIEstEIDPrivate);

window.addEventListener("load", LoadEstEID, false);
//window.addEventListener("unload", UnLoadEstEID, false);
