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

#ifndef ESTEID_PINPADDIALOG_H
#define ESTEID_PINPADDIALOG_H

#include "basepindialog.h"

class PinpadDialog : public BasePinDialog
{
public:
    PinpadDialog(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& refGlade);
    virtual ~PinpadDialog();

    void setTimeout(int timeout);
    void resetProgressbar();

protected:
    // reimplemented virtual functions
    virtual void on_map();
    virtual void on_unmap();

private:
    // Signal handlers:
    bool on_timer();

    static std::string format_seconds(int seconds);

    sigc::connection m_timer;
    int m_timeTotal;
    int m_timeRemaining;

    // Child widgets:
    Gtk::ProgressBar *m_progressbar;
};

#endif //ESTEID_PINPADDIALOG_H
