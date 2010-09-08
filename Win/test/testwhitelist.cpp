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

#include "whitelistdialog.h"
#include <iostream>
#include <boost/bind.hpp>
#include <boost/signals.hpp>

class TestWindowsUI
{
public:
    TestWindowsUI(WhitelistDialog& m)
        : m_whitelistDialog(m)
    {
        m_connection = m_whitelistDialog.connect(boost::bind(&TestWindowsUI::on_whitelistdialog_response, this, _1));
    }

    virtual ~TestWindowsUI()
    {
        m_whitelistDialog.disconnect(m_connection);
    }

    void on_whitelistdialog_response(int response)
    {
        std::cout << "on_whitelistdialog_response: ";
        typedef std::vector<std::string>::const_iterator Iter;

        if (response == WhitelistDialog::RESPONSE_OK) {
            std::cout << "OK" << std::endl;

            std::vector<std::string> sv = m_whitelistDialog.getWhitelist();
            for (Iter it = sv.begin(); it != sv.end(); ++it)
                std::cout << "--> " << *it << std::endl;

        } else {
            std::cout << "Cancel" << std::endl;
        }
    }


protected:
    WhitelistDialog& m_whitelistDialog;

private:
    WhitelistDialog::Connection m_connection;
};


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int iCmdShow)
{
    INITCOMMONCONTROLSEX InitCtrls;
    InitCtrls.dwICC = ICC_LISTVIEW_CLASSES;
    InitCtrls.dwSize = sizeof(INITCOMMONCONTROLSEX);
    BOOL bRet = InitCommonControlsEx(&InitCtrls);

    WhitelistDialog dialog(hInstance);
    TestWindowsUI ui(dialog);

    // Insert dummy sites
    std::vector<std::string> sv;
    sv.push_back("id.swedbank.ee");
    sv.push_back("id.seb.ee");
    sv.push_back("id.eesti.ee");
    dialog.addDefaultSites(sv);

    sv.clear();
    sv.push_back("not.in.default.whitelist.org");
    sv.push_back("user.added.ee");
    dialog.addSites(sv);

    dialog.doDialog();

    // Start the message loop. 
    MSG msg;
    while ((bRet = GetMessage(&msg, NULL, 0, 0)) != 0) {
        if (bRet == -1) {
            // handle the error and possibly exit
        } else {
            TranslateMessage(&msg); 
            DispatchMessage(&msg); 
        }
    } 

    return 0;
}
