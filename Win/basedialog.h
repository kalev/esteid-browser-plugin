#ifndef ESTEID_BASEDIALOG_H
#define ESTEID_BASEDIALOG_H

#include <windows.h>
#include <boost/signals.hpp>

class BaseDialog
{
public:
    BaseDialog(HINSTANCE hInst);
    virtual ~BaseDialog();

    enum ResponseType
    {
        RESPONSE_OK = -5,
        RESPONSE_CANCEL = -6,
    };

    typedef boost::signal<void (int)> ResponseSignal;
    typedef boost::signals::connection Connection;

    Connection connect(const ResponseSignal::slot_type& subscriber);
    void disconnect(Connection subscriber);
    virtual bool doDialog(int resourceID);

protected:
    HINSTANCE m_hInst;
    HWND m_hWnd;

    static LRESULT CALLBACK dialogProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    virtual LRESULT on_message(UINT message, WPARAM wParam, LPARAM lParam) = 0;
    ResponseSignal signalResponse;
};

#endif //ESTEID_BASEDIALOG_H
