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

#include "pininputdialog.h"
#include <iostream>
#include <boost/bind.hpp>
#include <boost/signals.hpp>
#include <commctrl.h>

class TestWindowsUI
{
public:
    TestWindowsUI(PinInputDialog& m)
        : m_pinInputDialog(m)
    {
        m_connection = m_pinInputDialog.connect(boost::bind(&TestWindowsUI::on_pininputdialog_response, this, _1));
    }

    virtual ~TestWindowsUI()
    {
        m_pinInputDialog.disconnect(m_connection);
    }

    void on_pininputdialog_response(int response)
    {
        std::cout << "on_pininputdialog_response: ";

        if (response == PinInputDialog::RESPONSE_OK) {
            std::cout << "OK" << std::endl;
            std::cout << "PIN is " << m_pinInputDialog.getPin() << std::endl;
        } else {
            std::cout << "Cancel" << std::endl;
        }
    }


protected:
    PinInputDialog& m_pinInputDialog;

private:
    PinInputDialog::Connection m_connection;
};


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int iCmdShow)
{
    INITCOMMONCONTROLSEX InitCtrls;
    InitCtrls.dwICC = ICC_LISTVIEW_CLASSES;
    InitCtrls.dwSize = sizeof(INITCOMMONCONTROLSEX);
    BOOL bRet = InitCommonControlsEx(&InitCtrls);

    PinInputDialog dialog(hInstance);
    TestWindowsUI ui(dialog);

    dialog.showWrongPin(2);
    dialog.showPinBlocked();

    dialog.setSubject("Peeter Pakiraam");

    std::cout << "Starting modal dialog:" << std::endl;
    int response = dialog.doModalDialog();
    if (response == PinInputDialog::RESPONSE_OK) {
        std::cout << "OK" << std::endl;
        std::cout << "PIN is " << dialog.getPin() << std::endl;
    } else {
        std::cout << "Cancel" << std::endl;
    }

    std::cout << "Starting non-modal dialog:" << std::endl;
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
