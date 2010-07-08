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

#ifndef ESTEID_WHITELISTDIALOG_H
#define ESTEID_WHITELISTDIALOG_H

#include <gtkmm.h>


class WhitelistDialog : public Gtk::Dialog
{
public:
    WhitelistDialog(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& refGlade);
    virtual ~WhitelistDialog();

    // Functions for setting up data for displaying
    void addSites(const std::vector<std::string> & sv);
    void addDefaultSites(const std::vector<std::string> & sv);
    void addSite(const std::string & site, bool defaultSite = false);
    void addDefaultSite(const std::string & site);
    void setEntryText(const std::string & site);
    void clear();

    // Call this to retrieve modified whitelist
    std::vector<std::string> getWhitelist();

protected:
    class WhitelistModelColumns : public Gtk::TreeModel::ColumnRecord
    {
    public:
        Gtk::TreeModelColumn<std::string> site;
        // sensitive = false: site from default whitelist (read-only)
        // sensitive = true: user set site
        Gtk::TreeModelColumn<bool> sensitive;

        WhitelistModelColumns() { add(site); add(sensitive); }
    };


    // Signal handlers:
    void on_entry_changed();
    void on_button_add();
    void on_button_edit();
    void on_button_delete();
    void on_button_ok();
    void on_button_cancel();
    void on_treeview_row_activated(const Gtk::TreeModel::Path & path, Gtk::TreeViewColumn *column);
    void on_treeview_cursor_changed();
    void on_cellrenderer_edited(const Glib::ustring& path_string, const Glib::ustring& new_text);

    void enableDisableButtons();
    Gtk::TreeView *getTreeView();
    Gtk::TreeModel::iterator getCurrentSelection();
    void startEditing(Gtk::TreePath& path);

    Glib::RefPtr<Gtk::Builder> m_refGlade;

    Gtk::Entry *m_entry;
    Gtk::Button *m_addButton;
    Gtk::Button *m_editButton;
    Gtk::Button *m_deleteButton;
    Gtk::Button *m_okButton;
    Gtk::Button *m_cancelButton;

    Gtk::TreeView *m_whitelistView;
    WhitelistModelColumns m_listColumns;
    Glib::RefPtr<Gtk::ListStore> m_listModel;
};

#endif //ESTEID_WHITELISTDIALOG_H
