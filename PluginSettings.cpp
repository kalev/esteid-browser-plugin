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
#include "debug.h"
#include "esteid-config.h"

using namespace boost::filesystem;

PluginSettings::PluginSettings()
    : allowLocal(true),
      allowDefaults(true)
{
    load();
}

path PluginSettings::globalSettingsDir()
{
#ifdef _WIN32
    // FIXME: lookup path from registry
    return ESTEID_CONFIGDIR;
#else // UNIX
    return ESTEID_CONFIGDIR;
#endif
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

path PluginSettings::globalSettingsFile()
{
    static const std::string fileName = "esteid-browser-plugin.conf";

    return globalSettingsDir() / fileName;
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

void PluginSettings::load(const boost::filesystem::path& filename,
                          std::vector<std::string>& out_whitelist)
{
    using boost::property_tree::ptree;

    ptree pt;
    read_xml(filename.string(), pt);

    BOOST_FOREACH(ptree::value_type& v, pt.get_child("settings.whitelist")) {
        if (v.first == "url") {
            out_whitelist.push_back(v.second.data());
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

void PluginSettings::convertLegacy()
{
    path legacySettings = legacySettingsFile();
    if (exists(legacySettings)) {
        loadLegacy(legacySettings);
        Save();
        remove(legacySettings);
    }
}

void PluginSettings::load()
{
    try {
        // Load systemwide settings file
        load(globalSettingsFile(), default_whitelist);
    } catch(const std::exception& e) {
        ESTEID_DEBUG("Error loading settings: %s", e.what());
    }

    try {
        // Load user settings file from home directory
        load(userSettingsFile(), whitelist);
    } catch(const std::exception& e) {
        ESTEID_DEBUG("Error loading user settings: %s", e.what());
    }

    try {
        // Convert legacy settings into the new format
        convertLegacy();
    } catch(const std::exception& e) {
        ESTEID_DEBUG("Error converting legacy settings: %s", e.what());
    }
}

void PluginSettings::save(const boost::filesystem::path& filename)
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

void PluginSettings::Save()
{
    removeDuplicateEntries(whitelist);
    removeDefaultEntries(whitelist);

    try {
        // Save user settings file
        save(userSettingsFile());
    } catch(const std::exception& e) {
        ESTEID_DEBUG("Error saving user settings: %s", e.what());
    }
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
