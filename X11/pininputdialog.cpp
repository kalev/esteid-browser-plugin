#include "pininputdialog.h"
#include <gtkmm.h>
#include <gtkmm/dialog.h>
#include <gtkmm/stock.h>
#include <iostream>


PinInputDialog::PinInputDialog(PinType pinType, const std::string& name)
    : m_leftAlign(Gtk::ALIGN_LEFT, Gtk::ALIGN_CENTER, 0.0, 0.0),
      m_moreInfoButton("_More Info", true),
      m_cancelButton(Gtk::Stock::CANCEL),
      m_okButton(Gtk::Stock::OK),
      m_minPinLength(5)
{

    set_title("Enter PIN");

    if (pinType == PIN1) {
        setText(name + " (PIN1)");
        setMinPinLength(4);
    } else {
        setText(name + " (PIN2)");
        setMinPinLength(5);
    }

    get_vbox()->pack_start(m_label);
    m_label.set_use_markup();

    get_vbox()->pack_start(m_entry);
    m_entry.set_max_length(500);
    m_entry.set_visibility(false);
    m_entry.set_activates_default();
    m_entry.signal_changed().connect( sigc::mem_fun (*this,
                &PinInputDialog::on_pin_changed) );

    get_action_area()->pack_start(m_leftAlign);
    m_leftAlign.add(m_moreInfoButton);
    m_moreInfoButton.signal_clicked().connect( sigc::mem_fun(*this,
                &PinInputDialog::on_button_more_info_clicked) );

    get_action_area()->pack_start(m_cancelButton);
    m_cancelButton.signal_clicked().connect( sigc::mem_fun(*this,
                &PinInputDialog::on_button_cancel_clicked) );

    get_action_area()->pack_start(m_okButton);
    m_okButton.set_flags(Gtk::CAN_DEFAULT);
    m_okButton.grab_default();
    m_okButton.set_sensitive(false);
    m_okButton.signal_clicked().connect( sigc::mem_fun(*this,
                &PinInputDialog::on_button_ok_clicked) );

    show_all_children();
}

PinInputDialog::~PinInputDialog()
{
}

void PinInputDialog::setText(std::string text)
{
    std::string tmp;
    tmp = "<big>";
    tmp += text;
    tmp += "</big>";
    m_label.set_text(tmp);
}

std::string PinInputDialog::getPin()
{
    return m_entry.get_text();
}

void PinInputDialog::setMinPinLength(int len)
{
    m_minPinLength = len;
}

void PinInputDialog::on_button_more_info_clicked()
{
    std::cout << "More Info clicked." << std::endl;
}

void PinInputDialog::on_button_cancel_clicked()
{
    std::cout << "Cancel clicked." << std::endl;
    response(Gtk::RESPONSE_CANCEL);
}

void PinInputDialog::on_button_ok_clicked()
{
    std::cout << "OK clicked." << std::endl;
    response(Gtk::RESPONSE_OK);
}

void PinInputDialog::on_pin_changed()
{
    // Enable/Disable the OK button appropriately
    m_okButton.set_sensitive(m_entry.get_text_length() >= m_minPinLength);
}
