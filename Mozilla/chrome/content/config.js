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

/* Configure Estonian ID Card support.
 * This JavaScript will handle CA certificate import and load required
 * security modules into browser.
 *
 * IMPORTANT NOTE: global functions esteid_log esteid_error and esteid_debug
 * must be defined before running the code in ConfigureEstEID();
 */

const nsNSSCertCache          = "@mozilla.org/security/nsscertcache;1";
const nsModDB                 = "@mozilla.org/security/pkcs11moduledb;1"
const nsX509CertDB            = "@mozilla.org/security/x509certdb;1"
const nsLocalFile             = "@mozilla.org/file/local;1";
const nsFileInputStream       = "@mozilla.org/network/file-input-stream;1";
const nsScriptableInputStream = "@mozilla.org/scriptableinputstream;1";
const nsPKCS11                = "@mozilla.org/security/pkcs11;1";
const nsIPKCS11Slot           = Components.interfaces.nsIPKCS11Slot;
const nsIPK11Token            = Components.interfaces.nsIPK11Token;
const nsHttpProtocolHandler   = "@mozilla.org/network/protocol;1?name=http";
const nsWindowsRegKey         = "@mozilla.org/windows-registry-key;1";

const brdBundleURL = "chrome://branding/locale/brand.properties";
const extBundleURL = "chrome://mozapps/locale/extensions/extensions.properties";
const eidBundleURL = "chrome://esteid/locale/esteid.properties";
const eidIconURL   = "chrome://esteid/skin/id-16.png";

const EstEidModName = "Estonian ID Card";
const soName = "onepin-opensc-pkcs11";

function PEM2Base64(pem) {
    var beginCert = "-----BEGIN CERTIFICATE-----";
    var endCert = "-----END CERTIFICATE-----";
    pem = pem.replace(/[\r\n]/g, "");
    var begin = pem.indexOf(beginCert);
    var end = pem.indexOf(endCert);
    return pem.substring(begin + beginCert.length, end);
}

function requestRestart() {
    var restartLabel, restartKey, restartMessage;
    var nb;

    try {
        nb = gBrowser.getNotificationBox();
    } catch(e) {
        esteid_error("Module reloading requires browser restart");
        return;
    }

    var sbs = Components.classes["@mozilla.org/intl/stringbundle;1"].
                  getService(Components.interfaces.nsIStringBundleService);

    /* Load restart button label and accelerator key
     * from extension manager string bundle
     * toolkit/mozapps/extensions/content/extensions.xul
     * toolkit/mozapps/extensions/content/extensions.js
     */ 
    try {
        var sb = sbs.createBundle(brdBundleURL);
        var appName = sb.GetStringFromName("brandShortName");
        sb = sbs.createBundle(extBundleURL);
        restartLabel = sb.formatStringFromName("restartButton", [appName], 1);
        restartKey   = sb.GetStringFromName("restartAccessKey");
    } catch(e) {
        alert(e);
        restartLabel = "Restart Now";
        restartKey = null;
    }
    try {
        var sb = sbs.createBundle(eidBundleURL);
        restartMessage = sb.GetStringFromName("pkcs11RestartMessage");
    } catch(e) {
        restartMessage = "Turvamooduli laadimise lõpetamine vajab restarti";
    }

    var buttons = [{
        label: restartLabel,
        accessKey: restartKey,
        popup: null,
        callback: doRestart
    }];
    nb.appendNotification(restartMessage, "restart-app",
                          eidIconURL,
                          nb.PRIORITY_WARNING_MEDIUM , buttons);
}

 /* Based on wizardFinish() in
  * toolkit/mozapps/plugins/content/pluginInstallerWizard.js
  */
function doRestart() {
    // Notify all windows that an application quit has been requested.
    var os = Components.classes["@mozilla.org/observer-service;1"]
                       .getService(Components.interfaces.nsIObserverService);
    var cancelQuit = Components.classes["@mozilla.org/supports-PRBool;1"]
                               .createInstance(Components.interfaces.nsISupportsPRBool);
    os.notifyObservers(cancelQuit, "quit-application-requested", "restart");

    // Something aborted the quit process.
    if (!cancelQuit.data) {
      var nsIAppStartup = Components.interfaces.nsIAppStartup;
      var appStartup = Components.classes["@mozilla.org/toolkit/app-startup;1"]
                                 .getService(nsIAppStartup);
      appStartup.quit(nsIAppStartup.eAttemptQuit | nsIAppStartup.eRestart);
      return true;
   }
}


/* Return true if all slots associated with this module have
 * friendly flag set.
 */
function allSlotsFriendly(module)
{
    var allFriendly = true;

    var slots = module.listSlots();
    var slots_done = false;
    try {
        slots.isDone();
    } catch (e) { slots_done = true; }
    while (!slots_done) {
        var slot = null;
        try {
            slot = slots.currentItem().QueryInterface(nsIPKCS11Slot);
        } catch (e) { slot = null; }
        if (slot != null) {
            var token = slot.getToken();
            if (!token.isFriendly()) {
                allFriendly = false;

                var tokenName;
                if (slot.tokenName)
                    tokenName = slot.tokenName;
                else
                    tokenName = slot.name;

                esteid_debug("Found non-friendly token: " + tokenName);
            }
        }
        try {
            slots.next();
        } catch (e) { slots_done = true; }
    }

    return allFriendly;
}


function ConfigureEstEID() {
    const Ci = Components.interfaces;
    const Cc = Components.classes;

    /* Detect platform.
     * We could use navigator.platform or navigator.oscpu, but unfortunately
     * those can be overriden by preferences so we have to go to the source
     * of this information which is calculated here 
     * mozilla/netwerk/protocol/http/src/nsHttpHandler.cpp
     */
    var httpHandler = Cc[nsHttpProtocolHandler]
                        .getService(Ci.nsIHttpProtocolHandler);
    var oscpu = httpHandler.oscpu;
    var platform = httpHandler.platform;

    esteid_log("OSCPU: " + oscpu);
    esteid_log("Platform: " + platform);

    var bits = (oscpu.indexOf(" x86_64") < 0 &&
                oscpu.indexOf(" x64") < 0) ? 32 : 64;

    esteid_log("Detected " + bits + " bit browser");

    var certDir;
    var moduleDlls;

    /* Set platform specific paths */
    if(platform == "Windows") {
        var EstEidInstallDir;
        var win32Dir;

        /* Load sysdir location from registry */
        try {
            var key = Cc[nsWindowsRegKey].createInstance(Ci.nsIWindowsRegKey);
            key.open(Ci.nsIWindowsRegKey.ROOT_KEY_LOCAL_MACHINE,
                     "Software\\Microsoft\\Windows NT\\CurrentVersion",
                     Ci.nsIWindowsRegKey.ACCESS_READ);
            win32Dir = key.readStringValue("SystemRoot");
        } catch(e) {
            var win32Dir = "C:\\WINDOWS";
        }
        win32Dir += "\\System32\\";

        /* Load EstEID stuff location from registry */
        try {
            var key = Cc[nsWindowsRegKey].createInstance(Ci.nsIWindowsRegKey);
            key.open(Ci.nsIWindowsRegKey.ROOT_KEY_LOCAL_MACHINE,
                     "Software\\Estonian ID Card",
                     Ci.nsIWindowsRegKey.ACCESS_READ);
            EstEidInstallDir = key.readStringValue("Installed");
        } catch(e) {
            esteid_error("Unable to read EstEID path from registry: " + e);
            EstEidInstallDir = "C:\\Program Files\\Estonian ID Card\\";
        }
        certDir = EstEidInstallDir + "certs";
        moduleDlls = [ EstEidInstallDir + soName + ".dll",
                       win32Dir + soName + ".dll" ];
    } else if(platform == "Macintosh") {
        certDir = "/usr/local/etc/digidocpp/certs";
        moduleDlls = [ "/Library/OpenSC/lib/" + soName + ".so" ];
    } else if(platform == "X11") { // Linux/Unix
        var libdir = (bits == 64) ? "/usr/lib64/" : "/usr/lib/";
        certDir = "/etc/digidocpp/certs";
        moduleDlls = [ libdir + soName + ".so",
                       libdir + "pkcs11/" + soName + ".so",
                       "/usr/pkg/lib/" + soName + ".so",
                       "/usr/local/lib/" + soName + ".so" ];
    } else {
        esteid_error("Unknown plaform " + platform);
        return;
    }

    esteid_log("Cert Dir: " + certDir);
    esteid_debug("Module Dlls: " + moduleDlls);

    var EstEidDll;
    var needtoload = false;

    /* Go through module path list and pick the first that actually exists */
    for(i in moduleDlls) {
        var file = Cc[nsLocalFile].createInstance(Ci.nsILocalFile);
        var readable = false;
        try {
            file.initWithPath(moduleDlls[i]);
            var readable = file.isReadable();
        } catch (e) { };

        if(readable) {
            EstEidDll = moduleDlls[i];
            needtoload = true;
            break;
        }
    }
    if(EstEidDll)
        esteid_log("Picked Module: " + EstEidDll);
    else
        esteid_error("Unable to find security module in any of the following locations: " + moduleDlls);

    var modulesToRemove = new Array();
    var restartNeeded = false;

    /* Search security module database */
    var secmoddb = Cc[nsModDB].createInstance(Ci.nsIPKCS11ModuleDB);
    var modules = secmoddb.listModules();

    for(;;) {
      var module = modules.currentItem()
                          .QueryInterface(Ci.nsIPKCS11Module);
        /* Remove modules that have recognized names but different DLLs */
        if (module) {
            if(module.libName == EstEidDll && module.name == EstEidModName) {
                if (allSlotsFriendly(module)) {
                    needtoload = false;
                } else {
                    /* Remove modules with one or more slots marked as not friendly */
                    esteid_debug("Marking non-friendly module for removal: " +
                          module.name + ":" + module.libName);
                    modulesToRemove.push(module.name);
                    needtoload = true;
                    restartNeeded = true;
                }
            }
            else if(module.name == EstEidModName ||
                    /(opensc-pkcs11|esteid-pkcs11)\.(so|dll)$/
                                                .test(module.libName)) {
                esteid_debug("Marking module for removal: " +
                      module.name + ":" + module.libName);
                modulesToRemove.push(module.name);

                /* It's impossible to reload the same DLL */
                if(module.libName == EstEidDll) restartNeeded = true;
            }
        }
        try { modules.next(); } catch(e) { break; }
    }

    /* Actually remove modules marked for removal */
    for(i in modulesToRemove) {
        var modName = modulesToRemove[i];

        /* Try to load sane interface (Firefox 3.5 and later) first */
        try {
            var pkcs11 = Cc[nsPKCS11].getService(Ci.nsIPKCS11);
            try {
                pkcs11.deleteModule(modName);
            } catch(e) {
                esteid_error("Unable to remove module: " + e);
            }
        } catch(e) {
            var pkcs11 = window.pkcs11;
            var res;
            do {
                res = pkcs11.deletemodule(modName);
            } while(res == -2); // Bug user until he/she clicks OK
            if(res < 0) esteid_error("Unable to remove module");
        }
    }

    /* Load EstEID module */
    if(needtoload) {
        /* Try to load sane interface (Firefox 3.5 and later) first */
        try {
            var pkcs11 = Cc[nsPKCS11].getService(Ci.nsIPKCS11);
            try {
                pkcs11.addModule(EstEidModName, EstEidDll, 0x1<<28, 0);
            } catch(e) {
                esteid_error("Unable to load module: " + e);
            }
        } catch(e) {
            var pkcs11 = window.pkcs11;
            var res;
            do {
                res = pkcs11.addmodule(EstEidModName, EstEidDll, 0x1<<28, 0);
            } while(res == -2); // Bug user until he/she clicks OK
            if(res < 0) esteid_error("Unable to load module");
        }
    }

    /* Find CA Certificates and construct a list of fingerprints */
    var caCertIDs = new Array();
    var certdb    = Cc[nsX509CertDB]  .createInstance(Ci.nsIX509CertDB);
    var certdb2   = certdb.QueryInterface(Ci.nsIX509CertDB2);

/*  This works as documented with FF >= 3.0,
 *  but we need Thunderbird 2.0 support as well so
 *  we are currently using code that relies on undocumented
 *  behaviour below :(
 *  FIXME: Use this code after dropping 2.0 support

    var certcache = Cc[nsNSSCertCache].createInstance(Ci.nsINSSCertCache);
    certcache.cacheAllCerts();
    var certList = certcache.getX509CachedCerts().getEnumerator();

    while (certList.hasMoreElements()) {
        var next = certList.getNext();
        var cert = next.QueryInterface(Ci.nsIX509Cert);
        var cert2 = next.QueryInterface(Ci.nsIX509Cert2);
        if (cert && cert2 && cert2.certType == Ci.nsIX509Cert.CA_CERT) {
            caCertIDs.push(cert.sha1Fingerprint);
        }
    }
*/

    var cnt = new Object(); var nicks = new Object(); 
    certdb.findCertNicknames(null, Ci.nsIX509Cert.CA_CERT, cnt, nicks);
    nicks = nicks.value;
    esteid_debug("Found " + nicks.length + " CA certs from database");
    for(var i = 0; i < nicks.length; i++) {
        /* UNDOCUMENTED CRAP!
         * The strings in array do NOT contain a nickname but
         * are actually in format: \001Token Name\001Cert Name\001DbKey
         * See: security/manager/ssl/src/nsNSSCertificateDB.cpp
         */
        var tmp = nicks[i].split("\001");
        var cert = certdb.findCertByDBKey(tmp[tmp.length-1], null);
        caCertIDs.push(cert.sha1Fingerprint);
    }

    var EstEIDcertIDs = new Array();

    /* Open Certificate directory */
    var i = null;
    try {
        var dir = Cc[nsLocalFile].createInstance(Ci.nsILocalFile);
        dir.initWithPath(certDir);
        i = dir.directoryEntries;
    } catch(e) {
        esteid_error("Unable to open certificate directory " + certDir + ": " + e);
    }

    /* Read files, one by one */
    while(i && i.hasMoreElements()) {
        var file = i.getNext().QueryInterface(Ci.nsILocalFile);
        if(file.isReadable()) {
            var fS = Cc[nsFileInputStream]
                              .createInstance(Ci.nsIFileInputStream);
            fS.init(file, 1, 0, false);
            var sS = Cc[nsScriptableInputStream]
                              .getService(Ci.nsIScriptableInputStream);
            sS.init(fS);
            var data = sS.read(sS.available());
            sS.close();
            fS.close();

            /* Parse file contents */
            try {
                var pem = PEM2Base64(data);
                var cert = certdb.constructX509FromBase64(pem);

                /* If NOT a CA cert go to next file */

                /* FIXME: Remove else block along with Thunderbird 2.0 support */
                if(Ci.nsIX509Cert2) {
                    var cert2 = cert.QueryInterface(Ci.nsIX509Cert2);
                    if(cert2.certType != Ci.nsIX509Cert.CA_CERT) continue;
                } else {
                    var r = cert.verifyForUsage(Ci.nsIX509Cert.CERT_USAGE_SSLCA);
                    if(r != Ci.nsIX509Cert.VERIFIED_OK &&
                       r != Ci.nsIX509Cert.ISSUER_NOT_TRUSTED &&
                       r != Ci.nsIX509Cert.ISSUER_UNKNOWN) continue;
                }

                EstEIDcertIDs.push(cert.sha1Fingerprint);
                if(caCertIDs.indexOf(cert.sha1Fingerprint) >= 0) continue;
                esteid_log("Adding to database: " + file.leafName);
                certdb2.addCertFromBase64(pem, "C,C,P", "");
            } catch(e) {
                esteid_error("Unable to parse " + file.leafName + ": " + e);
            }
        }
    }

    esteid_debug("Esteid Certs: " + EstEIDcertIDs.join("\n"));

    if(restartNeeded) requestRestart();
}

//delay of popups until after the browser is loaded, kind of a hack
//window.setTimeout('start()', 5000);
