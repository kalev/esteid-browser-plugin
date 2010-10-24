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
#include <cassert>
#include <sstream>
#include <gtkmm.h>
#include <gtkmm/dialog.h>
#include <gtkmm/stock.h>
#include <gtkmmconfig.h>
#include "debug.h"


PinInputDialog::PinInputDialog(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& refGlade)
    : Gtk::Dialog(cobject),
      m_refGlade(refGlade),
      m_okButton(NULL),
      m_entry(NULL),
      m_label(NULL),
      m_warningLabel(NULL),
      m_url(NULL),
      m_hash(NULL),
      m_minPinLength(5)
{
    // Get the Glade-instantiated widgets
    m_refGlade->get_widget("okButton", m_okButton);
    m_refGlade->get_widget("pin_entry", m_entry);
    m_refGlade->get_widget("subject_label", m_label);
    m_refGlade->get_widget("warning_label", m_warningLabel);
    m_refGlade->get_widget("url_value", m_url);
    m_refGlade->get_widget("hash_value", m_hash);

    m_entry->signal_changed().connect( sigc::mem_fun (*this,
                &PinInputDialog::on_pin_changed) );
}

PinInputDialog::~PinInputDialog()
{
}

void PinInputDialog::setSubject(const std::string& subject)
{
    m_label->set_label("<b>" + subject + " (PIN2)</b>");
}

void PinInputDialog::setUrl(const std::string& url)
{
#if GTKMM_MAJOR_VERSION == 2 && GTKMM_MINOR_VERSION < 18
    m_url->set_text(url);
#else
    std::string escaped = Glib::Markup::escape_text(url);
    m_url->set_label("<a href=\"" + escaped + "\">" + escaped + "</a>");
#endif
}

void PinInputDialog::setHash(const std::string& hash)
{
    m_hash->set_text(hash);
}

void PinInputDialog::setPinPadTimeout(int timeout)
{}

void PinInputDialog::setRetry(bool retry)
{
    if (retry)
        m_warningLabel->show();
    else
        m_warningLabel->hide();
}

void PinInputDialog::setTries(int tries)
{
    assert(tries > 0);

    static const std::string text = _("<span color=\"red\">Wrong PIN!</span> Tries left: ");
    std::stringstream out;
    out << text << tries;

    m_warningLabel->set_label(out.str());
}

void PinInputDialog::setParent(GdkWindow* parent)
{
    GdkWindow* window = get_window()->gobj();
    gdk_window_set_transient_for(window, parent);
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
