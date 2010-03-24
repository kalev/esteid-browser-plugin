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

#ifndef ESTEID_TESTGTKUI
#define ESTEID_TESTGTKUI

#include <gtkmm.h>

#include "whitelistdialog.h"

class TestGtkUI : public Gtk::Window
{
public:
    TestGtkUI();
    virtual ~TestGtkUI();

protected:
    int loadGladeUI(std::string gladeFile);

    // Signal handlers:
    void on_button_pinBlocked_clicked();
    void on_button_pinInput_clicked();
    void on_button_whitelist_clicked();

    // Child widgets:
    Gtk::VButtonBox m_ButtonBox;
    Gtk::Button m_pinBlockedButton;
    Gtk::Button m_pinInputButton;
    Gtk::Button m_whitelistButton;

    WhitelistDialog *m_whitelistDialog;

    // Glade interface description.
    Glib::RefPtr<Gtk::Builder> m_refGlade;
};

#endif //ESTEID_TESTGTKUI
