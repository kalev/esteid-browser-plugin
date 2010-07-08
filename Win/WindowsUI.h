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

#ifndef WINDOWSUI_H
#define WINDOWSUI_H

#include "PluginUI.h"
#include "Win/win_common.h"

class pinDialog;
class WhitelistDialog;

struct pinDialogPriv_l {
    HINSTANCE m_hInst;
    WORD m_resourceID;
};

class WindowsUI : public PluginUI {
public:
    WindowsUI(FB::AutoPtr<UICallbacks>);
    virtual ~WindowsUI();

    void PromptForSignPIN(std::string subject,
        std::string docUrl, std::string docHash,
        int pinPadTimeout, bool retry, int tries);
    void ClosePinPrompt();
#ifdef SUPPORT_OLD_APIS
    void WaitForPinPrompt();
#endif
    void ShowSettings(PluginSettings &conf, std::string pageUrl = "");
    void ShowPinBlockedMessage(int pin);

protected:
    pinDialog *m_pinDialog;
    WhitelistDialog *m_whitelistDialog;
    PluginSettings *m_conf;
};

#endif /* WINDOWSUI_H */
