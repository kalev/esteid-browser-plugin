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

#include "pininputdialog.h"
#include "basedialog.h"

#include <windows.h>
#include "Win/pininputdialog_res.h"

#define BUF_SIZE 100

PinInputDialog::PinInputDialog(HINSTANCE hInst)
    : BaseDialog(hInst),
      m_minPinLength(5),
      m_modalDialog(false)
{
}


PinInputDialog::~PinInputDialog()
{
}


void PinInputDialog::showPinBlocked()
{
    HWND hParent = GetForegroundWindow();

    MessageBox(hParent, L"PIN2 blocked.\nPlease run ID-card Utility to unlock the PIN.",
               L"Error", MB_OK | MB_ICONHAND);
}


void PinInputDialog::showWrongPin(int tries)
{
    HWND hParent = GetForegroundWindow();

    static const std::string text = "Wrong PIN! Tries left: ";
    std::stringstream out;
    out << text << tries;

    MessageBoxA(hParent, out.str().c_str(), "Warning", MB_OK | MB_ICONHAND);
}


void PinInputDialog::setSubject(const std::string& subject)
{
    m_subject = subject + " (PIN2)";
}


std::string PinInputDialog::getPinInternal()
{
    char *buf = new char[BUF_SIZE];

    GetDlgItemTextA(m_hWnd, IDC_PININPUT, buf, BUF_SIZE);

    std::string ret(buf);
    delete[] buf;

    return ret;
}


std::string PinInputDialog::getPin()
{
    return m_pin;
}


void PinInputDialog::clearPin()
{
    m_pin = "";
}


LRESULT PinInputDialog::on_initdialog(WPARAM wParam)
{
    SetWindowPos(m_hWnd, HWND_TOPMOST, 10, 10, 0, 0, SWP_NOSIZE);
    SetDlgItemTextA(m_hWnd, IDC_STATIC, m_subject.c_str());

    SendMessageA(m_hWnd, WM_SETICON, (WPARAM)1, (LPARAM)LoadIcon(m_hInst, MAKEINTRESOURCE(IDI_ICON1)));

    SendDlgItemMessage(m_hWnd, IDC_PININPUT, EM_SETLIMITTEXT, 12, 0);

    if (GetDlgCtrlID((HWND) wParam) != IDC_PININPUT) {
        SetFocus(GetDlgItem(m_hWnd, IDC_PININPUT));
        return FALSE;
    }

    return TRUE;
}


LRESULT PinInputDialog::on_command(WPARAM wParam, LPARAM lParam)
{
    switch (LOWORD(wParam)) {

    case IDC_PININPUT:
        if (HIWORD(wParam) == EN_CHANGE) {
            std::string pin = getPinInternal();

            if (pin.size() >= m_minPinLength) {
                EnableWindow(GetDlgItem(m_hWnd, IDOK), TRUE);
                SendMessage(m_hWnd, DM_SETDEFID, IDOK, 0);
            } else {
                EnableWindow(GetDlgItem(m_hWnd, IDOK), FALSE);
                SendMessage(m_hWnd, DM_SETDEFID, IDCANCEL, 0);
            }
        }
        break;

    case IDOK:
        m_pin = getPinInternal();
        SetDlgItemTextA(m_hWnd, IDC_PININPUT, "");

        if (m_modalDialog) {
            EndDialog(m_hWnd, wParam);
        } else {
            signalResponse(RESPONSE_OK);
            DestroyWindow(m_hWnd);
        }
        return TRUE;
        break;

    case IDCANCEL:
        m_pin = "";
        SetDlgItemTextA(m_hWnd, IDC_PININPUT, "");

        if (m_modalDialog) {
            EndDialog(m_hWnd, wParam);
        } else {
            signalResponse(RESPONSE_CANCEL);
            DestroyWindow(m_hWnd);
        }
        return TRUE;
        break;
    }

    return FALSE;
}


LRESULT PinInputDialog::on_message(UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message) {
    case WM_INITDIALOG:
        return on_initdialog(wParam);
        break;
    case WM_COMMAND:
        return on_command(wParam, lParam);
        break;
    case WM_CLOSE:
        DestroyWindow(m_hWnd);
        break;
    }

    return 0;
}


bool PinInputDialog::doDialog()
{
    m_modalDialog = false;

    return BaseDialog::doDialog(IDD_PIN_DIALOG_ENG);
}

int PinInputDialog::doModalDialog()
{
    m_modalDialog = true;

    int rv = BaseDialog::doModalDialog(IDD_PIN_DIALOG_ENG);
    if (rv == IDOK)
        return RESPONSE_OK;
    else
        return RESPONSE_CANCEL;
}
