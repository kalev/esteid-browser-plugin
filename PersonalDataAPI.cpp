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

#include "PersonalDataAPI.h"
#include <smartcardpp/esteid/EstEidCard.h>
#include "converter.h"
#include "JSUtil.h"

#define REGISTER_RO_PROPERTY(a) JS_REGISTER_RO_PROPERTY(PersonalDataAPI, a)

PersonalDataAPI::PersonalDataAPI(FB::BrowserHostPtr host, const std::vector<std::string>& pData)
    : m_host(host),
      m_pData(pData)
{
    REGISTER_RO_PROPERTY(lastName);
    REGISTER_RO_PROPERTY(firstName);
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
}

PersonalDataAPI::~PersonalDataAPI()
{
}

#define ESTEID_PD_GETTER_IMP(index, attr) \
    std::string PersonalDataAPI::get_##attr() \
    { \
        if (m_pData.size() <= index) \
            throw FB::script_error("PD index out of range"); \
        return Converter::CP1252_to_UTF8(m_pData[index]); \
    }

ESTEID_PD_GETTER_IMP(EstEidCard::SURNAME,         lastName)
ESTEID_PD_GETTER_IMP(EstEidCard::FIRSTNAME,       firstName);
ESTEID_PD_GETTER_IMP(EstEidCard::MIDDLENAME,      middleName);
ESTEID_PD_GETTER_IMP(EstEidCard::SEX,             sex);
ESTEID_PD_GETTER_IMP(EstEidCard::CITIZEN,         citizenship);
ESTEID_PD_GETTER_IMP(EstEidCard::BIRTHDATE,       birthDate);
ESTEID_PD_GETTER_IMP(EstEidCard::ID,              personalID);
ESTEID_PD_GETTER_IMP(EstEidCard::DOCUMENTID,      documentID);
ESTEID_PD_GETTER_IMP(EstEidCard::EXPIRY,          expiryDate);
ESTEID_PD_GETTER_IMP(EstEidCard::BIRTHPLACE,      placeOfBirth);
ESTEID_PD_GETTER_IMP(EstEidCard::ISSUEDATE,       issuedDate);
ESTEID_PD_GETTER_IMP(EstEidCard::RESIDENCEPERMIT, residencePermit);
ESTEID_PD_GETTER_IMP(EstEidCard::COMMENT1,        comment1);
ESTEID_PD_GETTER_IMP(EstEidCard::COMMENT2,        comment2);
ESTEID_PD_GETTER_IMP(EstEidCard::COMMENT3,        comment3);
ESTEID_PD_GETTER_IMP(EstEidCard::COMMENT4,        comment4);
