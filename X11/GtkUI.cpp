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

#include "GtkUI.h"

#include <stdio.h>
#include <iostream>

#include "X11/pininputdialog.h"
#include "X11/PluginWindowX11.h"
#include "X11/whitelistdialog.h"

#include "debug.h"


GtkUI::GtkUI(boost::shared_ptr<UICallbacks> cb)
    : PluginUI(cb),
      m_dialog_up(false)
{
    Gtk::Main::init_gtkmm_internals();

    Glib::RefPtr<Gtk::Builder> refGlade = Gtk::Builder::create();
    // Load the GtkBuilder file
    try {
        refGlade->add_from_file(WHITELISTDIALOG_UI);
    } catch(const Glib::Error& ex) {
        std::cerr << ex.what() << std::endl;
    }
    refGlade->get_widget_derived("WhitelistDialog", m_whitelistDialog);

    Glib::RefPtr<Gtk::Builder> refGlade2 = Gtk::Builder::create();
    // Load the GtkBuilder file
    try {
        refGlade2->add_from_file(PININPUTDIALOG_UI);
    } catch(const Glib::Error& ex) {
        std::cerr << ex.what() << std::endl;
    }
    refGlade2->get_widget_derived("PinInputDialog", m_pinInputDialog);

    // connect signals
    if (m_pinInputDialog) {
        m_pinInputConnection = m_pinInputDialog->signal_response().connect( sigc::mem_fun(*this,
                    &GtkUI::on_pininputdialog_response) );
    }
    if (m_whitelistDialog) {
        m_whitelistDialog->signal_response().connect( sigc::mem_fun(*this,
                    &GtkUI::on_whitelistdialog_response) );
    }
}


GtkUI::~GtkUI()
{
    delete m_whitelistDialog;
    delete m_pinInputDialog;
}


GdkWindow* GtkUI::browserWindow()
{
    if (m_window) {
        FB::PluginWindowX11* wnd = dynamic_cast<FB::PluginWindowX11*>(m_window);
        return gdk_window_foreign_new(wnd->getBrowserWindow());
    }

    return NULL;
}


void GtkUI::PromptForPinAsync(const std::string& subject,
        const std::string& docUrl, const std::string& docHash,
        bool retry, int tries)
{
    if (!m_pinInputDialog)
        throw std::runtime_error("PinInputDialog not loaded");

    if (m_dialog_up) {
        // Bring the window to the front
        m_pinInputDialog->present();
        return;
    }

    m_pinInputDialog->setSubject(subject);
    m_pinInputDialog->setUrl(docUrl);
    m_pinInputDialog->setHash(docHash);
    m_pinInputDialog->setRetry(retry);
    m_pinInputDialog->setTries(tries);

    m_pinInputDialog->setParent(browserWindow());

    m_pinInputDialog->show();
    m_dialog_up = true;
}

void GtkUI::ClosePinPrompt()
{
}


void GtkUI::ShowPinBlockedMessage(int pin)
{
    if (m_dialog_up)
        return;

    Gtk::MessageDialog dialog(_("PIN2 blocked"), false, Gtk::MESSAGE_WARNING);
    dialog.set_secondary_text(_("Please run ID card Utility to unlock the PIN."));
    dialog.signal_show().connect(sigc::bind(sigc::mem_fun(*this,
                &GtkUI::make_transient), &dialog));

    m_dialog_up = true;
    dialog.run();
    m_dialog_up = false;
}


void GtkUI::ShowSettings(PluginSettings& settings, const std::string& pageUrl)
{
    m_settings = &settings;

    if (!m_whitelistDialog)
        throw std::runtime_error("WhitelistDialog not loaded");

    if (m_dialog_up) {
        // Bring the window to the front
        m_whitelistDialog->present();
        return;
    }

    if (pageUrl.length() > 0)
        m_whitelistDialog->setEntryText(pageUrl);

    m_whitelistDialog->clear();
    m_whitelistDialog->addDefaultSites(m_settings->defaultWhitelist());
    m_whitelistDialog->addSites(m_settings->whitelist());

    m_whitelistDialog->setParent(browserWindow());

    m_whitelistDialog->show_all();
    m_dialog_up = true;
}


void GtkUI::iteration()
{
    Glib::MainContext::get_default()->iteration(true);
}


void GtkUI::on_pininputdialog_response(int response_id)
{
    std::string pin;

    m_pinInputDialog->hide();
    m_dialog_up = false;

    if (response_id == Gtk::RESPONSE_OK) {
        pin = m_pinInputDialog->getPin();
        m_callbacks->onPinEntered(pin);
    } else {
        m_callbacks->onPinCancelled();
    }

    // make sure the dialog doesn't cache PIN
    m_pinInputDialog->clearPin();
}


void GtkUI::on_whitelistdialog_response(int response_id)
{
    if (response_id == Gtk::RESPONSE_OK) {
        m_settings->setWhitelist(m_whitelistDialog->getWhitelist());
        try {
            m_settings->save();
        } catch(const std::exception& e) {
            Gtk::MessageDialog dialog(_("Error saving settings"), false, Gtk::MESSAGE_ERROR);
            dialog.set_secondary_text(e.what());
            dialog.run();
        }
    }

    m_whitelistDialog->hide();
    m_dialog_up = false;
}

void GtkUI::make_transient(Gtk::Window *window)
{
    GdkWindow *parent = browserWindow();
    if (parent) {
        GdkWindow *gdk_window = window->get_window()->gobj();
        gdk_window_set_transient_for(gdk_window, parent);
    }
}
