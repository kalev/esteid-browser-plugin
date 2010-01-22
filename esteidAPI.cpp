#include "BrowserObjectAPI.h"
#include "variant_list.h"
#include "DOM/JSAPI_DOMDocument.h"

#include "esteidAPI.h"

esteidAPI::esteidAPI(FB::BrowserHostWrapper *host) : m_host(host)
{
    // Methods
    registerMethod("getVersion",  make_method(this, &esteidAPI::getVersion));
    //registerMethod("sign",        make_method(this, &esteidAPI::sign));
    
    // Read-only properties
    registerProperty("lastName",
                     make_property(this, &esteidAPI::get_lastName));
    //registerProperty("residencePermit",
    //                 make_property(this, &esteidAPI::get_residencePermit));
}

esteidAPI::~esteidAPI()
{
}

std::string esteidAPI::get_lastName()
{
    return "Pakiraam";
}

std::string esteidAPI::getVersion()
{
    return "0.x.y";
}
