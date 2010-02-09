#include "BrowserObjectAPI.h"
#include "variant_list.h"
#include "DOM/JSAPI_DOMDocument.h"
#include "DOM/JSAPI_DOMWindow.h"
#include "Util/JSArray.h"
#include "config.h"

#include "Mozilla/MozillaUI.h"
#ifdef _WIN32
#include "Win/WindowsUI.h"
#endif

#include "esteidAPI.h"
#include "JSUtil.h"

/* UI Messages */
#define MSG_SETTINGS "Settings"
#define MSG_SITEACCESS "This site is trying to obtain access to your ID-card."
#define MSG_INSECURE "Access to ID-card was denied because the connection to the site is not secure."

#include <iconv.h>

#ifdef ICONV_SECOND_ARGUMENT_IS_CONST
    #define ICONV_CONST const
#else
    #define ICONV_CONST
#endif

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

#define ESTEID_DEBUG printf

// FIXME: Set error codes
#define ESTEID_ERROR_FROMCARD(e) { \
    ESTEID_DEBUG("Card error: %s\n", e.what()); \
    throw FB::script_error(e.what()); }
#define ESTEID_ERROR_CARD_ERROR(m) { \
    ESTEID_DEBUG("Card error: %s\n", m); \
    throw FB::script_error(m); }
#define ESTEID_ERROR_INVALID_ARG { \
    throw FB::script_error("Invalid argument"); }
#define ESTEID_ERROR_USER_ABORT { \
    throw FB::script_error("User cancelled operation"); }
#define ESTEID_ERROR_NO_CARD { \
    throw FB::script_error("No card in reader"); }
#define ESTEID_ERROR_NO_PERMISSION ESTEID_ERROR_NO_CARD

esteidAPI::esteidAPI(FB::BrowserHostWrapper *host) : 
    m_host(host), m_authCert(NULL), m_signCert(NULL),
    m_service(EstEIDService::getInstance()),
    m_settingsCallback(new SettingsCallback(host, *this)),
    m_closeCallback(new CloseCallback(host, *this))
{
    ESTEID_DEBUG("esteidAPI::esteidAPI()\n");

    REGISTER_METHOD(getVersion);
    REGISTER_METHOD(sign);

/*  FIXME: Investigate if this is needed at all?
    registerEvent("OnCardInserted");
    registerEvent("OnCardRemoved");
    registerEvent("OnReadersChanged");
*/

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

    ESTEID_DEBUG("esteidAPI: Page URL is %s\n", GetPageURL().c_str());

    /* Try to access Mozilla UI */
    m_UI = GetMozillaUI();

    /* Use platform specific UI if browser specific is not found */
    if(!m_UI) {
#ifdef _WIN32
        ESTEID_DEBUG("GetMozillaUI failed; trying to load WindowsUI\n");
        m_UI = new WindowsUI();
#endif
#ifdef SUCKOSX
        m_UI = new MacUI();
#endif
#ifdef UNIX_MUFF
       m_UI = new GtkUI();
#endif
    }

#if 0
    /* Die if UI initialization fails */
    if(!m_UI)
        throw FB::script_error("Unable to load plugin user interface");
#endif

    m_service->AddObserver(this);
}

esteidAPI::~esteidAPI()
{
    ESTEID_DEBUG("esteidAPI::~esteidAPI()\n");

    m_service->RemoveObserver(this);
}

PluginUI* esteidAPI::GetMozillaUI()
{
    if(!m_host) return NULL;

    /* Check if we are running under NPAPI */
    FB::Npapi::NpapiBrowserHost *nphost = \
      dynamic_cast<FB::Npapi::NpapiBrowserHost*> (m_host.ptr());

    if(!nphost) return NULL;

    ESTEID_DEBUG("findUI: detected NPAPI\n");

    /* Check for Mozilla */
    nsISupports *sm = NULL, *dw = NULL;
    nphost->GetValue(NPNVserviceManager, &sm);
    nphost->GetValue(NPNVDOMWindow, &dw);
    if(sm && dw) {
        try {
            return new MozillaUI(sm, dw);
        }
        catch(std::runtime_error &e) {
            ESTEID_DEBUG("%s\n", e.what());
            return NULL;
        }
    }

    return NULL;
}

bool esteidAPI::IsLocal() {
    std::string url = GetPageURL();
    // FIXME: This code is butt-ugly!
    if(!url.compare(0,  7, "file://"))            return true;
    if(!url.compare(0, 17, "http://localhost/"))  return true;
    if(!url.compare(0, 18, "https://localhost/")) return true;
    if(!url.compare(0, 17, "http://localhost:"))  return true;
    if(!url.compare(0, 18, "https://localhost:")) return true;
    return false;
}

bool esteidAPI::IsSecure() {
    if(!GetPageURL().compare(0, 8, "https://")) return true;
    if(m_conf.allowLocal && IsLocal())          return true;
    return false;
}

bool esteidAPI::IsWhiteListed() {
    if(m_conf.allowLocal && IsLocal()) return true;

    std::string url = GetPageURL();
    size_t pos1 = url.find("://") + 3, pos2 = url.find("/", pos1);
    if(pos1 >= pos2) return false;
    std::string host = url.substr(pos1, pos2 - pos1);
    if(m_conf.InWhitelist(host)) return true;

    return false;
}

std::string esteidAPI::GetPageURL(void) {
    if(m_pageURL.empty()) {
        /* Using method no. 1 from
         * https://developer.mozilla.org/en/Getting_the_page_URL_in_NPAPI_plugin
         */
        FB::JSAPI_DOMWindow dw = m_host->getDOMWindow();
        FB::JSAPI_DOMNode loc = dw.getProperty<FB::JSObject>("location");
        m_pageURL = loc.getProperty<std::string>("href");
    }

    return m_pageURL;
}

void esteidAPI::CreateNotificationBar(void) {
    FB::JSAPI_DOMDocument doc = m_host->getDOMDocument();

    /* Create notification bar div */
    FB::JSAPI_DOMElement bar = doc.callMethod<FB::JSObject>("createElement",
                                        FB::variant_list_of("div"));
    FB::JSAPI_DOMNode style = bar.getProperty<FB::JSObject>("style");
    style.setProperty("fontSize",         FB::variant("110%"));
    style.setProperty("backgroundColor",  FB::variant("#ffff66"));
    style.setProperty("position",         FB::variant("fixed"));
    style.setProperty("top",              FB::variant("0px"));
    style.setProperty("left",             FB::variant("0px"));
    style.setProperty("right",            FB::variant("0px"));
    style.setProperty("padding",          FB::variant("5px"));

    /* Create button bar div */
    FB::JSAPI_DOMElement btnbar = doc.callMethod<FB::JSObject>("createElement",
                                          FB::variant_list_of("div"));
    style = btnbar.getProperty<FB::JSObject>("style");
    style.setProperty("cssFloat",  FB::variant("right"));
    style.setProperty("width",     FB::variant("10em"));
    style.setProperty("textAlign", FB::variant("right"));

#if 1
    /* Create Settings button */
    FB::JSAPI_DOMElement btn = doc.callMethod<FB::JSObject>("createElement",
                                       FB::variant_list_of("input"));
    btn.setProperty("type",    FB::variant("button"));
    btn.setProperty("value",   FB::variant(MSG_SETTINGS));
    btn.callMethod<FB::variant>("addEventListener", 
            FB::variant_list_of("click")(m_settingsCallback)(false));
    btnbar.callMethod<FB::JSObject>("appendChild",
            FB::variant_list_of(btn.getJSObject()));

    /* Create Close button */
    btn = doc.callMethod<FB::JSObject>("createElement",
                                       FB::variant_list_of("input"));
    style = btn.getProperty<FB::JSObject>("style");
    btn.setProperty("type",    FB::variant("button"));
    btn.setProperty("value",   FB::variant("x"));
    btn.callMethod<FB::variant>("addEventListener", 
            FB::variant_list_of("click")(m_closeCallback)(false));
    btnbar.callMethod<FB::JSObject>("appendChild",
            FB::variant_list_of(btn.getJSObject()));
#else
    btnbar.setInnerHTML("\
        <input type=\"button\" value=\"Settings\" \
               onclick=\"alert('hee');\"></input>\
        <input type=\"button\" value=\"x\" \
               onclick=\"this.parentNode.parentNode.style.display='none';\"></input>\
    ");
#endif

    /* Insert buttonbar into container */
    bar.callMethod<FB::JSObject>("appendChild",
            FB::variant_list_of(btnbar.getJSObject()));
    
    /* Create message div */
    FB::JSAPI_DOMElement text = doc.callMethod<FB::JSObject>("createElement",
                                         FB::variant_list_of("div"));
    style = text.getProperty<FB::JSObject>("style");
    style.setProperty("marginLeft", FB::variant("2em"));
    m_msgElement = bar.callMethod<FB::JSObject>("appendChild",
                           FB::variant_list_of(text.getJSObject()));

    /* We can't inject divs into DOMDocument, we MUST find body tag */
    FB::JSArray tmp(doc.callMethod<FB::JSObject>("getElementsByTagName",
                            FB::variant_list_of("body")));
    FB::JSAPI_DOMElement body = tmp[0].convert_cast<FB::JSObject>();

    /* Insert notification bar into body */
    m_barElement = body.callMethod<FB::JSObject>("appendChild",
                            FB::variant_list_of(bar.getJSObject()));
}

void esteidAPI::DisplayNotification(std::string msg) {
    try {
        OpenNotificationBar();
        FB::JSAPI_DOMElement(m_msgElement).setInnerHTML(msg);
    } catch(std::exception &e) {
        ESTEID_DEBUG("Unable to display notification: %s\n", e.what());
    }
}

void esteidAPI::OpenNotificationBar(void) {
    if(!m_barElement) {
        CreateNotificationBar();
    }
}

void esteidAPI::CloseNotificationBar(void) {
    if(!m_barElement) return;

    FB::JSAPI_DOMElement bar(m_barElement);
    FB::JSAPI_DOMNode style = bar.getProperty<FB::JSObject>("style");
    style.setProperty("display", FB::variant("none"));
}

void esteidAPI::ShowSettings(void) {
    if(IsSecure())
        m_UI->ShowSettings(m_conf, GetPageURL());
    else
        m_UI->ShowSettings(m_conf);

    CloseNotificationBar();
}

void esteidAPI::onMessage(EstEIDService::msgType e, readerID i) {
    //const char *evtname;
    std::string evtname;

    switch(e) {
        case EstEIDService::CARD_INSERTED:   evtname = "OnCardInserted";  break;
        case EstEIDService::CARD_REMOVED:    evtname = "OnCardRemoved";   break;
        case EstEIDService::READERS_CHANGED: evtname = "OnReadersChanged";break;
        default: throw std::runtime_error("Invalid message type"); break;
    }
    ESTEID_DEBUG("onMessage: %s %d\n", evtname.c_str(), i);

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

std::string esteidAPI::sign(std::string hash, std::string url) {
    WHITELIST_REQUIRED;

    int tries;
    bool retrying = false, pinpad;

    // FIXME: This shouldn't happen. Remove this line after constructor is fixed
    if(!m_UI) throw FB::script_error("No UI, can't prompt for PIN");

    /* Extract subject line from Certificate */
    std::string subject = \
        static_cast<CertificateAPI*>(get_signCert().ptr())->get_CN();

    try {
        pinpad = m_service->hasSecurePinEntry();
    } catch(std::runtime_error &e) { 
        ESTEID_ERROR_FROMCARD(e);
    }

    // FIXME: Implement!
    std::string pageUrl = "";

    // FIXME: Hardcoded SHA1 support
    if(hash.length() != 40)
        ESTEID_ERROR_INVALID_ARG("Invalid hash");

    if(!url.length())
        ESTEID_ERROR_INVALID_ARG("Partial document URL must be specified");

    while(true) {
        std::string pin;

        tries = getPin2RetryCount();

        if(tries <= 0) {
            // This card is locked!
            m_UI->ShowPinBlockedMessage(2);
            ESTEID_ERROR_CARD_ERROR("PIN2 locked");
        }

        if(0) { //pinpad
            // FIXME: Implement
            throw FB::script_error("Unable to use PinPAD (yet)");
        } else {
            pin = m_UI->PromptForSignPIN(subject, url, hash, pageUrl,
                                         pinpad, retrying, tries);
            if(!pin.length()) {
                ESTEID_DEBUG("sign: got empty PIN from UI\n");
                ESTEID_ERROR_USER_ABORT;
            }

            try {
                return m_service->signSHA1(hash, EstEidCard::SIGN, pin);
            } catch(AuthError &e) {
                if(e.m_aborted) {
                    ESTEID_DEBUG("sign: cancel pressed on PinPAD\n");
                    ESTEID_ERROR_USER_ABORT;
                }
            } catch(std::runtime_error &e) {
                ESTEID_ERROR_FROMCARD(e);
            }
        }
        retrying = true;
    }
}

int esteidAPI::getPin2RetryCount() {
    try {
        byte puk, pin1, pin2;
        m_service->getRetryCounts(puk, pin1, pin2);
        return pin2;
    } catch(std::runtime_error &e) {
        ESTEID_ERROR_FROMCARD(e);
    }
}


std::string esteidAPI::iconvConvert(const std::string &str_in, const char *tocode, const char *fromcode)
{
    iconv_t iso_utf;
    ICONV_CONST char *inptr;
    char *outptr;
    size_t inbytes, outbytes, result;
    char outbuf[128];
    std::string out;

    iso_utf = iconv_open(tocode, fromcode);
    if (iso_utf == (iconv_t)-1) {
        ESTEID_DEBUG("error in iconv_open");
        return str_in;
    }

    inptr = (ICONV_CONST char*)str_in.c_str();
    inbytes = str_in.size();

    outptr = outbuf;
    outbytes = sizeof(outbuf) - 1;

    result = iconv(iso_utf, &inptr, &inbytes, &outptr, &outbytes);
    if (result == (size_t)-1) {
        ESTEID_DEBUG("error converting with iconv");
        return str_in;
    }
    *outptr = '\0';

    iconv_close(iso_utf);

    out += outbuf;
    return out;
}


std::string esteidAPI::CP1252_to_UTF8(const std::string &str_in)
{
    return iconvConvert(str_in, "UTF-8", "CP1252");
}


#define ESTEID_PD_GETTER_IMP(index, attr) \
    std::string esteidAPI::get_##attr() { \
        WHITELIST_REQUIRED; \
        UpdatePersonalData(); \
        if(m_pdata.size() <= index) \
            throw FB::script_error("PD index out of range"); \
        return CP1252_to_UTF8(m_pdata[index]); \
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
