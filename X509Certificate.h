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
    std::string getKeyUsage();
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
