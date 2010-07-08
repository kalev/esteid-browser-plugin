/*
 * esteid-browser-plugin - a browser plugin for Estonian EID card
 *
 * Copyright (C) 2010  Estonian Informatics Centre
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "whitelistdialog.h"
#include "debug.h"

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

    m_addButton->set_sensitive(false);
    m_editButton->set_sensitive(false);
    m_deleteButton->set_sensitive(false);

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
    m_listModel->set_sort_column(0, Gtk::SORT_ASCENDING);
    treeview->set_model(m_listModel);

    // Set up custom renderer to show some sites as not sensitive
    renderer = new Gtk::CellRendererText();
    treeview->append_column("Sites", *renderer);
    col0 = treeview->get_column(0);
    col0->add_attribute(*renderer, "text", 0);
    col0->add_attribute(*renderer, "sensitive", 1);
    col0->add_attribute(*renderer, "editable", true);

    renderer->signal_edited().connect( sigc::mem_fun(*this,
                &WhitelistDialog::on_cellrenderer_edited) );

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
    ESTEID_DEBUG("add pressed\n");

    addSite(m_entry->get_text());
    m_entry->set_text("");
}


void WhitelistDialog::on_button_edit()
{
    Gtk::TreeModel::iterator it;
    Gtk::TreePath path;

    ESTEID_DEBUG("edit pressed\n");

    it = getCurrentSelection();
    if (it) {
        path = m_listModel->get_path(it);
        startEditing(path);
    }
}


void WhitelistDialog::on_button_delete()
{
    Gtk::TreeModel::iterator it;

    ESTEID_DEBUG("delete pressed\n");

    it = getCurrentSelection();
    if (it) {
        m_listModel->erase(it);
        enableDisableButtons();
    }
}


void WhitelistDialog::on_button_ok()
{
    ESTEID_DEBUG("ok pressed\n");

    response(Gtk::RESPONSE_OK);
}


void WhitelistDialog::on_button_cancel()
{
    ESTEID_DEBUG("cancel pressed\n");

    response(Gtk::RESPONSE_CANCEL);
}


void WhitelistDialog::on_treeview_row_activated(const Gtk::TreeModel::Path & path, Gtk::TreeViewColumn * /* column */)
{
    ESTEID_DEBUG("row doubleclicked\n");

    /* FIXME: Not needed unless we are going to open
              a new dialog window in here. */
}


void WhitelistDialog::on_treeview_cursor_changed()
{
    ESTEID_DEBUG("row clicked\n");

    enableDisableButtons();
}


void WhitelistDialog::on_cellrenderer_edited(const Glib::ustring& path_string, const Glib::ustring& new_site)
{
    Gtk::TreeModel::iterator it;
    Gtk::TreePath path(path_string);

    ESTEID_DEBUG("finished editing\n");

    // Update the model with new value
    it = m_listModel->get_iter(path);
    if (it) {
        Gtk::TreeModel::Row row = *it;
        row[m_listColumns.site] = new_site;
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


void WhitelistDialog::startEditing(Gtk::TreePath& path)
{
    Gtk::TreeViewColumn *col0 = m_whitelistView->get_column(0);
    bool start_editing = true;

    m_whitelistView->set_cursor(path, *col0, start_editing);
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
