/*
 * esteid-browser-plugin - a browser plugin for Estonian EID card
 *
 * Copyright (C) 2011  Smartlink OÜ
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

#include "registry.h"
#include <stdexcept>
#include <vector>

RegistryKey::RegistryKey(HKEY key, const std::string& subkey, REGSAM samDesired)
    : m_hKey(0)
{
    RegOpenKeyExA(key, subkey.c_str(), NULL, samDesired, &m_hKey);
    if (!m_hKey)
        throw std::runtime_error("error opening registry key");
}

RegistryKey::~RegistryKey()
{
    RegCloseKey(m_hKey);
}

std::string RegistryKey::queryValue(const std::string& valueName, LPDWORD lpType)
{
    DWORD ret;
    DWORD bufsize = 0;

    // Get the required buffer size
    ret = RegQueryValueExA(m_hKey, valueName.c_str(), NULL, lpType, NULL, &bufsize);
    if (ret != ERROR_SUCCESS)
        throw std::runtime_error("error getting buffer size");

    std::vector<char> buf(bufsize);
    ret = RegQueryValueExA(m_hKey, valueName.c_str(), NULL, lpType, reinterpret_cast<BYTE*>(&buf[0]), &bufsize);
    if (ret != ERROR_SUCCESS)
        throw std::runtime_error("error reading registry");

    return std::string(buf.begin(), buf.end());
}

std::string Registry::readValue(HKEY key,
                                const std::string& subkey,
                                const std::string& valueName,
                                LPDWORD lpType)
{
    RegistryKey userKey(key, subkey, KEY_READ);
    return userKey.queryValue(valueName);
}
