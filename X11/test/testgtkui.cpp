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

#include "testgtkui.h"
#include "X11/pininputdialog.h"
#include "X11/whitelistdialog.h"
#include <gtkmm/messagedialog.h>
#include <iostream>


TestGtkUI::TestGtkUI()
    : m_pinBlockedButton("Show PIN blocked dialog"),
      m_pinInputButton("Show PIN input dialog"),
      m_whitelistButton("Show whitelist dialog")
{
    set_title("Plugin UI test");

    add(m_ButtonBox);

    m_ButtonBox.pack_start(m_pinBlockedButton);
    m_pinBlockedButton.signal_clicked().connect(sigc::mem_fun(*this,
            &TestGtkUI::on_button_pinBlocked_clicked));

    m_ButtonBox.pack_start(m_pinInputButton);
    m_pinInputButton.signal_clicked().connect(sigc::mem_fun(*this,
            &TestGtkUI::on_button_pinInput_clicked) );

    m_ButtonBox.pack_start(m_whitelistButton);
    m_whitelistButton.signal_clicked().connect(sigc::mem_fun(*this,
            &TestGtkUI::on_button_whitelist_clicked) );

    if (loadGladeUI(GLADE_FILE) != 0) {
        printf("loadGladeUI() failed\n");
        exit(1);
    }

    m_refGlade->get_widget_derived("WhitelistDialog", m_whitelistDialog);

    // Insert dummy sites
    std::vector<std::string> sv;
    sv.push_back("id.swedbank.ee");
    sv.push_back("id.seb.ee");
    sv.push_back("id.eesti.ee");
    m_whitelistDialog->addDefaultSites(sv);

    m_whitelistDialog->addSite("not.in.default.whitelist.org");
    m_whitelistDialog->addSite("user.added.ee");

    show_all_children();
}


TestGtkUI::~TestGtkUI()
{
}


int TestGtkUI::loadGladeUI(std::string gladeFile)
{
    //Load the GtkBuilder file and instantiate its widgets:
    m_refGlade = Gtk::Builder::create();

    try {
        m_refGlade->add_from_file(gladeFile);
    } catch(const Glib::FileError& ex) {
        std::cerr << "FileError: " << ex.what() << std::endl;
        return 1;
    } catch(const Gtk::BuilderError& ex) {
        std::cerr << "BuilderError: " << ex.what() << std::endl;
        return 1;
    }

    return 0;
}


void TestGtkUI::on_button_pinBlocked_clicked()
{
#if 0
    Gtk::MessageDialog *dialog = new Gtk::MessageDialog(*this, "PIN2 blocked", false, Gtk::MESSAGE_WARNING);
    dialog->set_secondary_text("Please run ID-card Utility to unlock the PIN.");

    dialog->run();
#else
    Gtk::MessageDialog dialog(*this, "PIN2 blocked", false, Gtk::MESSAGE_WARNING);
    dialog.set_secondary_text("Please run ID-card Utility to unlock the PIN.");

    dialog.run();
#endif
}


void TestGtkUI::on_button_pinInput_clicked()
{
    PinInputDialog dialog(PIN1, "Mari-Liis Männik");

    int result = dialog.run();

    //Handle the response:
    switch (result) {
    case Gtk::RESPONSE_OK:
        std::cout << "X: OK clicked." << std::endl;
        std::cout << "X: PIN is " << dialog.getPin() << std::endl;
        break;
    case Gtk::RESPONSE_CANCEL:
        std::cout << "X: Cancel clicked." << std::endl;
        std::cout << "X: PIN is " << dialog.getPin() << std::endl;
        break;
    case Gtk::RESPONSE_CLOSE:
        std::cout << "X: Close clicked." << std::endl;
        std::cout << "X: PIN is " << dialog.getPin() << std::endl;
        break;
    case Gtk::RESPONSE_DELETE_EVENT:
        std::cout << "X: DELETE_EVENT" << std::endl;
        std::cout << "X: PIN is " << dialog.getPin() << std::endl;
        break;
    default:
        std::cout << "X: Unexpected button clicked." << std::endl;
        std::cout << "X: PIN is " << dialog.getPin() << std::endl;
        break;
    }
}


void TestGtkUI::on_button_whitelist_clicked()
{
    if (m_whitelistDialog->get_modal())
        printf("whitelist is modal\n");

    //m_whitelistDialog->set_transient_for(*this);
    int result = m_whitelistDialog->run();

    switch (result) {
    case Gtk::RESPONSE_OK:
        std::cout << "X: OK clicked." << std::endl;
        break;
    case Gtk::RESPONSE_CANCEL:
        std::cout << "X: Cancel clicked." << std::endl;
        break;
    case Gtk::RESPONSE_CLOSE:
        std::cout << "X: Close clicked." << std::endl;
        break;
    case Gtk::RESPONSE_DELETE_EVENT:
        std::cout << "X: DELETE_EVENT" << std::endl;
        break;
    default:
        std::cout << "X: Unexpected button clicked." << std::endl;
        break;
    }

    m_whitelistDialog->hide();
}
