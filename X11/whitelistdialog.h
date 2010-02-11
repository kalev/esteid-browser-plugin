#ifndef ESTEID_WHITELISTDIALOG_H
#define ESTEID_WHITELISTDIALOG_H

#include <gtkmm.h>


class WhitelistDialog : public Gtk::Dialog
{
public:
    WhitelistDialog(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& refGlade);
    virtual ~WhitelistDialog();

protected:
    class WhitelistModelColumns : public Gtk::TreeModel::ColumnRecord
    {
    public:
        Gtk::TreeModelColumn<Glib::ustring> site;

        WhitelistModelColumns() { add(site); }
    };


    Gtk::TreeView *getTreeView();
    void addSites(const std::vector<std::string> & sv);
    void addSite(const std::string & site);

    // Signal handlers:
    void on_button_cancel();

    Glib::RefPtr<Gtk::Builder> m_refGlade;

    Gtk::Button *m_cancelButton;

    Gtk::TreeView *m_whitelistView;
    WhitelistModelColumns m_listColumns;
    Glib::RefPtr<Gtk::ListStore> m_listModel;
};

#endif //ESTEID_WHITELISTDIALOG_H
