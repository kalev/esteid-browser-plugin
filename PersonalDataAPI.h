/*
 * esteid-browser-plugin - a browser plugin for Estonian EID card
 *
 * Copyright (C) 2010  Estonian Informatics Centre
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

#ifndef PERSONALDATAAPI_H
#define PERSONALDATAAPI_H

#include <string>
#include <vector>
#include "JSAPIAuto.h"
#include "BrowserHost.h"

class PersonalDataAPI : public FB::JSAPIAuto
{
public:
    PersonalDataAPI(FB::BrowserHostPtr host, const std::vector<std::string>& pData);
    virtual ~PersonalDataAPI();

    /** Personal data file attributes (read-only properties) */
    std::string get_lastName();
    std::string get_firstName();
    std::string get_middleName();
    std::string get_sex();
    std::string get_citizenship();
    std::string get_birthDate();
    std::string get_personalID();
    std::string get_documentID();
    std::string get_expiryDate();
    std::string get_placeOfBirth();
    std::string get_issuedDate();
    std::string get_residencePermit();
    std::string get_comment1();
    std::string get_comment2();
    std::string get_comment3();
    std::string get_comment4();

private:
    FB::BrowserHostPtr m_host;
    std::vector<std::string> m_pData;
};

#endif //PERSONALDATAAPI_H
