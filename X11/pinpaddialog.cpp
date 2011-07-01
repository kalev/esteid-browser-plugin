/*
 * esteid-browser-plugin - a browser plugin for Estonian EID card
 *
 * Copyright (C) 2010  Estonian Informatics Centre
 * Copyright (C) 2010, 2011  Smartlink OÜ
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

#include "pinpaddialog.h"
#include <sstream>
#include <gtkmm.h>


PinpadDialog::PinpadDialog(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& refGlade)
    : BasePinDialog(cobject, refGlade),
      m_timeTotal(0),
      m_timeRemaining(0),
      m_progressbar(NULL)
{
    // Get the Glade-instantiated widgets
    m_refGlade->get_widget("progressbar", m_progressbar);

    // Hide the action area for now; it's currently not possible to cancel a
    // (pinpad) signing operation.
    get_action_area()->hide();
}

PinpadDialog::~PinpadDialog()
{
}

void PinpadDialog::setTimeout(int timeout)
{
    m_timeTotal = timeout;
}

void PinpadDialog::resetProgressbar()
{
    // reset the progressbar
    m_progressbar->set_fraction(1);
    m_progressbar->set_text(format_seconds(m_timeTotal));
    m_timeRemaining = m_timeTotal;

    // stop the old timer
    if (m_timer.connected())
        m_timer.disconnect();

    // start a new timer
    sigc::slot<bool> tslot = sigc::mem_fun(*this, &PinpadDialog::on_timer);
    m_timer = Glib::signal_timeout().connect(tslot, 1000);
}

std::string PinpadDialog::format_seconds(int seconds)
{
    std::ostringstream os;
    os << seconds << _("s");
    return os.str();
}

bool PinpadDialog::on_timer()
{
    m_timeRemaining--;
    m_progressbar->set_fraction(m_timeRemaining / static_cast<double>(m_timeTotal));
    m_progressbar->set_text(format_seconds(m_timeRemaining));

    return (m_timeRemaining > 0);
}

void PinpadDialog::on_map()
{
    resetProgressbar();

    // call the base class
    Gtk::Dialog::on_map();
}

void PinpadDialog::on_unmap()
{
    // stop the timer
    m_timer.disconnect();

    // call the base class
    Gtk::Dialog::on_unmap();
}
