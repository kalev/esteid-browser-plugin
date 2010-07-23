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

#include "basedialog.h"
#include <windows.h>

typedef BaseDialog::Connection Connection;

BaseDialog::BaseDialog(HINSTANCE hInst)
    : m_hInst(hInst),
      m_hWnd(NULL),
      m_modalDialog(false)
{
}

BaseDialog::~BaseDialog()
{
}

Connection BaseDialog::connect(const ResponseSignal::slot_type& subscriber)
{
    return signalResponse.connect(subscriber);
}

void BaseDialog::disconnect(Connection subscriber)
{
    subscriber.disconnect();
}

/* static */ LRESULT CALLBACK BaseDialog::dialogProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    BaseDialog *dlg = reinterpret_cast<BaseDialog*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
    if (!dlg) {
        if (message == WM_INITDIALOG) {
            dlg = reinterpret_cast<BaseDialog*>(lParam);
            dlg->m_hWnd = hWnd;
            SetWindowLongPtr(hWnd, GWLP_USERDATA, lParam);
        } else {
            return 0; // let system deal with message
        }
    }

    // forward message to member function handler
    return dlg->on_message(message, wParam, lParam);
}

LRESULT BaseDialog::on_message(UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message) {
    case WM_INITDIALOG:
        return on_initdialog(wParam);
        break;
    case WM_COMMAND:
        return on_command(wParam, lParam);
        break;
    case WM_NOTIFY:
        return on_notify(wParam, lParam);
        break;
    case WM_CLOSE:
        if (m_modalDialog) {
            EndDialog(m_hWnd, wParam);
        } else {
            DestroyWindow(m_hWnd);
            signalResponse(RESPONSE_CANCEL);
        }
        return TRUE;
        break;
    }

    return FALSE;
}

LRESULT BaseDialog::on_notify(WPARAM wParam, LPARAM lParam)
{
    return FALSE;
}

bool BaseDialog::doDialog(int resourceID)
{
    HWND hParent = GetForegroundWindow();
    m_modalDialog = false;

    m_hWnd = CreateDialogParam(m_hInst, MAKEINTRESOURCE(resourceID), hParent, (DLGPROC)dialogProc,
                         reinterpret_cast<LPARAM>(this));
    if (!m_hWnd)
        return false;

    ShowWindow(m_hWnd, SW_NORMAL);
    return true;
}

int BaseDialog::doModalDialog(int resourceID)
{
    HWND hParent = GetForegroundWindow();
    m_modalDialog = true;

    return DialogBoxParam(m_hInst, MAKEINTRESOURCE(resourceID), hParent, (DLGPROC)dialogProc,
                         reinterpret_cast<LPARAM>(this));
}
