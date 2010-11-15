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

function esteid_log(msg) {
    alert(msg);
}

function esteidUSleep(millis) {
  var start = (new Date()).getTime();
  var now = 0;
  while(now - start < millis) {
    now = (new Date()).getTime();
  }
}

/* TODO: esteidHackTable */

function esteidInjectJS(doc, js) {
  var se = doc.createElement('script');
  se.type = "text/javascript";
  var pe = doc.documentElement;
  se.innerHTML = js;
  pe.appendChild(se);
}

/* TODO: function esteidLoadHacks(doc) */

function esteidConvertObject(o, doc) {
  var p    = o.parentNode;
  var id   = o.getAttribute("id");
  var name = o.getAttribute("name");
  var ns   = doc.documentElement.namespaceURI;

  if(p == null) return;

  /* Support ancient Java Applets "parameter driven" mode */
  var op = null, fInit = null, fSig, fCert, fCancel, fError, hash, slot;
  var params = o.getElementsByTagName('param');
  for(var i in params) {
    try {
      var pname = params[i].getAttribute("name");
      var value = params[i].getAttribute("value");
      switch(pname) {
        case "name"            : name    = value; break;
        case "OPERATION"       : op      = value; break;
        case "HASH"            : hash    = value; break;
        case "TOKEN_ID"        : slot    = value; break;
        case "FUNC_SET_CERT"   : fCert   = value; break;
        case "FUNC_SET_SIGN"   : fSig    = value; break;
        case "FUNC_DRIVER_ERR" : fError  = value; break;
        case "FUNC_CANCEL"     : fCancel = value; break;
        case "FUNC_INIT"       : fInit   = value; break;
        default: break;
      }
    } catch(err) { }
  }

  esteid_log('Converting ' + (id ? id : name) + ' to new plugin on page ' +
             doc.location.href);

  o = p.removeChild(o);
  delete o;

  if(id == null)
    id = "Converted" + Math.floor(Math.random() * 1000);

  var e = doc.createElementNS(ns, 'object');
  e.id           = id;
  e.name         = name;
  e.type         = 'application/x-esteid';
  e.style.width  = '1px';
  e.style.height = '1px';

  e = p.appendChild(e);

  /* Execute "parameter driven" mode functions */
  if(fInit) {
    esteidInjectJS(doc, fInit + "();");
  }

  if(op) {
    var cmd = "document.getElementById('" + id + "')";

    if(op == "FINALIZE") {
      cmd += ".finalize('" + slot + "','" + hash + 
             "','" + fSig + "','" + fCancel + "','" + fError + "');";
    } else {
      var rnd = Math.floor(Math.random() * 1000);
      cmd += ".prepare('window.CertHack" + rnd +
             "','" + fCancel + "','" + fError + "');";
      cmd += "function CertHack" + rnd + "(a, b) { " + fCert + "(b, a); }";
    }
    esteidInjectJS(doc, cmd);
  }
}

/* Try to find Java code attribute
 * NB! Please keep this code in sync with components/OldJavaBlocker.js
 */
function esteidFindJavaCodeAttr(elem) {
  var code = elem.getAttribute("code");

  if(!code)
    code = elem.getAttribute("java_code");
  if(!code) {
    var clsid = elem.getAttribute("classid");
    if(clsid) {
      var cre = new RegExp("^java:(.*)$", "i");
      var res = cre.exec(clsid);
      if(res)
        code = res[1];
    }
  }

  if(!code) {
    var params = elem.getElementsByTagName('param');
    for(var i in params) {
      try {
        pname = params[i].getAttribute("name");
        if(pname == "code" || pname == "java_code")
          code = params[i].getAttribute("value");
      } catch(err) { }
    }
  }

  return code;
}

function esteidInspectObject(e) {
  /* Regex to find ActiveX tags (YES some sites send those to Firefox too) */
  var axreg = new RegExp("FC5B7BD2-584A-4153-92D7-4C5840E4BC28", "i");
  var code = esteidFindJavaCodeAttr(e);
  if(code == "SignatureApplet.class" || code == "SignApplet.class" ||
     code == "XMLSignApplet.class")
    return true;
  else if(axreg.exec(e.getAttribute("classid")))
    return true;

  return false;
}

function esteidConvertLegacy(doc) {
  if(!doc.location) return; // Optimize

  doc.esteidConvertLegacyRunning = true;

  /* Find candidate objects */
  var ot = [ "applet", "embed", "object" ];
  for(var i in ot) {
    var els = doc.getElementsByTagName(ot[i]);
    for(var j = 0; j < els.length; j++) {
      var e = els[j];
      if(esteidInspectObject(e)) esteidConvertObject(e, doc);
    }
  }
  doc.esteidConvertLegacyRunning = false;
}

/* TODO: function esteidRegisterLegacyConverter() { */
