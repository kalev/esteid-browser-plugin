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

/*
function makePluginObj(id) {
  const namespaceURI = document.documentElement.namespaceURI;
  var pluginObj = document.createElementNS(namespaceURI, 'div');
  pluginObj.innerHTML = "Java! Kaob uttu ja kohe!";
  pluginObj.style.height = "100px";
  pluginObj.id = id;

  return pluginObj;
}
*/

function esteidConvertObject(o, doc) {
  var p    = o.parentNode;
  var id   = o.getAttribute("id");
  var name = o.getAttribute("name");
  var ns   = doc.documentElement.namespaceURI;

  if(p == null) return;

  /* Support ancient Java Applets "parameter driven" mode */
///*
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
//*/
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
//*
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
//*/
}

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
}

