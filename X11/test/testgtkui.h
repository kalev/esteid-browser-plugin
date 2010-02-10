#ifndef ESTEID_TESTGTKUI
#define ESTEID_TESTGTKUI

#include <gtkmm.h>

#include "whitelistdialog.h"

class TestGtkUI : public Gtk::Window
{
public:
    TestGtkUI();
    virtual ~TestGtkUI();

protected:
    int loadGladeUI(std::string gladeFile);

    // Signal handlers:
    void on_button_pinBlocked_clicked();
    void on_button_pinInput_clicked();
    void on_button_whitelist_clicked();

    // Child widgets:
    Gtk::VButtonBox m_ButtonBox;
    Gtk::Button m_pinBlockedButton;
    Gtk::Button m_pinInputButton;
    Gtk::Button m_whitelistButton;

    WhitelistDialog *m_whitelistDialog;

    // Glade interface description.
    Glib::RefPtr<Gtk::Builder> m_refGlade;
};

#endif //ESTEID_TESTGTKUI
