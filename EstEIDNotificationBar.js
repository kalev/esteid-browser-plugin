/* The variable assignment in this file MUST be polylanguage eg. it MUST
 * work when included into JavaScript and it must work when included
 * into C++ code. This makes it easy to test it.
 * See TestEsteidNotificationBarJS.html
 *
 * FIXME: IE only supports position: fixed when in standards compliant mode
 */

EstEIDNotificationBarScript = "\
document.EstEIDNotificationBar = { \
  makeBar: function(color, slabel) { \
    /* Create notification bar div */ \
    var bar = document.createElement('div'); \
    bar.style.fontSize = '110%'; \
    bar.style.backgroundColor = color; \
    bar.style.position = 'fixed'; \
    bar.style.top = '0px'; \
    bar.style.left = '0px'; \
    bar.style.right = '0px'; \
    bar.style.padding = '3px'; \
    bar.style.zIndex = '2147483647'; /* Make sure the bar is always on top */ \
    bar.style.display = 'none'; /* Hide it for a start */ \
    /* Create button bar div */ \
    var btnbar = document.createElement('div'); \
    btnbar.style.cssFloat = 'right'; \
    btnbar.style.styleFloat = 'right'; /* IE */ \
    btnbar.style.width = '10em'; \
    btnbar.style.textAlign = 'right'; \
    if(slabel) { \
      /* Settings button */ \
      var btn = document.createElement('input'); \
      btn.type = 'button'; \
      btn.value = slabel; \
      btn.style.marginLeft = '3px'; \
      if(!btn.addEventListener) btn.attachEvent('onclick', this.settings); \
      else btn.addEventListener('click', this.settings, false); \
      this.settingsBtn = btnbar.appendChild(btn); \
    } \
    /* Close button */ \
    var btn = document.createElement('input'); \
    btn.type = 'button'; \
    btn.value = ' x '; \
    btn.style.marginLeft = '3px'; \
    if(!btn.addEventListener) btn.attachEvent('onclick', this.close); \
    else btn.addEventListener('click', this.close, false); \
    btnbar.appendChild(btn); \
    bar.appendChild(btnbar); \
    /* Message div */ \
    var text = document.createElement('div'); \
    text.style.marginLeft = '2em'; \
    /* We can't inject divs into DOMDocument, we MUST find body tag */ \
    var body = document.getElementsByTagName('body')[0]; \
    return { textDiv: bar.appendChild(text), barDiv: body.appendChild(bar) }; \
  }, \
  create: function(slabel, sl) { \
    this.callBack = sl; \
    this.warnObj = this.makeBar('#ffff66', slabel); \
    this.errObj = this.makeBar('#ff6666', null); \
  }, \
  show: function(msg) { \
    if(!this.warnObj) return; \
    this.warnObj.textDiv.innerHTML = msg; \
    this.warnObj.barDiv.style.display = 'block'; \
  }, \
  showError: function(msg) { \
    if(!this.errObj) return; \
    this.errObj.textDiv.innerHTML = msg; \
    this.errObj.barDiv.style.display = 'block'; \
  }, \
  settings: function(e) { \
    document.EstEIDNotificationBar.callBack.handleEvent(e); \
  }, \
  close: function() { \
    document.EstEIDNotificationBar.warnObj.barDiv.style.display = 'none'; \
    document.EstEIDNotificationBar.errObj.barDiv.style.display = 'none'; \
  }, \
  warnObj: null, \
  errObj: null, \
  callBack: null, \
  settingsBtn: null \
};\
";
