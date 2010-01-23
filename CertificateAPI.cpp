#include "BrowserObjectAPI.h"
#include "variant_list.h"
#include "DOM/JSAPI_DOMDocument.h"

#include "CertificateAPI.h"
#include "JSUtil.h"

#define REGISTER_RO_PROPERTY(a) JS_REGISTER_RO_PROPERTY(CertificateAPI, a)

CertificateAPI::CertificateAPI(FB::BrowserHostWrapper *host) : m_host(host)
{
    REGISTER_RO_PROPERTY(CN);
/*
    REGISTER_RO_PROPERTY(validFrom);
    REGISTER_RO_PROPERTY(validTo);
    REGISTER_RO_PROPERTY(issuerCN);
    REGISTER_RO_PROPERTY(keyUsage);
    REGISTER_RO_PROPERTY(cert);
    REGISTER_RO_PROPERTY(serial);
    REGISTER_RO_PROPERTY(expired);
*/
}

CertificateAPI::~CertificateAPI()
{
}

std::string CertificateAPI::get_CN()
{
    return "Pakiraam,Peeter,34512220463";
}
