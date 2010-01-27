var params = window.arguments[0];
var eidui = null;
var sb = null;

function updateUI() {
  var _wl = eidui.wl.split(",");
  var elt = document.getElementById('whitelist');

  for(var i = 0; i < _wl.length; i++)
    elt.appendItem(_wl[i]);

  var elt = document.getElementById('log');
  elt.value = eidui.log;
}

function onLoad() {
  var sbs = Components.classes["@mozilla.org/intl/stringbundle;1"]
                      .getService(Components.interfaces.nsIStringBundleService);
  eidui = Components.classes["@id.eesti.ee/esteid-private;1"]
                    .getService(Components.interfaces.nsIEstEIDPrivate);
  sb = sbs.createBundle("chrome://esteid/locale/esteid.properties");

  updateUI();

  if(params.host) {
    var elt = document.getElementById('addhost');
    elt.value = params.host;
  }
}

function removeSelected() {
  var elt = document.getElementById('whitelist');
  var i = elt.selectedIndex;
  if(i != -1) elt.removeItemAt(i);
}

function enableDelBtn() {
  var elt = document.getElementById('whitelist');
  var btn = document.getElementById('delbtn');
  if(elt.selectedIndex == -1)
    btn.disabled = true;
  else
    btn.disabled = false;
}

function addHost() {
  var elt = document.getElementById('addhost');
  var wl = document.getElementById('whitelist');

  if(elt.value) {
    var re1 = new RegExp("^https://(.*?)(/|$)");
    var m = re1.exec(elt.value);
    if(m) {
        wl.appendItem(m[1]);
        elt.value = '';
    }
    else {
      var re2 = new RegExp("^[^/;:,!#$%^&*()_=?]+$");
      var m = re2.exec(elt.value);
      if(m) {
          wl.appendItem(elt.value);
          elt.value = '';
      }
      else {
          alert(sb.GetStringFromName("invalidhost"));
      }
    }
  }
}

function doOK() {
  var wl = document.getElementById('whitelist');
  var _wl = [];
  
  for(var i = 0; i < wl.itemCount; i++) {
    _wl.push(wl.getItemAtIndex(i).label);
  }

  eidui.setWhitelist(_wl.join(","));
}
