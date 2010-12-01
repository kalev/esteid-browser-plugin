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

#include <fstream>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/foreach.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include "PluginSettings.h"
#include "whitelist.h"
#include "config.h"
#include "debug.h"
#include "esteid-config.h"

using namespace boost::filesystem;

PluginSettings::PluginSettings()
    : m_allowLocal(true)
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
                          Whitelist& out_whitelist)
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

    m_whitelist.clear();
    while (std::getline(input, line) && !line.empty()) {
        if (line == "@NODEFAULTS")
            ;
        else if (line == "@NOLOCAL")
            m_allowLocal = false;
        else
            m_whitelist.push_back(line);
    }
}

void PluginSettings::convertLegacy()
{
    path legacySettings = legacySettingsFile();
    if (exists(legacySettings)) {
        loadLegacy(legacySettings);
        save();
        remove(legacySettings);
    }
}

void PluginSettings::load()
{
    try {
        // Load systemwide settings file
        load(globalSettingsFile(), m_defaultWhitelist);
    } catch(const std::exception& e) {
        ESTEID_DEBUG("Error loading settings: %s", e.what());
    }

    try {
        // Load user settings file from home directory
        load(userSettingsFile(), m_whitelist);
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
    BOOST_FOREACH(const std::string& url, m_whitelist) {
        wl_tree.add("url", url);
    }

    settings.add_child("settings.whitelist", wl_tree);

    write_xml(filename.string(),
              settings,
              std::locale(),
              xml_writer_make_settings(' ', 4));
}

void PluginSettings::save()
{
    removeDuplicateEntries(m_whitelist);
    removeDuplicateEntries(m_whitelist, m_defaultWhitelist);

    try {
        // Save user settings file
        save(userSettingsFile());
    } catch(const std::exception& e) {
        ESTEID_DEBUG("Error saving user settings: %s", e.what());
    }
}

bool PluginSettings::inWhitelist(const std::string& s)
{
    bool ret = ::inWhitelist(m_defaultWhitelist, s) ||
               ::inWhitelist(m_whitelist, s);
    return ret;
}

Whitelist PluginSettings::defaultWhitelist()
{
    return m_defaultWhitelist;
}

Whitelist PluginSettings::whitelist()
{
    return m_whitelist;
}

void PluginSettings::setWhitelist(const Whitelist& whitelist)
{
    m_whitelist = whitelist;
}

void PluginSettings::addSite(const std::string& site)
{
    m_whitelist.push_back(site);
}

void PluginSettings::clearWhitelist()
{
    m_whitelist.clear();
}

bool PluginSettings::allowLocal()
{
    return m_allowLocal;
}
