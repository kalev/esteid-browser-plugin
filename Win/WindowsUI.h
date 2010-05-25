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

struct pinDialogPriv_l {
    HINSTANCE m_hInst;
    WORD m_resourceID;
};


class WindowsUI : public PluginUI {
public:
    WindowsUI(esteidAPI *esteidAPI);
    virtual ~WindowsUI();

    virtual std::string PromptForSignPIN(std::string subject,
        std::string docUrl, std::string docHash,
        std::string pageUrl, int pinPadTimeout,
        bool retry, int tries);
    virtual void ClosePinPrompt();
    virtual void ShowSettings(PluginSettings &conf, std::string pageUrl = "");
    virtual void ShowPinBlockedMessage(int pin);
};

#endif /* WINDOWSUI_H */
