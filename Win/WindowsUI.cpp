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


HWND WindowsUI::parentHWND()
{
    HWND hWnd = browserHWND();
    if (hWnd)
        return hWnd;
    else
        return GetForegroundWindow();
}


void WindowsUI::PromptForPinAsync(const std::string& subject,
        const std::string& docUrl, const std::string& docHash,
        int pinPadTimeout, bool retry, int tries)
{
    ESTEID_DEBUG("WindowsUI::PromptForPinAsync()");

    m_pinInputDialog->setSubject(subject);
    m_pinInputDialog->setRetry(retry);
    m_pinInputDialog->setTries(tries);
    m_pinInputDialog->doDialog(parentHWND());
}


#ifdef SUPPORT_OLD_APIS
std::string WindowsUI::PromptForPin(const std::string& subject,
        const std::string& docUrl, const std::string& docHash,
        int pinPadTimeout, bool retry, int tries)
{
    m_pinInputDialog->setSubject(subject);
    m_pinInputDialog->setRetry(retry);
    m_pinInputDialog->setTries(tries);
    m_pinInputDialog->doModalDialog(parentHWND());

    std::string pin = m_pinInputDialog->getPin();

    // make sure the dialog doesn't cache PIN
    m_pinInputDialog->clearPin();

    return pin;
}
#endif


void WindowsUI::ClosePinPrompt()
{
    ESTEID_DEBUG("WindowsUI::ClosePinPrompt()");
}


void WindowsUI::ShowPinBlockedMessage(int pin)
{
    ESTEID_DEBUG("WindowsUI::ShowPinBlockedMessage()");

    m_pinInputDialog->showPinBlocked(parentHWND());
}


void WindowsUI::ShowSettings(PluginSettings& settings, const std::string& pageUrl)
{
    ESTEID_DEBUG("WindowsUI::ShowSettings()");

    m_settings = &settings;

    m_whitelistDialog->addDefaultSites(m_settings->defaultWhitelist());
    m_whitelistDialog->addSites(m_settings->whitelist());
    m_whitelistDialog->doDialog(parentHWND());

    if (pageUrl.length() > 0)
        m_whitelistDialog->setEntryText(pageUrl);
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
