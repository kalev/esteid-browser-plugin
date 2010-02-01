#include "BrowserObjectAPI.h"
#include "variant_list.h"
#include "DOM/JSAPI_DOMDocument.h"

#include "CertificateAPI.h"
#include "JSUtil.h"

#define REGISTER_RO_PROPERTY(a) JS_REGISTER_RO_PROPERTY(CertificateAPI, a)

/*
CertificateAPI::CertificateAPI(FB::BrowserHostWrapper *host, X509Certificate cert) :
     m_host(host), m_cert(cert)
*/
CertificateAPI::CertificateAPI(FB::BrowserHostWrapper *host, ByteVec bv) :
     m_host(host), m_cert(bv)
{
    REGISTER_RO_PROPERTY(CN);
    REGISTER_RO_PROPERTY(validFrom);
    REGISTER_RO_PROPERTY(validTo);
    REGISTER_RO_PROPERTY(issuerCN);
    REGISTER_RO_PROPERTY(keyUsage);
    REGISTER_RO_PROPERTY(cert);
    REGISTER_RO_PROPERTY(serial);
    REGISTER_RO_PROPERTY(isValid);
}

CertificateAPI::~CertificateAPI()
{
}

std::string CertificateAPI::get_CN() {
    RTERROR_TO_SCRIPT(return m_cert.getSubjectCN());
}
// FIXME: Return dates in more usable format
std::string CertificateAPI::get_validFrom() { 
    RTERROR_TO_SCRIPT(return m_cert.getValidFrom());
}
std::string CertificateAPI::get_validTo() {
    RTERROR_TO_SCRIPT(return m_cert.getValidTo());
}
std::string CertificateAPI::get_issuerCN() { 
    RTERROR_TO_SCRIPT(return m_cert.getIssuerCN());
}
std::string CertificateAPI::get_keyUsage() { return ""; } // FIXME: Implement
std::string CertificateAPI::get_cert() {
    RTERROR_TO_SCRIPT(return m_cert.getPEM());
}
std::string CertificateAPI::get_serial() { 
    std::ostringstream os;
    RTERROR_TO_SCRIPT(os << m_cert.getSerial());
    return os.str();
}
bool CertificateAPI::get_isValid() { 
    RTERROR_TO_SCRIPT(return m_cert.isValid());
}
