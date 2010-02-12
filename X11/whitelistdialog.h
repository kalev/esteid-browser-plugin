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


    Gtk::TreeView *getTreeView();
    Gtk::TreeModel::iterator getCurrentSelection();

    // Signal handlers:
    void on_button_add();
    void on_button_edit();
    void on_button_delete();
    void on_button_ok();
    void on_button_cancel();
    void on_treeview_row_activated(const Gtk::TreeModel::Path & path, Gtk::TreeViewColumn *column);
    void enableDisableButtons();

    Glib::RefPtr<Gtk::Builder> m_refGlade;

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
