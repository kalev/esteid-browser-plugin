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
