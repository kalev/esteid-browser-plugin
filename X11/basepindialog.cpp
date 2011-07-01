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

#include "basepindialog.h"
#include <cassert>
#include <sstream>
#include <gtkmm.h>
#include <gtkmmconfig.h>


BasePinDialog::BasePinDialog(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& refGlade)
    : Gtk::Dialog(cobject),
      m_refGlade(refGlade),
      m_label(NULL),
      m_warningLabel(NULL),
      m_expander(NULL),
      m_url(NULL),
      m_hash(NULL)
{
    // Get the Glade-instantiated widgets
    m_refGlade->get_widget("subject_label", m_label);
    m_refGlade->get_widget("warning_label", m_warningLabel);
    m_refGlade->get_widget("details_area", m_expander);
    m_refGlade->get_widget("url_value", m_url);
    m_refGlade->get_widget("hash_value", m_hash);
}

BasePinDialog::~BasePinDialog()
{
}

void BasePinDialog::setSubject(const std::string& subject)
{
    m_label->set_label("<b>" + subject + " (PIN2)</b>");
}

void BasePinDialog::setUrl(const std::string& url)
{
#if GTKMM_MAJOR_VERSION == 2 && GTKMM_MINOR_VERSION < 18
    m_url->set_text(url);
#else
    std::string escaped = Glib::Markup::escape_text(url);
    m_url->set_label("<a href=\"" + escaped + "\">" + escaped + "</a>");
#endif
}

void BasePinDialog::setHash(const std::string& hash)
{
    m_hash->set_text(hash);
}

void BasePinDialog::setRetry(bool retry)
{
    if (retry)
        m_warningLabel->show();
    else
        m_warningLabel->hide();
}

void BasePinDialog::setTries(int tries)
{
    assert(tries > 0);

    static const std::string text = _("<span color=\"red\">Wrong PIN!</span> Tries left: ");
    std::stringstream out;
    out << text << tries;

    m_warningLabel->set_label(out.str());
}

void BasePinDialog::closeDetails()
{
    // close the details area
    m_expander->set_expanded(false);
}
