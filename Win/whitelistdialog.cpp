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

#include "whitelistdialog.h"
#include "basedialog.h"
#include "converter.h"

#include <windows.h>
#include "Win/whitelistdialog_res.h"
#include <commctrl.h>
#include <stdio.h>

#define BUF_SIZE 100

WhitelistDialog::WhitelistDialog(HINSTANCE hInst)
    : BaseDialog(hInst)
{
}


WhitelistDialog::~WhitelistDialog()
{
}


void WhitelistDialog::addSites(const std::vector<std::string> & sv)
{
    m_sites = sv;
}


void WhitelistDialog::addDefaultSites(const std::vector<std::string> & sv)
{
    m_defaultSites = sv;
}


std::vector<std::string> WhitelistDialog::getWhitelist()
{
    return m_sites;
}


void WhitelistDialog::setEntryText(const std::string & site)
{
    std::wstring ws = Converter::string_to_wstring(site);
    SetDlgItemText(m_hWnd, IDC_ADD, const_cast<wchar_t *>(ws.c_str()));
}


void WhitelistDialog::storeItems()
{
    wchar_t *buf = new wchar_t[BUF_SIZE];
    int itemCount = ListView_GetItemCount(m_hList);

    m_sites.clear();

    for (int i = 0; i < itemCount; ++i) {
        ListView_GetItemText(m_hList, i, 0, buf, BUF_SIZE);
        m_sites.push_back(Converter::wstring_to_string(buf));
    }

    delete[] buf;
}


void WhitelistDialog::insertItem(const std::wstring & name, bool editable)
{
    LVITEM lvItem;
    memset(&lvItem, 0, sizeof(lvItem));

    lvItem.mask = LVIF_TEXT;
    lvItem.cchTextMax = BUF_SIZE;
    lvItem.iItem = ListView_GetItemCount(m_hList); // Insert at the end
    lvItem.iSubItem = 0; // First column
    lvItem.pszText = const_cast<wchar_t *>(name.c_str());

    ListView_InsertItem(m_hList, &lvItem);
}


void WhitelistDialog::insertItem(const std::string & name, bool editable)
{
    std::wstring ws = Converter::string_to_wstring(name);
    insertItem(ws, editable);
}


LRESULT WhitelistDialog::on_initdialog(WPARAM wParam)
{
    RECT rect;
    LVCOLUMN column;
    m_hList = GetDlgItem(m_hWnd, IDC_LIST);

    SendMessageA(m_hWnd, WM_SETICON, (WPARAM)1, (LPARAM)LoadIcon(m_hInst, MAKEINTRESOURCE(IDI_ICON1)));

    ListView_SetTextBkColor(m_hList, CLR_NONE);
    ListView_SetExtendedListViewStyle(m_hList, LVS_EX_FULLROWSELECT);

    // Get the listview size so that we can set correct column width
    GetClientRect(m_hList, &rect);

    // Insert one column
    memset(&column, 0, sizeof(column));
    column.mask = LVCF_TEXT|LVCF_WIDTH|LVCF_SUBITEM;
    column.cx = rect.right - rect.left; // max width
    column.pszText = L"Sites";
    ListView_InsertColumn(m_hList, 0, &column);

    std::vector<std::string>::const_iterator it;
    for (it = m_sites.begin(); it != m_sites.end(); ++it)
        insertItem(*it, true);
    for (it = m_defaultSites.begin(); it != m_defaultSites.end(); ++it)
        insertItem(*it, false);

    return TRUE;
}

LRESULT WhitelistDialog::on_command(WPARAM wParam, LPARAM lParam)
{
    int currentIndex;
    wchar_t *buf = NULL;

    switch (LOWORD(wParam)) {

    case IDC_ADDBUTTON:
        buf = new wchar_t[BUF_SIZE];
        GetDlgItemText(m_hWnd, IDC_ADD, buf, BUF_SIZE);
        if ((lstrlen(buf)) > 0) {
            insertItem(buf, true);
            SetDlgItemText(m_hWnd, IDC_ADD, NULL);
        }
        delete[] buf;
        break;

    case IDC_REMOVEBUTTON:
        currentIndex = ListView_GetNextItem(m_hList, -1, LVNI_FOCUSED | LVNI_SELECTED);
        if (currentIndex >= 0)
            ListView_DeleteItem(m_hList, currentIndex);
        break;

    case IDC_EDITBUTTON:
        currentIndex = ListView_GetNextItem(m_hList, -1, LVNI_FOCUSED | LVNI_SELECTED);
        if (currentIndex >= 0) {
            SetFocus(m_hList);
            ListView_EditLabel(m_hList, currentIndex);
        }
        break;

    case IDC_CLOSEBUTTON:
        storeItems();
        signalResponse(RESPONSE_OK);

        DestroyWindow(m_hWnd);
        break;

    default:
        return FALSE;
    }

    return FALSE;
}

LRESULT WhitelistDialog::on_notify(WPARAM wParam, LPARAM lParam)
{
    switch (LOWORD(wParam)) {
    case IDC_LIST:
        if (((LPNMHDR)lParam)->code == LVN_ENDLABELEDIT) {
            NMLVDISPINFO *lpNmlvdispInfo = (NMLVDISPINFO*)lParam;
            if (lpNmlvdispInfo->item.pszText != NULL) {
                ::SetWindowLongPtr(m_hWnd, DWLP_MSGRESULT, TRUE);
                return TRUE;
            }
        }
        break;
    }

    return FALSE;
}

bool WhitelistDialog::doDialog(HWND hParent)
{
    return BaseDialog::doDialog(IDC_DIALOG, hParent);
}
