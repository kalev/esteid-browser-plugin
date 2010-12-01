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

#ifndef H_ESTEID_MacUI
#define H_ESTEID_MacUI

#include "../PluginUI.h"
#include "npapi.h"
#include <stdexcept>

class MacUI : public PluginUI {
public:
    MacUI(boost::shared_ptr<UICallbacks>);
    virtual ~MacUI();
        
    void PromptForPinAsync(const std::string& subject,
                           const std::string& docUrl, const std::string& docHash,
                           int pinPadTimeout, bool retry, int tries);
    void ClosePinPrompt();
#ifdef SUPPORT_OLD_APIS
    std::string PromptForPin(const std::string& subject,
                             const std::string& docUrl, const std::string& docHash,
                             int pinPadTimeout, bool retry, int tries);
#endif
    void ShowPinBlockedMessage(int pin);
    void ShowSettings(PluginSettings& settings, const std::string& pageUrl = "");
        
    static void SetWindow(void *window);
private:
    void *m_internal;
};

#endif /* H_ESTEID_MacUI */
