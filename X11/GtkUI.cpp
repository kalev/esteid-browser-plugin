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
#include "X11/pinpaddialog.h"
#include "X11/PluginWindowX11.h"
#include "X11/whitelistdialog.h"

#include "debug.h"


GtkUI::GtkUI(boost::shared_ptr<UICallbacks> cb)
    : PluginUI(cb)
{
    Gtk::Main::init_gtkmm_internals();

    // Load the GtkBuilder files
    try {
        Glib::RefPtr<Gtk::Builder> builder;

        builder = Gtk::Builder::create_from_file(WHITELISTDIALOG_UI);
        builder->get_widget_derived("WhitelistDialog", m_whitelistDialog);

        builder = Gtk::Builder::create_from_file(PININPUTDIALOG_UI);
        builder->get_widget_derived("PinInputDialog", m_pinInputDialog);

        builder = Gtk::Builder::create_from_file(PINPADDIALOG_UI);
        builder->get_widget_derived("PinpadDialog", m_pinpadDialog);
    } catch(const Glib::Error& ex) {
        std::cerr << ex.what() << std::endl;
    }

    // connect signals
    if (m_pinInputDialog) {
        m_pinInputDialog->signal_show().connect(sigc::bind(sigc::mem_fun(*this,
                    &GtkUI::make_transient), m_pinInputDialog));
        m_pinInputConnection = m_pinInputDialog->signal_response().connect( sigc::mem_fun(*this,
                    &GtkUI::on_pininputdialog_response) );
    }
    if (m_pinpadDialog) {
        m_pinpadDialog->signal_show().connect(sigc::bind(sigc::mem_fun(*this,
                    &GtkUI::make_transient), m_pinpadDialog));
    }
    if (m_whitelistDialog) {
        m_whitelistDialog->signal_show().connect(sigc::bind(sigc::mem_fun(*this,
                    &GtkUI::make_transient), m_whitelistDialog));
        m_whitelistDialog->signal_response().connect( sigc::mem_fun(*this,
                    &GtkUI::on_whitelistdialog_response) );
    }
}


GtkUI::~GtkUI()
{
    delete m_whitelistDialog;
    delete m_pinInputDialog;
    delete m_pinpadDialog;
}


GdkWindow* GtkUI::browserWindow()
{
    if (m_window) {
        FB::PluginWindowX11* wnd = dynamic_cast<FB::PluginWindowX11*>(m_window);
        return gdk_window_foreign_new(wnd->getBrowserWindow());
    }

    return NULL;
}


bool GtkUI::raiseVisiblePinDialog()
{
    if (m_pinInputDialog && m_pinInputDialog->get_visible()) {
        m_pinInputDialog->present();
        return true;
    } else if (m_pinpadDialog && m_pinpadDialog->get_visible()) {
        m_pinpadDialog->present();
        return true;
    }

    return false;
}


void GtkUI::pinDialog(const std::string& subject,
                      const std::string& docUrl,
                      const std::string& docHash)
{
    if (!m_pinInputDialog)
        throw std::runtime_error("PinInputDialog not loaded");

    if (raiseVisiblePinDialog())
        return;

    m_pinInputDialog->setSubject(subject);
    m_pinInputDialog->setUrl(docUrl);
    m_pinInputDialog->setHash(docHash);
    m_pinInputDialog->setRetry(false);
    m_pinInputDialog->closeDetails();

    m_pinInputDialog->show();
}

void GtkUI::pinpadDialog(const std::string& subject,
                         const std::string& docUrl,
                         const std::string& docHash,
                         int timeout)
{
    if (!m_pinpadDialog)
        throw std::runtime_error("PinpadDialog not loaded");

    if (raiseVisiblePinDialog())
        return;

    m_pinpadDialog->setSubject(subject);
    m_pinpadDialog->setUrl(docUrl);
    m_pinpadDialog->setHash(docHash);
    m_pinpadDialog->setTimeout(timeout);
    m_pinpadDialog->setRetry(false);
    m_pinpadDialog->closeDetails();

    m_pinpadDialog->show();
}


void GtkUI::retryPinDialog(int triesLeft)
{
    m_pinInputDialog->setTries(triesLeft);
    m_pinInputDialog->setRetry(true);
}


void GtkUI::retryPinpadDialog(int triesLeft)
{
    m_pinpadDialog->setTries(triesLeft);
    m_pinpadDialog->setRetry(true);

    m_pinpadDialog->resetProgressbar();
}


void GtkUI::closePinDialog()
{
    m_pinInputDialog->hide();
}


void GtkUI::closePinpadDialog()
{
    m_pinpadDialog->hide();
}


void GtkUI::pinBlockedMessage(int pin)
{
    closePinDialog();
    closePinpadDialog();

    Gtk::MessageDialog dialog(_("PIN2 blocked"), false, Gtk::MESSAGE_WARNING);
    dialog.set_secondary_text(_("Please run ID card Utility to unlock the PIN."));
    dialog.signal_show().connect(sigc::bind(sigc::mem_fun(*this,
                &GtkUI::make_transient), &dialog));

    dialog.run();
}


void GtkUI::settingsDialog(PluginSettings& settings, const std::string& pageUrl)
{
    m_settings = &settings;

    if (!m_whitelistDialog)
        throw std::runtime_error("WhitelistDialog not loaded");

    if (m_whitelistDialog->get_visible()) {
        m_whitelistDialog->present();
        return;
    }

    if (pageUrl.length() > 0)
        m_whitelistDialog->setEntryText(pageUrl);

    m_whitelistDialog->clear();
    m_whitelistDialog->addDefaultSites(m_settings->defaultWhitelist());
    m_whitelistDialog->addSites(m_settings->whitelist());

    m_whitelistDialog->show_all();
}


void GtkUI::iteration()
{
    Glib::MainContext::get_default()->iteration(true);
}


void GtkUI::on_pininputdialog_response(int response_id)
{
    std::string pin;

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
}

void GtkUI::make_transient(Gtk::Window *window)
{
    GdkWindow *parent = browserWindow();
    if (parent) {
        GdkWindow *gdk_window = window->get_window()->gobj();
        gdk_window_set_transient_for(gdk_window, parent);
    }
}
