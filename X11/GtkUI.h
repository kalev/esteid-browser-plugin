#include "PluginUI.h"
#include <stdexcept>

class GtkUI : public PluginUI {
public:
    GtkUI();
    virtual ~GtkUI();

    virtual std::string PromptForSignPIN(std::string subject,
        std::string docUrl, std::string docHash,
        std::string pageUrl, int pinPadTimeout,
        bool retry, int tries);
    virtual void ClosePinPrompt();
    virtual void ShowSettings(PluginSettings &conf, std::string pageUrl = "");
    virtual void ShowPinBlockedMessage(int pin);

private:
    bool m_dialog_up;
};
