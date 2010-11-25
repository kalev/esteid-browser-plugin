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

#ifndef COMPATAPIS_H
#define COMPATAPIS_H

#include "CertificateAPI.h"

#ifdef SUPPORT_OLD_APIS

#define MAGIC_ID  "37337F4CF4CE"
#define MAGIC_ID2 "37337A55F4CE"

/* SK Leakplugin */
class SKCertificateAPI : public CertificateAPI
{
public:
    SKCertificateAPI(FB::BrowserHostPtr host, ByteVec bv);
    virtual std::string get_id();
    virtual std::string get_cert();
    virtual std::string ToString();
};
#endif //SUPPORT_OLD_APIS
#endif //CERTIFICATEAPI_H
