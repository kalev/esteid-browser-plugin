/*
 * esteid-browser-plugin - a browser plugin for Estonian EID card
 *
 * Copyright (C) 2010  Estonian Informatics Centre
 * Copyright (C) 2010  Smartlink OÜ
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

#include "pininputdialog.h"
#include <gtkmm.h>


PinInputDialog::PinInputDialog(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& refGlade)
    : BasePinDialog(cobject, refGlade),
      m_okButton(NULL),
      m_entry(NULL),
      m_minPinLength(5)
{
    // Get the Glade-instantiated widgets
    m_refGlade->get_widget("okButton", m_okButton);
    m_refGlade->get_widget("pin_entry", m_entry);

    m_entry->signal_changed().connect( sigc::mem_fun (*this,
                &PinInputDialog::on_pin_changed) );
}

PinInputDialog::~PinInputDialog()
{
}

void PinInputDialog::on_map()
{
    // call the base class
    Gtk::Dialog::on_map();

    // set keyboard focus
    m_entry->grab_focus();
}

std::string PinInputDialog::getPin()
{
    return m_entry->get_text();
}

void PinInputDialog::clearPin()
{
    m_entry->set_text("");
}

void PinInputDialog::on_pin_changed()
{
    // Enable/Disable the OK button appropriately
    m_okButton->set_sensitive(m_entry->get_text_length() >= m_minPinLength);
}
