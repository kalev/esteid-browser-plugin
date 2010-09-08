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

#include "PluginUI.h"
#include <stdexcept>

#include <gtkmm.h>

class PinInputDialog;
class WhitelistDialog;
class PluginSettings;

class GtkUI : public PluginUI {
public:
    GtkUI(boost::shared_ptr<UICallbacks>);
    virtual ~GtkUI();

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
    void on_pininputdialog_response(int response_id);
    void on_whitelistdialog_response(int response_id);

    GdkWindow* browserWindow();

    PinInputDialog *m_pinInputDialog;
    WhitelistDialog *m_whitelistDialog;
    PluginSettings *m_conf;

private:
    bool m_dialog_up;
    sigc::connection m_pinInputConnection;
};
