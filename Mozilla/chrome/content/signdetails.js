var params = window.arguments[0];

function onLoad() {
  var elt = document.getElementById('url');
  elt.textContent = params.docUrl;

  var elt = document.getElementById('urltiptext');
  elt.value = params.docUrl;

  var elt = document.getElementById('hash');
  elt.value = params.docHash;

  var elt = document.getElementById('page');
  elt.textContent = params.pageUrl;
}

function openDoc() {
  /* We can't just use window.open because then the new browser window
   * will be modal and the user can't continue without closing it. Also
   * all sort of strange UI quirks show up when running a modal browser.
   */
  var ww = Components.classes["@mozilla.org/embedcomp/window-watcher;1"]
                     .getService(Components.interfaces.nsIWindowWatcher);

  /* So far so good, but without parent window, openWindow is 
   * unable to set proper display features. We can't use our real parent
   * because then the document will be opened in new tab which can't be
   * interacted with because the modal PIN prompt is open.
   * So we force another open an then close the intermediary.
   * this is a bloody hack, but it seems to work
   *
   * FIXME: Find a sane way to do this
   */
  var win = ww.openWindow(null, null, null, null, null);
  win.open(params.docUrl, "_blank");
  win.close();
}
