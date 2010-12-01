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

#ifndef H_ESTEID_PLUGINSETTINGS
#define H_ESTEID_PLUGINSETTINGS

#include <string>
#include <boost/filesystem/path.hpp>
#include "whitelist.h"

class PluginSettings {
    static boost::filesystem::path globalSettingsDir();
    static boost::filesystem::path userSettingsDir();
    static boost::filesystem::path globalSettingsFile();
    static boost::filesystem::path userSettingsFile();
    static boost::filesystem::path legacySettingsFile();

public:
    PluginSettings();
    virtual ~PluginSettings();

    /* Returns true if site s is in whitelist */
    bool inWhitelist(const std::string& s);

    void load(const boost::filesystem::path& filename,
              Whitelist& out_whitelist);
    void loadLegacy(const boost::filesystem::path& filename);
    void convertLegacy();
    void load();

    /** Saves config. Will throw ios_base::failure on failure */
    void save(const boost::filesystem::path& filename);
    void save();

    Whitelist defaultWhitelist();
    Whitelist whitelist();
    void setWhitelist(const Whitelist& whitelist);
    void addSite(const std::string& site);
    void clearWhitelist();
    bool allowLocal();

private:
    Whitelist m_whitelist;
    Whitelist m_defaultWhitelist;
    bool m_allowLocal;
};
#endif
