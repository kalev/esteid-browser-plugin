#ifndef H_ESTEID_X509CERTIFICATE
#define H_ESTEID_X509CERTIFICATE

#include <smartcardpp/smartcardpp.h>

#define USE_OPENSSL 1

#ifdef USE_OPENSSL
#include <openssl/x509.h>

typedef X509 ApiCert;
#endif

class X509Certificate {

public:
    X509Certificate(ByteVec cert);
    virtual ~X509Certificate();
    /** Returns Subject CN as an UTF8 encoded string */
    std::string getSubjectCN();
    /** Returns Issuer CN as an UTF8 encoded string */
    std::string getIssuerCN();
    std::string getValidFrom();
    std::string getValidTo();
    std::string getPEM();
    long getSerial();
    bool isValid();

protected:
    ApiCert* m_cert;
    std::string getError();
#ifdef USE_OPENSSL
    std::string X509NameToUTF8(X509_NAME *name, int nid);
    std::string X509TimeConvert(ASN1_TIME *date);
#endif
};
#endif
