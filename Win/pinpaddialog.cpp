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
#include "basedialog.h"
#include "utf8_tools.h"

#include <boost/lexical_cast.hpp>
#include <windows.h>
#include <commctrl.h>
#include "Win/pinpaddialog_res.h"

#define BUF_SIZE 100

PinpadDialog::PinpadDialog(HINSTANCE hInst)
    : BaseDialog(hInst),
      m_timeTotal(30)
{
}


PinpadDialog::~PinpadDialog()
{
}


void PinpadDialog::pinBlockedMessage(HWND a_hWnd)
{
    HWND hParent;
    if (IsWindowVisible(m_hWnd))
        hParent = m_hWnd;
    else
        hParent = a_hWnd;

    MessageBox(hParent, L"PIN2 blocked.\nPlease run ID card Utility to unlock the PIN.",
               L"Error", MB_OK | MB_ICONHAND);
}


void PinpadDialog::setSubject(const std::string& subject)
{
    m_subject = FB::utf8_to_wstring(subject) + L" (PIN2)";
}


void PinpadDialog::showRetry(int triesLeft)
{
    assert(triesLeft > 0);

    std::wstring label = L"Wrong PIN! Tries left: " + boost::lexical_cast<std::wstring>(triesLeft);
    SetDlgItemText(m_hWnd, IDC_WARNING, const_cast<wchar_t *>(label.c_str()));
    ShowWindow(GetDlgItem(m_hWnd, IDC_WARNING), SW_SHOW);
}


void PinpadDialog::hideRetry()
{
    ShowWindow(GetDlgItem(m_hWnd, IDC_WARNING), SW_HIDE);
}


void PinpadDialog::resetProgressbar()
{
    // reset the progressbar
    HWND hProgress = GetDlgItem(m_hWnd, IDC_PINPAD_PROGRESS);
    SendMessage(hProgress, PBM_SETRANGE, 0, MAKELPARAM(0, m_timeTotal));
    SendMessage(hProgress, PBM_SETPOS, (WPARAM)m_timeTotal, 0);
    m_timeRemaining = m_timeTotal;

    // stop the old timer
    KillTimer(m_hWnd, IDT_PINPAD_TIMER);

    // start a new timer
    SetTimer(m_hWnd, IDT_PINPAD_TIMER, 1000, NULL);
}


HANDLE PinpadDialog::getImage()
{
    return LoadImage(m_hInst, MAKEINTRESOURCE(IDI_PINPAD_ICON), IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
}


void PinpadDialog::setFontSize(HWND hText, int fontSize)
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
int PinpadDialog::preferredWidth(HWND hWnd, const std::wstring& text)
{
    HDC hdc;
    SIZE size;

    hdc = GetDC(hWnd);
    GetTextExtentPoint32(hdc, text.c_str(), (int)text.size(), &size);
    ReleaseDC(hWnd, hdc);

    return size.cx;
}


// get control's current width
int PinpadDialog::currentWidth(HWND hWnd)
{
    RECT rect;
    POINT ptDiff;

    GetClientRect(hWnd, &rect);
    ptDiff.x = (rect.right - rect.left);
    ptDiff.y = (rect.bottom - rect.top);

    return ptDiff.x;
}


void PinpadDialog::resizeWindow(HWND hWnd, int width, int height)
{
    RECT rect;
    POINT ptDiff;

    GetWindowRect(hWnd, &rect);
    ptDiff.x = (rect.right - rect.left);
    ptDiff.y = (rect.bottom - rect.top);

    MoveWindow(hWnd, rect.left, rect.top, ptDiff.x + width, ptDiff.y + height, TRUE);
}


void PinpadDialog::resizeControl(HWND hWnd, HWND hControl, int width, int height)
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


void PinpadDialog::moveControl(HWND hWnd, HWND hControl, int dx, int dy)
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


LRESULT PinpadDialog::on_initdialog(WPARAM wParam)
{
    HWND hLabel = GetDlgItem(m_hWnd, IDC_PINPAD_LABEL);
    HWND hProgress = GetDlgItem(m_hWnd, IDC_PINPAD_PROGRESS);

    SetDlgItemText(m_hWnd, IDC_PINPAD_LABEL, const_cast<wchar_t *>(m_subject.c_str()));

    setFontSize(hLabel, 10);
    resetProgressbar();

    // set pinpad image
    HANDLE hImage = getImage();
    SendDlgItemMessage(m_hWnd, IDC_PINPAD_ICON, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hImage);

    // resize dialog to fit long names
    if (currentWidth(hLabel) < preferredWidth(hLabel, m_subject)) {
        int dx = preferredWidth(hLabel, m_subject) - currentWidth(hLabel);
        // for reasons unknown, the width of IDC_LABEL and IDC_PROGRESS differ a little bit
        int dx2 = preferredWidth(hLabel, m_subject) - currentWidth(hProgress);

        resizeWindow(m_hWnd, dx, 0);
        resizeControl(m_hWnd, hLabel, dx, 0);
        resizeControl(m_hWnd, hProgress, dx2, 0);
    }

    return TRUE;
}


LRESULT PinpadDialog::on_command(WPARAM wParam, LPARAM lParam)
{
    return FALSE;
}


LRESULT PinpadDialog::on_ctlcolorstatic(WPARAM wParam, LPARAM lParam)
{
    if ((HWND)lParam == GetDlgItem(m_hWnd, IDC_WARNING)) {
        SetBkMode((HDC)wParam, TRANSPARENT);
        SetTextColor((HDC)wParam, RGB(255,0,0));
        return (BOOL)CreateSolidBrush (GetSysColor(COLOR_MENU));
    }

    return FALSE;
}


LRESULT PinpadDialog::on_timer(WPARAM wParam, LPARAM lParam)
{
    switch (LOWORD(wParam)) {
    case IDT_PINPAD_TIMER:
        if (m_timeRemaining <= 0)
            return FALSE;

        m_timeRemaining--;
        SendMessage(GetDlgItem(m_hWnd, IDC_PINPAD_PROGRESS), PBM_SETPOS, m_timeRemaining, 0);
        return TRUE;
        break;
    }

    return FALSE;
}


LRESULT PinpadDialog::on_destroy(WPARAM wParam, LPARAM lParam)
{
    KillTimer(m_hWnd, IDT_PINPAD_TIMER);

    return FALSE;
}


bool PinpadDialog::doDialog(HWND hParent)
{
    return BaseDialog::doDialog(IDD_PINPAD_DIALOG, hParent);
}
