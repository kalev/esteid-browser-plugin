/*
 * esteid-browser-plugin - a browser plugin for Estonian EID card
 *
 * Copyright (C) 2010  Antti Andreimann
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

#include "CompatAPIs.h"
#include "JSUtil.h"

// #define REGISTER_RO_PROPERTY(a) JS_REGISTER_RO_PROPERTY(CertificateAPI, a)

#ifdef SUPPORT_OLD_APIS

/* SK leakplugin */
SKCertificateAPI::SKCertificateAPI(FB::BrowserHostPtr host, ByteVec bv)
    : CertificateAPI(host, bv)
{
    JS_REGISTER_RO_PROPERTY(SKCertificateAPI, id);
}

std::string SKCertificateAPI::get_id()
{
    return MAGIC_ID2;
}

std::string SKCertificateAPI::get_cert()
{ 
    RTERROR_TO_SCRIPT(return m_cert.getHex());
}

std::string SKCertificateAPI::ToString() {
    /* Return "compatible" JSON */
    std::ostringstream buf;

    buf << "({certificates:[{";
    buf << "id:'" << MAGIC_ID << "',";
    buf << "cert:'";
    buf << get_certificateAsHex();       
    buf << "',";
    buf << "CN:'" << get_CN() << "',";
    buf << "issuerCN:'" << get_issuerCN() << "',";
    // JS using this old API expects the exact string "Non-Repudiation"
    buf << "keyUsage:'Non-Repudiation',";
    buf << "validFrom: new Date(),"; // TODO: Date(YYYY,MM,DD,HH,mm,SS)
    buf << "validTo: new Date()}],"; // TODO: Date(YYYY,MM,DD,HH,mm,SS)
    buf << "returnCode:0})";

    return buf.str();
}
#endif //SUPPORT_OLD_APIS
