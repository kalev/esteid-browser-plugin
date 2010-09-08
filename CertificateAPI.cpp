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
std::string CertificateAPI::get_keyUsage() {
    RTERROR_TO_SCRIPT(return m_cert.getKeyUsage());
}
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
