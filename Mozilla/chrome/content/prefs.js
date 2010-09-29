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

var params = window.arguments[0];
var sb = null;
var plugin = null;

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

  plugin = getPlugin();
  if(plugin != null) document.getElementById('wlbtn').disabled = false;
}

function doOK() {
}

function getPlugin() {
    // https://developer.mozilla.org/en/Working_with_windows_in_chrome_code
    var wm = Components.classes["@mozilla.org/appshell/window-mediator;1"]
                 .getService(Components.interfaces.nsIWindowMediator);
    var browserWindow = wm.getMostRecentWindow("navigator:browser");

    var doc = browserWindow.document;
    return doc.getElementById('eidplugin');
}

function openPluginSettings() {
    plugin.showSettings();
}
