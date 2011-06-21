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

#include "WindowsUI.h"

#include <stdio.h>
#include <vector>
#include <boost/bind.hpp>
#include <boost/signals.hpp>

#include "Win/PluginWindowWin.h"
#include "Win/pininputdialog.h"
#include "Win/whitelistdialog.h"

#include "utf8_tools.h"
#include "debug.h"

#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

WindowsUI::WindowsUI(boost::shared_ptr<UICallbacks> cb)
    : PluginUI(cb),
      m_pinInputDialog(new PinInputDialog(ATL::_AtlBaseModule.GetResourceInstance())),
      m_whitelistDialog(new WhitelistDialog(ATL::_AtlBaseModule.GetResourceInstance()))
{
    ESTEID_DEBUG("WindowsUI initialized");

    // connect signals
    m_pinInputConnection = m_pinInputDialog->connect(boost::bind(&WindowsUI::on_pininputdialog_response, this, _1));
    m_whitelistConnection = m_whitelistDialog->connect(boost::bind(&WindowsUI::on_whitelistdialog_response, this, _1));
}

WindowsUI::~WindowsUI()
{
    ESTEID_DEBUG("~WindowsUI()");
    m_pinInputDialog->disconnect(m_pinInputConnection);
    m_whitelistDialog->disconnect(m_whitelistConnection);
}


HWND WindowsUI::browserHWND()
{
    if (m_window) {
        FB::PluginWindowWin* wnd = reinterpret_cast<FB::PluginWindowWin*>(m_window);
        return wnd->getBrowserHWND();
    } else {
        return NULL;
    }
}


HWND WindowsUI::pluginHWND()
{
    if (m_window) {
        FB::PluginWindowWin* wnd = reinterpret_cast<FB::PluginWindowWin*>(m_window);
        return wnd->getHWND();
    } else {
        return NULL;
    }
}


HWND WindowsUI::parentHWND()
{
    HWND hWnd = browserHWND();
    if (hWnd)
        return hWnd;
    else
        return GetForegroundWindow();
}


void WindowsUI::pinDialog(const std::string& subject,
                          const std::string& docUrl,
                          const std::string& docHash,
                          bool retry, int tries)
{
    ESTEID_DEBUG("WindowsUI::pinDialog()");

    m_pinInputDialog->setSubject(subject);
    m_pinInputDialog->setRetry(retry);
    m_pinInputDialog->setTries(tries);
    m_pinInputDialog->doDialog(parentHWND());
}


void WindowsUI::closePinDialog()
{
    ESTEID_DEBUG("WindowsUI::closePinDialog()");
}


void WindowsUI::pinBlockedMessage(int pin)
{
    ESTEID_DEBUG("WindowsUI::pinBlockedMessage()");

    m_pinInputDialog->showPinBlocked(parentHWND());
}


void WindowsUI::settingsDialog(PluginSettings& settings, const std::string& pageUrl)
{
    ESTEID_DEBUG("WindowsUI::settingsDialog()");

    m_settings = &settings;

    m_whitelistDialog->addDefaultSites(m_settings->defaultWhitelist());
    m_whitelistDialog->addSites(m_settings->whitelist());
    m_whitelistDialog->doDialog(parentHWND());

    if (pageUrl.length() > 0)
        m_whitelistDialog->setEntryText(pageUrl);
}


void WindowsUI::iteration()
{
    MSG msg;

    if (!PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
        return;

    if (!IsDialogMessage(pluginHWND(), &msg)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}


void WindowsUI::on_pininputdialog_response(int response)
{
    std::string pin = m_pinInputDialog->getPin();

    // make sure the dialog doesn't cache PIN
    m_pinInputDialog->clearPin();

    if (response == PinInputDialog::RESPONSE_OK) {
        m_callbacks->onPinEntered(pin);
    } else {
        m_callbacks->onPinCancelled();
    }
}

void WindowsUI::on_whitelistdialog_response(int response)
{
    if (response == WhitelistDialog::RESPONSE_OK) {
        m_settings->setWhitelist(m_whitelistDialog->getWhitelist());
        try {
            m_settings->save();
        } catch(const std::exception& e) {
            std::wstring errorMessage = L"Error saving configuration.\n" +
                                        FB::utf8_to_wstring(e.what());
            MessageBox(parentHWND(), const_cast<wchar_t *>(errorMessage.c_str()),
                       L"Error", MB_OK | MB_ICONERROR);
        }
    }
}
