#include "whitelistdialog.h"

#include <iostream>

WhitelistDialog::WhitelistDialog(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& refGlade)
    : Gtk::Dialog(cobject),
      m_refGlade(refGlade),
      m_addButton(NULL),
      m_editButton(NULL),
      m_deleteButton(NULL),
      m_okButton(NULL),
      m_cancelButton(NULL)
{
    // Get the Glade-instantiated buttons
    m_refGlade->get_widget("addButton", m_addButton);
    m_refGlade->get_widget("editButton", m_editButton);
    m_refGlade->get_widget("deleteButton", m_deleteButton);
    m_refGlade->get_widget("okButton", m_okButton);
    m_refGlade->get_widget("cancelButton", m_cancelButton);

    // Connect buttons to their signal handlers
    if (m_addButton) {
        m_addButton->signal_clicked().connect( sigc::mem_fun(*this,
                    &WhitelistDialog::on_button_add) );
    }

    if (m_editButton) {
        m_editButton->signal_clicked().connect( sigc::mem_fun(*this,
                    &WhitelistDialog::on_button_edit) );
    }

    if (m_deleteButton) {
        m_deleteButton->signal_clicked().connect( sigc::mem_fun(*this,
                    &WhitelistDialog::on_button_delete) );
    }

    if (m_okButton) {
        m_okButton->signal_clicked().connect( sigc::mem_fun(*this,
                    &WhitelistDialog::on_button_ok) );
    }

    if (m_cancelButton) {
        m_cancelButton->signal_clicked().connect( sigc::mem_fun(*this,
                    &WhitelistDialog::on_button_cancel) );
    }

    // Set up treeview
    m_whitelistView = getTreeView();
    m_listModel->clear();

    m_whitelistView->signal_row_activated().connect( sigc::mem_fun(*this,
                &WhitelistDialog::on_treeview_row_activated) );

    m_whitelistView->signal_cursor_changed().connect( sigc::mem_fun(*this,
                &WhitelistDialog::on_treeview_cursor_changed) );
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


void WhitelistDialog::on_button_add()
{
    printf("add pressed\n");
}


void WhitelistDialog::on_button_edit()
{
    printf("edit pressed\n");
}


void WhitelistDialog::on_button_delete()
{
    Gtk::TreeModel::iterator it;

    printf("delete pressed\n");

    it = getCurrentSelection();
    if (it) {
        Gtk::TreeModel::Row row = *it;
        std::cout << "removing site: " << row[m_listColumns.site] << std::endl;
        m_listModel->erase(it);
    }
}


void WhitelistDialog::on_button_ok()
{
    printf("ok pressed\n");
}


void WhitelistDialog::on_button_cancel()
{
    printf("cancel pressed\n");

    hide();
}


void WhitelistDialog::on_treeview_row_activated(const Gtk::TreeModel::Path & path, Gtk::TreeViewColumn * /* column */)
{
    Gtk::TreeModel::iterator it;

    printf("row doubleclicked\n");

    it = m_listModel->get_iter(path);
    if (it) {
        Gtk::TreeModel::Row row = *it;
        std::cout << "selected site: " << row[m_listColumns.site] << std::endl;
    }
}


void WhitelistDialog::on_treeview_cursor_changed()
{
    Gtk::TreeModel::iterator it;

    printf("row changed\n");

    it = getCurrentSelection();
    if (it) {
        Gtk::TreeModel::Row row = *it;
        std::cout << "selected site: " << row[m_listColumns.site] << std::endl;
    }
}


Gtk::TreeModel::iterator WhitelistDialog::getCurrentSelection()
{
    Gtk::TreeModel::iterator it;

    return m_whitelistView->get_selection()->get_selected();
}
