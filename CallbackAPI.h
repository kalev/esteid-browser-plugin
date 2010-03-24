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

#include <string>
#include <sstream>
#include "JSAPIAuto.h"
#include "BrowserHostWrapper.h"

/** A base class for implementing event listeners.
 * Objects of this type can be attached to DOM events like this:
 *    domObj.callMethod<FB::variant>("addEventListener", 
 *                FB::variant_list_of("click")(CallbackInstance)(false));
 *
 */
class CallbackAPI : public FB::JSAPIAuto
{
public:
    CallbackAPI(FB::BrowserHostWrapper *host);
    virtual ~CallbackAPI();
    virtual bool handleEvent(FB::JSObject evt);

    /** Override this */
    virtual bool eventHandler() = 0;

    virtual FB::variant Invoke(std::string methodName, std::vector<FB::variant>& args);
    virtual bool HasMethod(std::string methodName);
private:
    FB::AutoPtr<FB::BrowserHostWrapper> m_host;
};
