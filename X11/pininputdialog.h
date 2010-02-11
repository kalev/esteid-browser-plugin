#ifndef ESTEID_PININPUTDIALOG_H
#define ESTEID_PININPUTDIALOG_H

#include <gtkmm.h>

enum PinType { PIN1, PIN2 };

class PinInputDialog : public Gtk::Dialog
{
public:
    PinInputDialog(PinType pinType = PIN2, const std::string& name = "ID-kaart");
    virtual ~PinInputDialog();
    void setMinPinLength(int len);
    void setText(std::string text);
    std::string getPin();

protected:
    // Signal handlers:
    void on_button_more_info_clicked();
    void on_button_cancel_clicked();
    void on_button_ok_clicked();
    void on_pin_changed();

    // Child widgets:
    Gtk::HButtonBox m_buttonBox;
    Gtk::Alignment m_leftAlign;
    Gtk::Alignment m_rightAlign;
    Gtk::Button m_moreInfoButton;
    Gtk::Button m_cancelButton;
    Gtk::Button m_okButton;
    Gtk::Entry m_entry;
    Gtk::Label m_label;

    int m_minPinLength;
};

#endif //ESTEID_PININPUTDIALOG_H
