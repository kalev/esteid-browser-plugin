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

#include <iomanip>
#include <boost/date_time.hpp>

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
#include "urlparser.h"

/* UI Messages */
#define MSG_SETTINGS "Settings"
#define MSG_SITEACCESS "This site is trying to obtain access to your ID-card."
#define MSG_INSECURE "Access to ID-card was denied because the connection to the site is not secure."

#define REGISTER_METHOD(a)      JS_REGISTER_METHOD(esteidAPI, a)
#define REGISTER_RO_PROPERTY(a) JS_REGISTER_RO_PROPERTY(esteidAPI, a)

esteidAPI::esteidAPI(FB::BrowserHost host) :
    m_host(host),
    m_service(EstEIDService::getInstance()),
    m_pageURL(pageURL()),
    m_settingsCallback(new SettingsCallback(host, *this)),
    m_closeCallback(new CloseCallback(host, *this)),
    m_uiCallback(new UICallback(*this))
{
    ESTEID_DEBUG("esteidAPI::esteidAPI()");

    /* Load JavaScript code to be evaluated in browser */
    #include "EstEIDNotificationBar.js"

    REGISTER_METHOD(getVersion);
    REGISTER_METHOD(signAsync);
    REGISTER_METHOD(showSettings);

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
    REGISTER_METHOD(isActive);
#endif

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

void esteidAPI::setWindow(FB::PluginWindow* win)
{
    m_UI->setWindow(win);
}

bool esteidAPI::IsLocal() {
    if (!m_conf.allowLocal)
        return false;

    if (m_pageURL.protocol() == "file" ||
        m_pageURL.hostname() == "localhost") {
        return true;
    }

    return false;
}

bool esteidAPI::IsSecure() {
    if (IsLocal() || m_pageURL.protocol() == "https")
        return true;

    return false;
}

bool esteidAPI::IsWhiteListed() {
    if (IsLocal() || m_conf.InWhitelist(m_pageURL.hostname()))
        return true;

    return false;
}

void esteidAPI::whitelistRequired() {
    if (!IsSecure()) {
        DisplayNotification(MSG_INSECURE);
        throw FB::script_error("No cards found");
    } else if (!IsWhiteListed()) {
        DisplayNotification(MSG_SITEACCESS);
        throw FB::script_error("No cards found");
    }
}

std::string esteidAPI::pageURL() {
    /* Using method no. 1 from
     * https://developer.mozilla.org/en/Getting_the_page_URL_in_NPAPI_plugin
     */
    FB::JSAPI_DOMWindow dw = m_host->getDOMWindow();
    FB::JSAPI_DOMNode loc = dw.getProperty<FB::JSObject>("location");

    std::string url = loc.getProperty<std::string>("href");
    ESTEID_DEBUG("Page URL is %s", url.c_str());
    return url;
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

// JS method exposed to browser to show preferences window 
// Direct access to this method will be exposed to a very few selected URL-s
void esteidAPI::showSettings(void) {
    if (m_pageURL.protocol() == "file" ||
        m_pageURL.protocol() == "chrome") {
        try {
            m_UI->ShowSettings(m_conf);
        } catch(const std::exception& e) {
            ESTEID_DEBUG("Unable to display whitelist editor: %s", e.what());
        }
    } else {
        throw FB::script_error("No such method");
    }
}

void esteidAPI::ShowSettings(void) {
    try {
        if (IsSecure())
            m_UI->ShowSettings(m_conf, m_pageURL.hostname());
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
    whitelistRequired();

    RTERROR_TO_SCRIPT(
        return FB::JSAPIPtr(new CertificateAPI(m_host, m_service->getAuthCert())));
}

FB::JSOutObject esteidAPI::get_signCert()
{
    whitelistRequired();

    RTERROR_TO_SCRIPT(
        return FB::JSAPIPtr(new CertificateAPI(m_host, m_service->getSignCert())));
}

std::string esteidAPI::getVersion()
{
    return FBSTRING_PLUGIN_VERSION;
}


/*
 * Ask for PIN and return; the signed hash is later asynchronously returned
 * through callback.
 */
void esteidAPI::signAsync(std::string hash, std::string url, const FB::JSObject& callback)
{
    m_signCallback = callback;

    try {
        whitelistRequired();

        prepareSign(hash, url);
        promptForPinAsync();
    } catch(const std::exception& e) {
        returnSignFailure(e.what());
        return;
    }
}


void esteidAPI::prepareSign(const std::string& hash, const std::string& url)
{
    if (hash.length() != 40)
        throw std::runtime_error("Invalid hash");

    if (url.empty())
        throw std::runtime_error("Partial document URL must be specified");

    /* Extract subject line from Certificate */
    std::string subjectRaw = static_cast<CertificateAPI*>(get_signCert().get())->get_CN();
    if (subjectRaw.empty())
        throw std::runtime_error("Empty subject");

    m_subject = subjectToHumanReadable(subjectRaw);
    m_hash = hash;
    m_url = url;

#if 0
    m_pinpad = m_service->hasSecurePinEntry();
#else
    m_pinpad = false;
#endif
}


void esteidAPI::promptForPinAsync(bool retrying)
{
    int triesLeft = getPin2RetryCount();
    if (triesLeft <= 0) {
        m_UI->ShowPinBlockedMessage(2);
        throw std::runtime_error("PIN2 locked");
    }

    m_UI->PromptForPinAsync(m_subject, m_url, m_hash,
                           m_pinpad, retrying, triesLeft);
}


std::string esteidAPI::signSHA1(const std::string& hash, const std::string& pin)
{
    if (pin.empty()) // shouldn't happen
        throw std::runtime_error("empty PIN");

    std::string signedHash = m_service->signSHA1(hash, EstEidCard::SIGN, pin);
    if (signedHash.empty()) // shouldn't happen
        throw std::runtime_error("empty hash");

    return signedHash;
}


/*
 * Callback from UI code.
 *
 * Make sure the function doesn't throw to avoid
 * unwinding through foreign frames.
 */
void esteidAPI::onPinEntered(std::string pin)
{
    try {
        std::string signedHash = signSHA1(m_hash, pin);
        returnSignedData(signedHash);
    } catch(AuthError &e) {
        if (e.m_aborted) { // pinpad
            returnSignFailure("pinpad operation cancelled");
            return;
        }

        try {
            // ask again for PIN
            promptForPinAsync(true);
        } catch(const std::exception& e) {
            returnSignFailure(e.what());
        }
    } catch(const std::exception& e) {
        returnSignFailure(e.what());
    }
}


void esteidAPI::returnSignedData(const std::string& data)
{
    try {
        m_signCallback->Invoke("onSuccess", FB::variant_list_of(data));
    } catch(const FB::script_error&) {
        returnSignFailure("Error executing JavaScript code");
    }
}


void esteidAPI::returnSignFailure(const std::string& msg)
{
    try {
        m_signCallback->Invoke("onError", FB::variant_list_of(msg));
    } catch(const FB::script_error&) {
        // can't really do anything useful here
    }
}

#ifdef SUPPORT_OLD_APIS

#define MAGIC_ID "37337F4CF4CE"
#define COMPAT_URL "http://code.google.com/p/esteid/wiki/OldPluginCompatibilityMode"

using namespace boost::date_time;

void esteidAPI::deprecatedCall()
{
    boost::posix_time::ptime date_for_activating_deprecate_messages(boost::gregorian::date(2011, May, 1));
    boost::system_time current_time = boost::get_system_time();

    if (current_time > date_for_activating_deprecate_messages)
        DisplayError("Website is using old signature APIs. Please contact site owner. Click <a href=\"" COMPAT_URL "\" target=\"_blank\" style=\"color: blue;\">here</a> for details.");
}

std::string esteidAPI::promptForPin(bool retrying)
{
    int triesLeft = getPin2RetryCount();
    if (triesLeft <= 0) {
        m_UI->ShowPinBlockedMessage(2);
        throw std::runtime_error("PIN2 locked");
    }

    std::string pin = m_UI->PromptForPin(m_subject, m_url, m_hash,
                                         m_pinpad, retrying, triesLeft);

    if (pin.empty())
        throw std::runtime_error(CANCEL_MSG);

    return pin;
}

std::string esteidAPI::askPinAndSign(const std::string& hash, const std::string& url)
{
    prepareSign(hash, url);

    bool retrying = false;
    for (;;) {
        std::string pin = promptForPin(retrying);

        try {
            std::string signedHash = signSHA1(hash, pin);
            return signedHash;
        } catch(AuthError &e) {
            if (e.m_aborted) // pinpad
                throw std::runtime_error("pinpad operation cancelled");

            // ask again for PIN
            retrying = true;
        }
    }
}

std::string esteidAPI::getCertificates() {
    whitelistRequired();
    deprecatedCall();

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
    whitelistRequired();
    deprecatedCall();

    std::string signedHash;

    if(!a.compare(MAGIC_ID)) { // Old Mozilla Plugin compat mode
        try {
            signedHash = askPinAndSign(b, std::string(COMPAT_URL));
            return "({signature:'" + signedHash + "', returnCode: 0})";
        } catch(std::runtime_error &e) {
            // TODO: Return proper error code from plugin (when it's implemented)
            return "({returnCode: 12})";
        }
    } else { // New plugin blocking API compatibility mode
        try {
            signedHash = askPinAndSign(a , (b.empty()) ? std::string(COMPAT_URL) : b);
        } catch(std::runtime_error &e) {
            throw FB::script_error(e.what());
        }

        return signedHash;
    }
}

std::string esteidAPI::getInfo() {
    deprecatedCall();

    return getVersion();
}

std::string esteidAPI::getSigningCertificate() {
    whitelistRequired();
    deprecatedCall();

    try {
        ByteVec bv = m_service->getSignCert();
        std::ostringstream buf;

        for(ByteVec::const_iterator it = bv.begin(); it!=bv.end();it++)
            buf << std::setfill('0') << std::setw(2) << std::hex << (short)*it;

        return buf.str();
    } catch(...) { return ""; } // This API returns nothing on Error
}

std::string esteidAPI::getSignedHash(std::string hash, std::string slot) {
    whitelistRequired();
    deprecatedCall();

    try {
        std::string signedHash = askPinAndSign(hash, std::string(COMPAT_URL));
        return signedHash;
    } catch(std::runtime_error &e) {
        // This API returns nothing on error
        return "";
    }
}

std::string esteidAPI::get_selectedCertNumber() {
    whitelistRequired();
    deprecatedCall();

    return "10"; // Dummy number
}

void esteidAPI::prepare(std::string onSuccess, std::string onCancel,
    std::string onError) {
    whitelistRequired();
    deprecatedCall();

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
    whitelistRequired();
    deprecatedCall();

    /* FIXME: The original API is non-blocking, but the callbacks
       are so braindead (callback function name is passed as a string)
       so we implement the compatibility version as a blocking call for now */
    try {
        std::string signedHash = askPinAndSign(hash, std::string(COMPAT_URL));

        m_host->evaluateJavaScript(onSuccess + "('" + signedHash + "');");
    } catch(std::runtime_error &e) {
        m_host->evaluateJavaScript(onCancel + "();");
    }
}

bool esteidAPI::isActive()
{
    return true;
}
#endif

int esteidAPI::getPin2RetryCount() {
    byte puk, pin1, pin2;
    m_service->getRetryCounts(puk, pin1, pin2);
    return pin2;
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
        whitelistRequired(); \
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
