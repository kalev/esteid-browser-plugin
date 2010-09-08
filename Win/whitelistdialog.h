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

#ifndef ESTEID_WHITELISTDIALOG_H
#define ESTEID_WHITELISTDIALOG_H

#include <windows.h>
#include <commctrl.h>
#include <string>
#include <vector>

#include "Win/basedialog.h"

class WhitelistDialog : public BaseDialog
{
public:
    WhitelistDialog(HINSTANCE hInst);
    virtual ~WhitelistDialog();

    // Functions for setting up data for displaying
    void addSites(const std::vector<std::string> & sv);
    void addDefaultSites(const std::vector<std::string> & sv);

    // Call this to retrieve modified whitelist
    std::vector<std::string> getWhitelist();

    void setEntryText(const std::string & site);

    bool doDialog(HWND hParent = NULL);

protected:
    void insertItem(const std::wstring & name, bool editable);
    void insertItem(const std::string & name, bool editable);

    void storeItems();

    LRESULT on_initdialog(WPARAM wParam);
    LRESULT on_command(WPARAM wParam, LPARAM lParam);
    LRESULT on_notify(WPARAM wParam, LPARAM lParam);

private:
    HWND m_hList;
    HWND m_hEdit;
    std::vector<std::string> m_sites;
    std::vector<std::string> m_defaultSites;
};

#endif //ESTEID_WHITELISTDIALOG_H
