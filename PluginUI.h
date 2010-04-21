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

#ifndef H_ESTEID_PLUGINUI
#define H_ESTEID_PLUGINUI

#include <string>
#include "PluginSettings.h"

// Forward declaration
class esteidAPI;

/** A base class for different UI implementations */
class PluginUI
{
public:
    /** Prompt for Signature PIN */
    virtual std::string PromptForSignPIN(std::string subject,
        std::string docUrl, std::string docHash,
        std::string pageUrl, int pinPadTimeout,
        bool retry, int tries) = 0;

    /** Close Signature PIN prompt (used when user hits cancel on pinpad) */
    virtual void ClosePinPrompt() = 0;

    /** Inform user that the PIN has been blocked */
    virtual void ShowPinBlockedMessage(int pin) = 0;

    /** Open Settings dialog */
    virtual void ShowSettings(PluginSettings &conf,
                              std::string pageUrl = "") = 0;

    PluginUI(esteidAPI *esteidAPI = 0);
    virtual ~PluginUI(void);

    // Support Reference counting
    void AddRef();
    unsigned int Release();

protected:
    unsigned int m_refCount;
    esteidAPI *m_esteidAPI;
};
#endif // H_ESTEID_PLUGINUI
