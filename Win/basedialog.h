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

#ifndef ESTEID_BASEDIALOG_H
#define ESTEID_BASEDIALOG_H

#include <windows.h>
#include <boost/signals.hpp>

class BaseDialog
{
public:
    BaseDialog(HINSTANCE hInst);
    virtual ~BaseDialog();

    enum ResponseType
    {
        RESPONSE_OK = -5,
        RESPONSE_CANCEL = -6,
    };

    typedef boost::signal<void (int)> ResponseSignal;
    typedef boost::signals::connection Connection;

    Connection connect(const ResponseSignal::slot_type& subscriber);
    void disconnect(Connection subscriber);
    virtual bool doDialog(int resourceID, HWND hParent = NULL);
    virtual int doModalDialog(int resourceID, HWND hParent = NULL);

protected:
    HINSTANCE m_hInst;
    HWND m_hWnd;
    HANDLE m_hModalDialogLock;
    bool m_modalDialog;

    static LRESULT CALLBACK dialogProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    virtual LRESULT on_message(UINT message, WPARAM wParam, LPARAM lParam);
    virtual LRESULT on_initdialog(WPARAM wParam) = 0;
    virtual LRESULT on_command(WPARAM wParam, LPARAM lParam) = 0;
    virtual LRESULT on_notify(WPARAM wParam, LPARAM lParam);
    HWND getIEModalLock(HWND hWnd);
    void releaseIEModalLock();
    ResponseSignal signalResponse;
};

#endif //ESTEID_BASEDIALOG_H
