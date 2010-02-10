#include "GtkUI.h"

#include <stdio.h>

#include "X11/pininput.h"
#include "X11/PluginWindowX11.h"

#define ESTEID_DEBUG printf

GtkUI::GtkUI():
    m_dialog_up(false)
{
    ESTEID_DEBUG("GtkUI intialized\n");

    Gtk::Main::init_gtkmm_internals();
}

GtkUI::~GtkUI()
{
    ESTEID_DEBUG("~GtkUI()\n");
}


std::string GtkUI::PromptForSignPIN(std::string subject,
        std::string docUrl, std::string docHash,
        std::string pageUrl, int pinPadTimeout, bool retry, int tries)
{
    ESTEID_DEBUG("GtkUI::PromptForSignPIN()\n");

    if (m_dialog_up)
        return "";

    std::string pin;

    PinInput dialog(PIN2, subject);
    m_dialog_up = true;
    int rv = dialog.run();
    m_dialog_up = false;

    if (rv == Gtk::RESPONSE_OK) {
        pin = dialog.getPin();
        ESTEID_DEBUG("GtkUI::PromptForSignPIN(): PIN is '%s'\n", pin.c_str());
    } else {
        ESTEID_DEBUG("GtkUI::PromptForSignPIN(): cancelled\n", pin.c_str());
    }

    return pin;
}


void GtkUI::ClosePinPrompt()
{
    ESTEID_DEBUG("GtkUI::ClosePinPrompt()\n");
}


void GtkUI::ShowPinBlockedMessage(int pin)
{
    ESTEID_DEBUG("GtkUI::ShowPinBlockedMessage()\n");

    if (m_dialog_up)
        return;

    Gtk::MessageDialog dialog("PIN2 blocked", false, Gtk::MESSAGE_WARNING);
    dialog.set_secondary_text("Please run ID-card Utility to unlock the PIN.");
    m_dialog_up = true;
    dialog.run();
    m_dialog_up = false;
}


void GtkUI::ShowSettings(PluginSettings &conf, std::string pageUrl)
{
    ESTEID_DEBUG("GtkUI::ShowSettings()\n");
}
