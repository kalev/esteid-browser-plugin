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

function esteidConvertObject(o, doc) {
    var p  = o.parentNode;
    var id = o.getAttribute("id");
    esteid_log('Converting ' + id + ' to new plugin on page ' + doc.location.href);

    o = p.removeChild(o);
    delete o;

    var e = doc.createElement('object');
    e.id           = id;
    e.type         = 'application/x-esteid';
    e.style.width  = '1px';
    e.style.height = '1px';

    e = p.appendChild(e);
}

function esteidConvertLegacy(doc) {
  if(!doc.location) return; // Optimize

  doc.esteidConvertLegacyRunning = true;

  /* Regex to find ActiveX tags (YES some sites send those to Firefox too) */
  var axreg = new RegExp("FC5B7BD2-584A-4153-92D7-4C5840E4BC28", "i");

  /* Find candidate objects */
  var ot = [ "applet", "embed", "object" ];
  for(var i in ot) {
    var els = doc.getElementsByTagName(ot[i]);
    for(var j = 0; j < els.length; j++) {
      var e = els[j];

      if(e.getAttribute("code") == "SignatureApplet.class")
        esteidConvertObject(e, doc);
      else if(axreg.exec(e.getAttribute("classid")))
        esteidConvertObject(e, doc);
    }
  }
  doc.esteidConvertLegacyRunning = false;
}

function esteidRegisterLegacyConverter(doc) {
  esteid_log('Activating legacy plugin tag converter');

  doc.addEventListener("DOMContentLoaded", function(aEvent) {
      var doc = aEvent.originalTarget;
      doc.addEventListener("DOMNodeInserted", function() {
        var doc = aEvent.originalTarget;
        if(!doc.esteidConvertLegacyRunning)
          esteidConvertLegacy(doc);
      }, false);
      esteidConvertLegacy(doc); }, false);
}
