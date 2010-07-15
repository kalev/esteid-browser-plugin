#ifndef ESTEID_BASEDIALOG_H
#define ESTEID_BASEDIALOG_H

#include <windows.h>

class BaseDialog
{
public:
    BaseDialog(HINSTANCE hInst);
    virtual ~BaseDialog();

    virtual bool doDialog(int resourceID);

protected:
    HINSTANCE m_hInst;
    HWND m_hWnd;

    static LRESULT CALLBACK dialogProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    virtual LRESULT on_message(UINT message, WPARAM wParam, LPARAM lParam) = 0;
};

#endif //ESTEID_BASEDIALOG_H
