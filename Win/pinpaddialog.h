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

#include <windows.h>
#include <string>

#include "Win/basedialog.h"

class PinpadDialog : public BaseDialog
{
public:
    PinpadDialog(HINSTANCE hInst);
    virtual ~PinpadDialog();

    void pinBlockedMessage(HWND hParent = NULL);
    void setSubject(const std::string& subject);
    void showRetry(int triesLeft);
    void hideRetry();
    void resetProgressbar();

    bool doDialog(HWND hParent = NULL);

protected:
    LRESULT on_initdialog(WPARAM wParam);
    LRESULT on_command(WPARAM wParam, LPARAM lParam);
    LRESULT on_ctlcolorstatic(WPARAM wParam, LPARAM lParam);
    LRESULT on_timer(WPARAM wParam, LPARAM lParam);
    LRESULT on_destroy(WPARAM wParam, LPARAM lParam);

private:
    HANDLE getImage();
    void setFontSize(HWND hText, int fontSize);
    int preferredWidth(HWND hWnd, const std::wstring& text);
    int currentWidth(HWND hWnd);
    void resizeWindow(HWND hWnd, int width, int height);
    void resizeControl(HWND hWnd, HWND hControl, int width, int height);
    void moveControl(HWND hWnd, HWND hControl, int dx, int dy);
    void showWrongPin(HWND hParent, int tries);

    std::wstring m_subject;
    int m_timeTotal;
    int m_timeRemaining;
};

#endif //ESTEID_PINPADDIALOG_H
