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

#ifndef ESTEID_BASEPINDIALOG_H
#define ESTEID_BASEPINDIALOG_H

#include <gtkmm.h>
#include "localize.h"

class BasePinDialog : public Gtk::Dialog
{
public:
    BasePinDialog(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& refGlade);
    virtual ~BasePinDialog();

    virtual void setSubject(const std::string& subject);
    virtual void setUrl(const std::string& url);
    virtual void setHash(const std::string& hash);
    virtual void setRetry(bool retry);
    virtual void setTries(int tries);
    virtual void closeDetails();

protected:
    Glib::RefPtr<Gtk::Builder> m_refGlade;

    // Child widgets:
    Gtk::Label *m_label;
    Gtk::Label *m_warningLabel;
    Gtk::Expander *m_expander;
    Gtk::Label *m_url;
    Gtk::Label *m_hash;
};

#endif //ESTEID_BASEPINDIALOG_H
