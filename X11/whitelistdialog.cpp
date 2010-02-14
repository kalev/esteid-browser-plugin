#include "whitelistdialog.h"

WhitelistDialog::WhitelistDialog(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& refGlade)
    : Gtk::Dialog(cobject),
      m_refGlade(refGlade),
      m_entry(NULL),
      m_addButton(NULL),
      m_editButton(NULL),
      m_deleteButton(NULL),
      m_okButton(NULL),
      m_cancelButton(NULL)
{
    // Get the Glade-instantiated widgets
    m_refGlade->get_widget("entry", m_entry);
    m_refGlade->get_widget("addButton", m_addButton);
    m_refGlade->get_widget("editButton", m_editButton);
    m_refGlade->get_widget("deleteButton", m_deleteButton);
    m_refGlade->get_widget("okButton", m_okButton);
    m_refGlade->get_widget("cancelButton", m_cancelButton);

    // Connect buttons to their signal handlers
    if (m_entry) {
        m_entry->signal_changed().connect( sigc::mem_fun(*this,
                    &WhitelistDialog::on_entry_changed) );
    }

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

    // Set the Add button as default widget so that user could
    // just press enter in text entry box to activate this button.
    m_addButton->set_flags(Gtk::CAN_DEFAULT);
    m_addButton->grab_default();

    m_addButton->set_sensitive(false);
    m_editButton->set_sensitive(false);
    m_deleteButton->set_sensitive(false);

    // Set up treeview
    m_whitelistView = getTreeView();
    m_listModel->clear();

    m_whitelistView->signal_row_activated().connect( sigc::mem_fun(*this,
                &WhitelistDialog::on_treeview_row_activated) );

    m_whitelistView->signal_cursor_changed().connect( sigc::mem_fun(*this,
                &WhitelistDialog::enableDisableButtons) );
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


void WhitelistDialog::setEntryText(const std::string & site)
{
    m_entry->set_text(site);
}


void WhitelistDialog::on_entry_changed()
{
    m_addButton->set_sensitive(m_entry->get_text_length() > 0);
}


void WhitelistDialog::on_button_add()
{
    printf("add pressed\n");

    addSite(m_entry->get_text());
    m_entry->set_text("");
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
        m_listModel->erase(it);
        enableDisableButtons();
    }
}


void WhitelistDialog::on_button_ok()
{
    printf("ok pressed\n");

    response(Gtk::RESPONSE_OK);
}


void WhitelistDialog::on_button_cancel()
{
    printf("cancel pressed\n");

    hide();
}


void WhitelistDialog::on_treeview_row_activated(const Gtk::TreeModel::Path & path, Gtk::TreeViewColumn * /* column */)
{
    /* FIXME: double click should open Edit window */
    Gtk::TreeModel::iterator it;

    printf("row doubleclicked\n");

    it = m_listModel->get_iter(path);
    if (it) {
        Gtk::TreeModel::Row row = *it;
    }
}


// Enable or disable buttons depending on what is currently selected.
void WhitelistDialog::enableDisableButtons()
{
    Gtk::TreeModel::iterator it;

    it = getCurrentSelection();
    if (it && (*it)[m_listColumns.sensitive]) {
        // Sensitive text is selected
        m_editButton->set_sensitive(true);
        m_deleteButton->set_sensitive(true);
    } else {
        // Either nothing or non-sensitive text is selected
        m_editButton->set_sensitive(false);
        m_deleteButton->set_sensitive(false);
    }
}


Gtk::TreeModel::iterator WhitelistDialog::getCurrentSelection()
{
    return m_whitelistView->get_selection()->get_selected();
}


std::vector<std::string> WhitelistDialog::getWhitelist()
{
    std::vector<std::string> ret;
    Gtk::TreeModel::iterator it;

    for (it = m_listModel->children().begin(); it != m_listModel->children().end(); ++it) {
        Gtk::TreeModel::Row row = *it;

        // Return sites that are user set and skip default (read-only) values.
        if (row[m_listColumns.sensitive]) {
            ret.push_back(row[m_listColumns.site]);
        }
    }

    return ret;
}
