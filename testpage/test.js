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

var esteid = null;
var bgok = "rgb(50, 255, 50)";
var bgerr = "rgb(255, 50, 50)";

var names = [ "firstName", "lastName", "middleName", "sex",
              "citizenship", "birthDate", "personalID", "documentID",
              "expiryDate", "placeOfBirth", "issuedDate", "residencePermit",
              "comment1", "comment2", "comment3", "comment4"
];

function initpdatatable() {
    var e = document.getElementById("persData");
    for(var i in names) {
        var r = e.insertRow(e.rows.length);
        var el = document.createElement('span');
	el.id = "s" + names[i];
	r.insertCell(0).innerHTML = names[i];
	r.insertCell(1).appendChild(el);
    }
}

var cnames = [ "CN", "issuerCN", "validFrom", "validTo",
              "keyUsage", "serial" //, "isValid"
];

function initcerttable(n) {
    var e = document.getElementById(n + "CertData");
    for(var i in cnames) {
        var r = e.insertRow(e.rows.length);
        var el = document.createElement('span');
        el.id = n + "Cert" + cnames[i];
        r.insertCell(0).innerHTML = cnames[i];
        r.insertCell(1).appendChild(el);
    }
}


function addEvent(obj, name, func)
{
    if (window.addEventListener) {
        obj.addEventListener(name, func, false); 
    } else {
        obj.attachEvent("on"+name, func);
    }
}


function inittest() {
    initpdatatable();
    initcerttable("auth");
    initcerttable("sign");

    esteid = document.getElementById("esteid");

    var e = document.getElementById("eidver");
    try {
        e.innerHTML = esteid.getVersion();
	e.style.background = bgok;
    }
    catch(err) {
        e.innerHTML = "Error: " + err.message;
	e.style.background = bgerr;
    }
    try {
        addEvent(esteid, "CardInserted", cardInserted);
        addEvent(esteid, "CardRemoved", cardRemoved);
        addEvent(esteid, "SignSuccess", signSuccess);
        addEvent(esteid, "SignFailure", signFailure);
    }
    catch(err) {
        var e = document.getElementById("cardstatus");
        e.innerHTML = "Error: " + err.message;
        e.style.background = bgerr;
    }
}

function cardInserted(reader) {
    var e = document.getElementById("cardstatus");
    e.innerHTML = "Sees";
    e.style.background = bgok;
    var c = document.getElementById("autoload");
    if(c.checked) {
        readPersonalData();
        readCertificateData("auth");
        readCertificateData("sign");
    }
}
function cardRemoved(reader) {
    var e = document.getElementById("cardstatus");
    e.innerHTML = "Väljas";
    e.style.background = "yellow";
    clearPersonalData();
    clearCertificateData("auth");
    clearCertificateData("sign");
}

function readPersonalData() {
    var stat = document.getElementById("pdstatus");
    try {
        for(var i in names) {
            var e = document.getElementById("s" + names[i]);
            e.innerHTML = esteid[names[i]];
        }
        stat.innerHTML = "OK";
        stat.style.background = "";
    }
    catch(err) {
        clearPersonalData();
        stat.innerHTML = "Error: " + err.message;
        stat.style.background = bgerr;
    }
}

function clearPersonalData() {
    for(var i in names) {
        var e = document.getElementById("s" + names[i]);
        e.innerHTML = "Not Available";
    }
}

function readCertificateData(n) {
    showCert(n);
    var stat = document.getElementById(n + "CertStatus");
    try {
        var cert = esteid[n + "Cert"];
        for(var i in cnames) {
            var e = document.getElementById(n + "Cert" + cnames[i]);
            e.innerHTML = cert[cnames[i]];
        }
        var e = document.getElementById(n + "CertDump");
        // Stupid IE will forget preformatting when assigning innerHTML
        // so we have to add another set of tags here.
        e.innerHTML = "<pre>" + cert.cert + "</pre>";
        stat.innerHTML = "OK";
        stat.style.background = "";
    }
    catch(err) {
        clearCertificateData(n);
        stat.innerHTML = "Error: " + err.message;
        stat.style.background = bgerr;
    }
}

function clearCertificateData(n) {
    showCert(n);
    for(var i in cnames) {
        var e = document.getElementById(n + "Cert" + cnames[i]);
        e.innerHTML = "Not Available";
    }
}

function showCert(n) {
    var tbl = document.getElementById(n + "CertData");
    var dmp = document.getElementById(n + "CertDump");
    tbl.style.display = "block";
    dmp.style.display = "none";
}

function showDump(n) {
    readCertificateData(n);
    var tbl = document.getElementById(n + "CertData");
    var dmp = document.getElementById(n + "CertDump");
    tbl.style.display = "none";
    dmp.style.display = "block";
}

function testPersonalData() {
    readPersonalData();
}

// Test signing with a dummy hash
function testSign() {
    var e = document.getElementById("signCertStatus");
    var hash = document.getElementById("testhash").value;
    var url = "https://id.smartlink.ee/plugin_tests/test.ddoc";

    e.innerHTML = "Started signing";
    e.style.background = "";
    esteid.signAsync(hash, url, {
        onSuccess: function(hex) { signSuccess(hex); },
        onError:   function(msg) { signFailure(msg); }
    }); 
}

// Callback for returning signed hash
function signSuccess(signedData) {
    var e = document.getElementById("signCertStatus");

    e.innerHTML = "OK: " + signedData;
    e.style.background = "";
}

// Callback for signing failure
function signFailure(message) {
    var e = document.getElementById("signCertStatus");

    e.innerHTML = "Error: " + message;
    e.style.background = bgerr;
}

function runCrashCode(code) {
    var e = document.getElementById("crashstatus");
    try {
        eval(code);
	e.innerHTML += '<span class="red">' + code + " did not throw exception</span>";
    }
    catch(err) {
        e.innerHTML += code + ": " + err.message;
    }
    e.innerHTML += "<br />";
}

function testCrash() {
    var e = document.getElementById("crashstatus");
    e.innerHTML = "";
    runCrashCode('addEvent(esteid, null, null);');
    runCrashCode('addEvent(esteid, "huinamuina", null);');
    runCrashCode('addEvent(esteid, "", null);');
    runCrashCode('addEvent(esteid, "CardInserted", null);');
    runCrashCode('addEvent(esteid, "CardRemoved", "");');
    runCrashCode('addEvent(esteid, "CardRemoved", 0);');
    runCrashCode('esteid.signAsync(null, null, null);');
    runCrashCode('esteid.signAsync("", "", "");');
    runCrashCode('esteid.signAsync(0, 0, 0);');
    runCrashCode('esteid.signAsync("", "", { onSuccess: nosuchfunction, onError: nosuchfunction });'); 
    runCrashCode('esteid.signAsync("", "", { onSuccess: function(hex) { }, });'); 
    runCrashCode('esteid.signAsync("", "", { onError:   function(msg) { } });'); 
    runCrashCode('esteid.signAsync("", "", {});');
}

function testWhatever() {
    var certList = signer.getCertificateList();
    var certArr = certList.split(",");
    for(i = 0; i < certArr.length; i++){
       dbgArea.value+= certArr[i] + "\r";
    }
    if (certArr.length > 1 ) {
        var cert= signer.getCertificateByThumbprint(certArr[1]);
        if (cert) {
            dbgArea.value+= cert.CN + "\r" + cert.issuerCN + "\r" ;
            signer.sign("blobofdata",cert);
        }
    }
}
