#include "BrowserObjectAPI.h"

#include "CallbackAPI.h"

#include <stdio.h> // FIXME: Remove

CallbackAPI::CallbackAPI(FB::BrowserHostWrapper *host) :
     m_host(host)
{
    registerMethod("handleEvent", make_method(this, &CallbackAPI::handleEvent));
    registerMethod("", make_method(this, &CallbackAPI::eventHandler));
}

CallbackAPI::~CallbackAPI()
{
}

bool CallbackAPI::handleEvent(FB::JSObject evt) {
    return eventHandler();
}

FB::variant CallbackAPI::Invoke(std::string methodName, std::vector<FB::variant>& args) {
    printf("CallbackAPI::Invoke(%s, ... )\n", methodName.c_str());
    return FB::JSAPIAuto::Invoke(methodName, args);
}

bool CallbackAPI::HasMethod(std::string methodName) {
    printf("CallbackAPI::HasMethod(%s)\n", methodName.c_str());
    return FB::JSAPIAuto::HasMethod(methodName);
}
