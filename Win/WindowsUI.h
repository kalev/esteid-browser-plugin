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
#include "Win/pininputdialog.h"
#include "Win/pinpaddialog.h"
#include "Win/whitelistdialog.h"

class WindowsUI : public PluginUI {
public:
    WindowsUI(boost::shared_ptr<UICallbacks>);
    virtual ~WindowsUI();

    void pinDialog(const std::string& subject,
                   const std::string& docUrl,
                   const std::string& docHash);
    void pinpadDialog(const std::string& subject,
                      const std::string& docUrl,
                      const std::string& docHash,
                      int timeout);
    void retryPinDialog(int triesLeft);
    void retryPinpadDialog(int triesLeft);
    void closePinDialog();
    void closePinpadDialog();
    void settingsDialog(PluginSettings& settings, const std::string& pageUrl = "");
    void pinBlockedMessage(int pin);
    void iteration();

protected:
    void on_pininputdialog_response(int response);
    void on_whitelistdialog_response(int response);

    HWND browserHWND();
    HWND pluginHWND();
    HWND parentHWND();

    PluginSettings *m_settings;
    std::auto_ptr<PinInputDialog> m_pinInputDialog;
    std::auto_ptr<PinpadDialog> m_pinpadDialog;
    std::auto_ptr<WhitelistDialog> m_whitelistDialog;
    PinInputDialog::Connection m_pinInputConnection;
    WhitelistDialog::Connection m_whitelistConnection;
};

#endif /* WINDOWSUI_H */
