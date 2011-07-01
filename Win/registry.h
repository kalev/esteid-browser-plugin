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

#ifndef REGISTRY_H
#define REGISTRY_H

#include <windows.h>
#include <string>

class RegistryKey
{
public:
    RegistryKey(HKEY key, const std::string& subkey, REGSAM samDesired);
    ~RegistryKey();

    std::string queryValue(const std::string& valueName, LPDWORD lpType = NULL);

private:
    HKEY m_hKey;
};

namespace Registry
{
    std::string readValue(HKEY key,
                          const std::string& subkey,
                          const std::string& valueName,
                          LPDWORD lpType = NULL);
};

#endif //REGISTRY_H
