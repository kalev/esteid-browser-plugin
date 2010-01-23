#include "BrowserObjectAPI.h"
#include "variant_list.h"
#include "DOM/JSAPI_DOMDocument.h"

#include "esteidAPI.h"
#include "JSUtil.h"

#define REGISTER_METHOD(a)      JS_REGISTER_METHOD(esteidAPI, a)
#define REGISTER_RO_PROPERTY(a) JS_REGISTER_RO_PROPERTY(esteidAPI, a)

esteidAPI::esteidAPI(FB::BrowserHostWrapper *host) : 
    m_host(host), m_authCert(NULL), m_signCert(NULL)
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
/*
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
*/
}

esteidAPI::~esteidAPI()
{
}

std::string esteidAPI::get_lastName()
{
    throw FB::script_error("Ikaldus");
}
std::string esteidAPI::get_firstName()
{
    return "Peeter";
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
    throw FB::script_error("Ei oska veel signeerida");
}
