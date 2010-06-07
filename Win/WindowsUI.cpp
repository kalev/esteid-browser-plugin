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

#include "WindowsUI.h"

#include <stdio.h>
#include <vector>

#include "Win/PluginWindowWin.h"
#include "utility/pinDialog.h"
#include "Win/whitelistdialog.h"

#include "debug.h"

WindowsUI::WindowsUI(esteidAPI *esteidAPI)
    : PluginUI(esteidAPI)
{
    ESTEID_DEBUG("WindowsUI intialized");
}

WindowsUI::~WindowsUI()
{
    ESTEID_DEBUG("~WindowsUI()");
}


std::string WindowsUI::PromptForSignPIN(std::string subject,
        std::string docUrl, std::string docHash,
        std::string pageUrl, int pinPadTimeout, bool retry, int tries)
{
    ESTEID_DEBUG("WindowsUI::PromptForSignPIN()");

    PinString pin;

    pinDialogPriv_l params = {
	    ATL::_AtlBaseModule.GetResourceInstance(),
            IDD_PIN_DIALOG_ENG
    };

    pinDialog dlg(&params, EstEidCard::SIGN);
    if (!dlg.doDialog())
        return "";

    pin = dlg.getPin();
    return pin.c_str();
}


void WindowsUI::ClosePinPrompt()
{
    ESTEID_DEBUG("WindowsUI::ClosePinPrompt()");
}


void WindowsUI::ShowPinBlockedMessage(int pin)
{
    ESTEID_DEBUG("WindowsUI::ShowPinBlockedMessage()");

    pinDialogPriv_l params = { NULL, NULL };
    pinDialog dlg(&params, "");
    dlg.showPrompt("PIN2 blocked.\nPlease run ID-card Utility to unlock the PIN.", false);
}


void WindowsUI::ShowSettings(PluginSettings &conf, std::string pageUrl)
{
    ESTEID_DEBUG("WindowsUI::ShowSettings()");

    m_conf = &conf;

    m_whitelistDialog = new WhitelistDialog();
    m_whitelistDialog->addDefaultSites(conf.default_whitelist);
    m_whitelistDialog->addSites(conf.whitelist);
    m_whitelistDialog->doDialog(ATL::_AtlBaseModule.GetResourceInstance(), conf);

    if (pageUrl.length() > 0)
        m_whitelistDialog->setEntryText(pageUrl);
}
