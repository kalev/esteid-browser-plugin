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

#ifndef H_ESTEID_PLUGINUI
#define H_ESTEID_PLUGINUI

#include <string>
#include <boost/shared_ptr.hpp>
#include "PluginWindow.h"
#include "PluginSettings.h"

// Forward declaration
class EsteidAPI;

/** A base class for different UI implementations */
class PluginUI
{
public:
    class UICallbacks {
    public:
        UICallbacks() {}
        virtual void onPinEntered(const std::string& pin) = 0;
        virtual void onPinCancelled() = 0;
    };

    /** Prompt for Signature PIN */
    virtual void pinDialog(const std::string& subject,
                           const std::string& docUrl,
                           const std::string& docHash,
                           bool retry, int tries) = 0;

    /** Close Signature PIN dialog */
    virtual void closePinDialog() = 0;

    /** Inform user that the PIN has been blocked */
    virtual void pinBlockedMessage(int pin) = 0;

    /** Open Settings dialog */
    virtual void settingsDialog(PluginSettings& conf,
                                const std::string& pageUrl = "") = 0;

    /** Run a single iteration of the underlying platform's event loop */
    virtual void iteration() = 0;

    PluginUI(boost::shared_ptr<UICallbacks>);
    virtual ~PluginUI();

    virtual void setWindow(FB::PluginWindow*);

protected:
    boost::shared_ptr<UICallbacks> m_callbacks;
    FB::PluginWindow* m_window;
};

#endif // H_ESTEID_PLUGINUI
