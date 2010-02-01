#include "BrowserObjectAPI.h"
#include "variant_list.h"
#include "DOM/JSAPI_DOMDocument.h"

#include "Mozilla/MozillaUI.h"
#ifdef _WIN32
#include "Win/WindowsUI.h"
#endif

#include "esteidAPI.h"
#include "JSUtil.h"

#define REGISTER_METHOD(a)      JS_REGISTER_METHOD(esteidAPI, a)
#define REGISTER_RO_PROPERTY(a) JS_REGISTER_RO_PROPERTY(esteidAPI, a)

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

esteidAPI::esteidAPI(FB::BrowserHostWrapper *host) : 
    m_host(host), m_authCert(NULL), m_signCert(NULL),
    m_service(EstEIDService::getInstance())
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

    /* FIXME: Prefixing every event name with an additional "on" is a bloody
              hack. We either need to fix firebreath or our JS API spec. */
    FireEvent("on" + evtname, FB::variant_list_of(i));
}

void esteidAPI::UpdatePersonalData()
{
    try {
        m_service->readPersonalData(m_pdata);
    }
    catch(std::runtime_error &err) {
        throw FB::script_error(err.what());
    }
}


FB::JSOutObject esteidAPI::get_authCert()
{
    if(m_authCert == NULL)
        m_authCert = new CertificateAPI(m_host);

    return m_authCert;
}

FB::JSOutObject esteidAPI::get_signCert()
{
    if(m_signCert == NULL)
        m_signCert = new CertificateAPI(m_host);

    return m_signCert;
}

std::string esteidAPI::getVersion()
{
    return "0.x.y";
}

std::string esteidAPI::sign(std::string hash, std::string url) {
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

        if(pinpad) {
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

#define ESTEID_WHITELIST_REQUIRED 

#define ESTEID_PD_GETTER_IMP(index, attr) \
    std::string esteidAPI::get_##attr() { \
            ESTEID_WHITELIST_REQUIRED; \
        UpdatePersonalData(); \
        if(m_pdata.size() <= index) \
            throw FB::script_error("PD index out of range"); \
        return m_pdata[index]; \
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
