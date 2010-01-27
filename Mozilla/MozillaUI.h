#include "../PluginUI.h"
#include <stdexcept>

class MozillaUI : public PluginUI {
public:
    MozillaUI(void *smp, void *dwp);
    virtual ~MozillaUI();

    virtual std::string PromptForSignPIN(std::string subject,
        std::string docUrl, std::string docHash,
        std::string pageUrl, int pinPadTimeout,
        bool retry, int tries);
    virtual void ClosePinPrompt();
    virtual void ShowSettings(std::string pageUrl = "");
    virtual void ShowPinBlockedMessage(int pin);

private:
    /* Pointers to Mozilla interfaces.
       We use void type because we can not pull in XPCOM headers
       for the rest of the plugin that needs to include this file */
    void *m_eid;  // nsIEstEIDPrivate
    void *m_dw;   // nsIDOMWindow
};
