// Start scripts are executed on every page load

/*function blockApplets(event) {
  const target = event.target;
  if (target.getElementsByTagName == undefined) return;
  
  //var applets = target.getElementsByTagName('applet');
  var applets = target.getElementsByTagName('embed');
  while (applets.length) {
    var applet = applets[0];
    var file = applet.archive == "" ? applets[0].code : applets[0].archive;
    blockResource(applet, event);
  }
}*/


function blockResource(tag, event) {
try {
  const tagName = tag.nodeName.toUpperCase();
  if (tagName == 'OBJECT' || tagName == 'EMBED' || tagName == 'APPLET') {
    if(esteidInspectObject(tag)) {
      esteidConvertObject(tag, document);
      if (event) {
        event.preventDefault();
        event.stopPropagation();
      }
    }
  }
} catch(err) { alert(err); }
}

(function init() {
  //if(safari.extension.settings.getItem('convertLegacy')) {
    esteidRegisterLegacyConverter(document);

    //document.addEventListener('DOMContentLoaded', blockApplets, false);
    //document.addEventListener('DOMNodeInserted', blockApplets, false);

    document.addEventListener('DOMContentLoaded', function(event) {
      //alert("Porno");
    }, false);

    // The beforeload event is WebKit specific
    document.addEventListener('beforeload', function(event) {
      blockResource(event.target, event);
      //alert(event);    
    }, true);
  //}
})();
