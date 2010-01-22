/**********************************************************\
Original Author: Richard Bateman and Georg Fritzsche

Created:    December 3, 2009
License:    Dual license model; choose one of two:
            Eclipse Public License - Version 1.0
            http://www.eclipse.org/legal/epl-v10.html
            - or -
            GNU Lesser General Public License, version 2.1
            http://www.gnu.org/licenses/lgpl-2.1.html

Copyright 2009 PacketPass Inc, Georg Fritzsche,
               Firebreath development team
\**********************************************************/

#include "BrowserObjectAPI.h"
#include "variant_list.h"
#include "DOM/JSAPI_DOMDocument.h"

#include "esteidAPI.h"

esteidAPI::esteidAPI(FB::BrowserHostWrapper *host) : m_host(host)
{
    registerMethod("echo",  make_method(this, &esteidAPI::echo));

    // Read-write property
    registerProperty("testString",
                     make_property(this,
                        &esteidAPI::get_testString,
                        &esteidAPI::set_testString));

    // Read-only property
    registerProperty("version",
                     make_property(this,
                        &esteidAPI::get_version));
}

esteidAPI::~esteidAPI()
{
}

// Read/Write property testString
std::string esteidAPI::get_testString()
{
    return m_testString;
}
void esteidAPI::set_testString(std::string val)
{
    m_testString = val;
}

// Read-only property version
std::string esteidAPI::get_version()
{
    return "CURRENT_VERSION";
}

// Method echo
FB::variant esteidAPI::echo(FB::variant msg)
{
    return msg;
}
