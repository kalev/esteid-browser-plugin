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
