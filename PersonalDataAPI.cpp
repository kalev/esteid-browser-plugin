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

std::string PersonalDataAPI::getRecord(size_t index)
{
    if (m_pData.size() <= index)
        throw FB::script_error("PD index out of range");

    return Converter::CP1252_to_UTF8(m_pData[index]);
}

std::string PersonalDataAPI::get_lastName()        { return getRecord(EstEidCard::SURNAME); }
std::string PersonalDataAPI::get_firstName()       { return getRecord(EstEidCard::FIRSTNAME); }
std::string PersonalDataAPI::get_middleName()      { return getRecord(EstEidCard::MIDDLENAME); }
std::string PersonalDataAPI::get_sex()             { return getRecord(EstEidCard::SEX); }
std::string PersonalDataAPI::get_citizenship()     { return getRecord(EstEidCard::CITIZEN); }
std::string PersonalDataAPI::get_birthDate()       { return getRecord(EstEidCard::BIRTHDATE); }
std::string PersonalDataAPI::get_personalID()      { return getRecord(EstEidCard::ID); }
std::string PersonalDataAPI::get_documentID()      { return getRecord(EstEidCard::DOCUMENTID); }
std::string PersonalDataAPI::get_expiryDate()      { return getRecord(EstEidCard::EXPIRY); }
std::string PersonalDataAPI::get_placeOfBirth()    { return getRecord(EstEidCard::BIRTHPLACE); }
std::string PersonalDataAPI::get_issuedDate()      { return getRecord(EstEidCard::ISSUEDATE); }
std::string PersonalDataAPI::get_residencePermit() { return getRecord(EstEidCard::RESIDENCEPERMIT); }
std::string PersonalDataAPI::get_comment1()        { return getRecord(EstEidCard::COMMENT1); }
std::string PersonalDataAPI::get_comment2()        { return getRecord(EstEidCard::COMMENT2); }
std::string PersonalDataAPI::get_comment3()        { return getRecord(EstEidCard::COMMENT3); }
std::string PersonalDataAPI::get_comment4()        { return getRecord(EstEidCard::COMMENT4); }
