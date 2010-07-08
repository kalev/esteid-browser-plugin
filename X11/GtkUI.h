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

#include "PluginUI.h"
#include <stdexcept>

#include <gtkmm.h>

class PinInputDialog;
class WhitelistDialog;
class PluginSettings;
class esteidAPI;

class GtkUI : public PluginUI {
public:
    GtkUI(FB::AutoPtr<UICallbacks>);
    virtual ~GtkUI();

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
    void on_pininputdialog_response(int response_id);
    void on_whitelistdialog_response(int response_id);

    PinInputDialog *m_pinInputDialog;
    WhitelistDialog *m_whitelistDialog;
    PluginSettings *m_conf;

private:
    bool m_dialog_up;
};
