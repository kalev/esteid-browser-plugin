#include "basedialog.h"
#include <windows.h>

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

bool BaseDialog::doDialog(int resourceID)
{
    HWND hParent = GetForegroundWindow();

    m_hWnd = CreateDialogParam(m_hInst, MAKEINTRESOURCE(resourceID), hParent, (DLGPROC)dialogProc,
                         reinterpret_cast<LPARAM>(this));
    if (!m_hWnd)
        return false;

    ShowWindow(m_hWnd, SW_NORMAL);
    return true;
}
