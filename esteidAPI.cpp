#include "BrowserObjectAPI.h"
#include "variant_list.h"
#include "DOM/JSAPI_DOMDocument.h"

#include "Mozilla/MozillaUI.h"

#include "esteidAPI.h"
#include "JSUtil.h"

#define REGISTER_METHOD(a)      JS_REGISTER_METHOD(esteidAPI, a)
#define REGISTER_RO_PROPERTY(a) JS_REGISTER_RO_PROPERTY(esteidAPI, a)

#define ESTEID_DEBUG printf

esteidAPI::esteidAPI(FB::BrowserHostWrapper *host) : 
    m_host(host), m_authCert(NULL), m_signCert(NULL),
    m_service(EstEIDService::getInstance())
{
    REGISTER_METHOD(getVersion);
    REGISTER_METHOD(sign);
/*
    REGISTER_METHOD(addEventListener);
    REGISTER_METHOD(removeEventListener);
*/
    REGISTER_RO_PROPERTY(authCert);
//    REGISTER_RO_PROPERTY(signCert);
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
#ifdef WINDOOZ
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
}

esteidAPI::~esteidAPI()
{
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

std::string esteidAPI::getVersion()
{
    return "0.x.y";
}

std::string esteidAPI::sign(std::string hash, std::string url) {

    m_UI->ShowPinBlockedMessage(2);
    throw FB::script_error("PIN2 is blocked");

    return "Jee";
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
