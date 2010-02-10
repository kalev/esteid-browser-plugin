#ifndef ESTEID_WHITELISTDIALOG_H
#define ESTEID_WHITELISTDIALOG_H

#include <gtkmm.h>


class WhitelistDialog : public Gtk::Dialog
{
public:
    WhitelistDialog(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& refGlade);
    virtual ~WhitelistDialog();

protected:
    // Signal handlers:
    void on_button_cancel();

    Glib::RefPtr<Gtk::Builder> m_refGlade;

    Gtk::ScrolledWindow *m_scrolledWindow;
    Gtk::ListViewText *m_whitelistView;
    Gtk::Button *m_cancelButton;
};

#endif //ESTEID_WHITELISTDIALOG_H
