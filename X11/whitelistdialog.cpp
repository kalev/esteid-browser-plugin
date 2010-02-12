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
    m_listModel->clear();
}


WhitelistDialog::~WhitelistDialog()
{
}


Gtk::TreeView *WhitelistDialog::getTreeView()
{
    Gtk::TreeView *treeview;
    Gtk::CellRendererText *renderer;
    Gtk::TreeViewColumn *col0;

    m_refGlade->get_widget("whitelistView", treeview);

    m_listModel = Gtk::ListStore::create(m_listColumns);
    m_listModel->set_sort_column_id(0, Gtk::SORT_ASCENDING);
    treeview->set_model(m_listModel);

    // Set up custom renderer to show some sites as not sensitive
    renderer = new Gtk::CellRendererText();
    treeview->append_column("Sites", *renderer);
    col0 = treeview->get_column(0);
    col0->add_attribute(*renderer, "text", 0);
    col0->add_attribute(*renderer, "sensitive", 1);

    return treeview;
}


void WhitelistDialog::addSites(const std::vector<std::string> & sv)
{
    std::vector<std::string>::const_iterator it;

    for (it = sv.begin(); it != sv.end(); ++it)
        addSite(*it);
}


void WhitelistDialog::addDefaultSites(const std::vector<std::string> & sv)
{
    std::vector<std::string>::const_iterator it;

    for (it = sv.begin(); it != sv.end(); ++it)
        addDefaultSite(*it);
}


void WhitelistDialog::addSite(const std::string & site, bool defaultSite)
{
    Gtk::TreeModel::Row row;

    row = *(m_listModel->append());
    row[m_listColumns.site] = site;
    if (defaultSite) // Mark sites in default whitelist as not sensitive
        row[m_listColumns.sensitive] = false;
    else
        row[m_listColumns.sensitive] = true;
}


void WhitelistDialog::addDefaultSite(const std::string & site)
{
    addSite(site, true);
}


void WhitelistDialog::on_button_cancel()
{
    hide();
}
