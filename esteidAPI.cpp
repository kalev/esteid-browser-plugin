/*
 * esteid-browser-plugin - a browser plugin for Estonian EID card
 *
 * Copyright (C) 2010  Estonian Informatics Centre
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

#include "BrowserObjectAPI.h"
#include "variant_list.h"
#include "DOM/JSAPI_DOMDocument.h"
#include "DOM/JSAPI_DOMWindow.h"
#include "Util/JSArray.h"
#include "config.h"

#ifdef _WIN32
#include "Win/WindowsUI.h"
#else
#ifdef __APPLE__
#include "Mac/MacUI.h"
#else
#include "X11/GtkUI.h"
#endif
#endif

#include "esteidAPI.h"
#include "JSUtil.h"
#include "converter.h"
#include "debug.h"

/* UI Messages */
#define MSG_SETTINGS "Settings"
#define MSG_SITEACCESS "This site is trying to obtain access to your ID-card."
#define MSG_INSECURE "Access to ID-card was denied because the connection to the site is not secure."

#define REGISTER_METHOD(a)      JS_REGISTER_METHOD(esteidAPI, a)
#define REGISTER_RO_PROPERTY(a) JS_REGISTER_RO_PROPERTY(esteidAPI, a)

#define WHITELIST_REQUIRED \
    if(!IsSecure()) { \
        DisplayNotification(MSG_INSECURE); \
        ESTEID_ERROR_NO_PERMISSION; \
    } else if(!IsWhiteListed()) { \
        DisplayNotification(MSG_SITEACCESS); \
        ESTEID_ERROR_NO_PERMISSION; \
    }

// FIXME: Set error codes
#define ESTEID_ERROR_FROMCARD(e) { \
    ESTEID_DEBUG("Card error: %s", e.what()); \
    throw FB::script_error(e.what()); }
#define ESTEID_ERROR_CARD_ERROR(m) { \
    ESTEID_DEBUG("Card error: %s", m); \
    throw FB::script_error(m); }
#define ESTEID_ERROR_INVALID_ARG { \
    throw FB::script_error("Invalid argument"); }
#define ESTEID_ERROR_USER_ABORT { \
    throw FB::script_error(CANCEL_MSG); }
#define ESTEID_ERROR_NO_CARD { \
    throw FB::script_error("No cards found"); }
#define ESTEID_ERROR_NO_PERMISSION ESTEID_ERROR_NO_CARD

esteidAPI::esteidAPI(FB::BrowserHostWrapper *host) :
    m_host(host), m_authCert(NULL), m_signCert(NULL),
    m_service(EstEIDService::getInstance()),
    m_settingsCallback(new SettingsCallback(host, *this)),
    m_closeCallback(new CloseCallback(host, *this)),
    m_uiCallback(new UICallback(*this))
{
    ESTEID_DEBUG("esteidAPI::esteidAPI()");

    /* Load JavaScript code to be evaluated in browser */
    #include "EstEIDNotificationBar.js"

    REGISTER_METHOD(getVersion);
    REGISTER_METHOD(signAsync);

    registerEvent("onCardInserted");
    registerEvent("onCardRemoved");
    registerEvent("onReadersChanged");

/*  FIXME: Those will be catched by firebreath itself for
           NPAPI plugins, but how about ActiveX?
    REGISTER_METHOD(addEventListener);
    REGISTER_METHOD(removeEventListener);
*/

    REGISTER_RO_PROPERTY(authCert);
    REGISTER_RO_PROPERTY(signCert);
    REGISTER_RO_PROPERTY(lastName);
    REGISTER_RO_PROPERTY(firstName);
    REGISTER_RO_PROPERTY(middleName);
    REGISTER_RO_PROPERTY(sex);
    REGISTER_RO_PROPERTY(citizenship);
    REGISTER_RO_PROPERTY(birthDate);
    REGISTER_RO_PROPERTY(personalID);
    REGISTER_RO_PROPERTY(documentID);
    REGISTER_RO_PROPERTY(expiryDate);
    REGISTER_RO_PROPERTY(placeOfBirth);
    REGISTER_RO_PROPERTY(issuedDate);
    REGISTER_RO_PROPERTY(residencePermit);
    REGISTER_RO_PROPERTY(comment1);
    REGISTER_RO_PROPERTY(comment2);
    REGISTER_RO_PROPERTY(comment3);
    REGISTER_RO_PROPERTY(comment4);

#ifdef SUPPORT_OLD_APIS
    REGISTER_METHOD(getCertificates);
    REGISTER_METHOD(sign);
    REGISTER_METHOD(getInfo);
    REGISTER_METHOD(getSigningCertificate);
    REGISTER_METHOD(getSignedHash);
    REGISTER_RO_PROPERTY(selectedCertNumber);
    REGISTER_METHOD(prepare);
    REGISTER_METHOD(finalize);
#endif

    m_pageURL = GetPageURL();
    ESTEID_DEBUG("esteidAPI: Page URL is %s", m_pageURL.c_str());

    /* Use platform specific UI */
#ifdef _WIN32
    ESTEID_DEBUG("Trying to load WindowsUI");
    m_UI = boost::shared_ptr<PluginUI>(new WindowsUI(m_uiCallback));
#else
#ifdef __APPLE__
    ESTEID_DEBUG("Trying to load MacUI");
    m_UI = boost::shared_ptr<PluginUI>(new MacUI(m_uiCallback));
#else
    ESTEID_DEBUG("Trying to load GtkUI");
    m_UI = boost::shared_ptr<PluginUI>(new GtkUI(m_uiCallback));
#endif
#endif

#if 0
    /* Die if UI initialization fails */
    if(!m_UI)
        throw FB::script_error("Unable to load plugin user interface");
#endif

    m_service->AddObserver(this);
}

esteidAPI::~esteidAPI()
{
    ESTEID_DEBUG("esteidAPI::~esteidAPI()");

    m_service->RemoveObserver(this);
}

bool esteidAPI::IsLocal() {
    // FIXME: This code is butt-ugly!
    if(!m_pageURL.compare(0,  7, "file://"))            return true;
    if(!m_pageURL.compare(0, 17, "http://localhost/"))  return true;
    if(!m_pageURL.compare(0, 18, "https://localhost/")) return true;
    if(!m_pageURL.compare(0, 17, "http://localhost:"))  return true;
    if(!m_pageURL.compare(0, 18, "https://localhost:")) return true;
    return false;
}

bool esteidAPI::IsSecure() {
    if(!m_pageURL.compare(0, 8, "https://")) return true;
    if(m_conf.allowLocal && IsLocal())          return true;
    return false;
}

bool esteidAPI::IsWhiteListed() {
    if (m_conf.allowLocal && IsLocal())
        return true;

    std::string host = GetHostName();
    if (host.empty())
        return false;
    if (m_conf.InWhitelist(host))
        return true;

    return false;
}

std::string esteidAPI::GetHostName() {
    size_t pos1 = m_pageURL.find("://") + 3, pos2 = m_pageURL.find("/", pos1);
    if (pos1 >= pos2)
        return "";
    std::string host = m_pageURL.substr(pos1, pos2 - pos1);

    return host;
}

std::string esteidAPI::GetPageURL(void) {
    /* Using method no. 1 from
     * https://developer.mozilla.org/en/Getting_the_page_URL_in_NPAPI_plugin
     */
    FB::JSAPI_DOMWindow dw = m_host->getDOMWindow();
    FB::JSAPI_DOMNode loc = dw.getProperty<FB::JSObject>("location");
    m_pageURL = loc.getProperty<std::string>("href");

    return m_pageURL;
}

void esteidAPI::CreateNotificationBar(void) {
    m_host->evaluateJavaScript(EstEIDNotificationBarScript);
    m_barJSO = m_host->getDOMDocument()
               .getProperty<FB::JSObject>("EstEIDNotificationBar");
    m_barJSO->Invoke("create",
                     FB::variant_list_of(MSG_SETTINGS)(m_settingsCallback));
}

void esteidAPI::DisplayError(std::string msg) {
    try {
        OpenNotificationBar();
        m_barJSO->Invoke("showError", FB::variant_list_of(msg));
    } catch(std::exception &e) {
        ESTEID_DEBUG("Unable to display error: %s", e.what());
    }
}

void esteidAPI::DisplayNotification(std::string msg) {
    try {
        OpenNotificationBar();
        m_barJSO->Invoke("show", FB::variant_list_of(msg));
    } catch(std::exception &e) {
        ESTEID_DEBUG("Unable to display notification: %s", e.what());
    }
}

void esteidAPI::OpenNotificationBar(void) {
    if(!m_barJSO) {
        CreateNotificationBar();
    }
}

void esteidAPI::CloseNotificationBar(void) {
    if(!m_barJSO) return;

    m_barJSO->Invoke("close", FB::variant_list_of(0));
}

void esteidAPI::ShowSettings(void) {
    ESTEID_DEBUG("esteidAPI::ShowSettings()");

    try {
        if (IsSecure())
            m_UI->ShowSettings(m_conf, GetHostName());
        else
            m_UI->ShowSettings(m_conf);
    } catch(const std::exception& e) {
        ESTEID_DEBUG("Unable to display whitelist editor: %s", e.what());
    }

    CloseNotificationBar();
}

void esteidAPI::onMessage(EstEIDService::msgType e, readerID i) {
    //const char *evtname;
    std::string evtname;

    switch(e) {
        case EstEIDService::CARD_INSERTED:   evtname = "CardInserted";  break;
        case EstEIDService::CARD_REMOVED:    evtname = "CardRemoved";   break;
        case EstEIDService::READERS_CHANGED: evtname = "ReadersChanged";break;
        default: throw std::runtime_error("Invalid message type"); break;
    }
    ESTEID_DEBUG("onMessage: %s %d", evtname.c_str(), i);

    if(!IsWhiteListed()) return;

    /* FIXME: Prefixing every event name with an additional "on" is a bloody
              hack. We either need to fix firebreath or our JS API spec. */
    FireEvent("on" + evtname, FB::variant_list_of(i));
}

void esteidAPI::UpdatePersonalData()
{
    RTERROR_TO_SCRIPT(m_service->readPersonalData(m_pdata));
}

// TODO: Optimize memory usage. Don't create new object if cert hasn't changed.
FB::JSOutObject esteidAPI::get_authCert()
{
    WHITELIST_REQUIRED;

    RTERROR_TO_SCRIPT(
        return new CertificateAPI(m_host, m_service->getAuthCert()));
}

FB::JSOutObject esteidAPI::get_signCert()
{
    WHITELIST_REQUIRED;

    RTERROR_TO_SCRIPT(
        return new CertificateAPI(m_host, m_service->getSignCert()));
}

std::string esteidAPI::getVersion()
{
    return FBSTRING_PLUGIN_VERSION;
}


void esteidAPI::signAsync(std::string hash, std::string url, const FB::JSObject callback)
{
    WHITELIST_REQUIRED;

    m_signCallback = callback;

    startSign(hash, url);
}

void esteidAPI::startSign(std::string hash, std::string url) {
    std::string subjectRaw;

    /* Extract subject line from Certificate */
    try {
        subjectRaw = static_cast<CertificateAPI*>(get_signCert().ptr())->get_CN();
    } catch(const FB::script_error& e) {
        returnSignFailure(e.what());
        return;
    }

    m_subject = subjectToHumanReadable(subjectRaw);
    m_hash = hash;
    m_url = url;

    promptForSignPIN();
}


void esteidAPI::promptForSignPIN(bool retrying)
{
    int triesLeft;
    bool pinpad;

    if (m_subject.empty()) {
        returnSignFailure("Empty subject");
        return;
    }

    if (m_url.empty()) {
        returnSignFailure("Partial document URL must be specified");
        return;
    }

    // FIXME: Hardcoded SHA1 support
    if (m_hash.length() != 40) {
        returnSignFailure("Invalid hash");
        return;
    }

#if 0
    try {
        pinpad = m_service->hasSecurePinEntry();
    } catch(std::runtime_error &e) {
        returnSignFailure(e.what());
        return;
    }
#else
    pinpad = false;
#endif

    triesLeft = getPin2RetryCount();
    if (triesLeft <= 0) {
        m_UI->ShowPinBlockedMessage(2);
        returnSignFailure("PIN2 locked");
        return;
    }

    try {
        m_UI->PromptForSignPIN(m_subject, m_url, m_hash,
                               pinpad, retrying, triesLeft);
    } catch(const std::exception& e) {
        returnSignFailure(e.what());
        return;
    }
}


void esteidAPI::onPinEntered(std::string pin)
{
    std::string hash;

    if (pin.empty()) {
        // Shouldn't happen
        ESTEID_DEBUG("sign: got empty PIN from UI");
        returnSignFailure("empty PIN");
        return;
    }

    try {
        hash = m_service->signSHA1(m_hash, EstEidCard::SIGN, pin);
    } catch(AuthError &e) {
        if (e.m_aborted) { // pinpad
            ESTEID_DEBUG("sign: cancel pressed on PinPAD");
            returnSignFailure("pinpad operation cancelled");
            return;
        }

        // ask again for PIN
        promptForSignPIN(true);
        return;
    } catch(std::runtime_error &e) {
        returnSignFailure(e.what());
        return;
    }

    if (hash.empty()) {
        // Shouldn't happen
        returnSignFailure("empty hash");
        return;
    }

    returnSignedData(hash);
}


void esteidAPI::returnSignedData(const std::string& data)
{
#ifdef SUPPORT_OLD_APIS
    m_hex = data;
#endif

    if(!m_signCallback) return;

    try {
        m_signCallback->Invoke("onSuccess", FB::variant_list_of(data));
    } catch(const FB::script_error&) {
        returnSignFailure("Error executing JavaScript code");
    }
}


void esteidAPI::returnSignFailure(const std::string& msg)
{
#ifdef SUPPORT_OLD_APIS
    m_err = msg;
#endif

    if(!m_signCallback) return;

    try {
        m_signCallback->Invoke("onError", FB::variant_list_of(msg));
    } catch(const FB::script_error&) {
        // can't really do anything useful here
    }
}

#ifdef SUPPORT_OLD_APIS

#define MAGIC_ID "37337F4CF4CE"
#define COMPAT_URL "http://code.google.com/p/esteid/wiki/OldPluginCompatibilityMode"
#define DEPRECATED_CALL DisplayError("Website is using old signature APIs. Please contact site owner. Click <a href=\"" COMPAT_URL "\" target=\"_blank\" style=\"color: blue;\">here</a> for details.");

std::string esteidAPI::getCertificates() {
    WHITELIST_REQUIRED;
    DEPRECATED_CALL;

    try { RTERROR_TO_SCRIPT(
        ByteVec bv = m_service->getSignCert();
        X509Certificate cert(bv);
        std::ostringstream buf;

        /* Return "compatible" JSON */
        buf << "({certificates:[{";
        buf << "id:'" << MAGIC_ID << "',";
        buf << "cert:'";
        for(ByteVec::const_iterator it = bv.begin(); it!=bv.end();it++)
            buf << std::setfill('0') << std::setw(2) << std::hex << (short)*it;
        buf << "',";
        buf << "CN:'" << cert.getSubjectCN() << "',";
        buf << "issuerCN:'" << cert.getIssuerCN() << "',";
        // JS using this old API expects the exact string "Non-Repudiation"
        buf << "keyUsage:'Non-Repudiation',";
        buf << "validFrom: new Date(),"; // TODO: Date(YYYY,MM,DD,HH,mm,SS)
        buf << "validTo: new Date()}],"; // TODO: Date(YYYY,MM,DD,HH,mm,SS)
        buf << "returnCode:0})";

        return buf.str();

    // TODO: Return proper error code from plugin (when it's implemented)
    )} catch(...) { return "({returnCode: 12})"; }
}

std::string esteidAPI::sign(std::string a, std::string b) {
    WHITELIST_REQUIRED;
    DEPRECATED_CALL;

    m_signCallback = NULL;
    m_hex = "";

    if(!a.compare(MAGIC_ID)) { // Old Mozilla Plugin compat mode
        startSign(b, std::string(COMPAT_URL));
        m_UI->WaitForPinPrompt();
        if(!m_hex.empty()) {
            std::string rv = "({signature:'" + m_hex + "', returnCode: 0})";
            return rv;
        // TODO: Return proper error code from plugin (when it's implemented)
        } else { return "({returnCode: 12})"; }
    } else { // New plugin blocking API compatibility mode
        startSign(a , (b.empty()) ? std::string(COMPAT_URL) : b);
        m_UI->WaitForPinPrompt();

        if(!m_hex.empty()) return m_hex;
        else throw FB::script_error(m_err);
    }
}

std::string esteidAPI::getInfo() {
    DEPRECATED_CALL;

    return getVersion();
}

std::string esteidAPI::getSigningCertificate() {
    WHITELIST_REQUIRED;
    DEPRECATED_CALL;

    try {
        ByteVec bv = m_service->getSignCert();
        std::ostringstream buf;

        for(ByteVec::const_iterator it = bv.begin(); it!=bv.end();it++)
            buf << std::setfill('0') << std::setw(2) << std::hex << (short)*it;

        return buf.str();
    } catch(...) { return ""; } // This API returns nothing on Error
}

std::string esteidAPI::getSignedHash(std::string hash, std::string slot) {
    WHITELIST_REQUIRED;
    DEPRECATED_CALL;

    m_signCallback = NULL;
    m_hex = "";

    startSign(hash, std::string(COMPAT_URL));
    m_UI->WaitForPinPrompt();

    return m_hex; // This API returns nothing on error
}

std::string esteidAPI::get_selectedCertNumber() {
    DEPRECATED_CALL;

    return "10"; // Dummy number
}

void esteidAPI::prepare(std::string onSuccess, std::string onCancel,
    std::string onError) {
    WHITELIST_REQUIRED;
    DEPRECATED_CALL;

    try {
        ByteVec bv = m_service->getSignCert();
        std::ostringstream buf;

        for(ByteVec::const_iterator it = bv.begin(); it!=bv.end();it++)
            buf << std::setfill('0') << std::setw(2) << std::hex << (short)*it;

        m_host->evaluateJavaScript(onSuccess + "(10, '" + buf.str() + "');");
    } catch(std::runtime_error &e) {
        m_host->evaluateJavaScript(onError + "(12, '" + e.what() + "');");
    }
}

void esteidAPI::finalize(std::string slot, std::string hash,
    std::string onSuccess, std::string onCancel, std::string onError) {
    WHITELIST_REQUIRED;
    DEPRECATED_CALL;

    /* FIXME: The original API is non-blocking, but the callbacks
       are so braindead (callback function name is passed as a string)
       so we implement the compatibility version as a blocking call for now */

    m_signCallback = NULL;
    m_hex = "";

    startSign(hash, std::string(COMPAT_URL));
    m_UI->WaitForPinPrompt();

    if(!m_hex.empty())
        m_host->evaluateJavaScript(onSuccess + "('" + m_hex + "');");
    else
        m_host->evaluateJavaScript(onCancel + "();");
}
#endif

int esteidAPI::getPin2RetryCount() {
    try {
        byte puk, pin1, pin2;
        m_service->getRetryCounts(puk, pin1, pin2);
        return pin2;
    } catch(std::runtime_error &e) {
        returnSignFailure(e.what());
        return -1;
    }
}


std::string esteidAPI::subjectToHumanReadable(std::string& subject)
{
    /* Certificates on Estonian ID card have their subjectCN fields in format:
     *    lastName,firstName,personalID
     * We split it here to show our prompt in a more human readable way:
     *    Firstname Lastname (PIN2)
     * If we can not split the CN properly, we show it as is */

    std::string ret;
    std::vector<std::string> sf = stringSplit(subject, ",");

    if (sf.size() == 3) {
        ret = sf[1] + " " + sf[0];
    } else {
        ret = subject;
    }

    return ret;
}


std::vector<std::string> esteidAPI::stringSplit(std::string str, std::string separator)
{
    std::vector<std::string> results;

    for (size_t found = 0; found != std::string::npos; found = str.find_first_of(separator)) {
        if (found > 0) {
            results.push_back(str.substr(0, found));
            str = str.substr(found+1);
        }
    }

    if (!str.empty()) {
        results.push_back(str);
    }

    return results;
}


#define ESTEID_PD_GETTER_IMP(index, attr) \
    std::string esteidAPI::get_##attr() { \
        WHITELIST_REQUIRED; \
        UpdatePersonalData(); \
        if(m_pdata.size() <= index) \
            throw FB::script_error("PD index out of range"); \
        return Converter::CP1252_to_UTF8(m_pdata[index]); \
    }

ESTEID_PD_GETTER_IMP(EstEidCard::SURNAME,         lastName)
ESTEID_PD_GETTER_IMP(EstEidCard::FIRSTNAME,       firstName);
ESTEID_PD_GETTER_IMP(EstEidCard::MIDDLENAME,      middleName);
ESTEID_PD_GETTER_IMP(EstEidCard::SEX,             sex);
ESTEID_PD_GETTER_IMP(EstEidCard::CITIZEN,         citizenship);
ESTEID_PD_GETTER_IMP(EstEidCard::BIRTHDATE,       birthDate);
ESTEID_PD_GETTER_IMP(EstEidCard::ID,              personalID);
ESTEID_PD_GETTER_IMP(EstEidCard::DOCUMENTID,      documentID);
ESTEID_PD_GETTER_IMP(EstEidCard::EXPIRY,          expiryDate);
ESTEID_PD_GETTER_IMP(EstEidCard::BIRTHPLACE,      placeOfBirth);
ESTEID_PD_GETTER_IMP(EstEidCard::ISSUEDATE,       issuedDate);
ESTEID_PD_GETTER_IMP(EstEidCard::RESIDENCEPERMIT, residencePermit);
ESTEID_PD_GETTER_IMP(EstEidCard::COMMENT1,        comment1);
ESTEID_PD_GETTER_IMP(EstEidCard::COMMENT2,        comment2);
ESTEID_PD_GETTER_IMP(EstEidCard::COMMENT3,        comment3);
ESTEID_PD_GETTER_IMP(EstEidCard::COMMENT4,        comment4);
