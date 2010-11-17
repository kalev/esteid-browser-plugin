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
#include "basedialog.h"
#include "utf8_tools.h"

#include <boost/lexical_cast.hpp>
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


void PinInputDialog::showPinBlocked(HWND hParent)
{
    MessageBox(hParent, L"PIN2 blocked.\nPlease run ID card Utility to unlock the PIN.",
               L"Error", MB_OK | MB_ICONHAND);
}


void PinInputDialog::setSubject(const std::string& subject)
{
    m_subject = FB::utf8_to_wstring(subject) + L" (PIN2)";
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


// calculate control's width needed to fit text
int PinInputDialog::preferredWidth(HWND hWnd, const std::wstring& text)
{
    HDC hdc;
    SIZE size;

    hdc = GetDC(hWnd);
    GetTextExtentPoint32(hdc, text.c_str(), (int)text.size(), &size);
    ReleaseDC(hWnd, hdc);

    return size.cx;
}


// get control's current width
int PinInputDialog::currentWidth(HWND hWnd)
{
    RECT rect;
    POINT ptDiff;

    GetClientRect(hWnd, &rect);
    ptDiff.x = (rect.right - rect.left);
    ptDiff.y = (rect.bottom - rect.top);

    return ptDiff.x;
}


void PinInputDialog::resizeWindow(HWND hWnd, int width, int height)
{
    RECT rect;
    POINT ptDiff;

    GetWindowRect(hWnd, &rect);
    ptDiff.x = (rect.right - rect.left);
    ptDiff.y = (rect.bottom - rect.top);

    MoveWindow(hWnd, rect.left, rect.top, ptDiff.x + width, ptDiff.y + height, TRUE);
}


void PinInputDialog::resizeControl(HWND hWnd, HWND hControl, int width, int height)
{
    RECT rect;
    POINT point;

    GetWindowRect(hControl, &rect);
    point.x = rect.left;
    point.y = rect.top;
    ScreenToClient(hWnd, &point);
    GetClientRect(hControl, &rect);

    MoveWindow(hControl, point.x, point.y, (rect.right - rect.left) + width, (rect.bottom - rect.top) + height, TRUE);
}


void PinInputDialog::moveControl(HWND hWnd, HWND hControl, int dx, int dy)
{
    RECT rect;
    POINT point;

    GetWindowRect(hControl, &rect);
    point.x = rect.left;
    point.y = rect.top;
    ScreenToClient(hWnd, &point);
    GetClientRect(hControl, &rect);

    MoveWindow(hControl, point.x + dx, point.y + dy, rect.right - rect.left, rect.bottom - rect.top, TRUE);
}


void PinInputDialog::showWrongPin(HWND hParent, int tries)
{
    static const std::wstring title = L"Wrong PIN!";
    std::wstring text(L"Tries left: " + boost::lexical_cast<std::wstring>(tries));

// mingw doesn't have EDITBALLOONTIP
#ifdef EM_SHOWBALLOONTIP
    EDITBALLOONTIP ebt = {0};
    ebt.cbStruct = sizeof(EDITBALLOONTIP);
    ebt.pszTitle = const_cast<wchar_t *>(title.c_str());
    ebt.pszText = const_cast<wchar_t *>(text.c_str());
    ebt.ttiIcon = TTI_ERROR;
    if (!SendMessage(hParent, EM_SHOWBALLOONTIP, 0, (LPARAM)&ebt)) {
#endif
        MessageBox(hParent, const_cast<wchar_t *>((title + L"\n" + text).c_str()), L"Warning", MB_OK | MB_ICONHAND);
#ifdef EM_SHOWBALLOONTIP
    }
#endif
}


LRESULT PinInputDialog::on_initdialog(WPARAM wParam)
{
    HWND hLabel = GetDlgItem(m_hWnd, IDC_LABEL);
    HWND hPinedit = GetDlgItem(m_hWnd, IDC_PINEDIT);

    SetDlgItemText(m_hWnd, IDC_LABEL, const_cast<wchar_t *>(m_subject.c_str()));

    setFontSize(hLabel, 10);

    // set icon
    HICON icon = getIcon();
    SendDlgItemMessage(m_hWnd, IDI_PINICON, STM_SETIMAGE, IMAGE_ICON, (LPARAM)icon);

    // set maximum pin length
    SendDlgItemMessage(m_hWnd, IDC_PINEDIT, EM_SETLIMITTEXT, 12, 0);

    // resize dialog to fit long names
    if (currentWidth(hLabel) < preferredWidth(hLabel, m_subject)) {
        int dx = preferredWidth(hLabel, m_subject) - currentWidth(hLabel);
        // for reasons unknown, the width of IDC_LABEL and IDC_PINEDIT differ a little bit
        int dx2 = preferredWidth(hLabel, m_subject) - currentWidth(hPinedit);

        resizeWindow(m_hWnd, dx, 0);
        resizeControl(m_hWnd, hLabel, dx, 0);
        resizeControl(m_hWnd, hPinedit, dx2, 0);
        moveControl(m_hWnd, GetDlgItem(m_hWnd, IDOK), dx, 0);
        moveControl(m_hWnd, GetDlgItem(m_hWnd, IDCANCEL), dx, 0);
    }

    if (m_retry)
        showWrongPin(hPinedit, m_triesLeft);

    if (GetDlgCtrlID((HWND) wParam) != IDC_PINEDIT) {
        SetFocus(hPinedit);
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
            releaseIEModalLock();
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
            releaseIEModalLock();
            signalResponse(RESPONSE_CANCEL);
        }
        return TRUE;
        break;
    }

    return FALSE;
}


bool PinInputDialog::doDialog(HWND hParent)
{
    return BaseDialog::doDialog(IDD_PINDIALOG, hParent);
}

int PinInputDialog::doModalDialog(HWND hParent)
{
    int rv = BaseDialog::doModalDialog(IDD_PINDIALOG, hParent);
    if (rv == IDOK)
        return RESPONSE_OK;
    else
        return RESPONSE_CANCEL;
}
