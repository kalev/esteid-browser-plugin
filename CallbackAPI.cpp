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

#include "JSObject.h"
#include "CallbackAPI.h"
#include "debug.h"

CallbackAPI::CallbackAPI(FB::BrowserHostPtr host) :
     m_host(host)
{
    registerMethod("handleEvent", make_method(this, &CallbackAPI::handleEvent));
    registerMethod("", make_method(this, &CallbackAPI::eventHandler));
}

CallbackAPI::~CallbackAPI()
{
}

bool CallbackAPI::handleEvent(const FB::JSObjectPtr& evt) {
    return eventHandler();
}

FB::variant CallbackAPI::Invoke(std::string methodName, std::vector<FB::variant>& args) {
    ESTEID_DEBUG("CallbackAPI::Invoke(%s, ... )\n", methodName.c_str());
    return FB::JSAPIAuto::Invoke(methodName, args);
}

bool CallbackAPI::HasMethod(std::string methodName) {
    ESTEID_DEBUG("CallbackAPI::HasMethod(%s)\n", methodName.c_str());
    return FB::JSAPIAuto::HasMethod(methodName);
}
