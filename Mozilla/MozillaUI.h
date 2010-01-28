#include "../PluginUI.h"
#include <stdexcept>

/* Forward declarations */
class nsIEstEIDPrivate;
class nsIDOMWindow;
class nsISupports;

class MozillaUI : public PluginUI {
public:
    MozillaUI(nsISupports *smp, nsISupports *dwp);
    virtual ~MozillaUI();

    virtual std::string PromptForSignPIN(std::string subject,
        std::string docUrl, std::string docHash,
        std::string pageUrl, int pinPadTimeout,
        bool retry, int tries);
    virtual void ClosePinPrompt();
    virtual void ShowSettings(std::string pageUrl = "");
    virtual void ShowPinBlockedMessage(int pin);

private:
    /* Pointers to Mozilla interfaces. */
    nsIEstEIDPrivate *m_eid;
    nsIDOMWindow *m_dw;
};
