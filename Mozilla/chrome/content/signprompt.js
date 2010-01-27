var params = window.arguments[0];

function onType() {
  var pinbox = document.getElementById("pinbox");
  if(pinbox.textLength >= 5) {
    acceptButtonDisabled(false);
  } else {
    acceptButtonDisabled(true);
  }
}

function acceptButtonDisabled(stat) {
  var acceptButt = document.documentElement.getButton("accept")
  acceptButt.disabled = stat;
}

function doTimer() {
  if(params.forceClose) {
    params.timeout = 0;
    document.documentElement.cancelDialog();
    return;
  }

  if(params.timeout < 0) {
    params.timeout = 0;
    var cancelButt = document.documentElement.getButton("cancel");
    cancelButt.hidden = false;
  }

  var elt = document.getElementById("timer");
  elt.value = params.timeout;
  params.timeout--;
  window.setTimeout('doTimer()', 1000);
}

function setPinPadControls() {
  var pinbox     = document.getElementById("pinbox");
  var pinpadbox  = document.getElementById("pinpadbox");
  var acceptButt = document.documentElement.getButton("accept");
  var cancelButt = document.documentElement.getButton("cancel");
  pinbox.hidden = true;
  pinpadbox.hidden = false;
  acceptButt.hidden = true;
  cancelButt.hidden = true;
  canCancel = false;
}

function onLoad() {
  acceptButtonDisabled(true);
  doTimer();
  if(params.timeout > 0) setPinPadControls();

  /* Certificates on ID-card have their subjectCN fields in format:
   *    lastName,firstName,personalID
   * We split it here to show our prompt in a more human readable way:
   *    Firstname Lastname (PIN2)
   * If we can not split the CN properly, we show it as is */

  var subject;
  var sf = params.subject.split(",");
  if(sf.length == 3) {
    subject = sf[1] + " " + sf[0];
  } else {
    subject = params.subject;
  }

  var elt = document.getElementById('subject');
  elt.value = subject + " (PIN2)";

  var elt = document.getElementById('tries');
  elt.value = params.tries;

  var elt = document.getElementById('retrybox');
  if(params.retry) {
    elt.style.visibility = "visible";
  } else {
    elt.style.visibility = "collapse";
  }
}

function doOK() {
  var pinbox = document.getElementById("pinbox");
  params.out = pinbox.value;
  return true;
}

function doCancel() {
  // Allow user to close the dialog only when pinpad is not in use
  if(params.timeout > 0) return false;
  else return true;
}

function doDisclosure() {
  window.openDialog("chrome://esteid/content/signdetails.xul",
                    "_blank", "centerscreen,chrome,titlebar", params);
  return true;
}
