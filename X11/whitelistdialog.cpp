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

    m_whitelistView = getTreeView();


    // Insert dummy sites
    std::vector<std::string> sv;
    sv.push_back("id.swedbank.ee");
    sv.push_back("id.seb.ee");
    sv.push_back("id.eesti.ee");

    m_listModel->clear();
    addSites(sv);
}


WhitelistDialog::~WhitelistDialog()
{
}


Gtk::TreeView *WhitelistDialog::getTreeView()
{
    Gtk::TreeView *treeview;

    m_refGlade->get_widget("whitelistView", treeview);

    m_listModel = Gtk::ListStore::create(m_listColumns);
    m_listModel->set_sort_column_id(0, Gtk::SORT_ASCENDING);
    treeview->set_model(m_listModel);
    treeview->append_column("Sites", m_listColumns.site);

    return treeview;
}


void WhitelistDialog::addSites(const std::vector<std::string> & sv)
{
    std::vector<std::string>::const_iterator it;

    for (it = sv.begin(); it != sv.end(); ++it)
        addSite(*it);
}


void WhitelistDialog::addSite(const std::string & site)
{
    Gtk::TreeModel::iterator it;

    it = m_listModel->append();
    (*it)[m_listColumns.site] = site;
}


void WhitelistDialog::on_button_cancel()
{
    hide();
}
