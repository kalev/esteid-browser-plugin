var errflag = false;

try {
  Components.utils.import("resource://esteid/global.jsm");
} catch(e) {
  // Too old firefox / thunderbird. Default to window-local storage.
  var esteidglobal = {  
    log : "",
    isConfigured : false,
  };
  log_debug("No global JSM available, logs will be window specific");
}

function esteid_log(a) {
  esteidglobal.log += a + "\n";
}

function esteid_debug(a) {
  esteid_log("DEBUG: " + a);
}

function esteid_error(a) {
  esteid_log("ERROR: " + a);
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

function showPrefs() {
    // Do not show multiple windows
    var ww = Components.classes["@mozilla.org/embedcomp/window-watcher;1"]
                       .getService(Components.interfaces.nsIWindowWatcher);
    var win = ww.getWindowByName("esteidprefs", null);

    if(win && !win.closed) {
      win.focus();
      return;
    }

    var params = { log: esteidglobal.log, out:null};
    window.openDialog("chrome://esteid/content/prefs.xul",
                       "esteidprefs", "chrome,centerscreen", params);
}

function LoadEstEID() {
    if(isBrowser()) {
        try {
            var com = document.getElementById('eidplugin');

            if(!esteidglobal.isConfigured)
                esteid_log("Plugin Version: " + com.getVersion() + "\n");

            var elt = document.getElementById('esteidbrowserpanel');
	    elt.setAttribute("tooltiptext", "EstEID plugin " + com.getVersion());
        } catch (anError) {
            esteid_error("Can't find signing plugin: " + anError);
        }
        try {
            var appcontent = document.getElementById("appcontent");
            esteidRegisterLegacyConverter(appcontent);
        } catch (anError) {
            esteid_error("Can't register legacy converter: " + anError);
        }
    }

    try {
        if(!esteidglobal.isConfigured)
            ConfigureEstEID();
    } catch (anError) {
        esteid_error(anError);
    }

    if(errflag) showErrorIcon();
    else        showNormalIcon();

    errflag = false;
    esteidglobal.isConfigured = true;
}

window.addEventListener("load", LoadEstEID, false);
//window.addEventListener("unload", UnLoadEstEID, false);
