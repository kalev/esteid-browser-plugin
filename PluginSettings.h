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

#ifndef H_ESTEID_PLUGINSETTINGS
#define H_ESTEID_PLUGINSETTINGS

#include <string>
#include <vector>
#include <boost/filesystem/path.hpp>

class PluginSettings {
public:
    std::vector<std::string> whitelist;
    std::vector<std::string> default_whitelist;
    bool allowLocal;
    bool allowDefaults;
    PluginSettings();
    ~PluginSettings();

    bool InWhitelist(std::string s);

    /** Loads config. Will fail silently if config is not found */
    void Load();
    /** Saves config. Will throw ios_base::failure on failure */
    void Save();
private:
    void removeDuplicateEntries(std::vector<std::string>& v);
    void removeDefaultEntries(std::vector<std::string>& v);
    bool inDefaultWhitelist(const std::string& s);
    boost::filesystem::path configDirectory();

    boost::filesystem::path m_configFile;
};
#endif
