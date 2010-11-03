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

#ifndef _WIN32
#include <stdlib.h>
#endif

#include <algorithm>
#include <fstream>
#include <boost/bind.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/fstream.hpp>

#include "PluginSettings.h"
#include "config.h"

using namespace boost::filesystem;

PluginSettings::PluginSettings()
    : allowLocal(true),
      allowDefaults(true)
{
    static const std::string fileName = "esteidplugin.conf";

    m_configFile = configDirectory() / fileName;
    loadConfig(m_configFile);

    /* Builtin whitelist */
    default_whitelist.clear();
    default_whitelist.push_back("id.swedbank.ee");
    default_whitelist.push_back("id.business.swedbank.ee");
    default_whitelist.push_back("id.seb.ee");
    default_whitelist.push_back("idnetbank.nordea.com");
    default_whitelist.push_back("id.eesti.ee");
    default_whitelist.push_back("www.eesti.ee");
    default_whitelist.push_back("digidoc.sk.ee");
    default_whitelist.push_back("xbrl.rik.ee");
    default_whitelist.push_back("ettevotjaportaal.rik.ee");
}


path PluginSettings::configDirectory()
{
#ifdef _WIN32
    return path(getenv("APPDATA"), native);
#else // UNIX
    path home(getenv("HOME"), native);
# ifdef __APPLE__
    return home / "Library/Application Support";
# else
    return home / ".config";
# endif
#endif
}

PluginSettings::~PluginSettings()
{
}

void PluginSettings::loadConfig(const boost::filesystem::path& configFile)
{
    std::string line;
    ifstream input(configFile);

    whitelist.clear();
    while (std::getline(input, line) && !line.empty()) {
        if (line == "@NODEFAULTS")
            allowDefaults = false;
        else if (line == "@NOLOCAL")
            allowLocal = false;
        else
            whitelist.push_back(line);
    }
}

void PluginSettings::Save()
{
    std::vector<std::string>::const_iterator i;

    removeDuplicateEntries(whitelist);
    removeDefaultEntries(whitelist);

    path configDir = configDirectory();
    if (!exists(configDir))
        create_directory(configDir);

    ofstream output;
    output.exceptions(std::ofstream::failbit | std::ofstream::badbit);

    output.open(m_configFile);
    if (!allowDefaults)
        output << "@NODEFAULTS" << std::endl;
    if (!allowLocal)
        output << "@NOLOCAL" << std::endl;

    for (i = whitelist.begin(); i != whitelist.end(); ++i)
        output << *i << std::endl;

    output.close();
}

bool PluginSettings::InWhitelist(const std::string& s)
{
    if (s.empty())
        return false;

    if (allowDefaults && find(default_whitelist.begin(), default_whitelist.end(), s) != default_whitelist.end()) {
        return true;
    } else if (find(whitelist.begin(), whitelist.end(), s) != whitelist.end()) {
        return true;
    }

    return false;
}

void PluginSettings::removeDuplicateEntries(std::vector<std::string>& v)
{
    std::sort(v.begin(), v.end());
    v.erase(std::unique(v.begin(), v.end()), v.end());
}

// remove all entries which are also in default whitelist
void PluginSettings::removeDefaultEntries(std::vector<std::string>& v)
{
    v.erase(remove_if(v.begin(), v.end(), boost::bind(&PluginSettings::inDefaultWhitelist, this, _1)), v.end());
}

// predicate for removeDefaultEntries
bool PluginSettings::inDefaultWhitelist(const std::string& s)
{
    return find(default_whitelist.begin(), default_whitelist.end(), s) != default_whitelist.end();
}
