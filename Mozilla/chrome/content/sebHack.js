/*
 * esteid-browser-plugin - a browser plugin for Estonian EID card
 *
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

document.XMLSignApplet = createEstEidObject("SEBHackSigner");
