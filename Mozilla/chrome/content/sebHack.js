/* Hack SEB U-Net to work with new plugins.
 *
 * TODO: Remove when SEB guys finally update their website
 */

function breakFreeFromFrames() {
  var doc = document;
  if (doc.defaultView.frameElement) {
    while (doc.defaultView.frameElement) {
      doc = doc.defaultView.frameElement.ownerDocument;
    }
  }
  return doc;
}

function findFormElement(f, name) {
    if(f.elements) return f.elements[name];
    else return null;
}

function getSignButton() {
  return document.getElementById("digikinnitan");
}

function showSignButton() {
  var e = getSignButton();
  e.style.visibility = "visible";
}

function hideSignButton() {
  var e = getSignButton();
  e.style.visibility = "hidden";
}

function findMainForm(doc) {
/*
  var mf = doc.getElementsByName("unmainForm")[0];
  if(mf == null) { mf = doc.getElementsByName("vorm")[0]; } 
*/

  var mf = null;
  var forms = doc.getElementsByTagName("form");
  for(i in forms) {
    if(findFormElement(forms[i], "allkirjastatav")) mf = forms[i];
  }
  if(mf == null) throw "Can't find main form";
  return mf;
}

function findHashForm(doc) {
  return doc.getElementsByName("hashreqform")[0];
}

function findSignObject(doc) {
  return doc.getElementById("AXSignObject");
}

/* Re-implementation of SEB's VBScript signing code in JS */
function DoDigiSignAX() {
  //alert("SEBHack DoDigiSignAX");
  if(digiprepsta == 0) {
    showSignButton();
    return false;
  }
  clearHashReqWatch();
  try {
    var doc        = breakFreeFromFrames();
    var e          = findSignObject(doc);
    var mf         = findMainForm(doc);
    var ifrm       = doc.getElementsByName("reqframeforhash")[0].contentDocument;
    var f          = findHashForm(ifrm);
    var statInput  = findFormElement(f, "status");
    var hashInput  = findFormElement(f, "signhash");
    var docidInput = findFormElement(f, "docid");
    var akInput    = findFormElement(mf, "allkiri");
    var codeInput  = findFormElement(mf, "code");

    if(statInput.value != "50" || hashInput.value == "" ||
       docidInput.value == "") {
        throw { message: "Request failed", sebError: true };
    }

    akInput.value = e.getSignedHash(hashInput.value, e.selectedCertNumber);
    codeInput.value = docidInput.value;
    if(akInput.value == "" || codeInput.value == "") {
        throw { message: "Signing failed", sebError: true };
    }

    doSubmit();
    return false;

  } catch(e) {
    digiprepsta = 0
    showSignButton();
    if(e.sebError) {
      alert(errMes4);
    } else {
      alert("SEBHack error: " + e);
    }
    return false;
  }
}

/* Re-implementation of SEB's VBScript signing code in JS */
function DoDigiSignHash() {
  //alert("SEBHack DoDigiSignHash");

  if(digiprepsta < 0) {
    alert(errMes1);
    clearHashReqWatch();
    return false;
  }

  if(digiprepsta == 1) {
    alert(errMes5);
    return false;
  }

  try {
    var doc       = breakFreeFromFrames();
    var e         = findSignObject(doc);
    var f         = findHashForm(doc);
    var mf        = findMainForm(doc);
    var certInput = findFormElement(f, "signCertHex");
    var ssInput   = findFormElement(f, "signStr");
    var alInput   = findFormElement(mf, "allkirjastatav");

    certInput.value = e.getSigningCertificate();
    ssInput.value = alInput.value;

    if(ssInput.value == "") {
      alert(errMes3);
      return false;
    }
      
    digiprepsta = 1;
    f.submit();

    return false;
  } catch(e) {
    alert("SEBHack error: " + e);
    return false;
  }
}


function appendToBody(e) {
  var b = document.getElementsByTagName("body")[0];
  return b.appendChild(e);
}

function createEstEidObject(id) {
  var e = document.createElement('span');
  e.innerHTML = '<object style="width: 1px; height: 1px;" id="' + id +
                      '" type="application/x-esteid" />';
  e = appendToBody(e);

  return document.getElementById(id);
}

/* Extract hash and docid from a response that looks like this:
 * <html xmlns:fo="http://www.w3.org/1999/XSL/Format">
 * <body>
 * <form name="hashreqform">
 * <input type="hidden" name="status" value="50">
 * <input type="hidden" name="signhash" value="...">
 * <input type="hidden" name="docid" value="...">
 * </form>
 * </body>
 * </html>
*/

function extractHashAndSign(res) {
  //alert("Result: " + res);

  var hashreg = new RegExp('name="signhash" value="(.*?)"', "i");
  var docidreg = new RegExp('name="docid" value="(.*?)"', "i");
  var hash = hashreg.exec(res)[1];
  var docid = docidreg.exec(res)[1];
  if(hash == null || docid == null || hash == "" || docid == "") {
    alert("SEBHack error: Unable to get hash code from server");
    showSignButton();
    window.CancelDigiSign();
  } else {
    //alert("Extracted Hash: " + hash + " DocId: " + docid);
    var e = document.getElementById("SEBHackSigner");
    var hex = e.getSignedHash(hash, e.selectedCertNumber);
    if(hex == "") {
      showSignButton();
      window.CancelDigiSign();
    }
    else {
      var mf        = findMainForm(document);
      var codeInput = findFormElement(mf, "code");
      codeInput.value = docid;
      window.SetDigiSign(hex);
    }
  }
}

/* Send a hash request to U-Net
 * An original request form looks like this:
 *
 * <form method="post" action="un3.w" name="hashreqform" target="reqframeforhash">
 * <input type="hidden" name="signCertHex">
 * <input type="hidden" name="signStr" value="...">
 * <input type="hidden" name="act" value="dshash">
 * <input type="hidden" name="sesskey" value="...">
 * <input type="hidden" name="frnam" value="0">
 * <input type="hidden" name="unetmenuhigh" value="">
 * <input type="hidden" name="unetmenulow" value="">
 * <input type="hidden" name="unetmenulowdiv" value="">
 * <input type="hidden" name="lang" value="EST">
 * </form>
 */

function getHashFromUnet(cert) {
  var url = "un3.w";

  try {
    var http       = new XMLHttpRequest();
    var doc        = breakFreeFromFrames();
    var mf         = findMainForm(doc);
    var alInput    = findFormElement(mf, "allkirjastatav");
    var skInput    = findFormElement(mf, "sesskey");
    var codeInput  = findFormElement(mf, "code");

    if(codeInput == null) {
      codeInput = document.createElement("input");
      codeInput.type = "hidden";
      codeInput.name = "code";
      codeInput = mf.appendChild(codeInput);
    }

    var params  = "signCertHex=" + encodeURIComponent(cert);
        params += "&signStr="    + encodeURIComponent(alInput.value);
        params += "&sesskey="    + encodeURIComponent(skInput.value);
        params += "&act=dshash";
        params += "&frnam=0&unetmenuhigh=&unetmenulow=&unetmenulowdiv=&lang=EST";

    http.open("POST", url, true);
    http.setRequestHeader("Content-type", "application/x-www-form-urlencoded");
    http.setRequestHeader("Content-length", params.length);
    http.setRequestHeader("Connection", "close");

    http.onreadystatechange = function() {
      if(http.readyState == 4 && http.status == 200) {
        var res = http.responseText;
        extractHashAndSign(res);
      } // FIXME: handle HTTP errors
    };
    http.send(params);
  } catch(e) {
    alert("SEBHack error: " + e);
  }
}

/* Re-implementation of SEB's signing code to use new plugins */
function DoDigiSign() {
  //alert("SEBHack DoDigiSign");
  try {
    var e = document.getElementById("SEBHackSigner");
    if(e == null) e = createEstEidObject("SEBHackSigner");
    var cert = e.getSigningCertificate();
    if(cert == "") {
      window.CancelDigiSign();
    } else {
      hideSignButton();
      getHashFromUnet(cert);
    }
  } catch(e) {
    alert("SEBHack error: " + e);
  }
  return false;
}
