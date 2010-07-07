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
    e = doc.getElementById(id);
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
