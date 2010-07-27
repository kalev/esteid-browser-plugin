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

#ifndef ESTEID_PININPUTDIALOG_H
#define ESTEID_PININPUTDIALOG_H

#include <windows.h>
#include <string>

#include "Win/basedialog.h"

class PinInputDialog : public BaseDialog
{
public:
    PinInputDialog(HINSTANCE hInst);
    virtual ~PinInputDialog();

    void showPinBlocked();
    void setSubject(const std::string& subject);
    void setRetry(bool retry);
    void setTries(int tries);
    std::string getPin();
    void clearPin();

    bool doDialog();
    int doModalDialog();

private:
    LRESULT on_initdialog(WPARAM wParam);
    LRESULT on_command(WPARAM wParam, LPARAM lParam);

    std::string getPinInternal();
    HICON getCreduiIcon();
    HICON getIcon();
    void setFontSize(HWND hText, int fontSize);
    int preferredWidth(HWND hWnd, const std::wstring& text);
    int currentWidth(HWND hWnd);
    void resizeWindow(HWND hWnd, int width, int height);
    void resizeControl(HWND hWnd, HWND hControl, int width, int height);
    void moveControl(HWND hWnd, HWND hControl, int dx, int dy);
    void showWrongPin(HWND hWnd, int tries);

    std::wstring m_subject;
    bool m_retry;
    int m_triesLeft;
    std::string m_pin;
    size_t m_minPinLength;
};

#endif //ESTEID_PININPUTDIALOG_H
