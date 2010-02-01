#include <string>
#include <sstream>
#include "JSAPIAuto.h"
#include "BrowserHostWrapper.h"
#include "X509Certificate.h"

class CertificateAPI : public FB::JSAPIAuto
{
public:
    //CertificateAPI(FB::BrowserHostWrapper *host, X509Certificate cert);
    CertificateAPI(FB::BrowserHostWrapper *host, ByteVec bv);
    virtual ~CertificateAPI();

    /** Subject CN */
    std::string get_CN();
    /** Date and time in format dd.mm.yyyy hh:mm:ss */
    std::string get_validFrom();
    /** Date and time in format dd.mm.yyyy hh:mm:ss */
    std::string get_validTo();
    /** Issuer CN */
    std::string get_issuerCN();
    /** security, Non-Repudiation, digital signing etc. */
    std::string get_keyUsage();
    /** Certificate in PEM format */
    std::string get_cert();
    /** A unique serial number for this certificate */
    std::string get_serial();
    /** True if certificate seems to be valid (check against workstation clock) */
    bool get_isValid();

private:
    FB::AutoPtr<FB::BrowserHostWrapper> m_host;

    X509Certificate m_cert;
};
