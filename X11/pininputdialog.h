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

#ifndef ESTEID_PININPUTDIALOG_H
#define ESTEID_PININPUTDIALOG_H

#include <gtkmm.h>

class PinInputDialog : public Gtk::Dialog
{
public:
    PinInputDialog(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& refGlade);
    virtual ~PinInputDialog();

    void setSubject(const std::string& subject);
    void setUrl(const std::string& url);
    void setHash(const std::string& hash);
    void setPinPadTimeout(int timeout);
    void setRetry(bool retry);
    void setTries(int tries);
    void setParent(GdkWindow* parent);
    std::string getPin();
    void clearPin();

protected:
    // Signal handlers:
    void on_pin_changed();

    Glib::RefPtr<Gtk::Builder> m_refGlade;

    // Child widgets:
    Gtk::Button *m_okButton;
    Gtk::Entry *m_entry;
    Gtk::Label *m_label;
    Gtk::Label *m_warningLabel;
    Gtk::Label *m_url;
    Gtk::Label *m_hash;

    int m_minPinLength;
};

#endif //ESTEID_PININPUTDIALOG_H
