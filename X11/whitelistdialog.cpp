#include "whitelistdialog.h"

WhitelistDialog::WhitelistDialog(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& refGlade)
    : Gtk::Dialog(cobject),
      m_refGlade(refGlade),
      m_cancelButton(0)
{
    // Get the Glade-instantiated Button, and connect a signal handler:
    m_refGlade->get_widget("cancelButton", m_cancelButton);
    if (m_cancelButton) {
        m_cancelButton->signal_clicked().connect( sigc::mem_fun(*this,
                    &WhitelistDialog::on_button_cancel) );
    }

    m_refGlade->get_widget("scrolledWindow", m_scrolledWindow);

    m_whitelistView = new Gtk::ListViewText(1);
    m_scrolledWindow->add(*m_whitelistView);
    m_whitelistView->set_headers_visible(false);


    std::vector<std::string> sv;
    sv.push_back("id.swedbank.ee");
    sv.push_back("id.seb.ee");
    sv.push_back("id.eesti.ee");

    m_whitelistView->clear_items();

    guint row_number;

    std::vector<std::string>::const_iterator it;
    for (it = sv.begin(); it != sv.end(); ++it) {
        row_number = m_whitelistView->append_text();
        m_whitelistView->set_text(row_number,  0, (*it).c_str());
    }

    m_whitelistView->show();
}

WhitelistDialog::~WhitelistDialog()
{
}

void WhitelistDialog::on_button_cancel()
{
    hide();
}
