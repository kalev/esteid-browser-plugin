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
#include "converter.h"

#include <windows.h>
#include <commctrl.h>
#include "Win/pininputdialog_res.h"

#define BUF_SIZE 100

PinInputDialog::PinInputDialog(HINSTANCE hInst)
    : BaseDialog(hInst),
      m_retry(false),
      m_triesLeft(0),
      m_minPinLength(5)
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


void PinInputDialog::setSubject(const std::string& subject)
{
    m_subject = Converter::string_to_wstring(subject) + L" (PIN2)";
}


void PinInputDialog::setRetry(bool retry)
{
    m_retry = retry;
}


void PinInputDialog::setTries(int tries)
{
    m_triesLeft = tries;
}


std::string PinInputDialog::getPinInternal()
{
    char *buf = new char[BUF_SIZE];

    GetDlgItemTextA(m_hWnd, IDC_PINEDIT, buf, BUF_SIZE);

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


HICON PinInputDialog::getCreduiIcon()
{
    HMODULE module = LoadLibraryExA("credui.dll", NULL, LOAD_LIBRARY_AS_DATAFILE);
    if (!module)
        return NULL;
    return LoadIcon(module, MAKEINTRESOURCE(1201));
}


HICON PinInputDialog::getIcon()
{
    HICON icon = getCreduiIcon();
    if (!icon) // fall back to embedded icon
        icon = LoadIcon(m_hInst, MAKEINTRESOURCE(IDI_PINICON));
    return icon;
}


void PinInputDialog::setFontSize(HWND hText, int fontSize)
{
    HFONT hOldFont = (HFONT)SendMessage(hText, WM_GETFONT, 0, 0);
    HDC hDC = GetDC(hText);
    LOGFONT lf;
    GetObject(hOldFont, sizeof(LOGFONT), &lf);
    lf.lfHeight = -MulDiv(fontSize, GetDeviceCaps(hDC, LOGPIXELSY), 72);
    HFONT hNewFont = CreateFontIndirect(&lf);
    SendMessage(hText, WM_SETFONT, (WPARAM)hNewFont, MAKELPARAM(TRUE, 0));
    ReleaseDC(hText, hDC);
}


void PinInputDialog::showWrongPin(HWND hWnd, int tries)
{
    static const std::wstring title = L"Wrong PIN!";
    std::wstringstream out;
    out << L"Tries left: " << m_triesLeft;
    std::wstring text = out.str();

// mingw doesn't have EDITBALLOONTIP
#ifdef EM_SHOWBALLOONTIP
    EDITBALLOONTIP ebt = {0};
    ebt.cbStruct = sizeof(EDITBALLOONTIP);
    ebt.pszTitle = const_cast<wchar_t *>(title.c_str());
    ebt.pszText = const_cast<wchar_t *>(text.c_str());
    ebt.ttiIcon = TTI_ERROR;
    if (!SendMessage(hWnd, EM_SHOWBALLOONTIP, 0, (LPARAM)&ebt)) {
#endif
        MessageBox(hWnd, const_cast<wchar_t *>((title + L"\n" + text).c_str()), L"Warning", MB_OK | MB_ICONHAND);
#ifdef EM_SHOWBALLOONTIP
    }
#endif
}


LRESULT PinInputDialog::on_initdialog(WPARAM wParam)
{
    SetDlgItemText(m_hWnd, IDC_LABEL, const_cast<wchar_t *>(m_subject.c_str()));

    setFontSize(GetDlgItem(m_hWnd, IDC_LABEL), 10);

    // set icon
    HICON icon = getIcon();
    SendDlgItemMessage(m_hWnd, IDI_PINICON, STM_SETIMAGE, IMAGE_ICON, (LPARAM)icon);

    // set maximum pin length
    SendDlgItemMessage(m_hWnd, IDC_PINEDIT, EM_SETLIMITTEXT, 12, 0);

    if (m_retry)
        showWrongPin(GetDlgItem(m_hWnd, IDC_PINEDIT), m_triesLeft);

    if (GetDlgCtrlID((HWND) wParam) != IDC_PINEDIT) {
        SetFocus(GetDlgItem(m_hWnd, IDC_PINEDIT));
        return FALSE;
    }

    return TRUE;
}


LRESULT PinInputDialog::on_command(WPARAM wParam, LPARAM lParam)
{
    switch (LOWORD(wParam)) {

    case IDC_PINEDIT:
        if (HIWORD(wParam) == EN_CHANGE) {
            std::string pin = getPinInternal();
            if (pin.size() >= m_minPinLength)
                EnableWindow(GetDlgItem(m_hWnd, IDOK), TRUE);
            else
                EnableWindow(GetDlgItem(m_hWnd, IDOK), FALSE);
        }
        break;

    case IDOK:
        m_pin = getPinInternal();
        SetDlgItemTextA(m_hWnd, IDC_PINEDIT, "");

        if (m_modalDialog) {
            EndDialog(m_hWnd, wParam);
        } else {
            DestroyWindow(m_hWnd);
            signalResponse(RESPONSE_OK);
        }
        return TRUE;
        break;

    case IDCANCEL:
        m_pin = "";
        SetDlgItemTextA(m_hWnd, IDC_PINEDIT, "");

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


bool PinInputDialog::doDialog()
{
    return BaseDialog::doDialog(IDD_PINDIALOG);
}

int PinInputDialog::doModalDialog()
{
    int rv = BaseDialog::doModalDialog(IDD_PINDIALOG);
    if (rv == IDOK)
        return RESPONSE_OK;
    else
        return RESPONSE_CANCEL;
}
