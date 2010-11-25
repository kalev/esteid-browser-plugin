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
#include <boost/algorithm/string.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp> 

#ifdef USE_OPENSSL
#include <openssl/sha.h>
#endif

#ifdef SUPPORT_OLD_APIS
#include "Base64.h"
#include "utility/converters.h"
#include "CompatAPIs.h"
#endif

#include "JSObject.h"
#include "variant_list.h"
#include "DOM/Document.h"
#include "DOM/Window.h"
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

#include "EsteidAPI.h"
#include "CertificateAPI.h"
#include "PersonalDataAPI.h"
#include "JSUtil.h"
#include "debug.h"
#include "esteid-config.h"
#include "urlparser.h"

/* UI Messages */
#define MSG_SETTINGS _("Settings")
#define MSG_SITEACCESS _("This site is trying to obtain access to your ID card.")
#define MSG_INSECURE _("Access to ID card was denied because the connection to the site is not secure.")

#define REGISTER_METHOD(a)      JS_REGISTER_METHOD(EsteidAPI, a)
#define REGISTER_RO_PROPERTY(a) JS_REGISTER_RO_PROPERTY(EsteidAPI, a)

EsteidAPI::EsteidAPI(FB::BrowserHostPtr host, const std::string& mimetype) :
    m_host(host),
    m_pageURL(pageURL()),
    m_settingsCallback(new SettingsCallback(host, *this)),
    m_closeCallback(new CloseCallback(host, *this)),
    m_service(CardService::getInstance()),
    m_mimeType(mimetype),
    m_uiCallback(new UICallback(*this))
{
    ESTEID_DEBUG("EsteidAPI::EsteidAPI(%s)", m_mimeType.c_str());

#ifdef HAVE_LIBINTL_H
    bindtextdomain("esteid-browser-plugin", ESTEID_LOCALEDIR);
    textdomain("esteid-browser-plugin");
#endif

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
    REGISTER_RO_PROPERTY(personalData);
    REGISTER_RO_PROPERTY(errorCode);
    REGISTER_RO_PROPERTY(errorMessage);

#ifdef SUPPORT_OLD_APIS
    if(m_mimeType == "application/x-digidoc") {
        registerMethod("getCertificates",
            make_method(this, &EsteidAPI::getCertificatesSK));
        registerMethod("sign", make_method(this, &EsteidAPI::signSK));
    } else if(m_mimeType == "application/x-idcard-plugin") {
        registerMethod("getCertificates",
            make_method(this, &EsteidAPI::getCertificatesMoz));
        registerMethod("sign", make_method(this, &EsteidAPI::sign));
    }
    else {
        REGISTER_METHOD(getCertificates);
        registerMethod("sign", boost::bind(sign_method_wrapper(), this, _1));
    }
    REGISTER_RO_PROPERTY(version); // SK betaplugin
    REGISTER_METHOD(getCertificate); // SK betaplugin
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

    m_service->addObserver(this);
}

EsteidAPI::~EsteidAPI()
{
    ESTEID_DEBUG("EsteidAPI::~EsteidAPI()");

    m_service->removeObserver(this);
}

void EsteidAPI::setWindow(FB::PluginWindow* win)
{
    m_UI->setWindow(win);
}

bool EsteidAPI::IsLocal()
{
    if (!m_conf.allowLocal)
        return false;

    if (m_pageURL.protocol() == "file" ||
        m_pageURL.hostname() == "localhost") {
        return true;
    }

    return false;
}

bool EsteidAPI::IsSecure()
{
    if (IsLocal() || m_pageURL.protocol() == "https")
        return true;

    return false;
}

bool EsteidAPI::IsWhiteListed()
{
    if (IsLocal() || m_conf.InWhitelist(m_pageURL.hostname()))
        return true;

    return false;
}

void EsteidAPI::whitelistRequired()
{
    if (!IsSecure()) {
        DisplayNotification(MSG_INSECURE);
        throw FB::script_error("No cards found");
    } else if (!IsWhiteListed()) {
        DisplayNotification(MSG_SITEACCESS);
        throw FB::script_error("No cards found");
    }
}

std::string EsteidAPI::pageURL()
{
    return m_host->getDOMWindow()->getLocation();
}

void EsteidAPI::CreateNotificationBar()
{
    const std::string label = MSG_SETTINGS;
    m_host->evaluateJavaScript(EstEIDNotificationBarScript);
    m_barJSO = m_host->getDOMDocument()
               ->getProperty<FB::JSObjectPtr>("EstEIDNotificationBar");
    m_barJSO->Invoke("create",
                     FB::variant_list_of(label)(m_settingsCallback));
}

void EsteidAPI::DisplayError(const std::string& msg)
{
    try {
        OpenNotificationBar();
        m_barJSO->Invoke("showError", FB::variant_list_of(msg));
    } catch(const std::exception& e) {
        ESTEID_DEBUG("Unable to display error: %s", e.what());
    }
}

void EsteidAPI::DisplayNotification(const std::string& msg)
{
    try {
        OpenNotificationBar();
        m_barJSO->Invoke("show", FB::variant_list_of(msg));
    } catch(const std::exception& e) {
        ESTEID_DEBUG("Unable to display notification: %s", e.what());
    }
}

void EsteidAPI::OpenNotificationBar()
{
    if(!m_barJSO) {
        CreateNotificationBar();
    }
}

void EsteidAPI::CloseNotificationBar()
{
    if(!m_barJSO) return;

    m_barJSO->Invoke("close", FB::variant_list_of(0));
}

// JS method exposed to browser to show preferences window 
// Direct access to this method will be exposed to a very few selected URL-s
void EsteidAPI::showSettings()
{
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

void EsteidAPI::ShowSettings()
{
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

void EsteidAPI::onMessage(CardService::MsgType e, ReaderID i)
{
    //const char *evtname;
    std::string evtname;

    switch(e) {
        case CardService::CARD_INSERTED:   evtname = "CardInserted";  break;
        case CardService::CARD_REMOVED:    evtname = "CardRemoved";   break;
        case CardService::READERS_CHANGED: evtname = "ReadersChanged";break;
        default: throw std::runtime_error("Invalid message type"); break;
    }
    ESTEID_DEBUG("onMessage: %s %d", evtname.c_str(), i);

    if(!IsWhiteListed()) return;

    /* FIXME: Prefixing every event name with an additional "on" is a bloody
              hack. We either need to fix firebreath or our JS API spec. */
    FireEvent("on" + evtname, FB::variant_list_of(i));
}

// TODO: Optimize memory usage. Don't create new object if cert hasn't changed.
FB::JSAPIPtr EsteidAPI::get_authCert()
{
    whitelistRequired();

    RTERROR_TO_SCRIPT(
        return FB::JSAPIPtr(new CertificateAPI(m_host, m_service->getAuthCert())));
}

FB::JSAPIPtr EsteidAPI::get_signCert()
{
    whitelistRequired();

    RTERROR_TO_SCRIPT(
        return FB::JSAPIPtr(new CertificateAPI(m_host, m_service->getSignCert())));
}

FB::JSAPIPtr EsteidAPI::get_personalData()
{
    whitelistRequired();

    RTERROR_TO_SCRIPT(
        std::vector<std::string> pData;
        m_service->readPersonalData(pData);
        return FB::JSAPIPtr(new PersonalDataAPI(m_host, pData))
    );
}

std::string EsteidAPI::getVersion()
{
    return FBSTRING_PLUGIN_VERSION;
}


int EsteidAPI::get_errorCode()
{
    try {
        whitelistRequired();

        return 0;
    } catch(...) {
        return 0;
    }
}

std::string EsteidAPI::get_errorMessage()
{
    try {
        whitelistRequired();

        return "";
    } catch(...) {
        return "No cards found"; // FIXME: Use translatable messages
    }
}

/*
 * Ask for PIN and return; the signed hash is later asynchronously returned
 * through callback.
 */
void EsteidAPI::signAsync(const std::string& hash, const std::string& url, const FB::JSObjectPtr& callback)
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


void EsteidAPI::prepareSign(const std::string& hash, const std::string& url)
{
    if (hash.length() != 40)
        throw std::runtime_error("Invalid hash");

    if (url.empty())
        throw std::runtime_error("Partial document URL must be specified");

    /* Extract subject line from Certificate */
    std::string subjectRaw = FB::ptr_cast<CertificateAPI>(get_signCert())->get_CN();
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


void EsteidAPI::promptForPinAsync(bool retrying)
{
    int triesLeft = getPin2RetryCount();
    if (triesLeft <= 0) {
        m_UI->ShowPinBlockedMessage(2);
        throw std::runtime_error("PIN2 locked");
    }

    m_UI->PromptForPinAsync(m_subject, m_url, m_hash,
                           m_pinpad, retrying, triesLeft);
}


std::string EsteidAPI::signSHA1(const std::string& hash, const std::string& pin)
{
    if (pin.empty()) // shouldn't happen
        throw std::runtime_error("empty PIN");

    std::string in(hash);

    // Strip spaces and newlines from hash HEX
    in.erase(std::remove(in.begin(), in.end(), '\n'), in.end());
    in.erase(std::remove(in.begin(), in.end(), '\r'), in.end());
    in.erase(std::remove(in.begin(), in.end(), ' '), in.end());

    std::string signedHash = m_service->signSHA1(in, EstEidCard::SIGN, pin);
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
void EsteidAPI::onPinEntered(const std::string& pin)
{
    try {
        std::string signedHash = signSHA1(m_hash, pin);
        returnSignedData(signedHash);
    } catch(const AuthError& e) {
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


void EsteidAPI::returnSignedData(const std::string& data)
{
    try {
        m_signCallback->Invoke("onSuccess", FB::variant_list_of(data));
    } catch(const FB::script_error&) {
        returnSignFailure("Error executing JavaScript code");
    }
}


void EsteidAPI::returnSignFailure(const std::string& msg)
{
    try {
        m_signCallback->Invoke("onError", FB::variant_list_of(msg));
    } catch(const FB::script_error&) {
        // can't really do anything useful here
    }
}

#ifdef SUPPORT_OLD_APIS
#define COMPAT_URL "http://code.google.com/p/esteid/wiki/OldPluginCompatibilityMode"

using namespace boost::date_time;

void EsteidAPI::deprecatedCall()
{
    boost::posix_time::ptime date_for_activating_deprecate_messages(boost::gregorian::date(2011, May, 1));
    boost::system_time current_time = boost::get_system_time();

    if (current_time > date_for_activating_deprecate_messages) {
	std::string msg1 = _("Website is using old signature APIs. Please contact site owner. Click <a href=\"");
	std::string msg2 = _("\" target=\"_blank\" style=\"color: blue;\">here</a> for details.");
        DisplayError(msg1 + COMPAT_URL + msg2);
    }
}

std::string EsteidAPI::promptForPin(bool retrying)
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

std::string EsteidAPI::askPinAndSign(const std::string& hash, const std::string& url)
{
    prepareSign(hash, url);

    bool retrying = false;
    for (;;) {
        std::string pin = promptForPin(retrying);

        try {
            std::string signedHash = signSHA1(hash, pin);
            return signedHash;
        } catch(const AuthError& e) {
            if (e.m_aborted) // pinpad
                throw std::runtime_error("pinpad operation cancelled");

            // ask again for PIN
            retrying = true;
        }
    }
}

/* Old Mozilla plugin */
std::string EsteidAPI::getCertificatesMoz()
{
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

std::string EsteidAPI::sign(const std::string& a, const std::string& b)
{
    whitelistRequired();
    deprecatedCall();

    std::string signedHash;

    if(!a.compare(MAGIC_ID2)) { // SK leakplugin compat mode
        return signSK(a, b);
    } else if(!a.compare(MAGIC_ID)) { // Old Mozilla Plugin compat mode
        try {
            signedHash = askPinAndSign(b, std::string(COMPAT_URL));
            return "({signature:'" + signedHash + "', returnCode: 0})";
        } catch(const std::runtime_error& e) {
            // TODO: Return proper error code from plugin (when it's implemented)
            return "({returnCode: 12})";
        }
    } else { // New plugin blocking API compatibility mode
        try {
            signedHash = askPinAndSign(a , (b.empty()) ? std::string(COMPAT_URL) : b);
        } catch(const std::runtime_error& e) {
            throw FB::script_error(e.what());
        }

        return signedHash;
    }
}

/* Emulate SK leakplugin AND the old Mozilla plugin in one function */
FB::variant EsteidAPI::getCertificates() {
    try {
        return getCertificatesSK();
    } catch(...) {
        // TODO: Return proper error code from plugin (when it's implemented)
        return "({returnCode: 12})";
    }
}

/* Emulate SK leakplugin (application/x-digidoc) */
std::string EsteidAPI::get_version()
{
    return getVersion();
}

FB::JSAPIPtr EsteidAPI::getCertificate() {
    whitelistRequired();
    deprecatedCall();

    RTERROR_TO_SCRIPT(
        FB::VariantList outVar;
        ByteVec bv = m_service->getSignCert();
        return FB::JSAPIPtr(new SKCertificateAPI(m_host, bv));
    );
}

FB::VariantList EsteidAPI::getCertificatesSK() {
    whitelistRequired();
    deprecatedCall();

    RTERROR_TO_SCRIPT(
        FB::VariantList outVar;
        ByteVec bv = m_service->getSignCert();
        outVar.push_back(FB::JSAPIPtr(new SKCertificateAPI(m_host, bv)));
        return outVar;
    );
}

std::string EsteidAPI::signSK(const std::string& id,
                              const std::string& hash, FB::variant crap)
{
    whitelistRequired();
    deprecatedCall();

    RTERROR_TO_SCRIPT(
        return askPinAndSign(hash, std::string(COMPAT_URL)));
}

/* This emulates old Java XMLSignApplet behaviour.
 * XML manipulations done here are butt ugly, just like the "real solution",
 * that was originally written by Mr. Veiko Sinivee.
 *
 * NB! This is a compatibility mode function and should
 *     NEVER be used in any new code.
 */

#ifdef USE_OPENSSL
/* Calculate SHA1 from ByteVec and encode it to Base64 */
static std::string B64SHA1(const ByteVec& in) {
    std::string out(20, '\0');
    SHA1(reinterpret_cast<const unsigned char *>(&in[0]), in.size(),
         reinterpret_cast<unsigned char *>(&out[0]));
    return base64_encode(out);
}
/* Calculate SHA1 from string and encode it to Base64 */
static std::string B64SHA1(const std::string& in) {
    std::string out(20, '\0');
    SHA1(reinterpret_cast<const unsigned char *>(&in[0]), in.size(),
         reinterpret_cast<unsigned char *>(&out[0]));
    return base64_encode(out);
}
/* Calculate SHA1 from string and encode it to Hex */
std::string HEXSHA1(const std::string& in) {
    ByteVec bv(20, '\0');

    SHA1(reinterpret_cast<const unsigned char *>(in.data()), in.size(),
         &bv[0]);

    std::ostringstream buf;
    for(ByteVec::const_iterator it = bv.begin(); it!=bv.end();it++)
        buf << std::setfill('0') << std::setw(2) << std::hex << (short)*it;

    return buf.str();
}
#endif

void EsteidAPI::signXML(
    const std::string& data,
    const std::string& onSuccess,
    const std::string& lang,
    const std::string& charset,
    const std::string& encoding,
    const std::string& onCancel)

{
    ESTEID_DEBUG("signXML('%s','%s','%s','%s','%s','%s')\n", data.c_str(),
           onSuccess.c_str(), lang.c_str(), charset.c_str(),
           encoding.c_str(), onCancel.c_str());

#ifdef USE_OPENSSL
    /* SHA1 Digest URL used in XML generation */
    const std::string sha1Url = "http://www.w3.org/2000/09/xmldsig#sha1";

    std::string decoded_data = data;
    if(encoding == "EMBEDDED_BASE64") {
        decoded_data = base64_decode(data);
    }
    std::string dataLen = boost::lexical_cast<std::string>(decoded_data.length());

    /* FIXME: Old XMLSignApplet recodes character data to UTF-8,
     *        replaces newlines (\n and \r) with spaces and re-encodes
     *        back to Base64, but is this actually needed? I don't think so.
     */

    //recode_from_charset_to_UTF8(data);
    //std::replace(decoded_data.begin(), decoded_data.end(), '\n', ' ');
    //std::replace(decoded_data.begin(), decoded_data.end(), '\r', ' ');
    //if(encoding == "EMBEDDED_BASE64") {
    //    data = base64_encode(decoded_data);
    //}

    /* Get required info from certificate */
    std::string certDigest;
    std::string certSerial;
    std::string certData;

    try { RTERROR_TO_SCRIPT(
        ByteVec bv = m_service->getSignCert();
        X509Certificate cert(bv);

        certSerial = boost::lexical_cast<std::string>(cert.getSerial());
        certDigest = B64SHA1(bv);
        certData = base64_encode(bv);
    )} catch(...) {
        m_host->evaluateJavaScript(onCancel + "();");
        return;
    }

    /* Make current time
     * Please note that this is NOT valid XML date format.
     * XML Date format is %Y-%m-%dT%H:%M:%SZ, where
     * Z denotes universal time. However XMLSignApplet
     * separates date components with dots and uses local time
     * so we implement it incorrectly too in order to maintain
     * bug-for-bug compatibility */
    boost::posix_time::time_facet* tf = 
        new boost::posix_time::time_facet("%Y.%m.%dT%H:%M:%SZ");
    std::stringstream tmp;
    tmp.imbue(std::locale(tmp.getloc(), tf));
    tmp << boost::posix_time::second_clock::local_time();
    std::string sigTime = tmp.str();

    /* Start constructing XML */

    // <DataFile>
    std::string dataFileXml = 
      "<DataFile ContentType=\"" + encoding + "\" "
        "Filename=\"msg.xml\" Id=\"D0\" MimeType=\"text/xml\" "
        "Size=\"" + dataLen + "\">" + data +
      "</DataFile>";

    // <SignedProperties>
    std::string sigPropXml =
      "<SignedProperties xmlns=\"http://www.w3.org/2000/09/xmldsig#\" "
          "Id=\"S0-SignedProperties\" Target=\"#S0\">"
        "<SignedSignatureProperties>"
          "<SigningTime>" + sigTime + "</SigningTime>"
          "<SigningCertificate><Cert Id=\"S0-CERTINFO\">"
            "<CertDigest>"
              "<DigestMethod Algorithm=\"" + sha1Url + "\"></DigestMethod>"
              "<DigestValue>" + certDigest + "</DigestValue>"
            "</CertDigest>"
            "<IssuerSerial>" + certSerial + "</IssuerSerial>"
          "</Cert></SigningCertificate>"
          "<SignaturePolicyIdentifier>"
            "<SignaturePolicyImplied></SignaturePolicyImplied>"
          "</SignaturePolicyIdentifier>"
          "<SignatureProductionPlace></SignatureProductionPlace>"
          "<SignerRole></SignerRole>"
        "</SignedSignatureProperties>"
        "<SignedDataObjectProperties></SignedDataObjectProperties>"
      "</SignedProperties>";

    // <SignedInfo>
    std::string sigInfoXml =
      "<SignedInfo xmlns=\"http://www.w3.org/2000/09/xmldsig#\">"
        "<CanonicalizationMethod "
          "Algorithm=\"http://www.w3.org/TR/2001/REC-xml-c14n-20010315\">"
        "</CanonicalizationMethod>"
        "<SignatureMethod "
          "Algorithm=\"http://www.w3.org/2000/09/xmldsig#rsa-sha1\">"
        "</SignatureMethod>"
        "<Reference URI=\"#D0\">"
          "<DigestMethod Algorithm=\"" + sha1Url + "\"></DigestMethod>"
          "<DigestValue>" + B64SHA1(dataFileXml) + "</DigestValue>"
        "</Reference>"
        "<Reference URI=\"#S0-SignedProperties\">"
          "<DigestMethod Algorithm=\"" + sha1Url + "\"></DigestMethod>"
          "<DigestValue>" + B64SHA1(sigPropXml) + "</DigestValue>"
        "</Reference>"
      "</SignedInfo>";

    /* Perform signing operation */

    /* FIXME: The original API is non-blocking, but the callbacks
       are so braindead (callback function name is passed as a string)
       so we implement the compatibility version as a blocking call for now */
    std::string sigValue;

    try {
        std::string signedHash = 
            askPinAndSign(HEXSHA1(sigInfoXml), std::string(COMPAT_URL));
        sigValue = base64_encode(fromHex(signedHash));
    } catch(const std::runtime_error& e) {
        m_host->evaluateJavaScript(onCancel + "();");
        return;
    }

    // <Signature>
    std::string sigXml =
      "<Signature Id=\"S0\" xmlns=\"http://www.w3.org/2000/09/xmldsig#\">"
        /* <SignedInfo> */ + sigInfoXml +
        "<SignatureValue Id=\"S0-SIG\">" + sigValue + "</SignatureValue>"
        "<KeyInfo>"
          "<X509Data>"
            "<X509Certificate Id=\"S0-CERT\">" + certData + "</X509Certificate>"
          "</X509Data>"
        "</KeyInfo>"
        "<Object>"
          "<QualifyingProperties>" + sigPropXml + "</QualifyingProperties>"
        "</Object>"
      "</Signature>";

    std::string finalXml =
      "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
      "<SignedDoc format=\"DIGIDOC-XML\" version=\"1.1\">"
        /* <DataFile> */  + dataFileXml
        /* <Signature> */ + sigXml +
      "</SignedDoc>";

    ESTEID_DEBUG("\n%s\n", finalXml.c_str());

    m_host->evaluateJavaScript(onSuccess + "('" + finalXml + "');");
#else
    throw FB::script_error("XML Signer requires OpenSSL-enabled build");
#endif
}

std::string EsteidAPI::getInfo()
{
    deprecatedCall();

    return getVersion();
}

std::string EsteidAPI::getSigningCertificate()
{
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

std::string EsteidAPI::getSignedHash(const std::string& hash, const std::string& slot)
{
    whitelistRequired();
    deprecatedCall();

    try {
        std::string signedHash = askPinAndSign(hash, std::string(COMPAT_URL));
        return signedHash;
    } catch(const std::runtime_error& e) {
        // This API returns nothing on error
        return "";
    }
}

std::string EsteidAPI::get_selectedCertNumber()
{
    whitelistRequired();
    deprecatedCall();

    return "10"; // Dummy number
}

void EsteidAPI::prepare(const std::string& onSuccess,
                        const std::string& onCancel,
                        const std::string& onError)
{
    whitelistRequired();
    deprecatedCall();

    try {
        ByteVec bv = m_service->getSignCert();
        std::ostringstream buf;

        for(ByteVec::const_iterator it = bv.begin(); it!=bv.end();it++)
            buf << std::setfill('0') << std::setw(2) << std::hex << (short)*it;

        m_host->evaluateJavaScript(onSuccess + "(10, '" + buf.str() + "');");
    } catch(const std::runtime_error& e) {
        m_host->evaluateJavaScript(onError + "(12, '" + e.what() + "');");
    }
}

void EsteidAPI::finalize(const std::string& slot,
                         const std::string& hash,
                         const std::string& onSuccess,
                         const std::string& onCancel,
                         const std::string& onError)
{
    whitelistRequired();
    deprecatedCall();

    /* FIXME: The original API is non-blocking, but the callbacks
       are so braindead (callback function name is passed as a string)
       so we implement the compatibility version as a blocking call for now */
    try {
        std::string signedHash = askPinAndSign(hash, std::string(COMPAT_URL));

        m_host->evaluateJavaScript(onSuccess + "('" + signedHash + "');");
    } catch(const std::runtime_error& e) {
        m_host->evaluateJavaScript(onCancel + "();");
    }
}

bool EsteidAPI::isActive()
{
    return true;
}
#endif

int EsteidAPI::getPin2RetryCount()
{
    byte puk, pin1, pin2;
    m_service->getRetryCounts(puk, pin1, pin2);
    return pin2;
}


std::string EsteidAPI::subjectToHumanReadable(const std::string& subject)
{
    /* Certificates on Estonian ID card have their subjectCN fields in format:
     *    lastName,firstName,personalID
     * We split it here to show our prompt in a more human readable way:
     *    Firstname Lastname (PIN2)
     * If we can not split the CN properly, we show it as is */

    std::string ret;
    std::vector<std::string> sf;
    boost::algorithm::split(sf, subject, boost::algorithm::is_any_of(","));

    if (sf.size() == 3) {
        ret = sf[1] + " " + sf[0];
    } else {
        ret = subject;
    }

    return ret;
}
