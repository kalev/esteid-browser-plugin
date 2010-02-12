#ifndef ESTEID_WHITELISTDIALOG_H
#define ESTEID_WHITELISTDIALOG_H

#include <gtkmm.h>


class WhitelistDialog : public Gtk::Dialog
{
public:
    WhitelistDialog(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& refGlade);
    virtual ~WhitelistDialog();

    void addSites(const std::vector<std::string> & sv);
    void addDefaultSites(const std::vector<std::string> & sv);
    void addSite(const std::string & site, bool defaultSite = false);
    void addDefaultSite(const std::string & site);

protected:
    class WhitelistModelColumns : public Gtk::TreeModel::ColumnRecord
    {
    public:
        Gtk::TreeModelColumn<Glib::ustring> site;
        // Default sites are shown Gray to indicate that they cannot be edited
        Gtk::TreeModelColumn<bool> sensitive;

        WhitelistModelColumns() { add(site); add(sensitive); }
    };


    Gtk::TreeView *getTreeView();

    // Signal handlers:
    void on_button_cancel();

    Glib::RefPtr<Gtk::Builder> m_refGlade;

    Gtk::Button *m_cancelButton;

    Gtk::TreeView *m_whitelistView;
    WhitelistModelColumns m_listColumns;
    Glib::RefPtr<Gtk::ListStore> m_listModel;
};

#endif //ESTEID_WHITELISTDIALOG_H
