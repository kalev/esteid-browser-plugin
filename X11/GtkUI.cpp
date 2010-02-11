#include "GtkUI.h"

#include <stdio.h>
#include <iostream>

#include "X11/pininputdialog.h"
#include "X11/PluginWindowX11.h"
#include "X11/whitelistdialog.h"

#define ESTEID_DEBUG printf


GtkUI::GtkUI():
    m_dialog_up(false)
{
    ESTEID_DEBUG("GtkUI intialized\n");

    Gtk::Main::init_gtkmm_internals();

    if (loadGladeUI("whitelist.glade") != 0) {
        ESTEID_DEBUG("GtkUI::GtkUI(): loadGladeUI() failed\n");
    }

    m_refGlade->get_widget_derived("WhitelistDialog", m_whitelistDialog);
}


GtkUI::~GtkUI()
{
    ESTEID_DEBUG("~GtkUI()\n");
}


int GtkUI::loadGladeUI(std::string gladeFile)
{
    //Load the GtkBuilder file and instantiate its widgets:
    m_refGlade = Gtk::Builder::create();
#ifdef GLIBMM_EXCEPTIONS_ENABLED
    try {
        m_refGlade->add_from_file(gladeFile);
    } catch(const Glib::FileError& ex) {
        std::cerr << "FileError: " << ex.what() << std::endl;
        return 1;
    } catch(const Gtk::BuilderError& ex) {
        std::cerr << "BuilderError: " << ex.what() << std::endl;
        return 1;
    }
#else
    std::auto_ptr<Glib::Error> error;

    if (!m_refGlade->add_from_file(gladeFile, error)) {
        std::cerr << error->what() << std::endl;
        return 1;
    }
#endif /* !GLIBMM_EXCEPTIONS_ENABLED */

    return 0;
}


std::string GtkUI::PromptForSignPIN(std::string subject,
        std::string docUrl, std::string docHash,
        std::string pageUrl, int pinPadTimeout, bool retry, int tries)
{
    ESTEID_DEBUG("GtkUI::PromptForSignPIN()\n");

    if (m_dialog_up)
        return "";

    std::string pin;

    PinInputDialog dialog(PIN2, subject);
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

    if (!m_whitelistDialog)
        return;

    if (m_dialog_up)
        return;

    m_dialog_up = true;
    m_whitelistDialog->run();
    m_dialog_up = false;
}
