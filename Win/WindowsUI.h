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

#ifndef WINDOWSUI_H
#define WINDOWSUI_H

#include "PluginUI.h"
#include "Win/win_common.h"
#include "Win/pininputdialog.h"
#include "Win/whitelistdialog.h"

class WindowsUI : public PluginUI {
public:
    WindowsUI(boost::shared_ptr<UICallbacks>);
    virtual ~WindowsUI();

    void PromptForPinAsync(const std::string& subject,
        const std::string& docUrl, const std::string& docHash,
        int pinPadTimeout, bool retry, int tries);
    void ClosePinPrompt();
#ifdef SUPPORT_OLD_APIS
    std::string PromptForPin(const std::string& subject,
        const std::string& docUrl, const std::string& docHash,
        int pinPadTimeout, bool retry, int tries);
#endif
    void ShowSettings(PluginSettings& conf, const std::string& pageUrl = "");
    void ShowPinBlockedMessage(int pin);

protected:
    void on_pininputdialog_response(int response);
    void on_whitelistdialog_response(int response);

    HWND browserHWND();
    HWND parentHWND();

    PluginSettings *m_conf;
    std::auto_ptr<PinInputDialog> m_pinInputDialog;
    std::auto_ptr<WhitelistDialog> m_whitelistDialog;
    PinInputDialog::Connection m_pinInputConnection;
    WhitelistDialog::Connection m_whitelistConnection;
};

#endif /* WINDOWSUI_H */
