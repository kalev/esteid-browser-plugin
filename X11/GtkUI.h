#include "PluginUI.h"
#include <stdexcept>

#include <gtkmm.h>

class WhitelistDialog;
class PluginSettings;

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

protected:
    int loadGladeUI(std::string gladeFile);

    WhitelistDialog *m_whitelistDialog;
    PluginSettings *m_conf;

    // Glade interface description.
    Glib::RefPtr<Gtk::Builder> m_refGlade;

private:
    bool m_dialog_up;
};
