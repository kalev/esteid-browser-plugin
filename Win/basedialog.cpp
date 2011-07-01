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

#include "basedialog.h"
#include <windows.h>
#include <IEProcess.h>

typedef BaseDialog::Connection Connection;

BaseDialog::BaseDialog(HINSTANCE hInst)
    : m_hInst(hInst),
      m_hWnd(NULL)
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

void BaseDialog::setFontSize(HWND hText, int fontSize)
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
int BaseDialog::preferredWidth(HWND hWnd, const std::wstring& text)
{
    HDC hdc;
    SIZE size;

    hdc = GetDC(hWnd);
    GetTextExtentPoint32(hdc, text.c_str(), (int)text.size(), &size);
    ReleaseDC(hWnd, hdc);

    return size.cx;
}

// get control's current width
int BaseDialog::currentWidth(HWND hWnd)
{
    RECT rect;
    POINT ptDiff;

    GetClientRect(hWnd, &rect);
    ptDiff.x = (rect.right - rect.left);
    ptDiff.y = (rect.bottom - rect.top);

    return ptDiff.x;
}

void BaseDialog::resizeWindow(HWND hWnd, int width, int height)
{
    RECT rect;
    POINT ptDiff;

    GetWindowRect(hWnd, &rect);
    ptDiff.x = (rect.right - rect.left);
    ptDiff.y = (rect.bottom - rect.top);

    MoveWindow(hWnd, rect.left, rect.top, ptDiff.x + width, ptDiff.y + height, TRUE);
}

void BaseDialog::resizeControl(HWND hWnd, HWND hControl, int width, int height)
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

void BaseDialog::moveControl(HWND hWnd, HWND hControl, int dx, int dy)
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


static HHOOK s_hHook = NULL;
static HWND s_hWnd = NULL;

LRESULT CALLBACK GetMsgProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    LPMSG lpMsg = reinterpret_cast<LPMSG>(lParam);

    if (nCode >= 0 && wParam == PM_REMOVE &&
              lpMsg->message >= WM_KEYFIRST &&
              lpMsg->message <= WM_KEYLAST) {

        if (IsWindow(s_hWnd) && IsDialogMessage(s_hWnd, lpMsg)) {
            // The value returned from this hookproc is ignored, and it cannot
            // be used to tell Windows the message has been handled. To avoid
            // further processing, convert the message to WM_NULL before
            // returning.
            lpMsg->message = WM_NULL;
            lpMsg->lParam = 0L;
            lpMsg->wParam = 0;
        }
    }

    return CallNextHookEx(s_hHook, nCode, wParam, lParam);
}

LRESULT BaseDialog::on_message(UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message) {
    case WM_INITDIALOG:
        s_hWnd = m_hWnd;
        s_hHook = SetWindowsHookEx(WH_GETMESSAGE, GetMsgProc, NULL, GetCurrentThreadId());
        return on_initdialog(wParam);
        break;
    case WM_COMMAND:
        return on_command(wParam, lParam);
        break;
    case WM_CTLCOLORSTATIC:
        return on_ctlcolorstatic(wParam, lParam);
        break;
    case WM_TIMER:
        return on_timer(wParam, lParam);
        break;
    case WM_NOTIFY:
        return on_notify(wParam, lParam);
        break;
    case WM_CLOSE:
        signalResponse(RESPONSE_CANCEL);
        return TRUE;
        break;
    case WM_DESTROY:
        UnhookWindowsHookEx(s_hHook);
        return on_destroy(wParam, lParam);
        break;
    }

    return FALSE;
}

LRESULT BaseDialog::on_notify(WPARAM wParam, LPARAM lParam)
{
    return FALSE;
}

HWND BaseDialog::getIEModalLock(HWND hWnd)
{
    HMODULE ieModule = IEProcess::GetProcessModule();
    if (ieModule) {
        // IE 8.0 or newer
        HRESULT hr = IEProcess::GetTabWindowExports(ieModule)->AcquireModalDialogLockAndParent(hWnd, &hWnd, &m_hModalDialogLock);
    }
    return hWnd;
}

void BaseDialog::releaseIEModalLock()
{
    HMODULE ieModule = IEProcess::GetProcessModule();
    if (ieModule) {
        // IE 8.0 or newer
        IEProcess::GetTabWindowExports(ieModule)->ReleaseModalDialogLockAndParent(m_hModalDialogLock);
    }
}

void BaseDialog::close()
{
    DestroyWindow(m_hWnd);
    releaseIEModalLock();
}

bool BaseDialog::doDialog(int resourceID, HWND hParent)
{
    m_hWnd = CreateDialogParam(m_hInst,
                               MAKEINTRESOURCE(resourceID),
                               getIEModalLock(hParent),
                               (DLGPROC)dialogProc,
                               reinterpret_cast<LPARAM>(this));
    if (!m_hWnd)
        return false;

    ShowWindow(m_hWnd, SW_NORMAL);
    return true;
}
