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
#include <boost/foreach.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include "PluginSettings.h"
#include "config.h"

using namespace boost::filesystem;

PluginSettings::PluginSettings()
    : allowLocal(true),
      allowDefaults(true)
{
    load();

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
    default_whitelist.push_back("id.lhv.ee");
}

path PluginSettings::userSettingsDir()
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

path PluginSettings::userSettingsFile()
{
    static const std::string fileName = "esteid-browser-plugin.conf";

    return userSettingsDir() / fileName;
}

path PluginSettings::legacySettingsFile()
{
    static const std::string fileName = "esteidplugin.conf";

    return userSettingsDir() / fileName;
}

void PluginSettings::loadUser(const boost::filesystem::path& filename)
{
    using boost::property_tree::ptree;

    ptree pt;
    read_xml(filename.string(), pt);

    BOOST_FOREACH(ptree::value_type& v, pt.get_child("settings.whitelist")) {
        if (v.first == "url") {
            whitelist.push_back(v.second.data());
        }
    }
}

void PluginSettings::loadLegacy(const boost::filesystem::path& configFile)
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

void PluginSettings::load()
{
    loadLegacy(legacySettingsFile());
}

void PluginSettings::saveUser(const boost::filesystem::path& filename)
{
    using boost::property_tree::ptree;
    using boost::property_tree::xml_writer_make_settings;

    path settingsDir = filename.parent_path();
    if (!exists(settingsDir))
        create_directory(settingsDir);

    // Create empty property tree object
    ptree settings;

    ptree wl_tree;
    BOOST_FOREACH(const std::string& url, whitelist) {
        wl_tree.add("url", url);
    }

    settings.add_child("settings.whitelist", wl_tree);

    write_xml(filename.string(),
              settings,
              std::locale(),
              xml_writer_make_settings(' ', 4));
}

void PluginSettings::saveLegacy(const boost::filesystem::path& configFile)
{
    std::vector<std::string>::const_iterator i;

    removeDuplicateEntries(whitelist);
    removeDefaultEntries(whitelist);

    path configDir = configFile.parent_path();
    if (!exists(configDir))
        create_directory(configDir);

    ofstream output;
    output.exceptions(std::ofstream::failbit | std::ofstream::badbit);

    output.open(configFile);
    if (!allowDefaults)
        output << "@NODEFAULTS" << std::endl;
    if (!allowLocal)
        output << "@NOLOCAL" << std::endl;

    for (i = whitelist.begin(); i != whitelist.end(); ++i)
        output << *i << std::endl;

    output.close();
}

void PluginSettings::Save()
{
    saveLegacy(legacySettingsFile());
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
