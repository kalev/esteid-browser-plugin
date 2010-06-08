/*!
	\file		pinDialog.cpp
	\copyright	(c) Kaido Kert ( kaidokert@gmail.com )
	\licence	BSD
	\author		$Author: kaidokert $
	\date		$Date: 2010-02-16 08:28:37 +0200 (T, 16 veebr 2010) $
*/
// Revision $Revision: 517 $

#include "precompiled.h"
#include "pinDialog.h"
#include "threadObj.h"

#ifdef _WIN32
#include <windows.h>
#include <crtdbg.h>

struct iconHandle {
	HMODULE m_module;
	HICON m_icon;
	iconHandle(const char *modName,int id) {
		m_module = LoadLibraryExA(modName,NULL,LOAD_LIBRARY_AS_DATAFILE);
		m_icon = LoadIcon(m_module,MAKEINTRESOURCE(id));
		}
	~iconHandle() {
		if (m_icon) DestroyIcon(m_icon);
		if (m_module) FreeLibrary(m_module);
		}
	operator const HANDLE() { return m_icon;}
	operator const LPARAM() { return (LPARAM) m_icon;}
	};

struct pinDialogPriv {
	struct _icontag {
		const char *module;
		int id;
	} static iconSet[2];
	struct pinDialogPriv_a {
		HINSTANCE m_hInst;
		WORD m_resourceID;
	} params;
	pinDialog &m_dlg;
	HWND m_hwnd;
	std::vector<char,locked_allocator < char > > m_buffer;
	iconHandle *dlgIcon,*appIcon;
	pinDialogPriv(pinDialog &ref,const void * opsysParam) : 
		m_buffer(20,'0'),m_dlg(ref),dlgIcon(NULL),appIcon(NULL) {
		params = *((pinDialogPriv_a*) opsysParam);
		}
	~pinDialogPriv() {
		if (dlgIcon) delete dlgIcon;
		if (appIcon) delete appIcon;
		if (m_hwnd) DestroyWindow(m_hwnd);
		}
	virtual LRESULT on_message(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
	virtual LRESULT on_command(WPARAM wParam, LPARAM lParam);
	virtual LRESULT on_init_dlg(WPARAM wParam);
	static LRESULT CALLBACK dialogProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK nonmodalDialogProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam );
	bool doDialog();
	bool showPrompt(std::string,bool allowRetry=false);
	bool pinDialogPriv::doNonmodalNotifyDlg(bool messageLoop);
	PinString getPin();
private:
	const pinDialogPriv &operator=(const pinDialogPriv &o);
};

pinDialogPriv::_icontag pinDialogPriv::iconSet[2] = {{"cryptui.dll", 4998},{"cryptui.dll" ,3425}};

LRESULT pinDialogPriv::on_init_dlg(WPARAM wParam) {
	SetWindowPos(m_hwnd, HWND_TOPMOST, 10,10,0,0,SWP_NOSIZE);
	SetDlgItemTextA(m_hwnd,IDC_STATIC, m_dlg.m_prompt.c_str() );
	if (!m_dlg.m_displayName.empty())
		SetDlgItemTextA(m_hwnd,IDC_STATIC, m_dlg.m_displayName.c_str() );

	SendDlgItemMessage( m_hwnd, IDC_PININPUT , EM_SETLIMITTEXT, 12, 0 );

	dlgIcon = new iconHandle(iconSet[m_dlg.m_key].module,iconSet[m_dlg.m_key].id);
	appIcon = new iconHandle("shell32",48);
	SendDlgItemMessage(m_hwnd,IDI_DLGICON,STM_SETIMAGE,IMAGE_ICON,(LPARAM)*dlgIcon);
	SendMessage(m_hwnd,WM_SETICON,(WPARAM) ICON_SMALL,(LPARAM) *appIcon);
	SendMessage(m_hwnd,WM_SETICON,(WPARAM) ICON_BIG,(LPARAM) *appIcon);
	if (GetDlgCtrlID((HWND) wParam) != IDC_PININPUT) {
		SetFocus(GetDlgItem(m_hwnd, IDC_PININPUT));
		return FALSE;
		}
	return TRUE;
}

LRESULT pinDialogPriv::on_command(WPARAM wParam, LPARAM lParam) {
	switch (LOWORD(wParam)) {
		case IDC_PININPUT: {
			if (HIWORD(wParam) == EN_CHANGE) {
				GetDlgItemTextA(m_hwnd,IDC_PININPUT,&m_buffer[0],(int)m_buffer.size());
				if (lstrlenA(&m_buffer[0]) >= (LONG ) m_dlg.m_minLen) {
					EnableWindow(GetDlgItem(m_hwnd,IDOK),TRUE);
					SendMessage(m_hwnd,DM_SETDEFID,IDOK,0);
					}
				else {
					EnableWindow(GetDlgItem(m_hwnd,IDOK),FALSE);
					SendMessage(m_hwnd,DM_SETDEFID,IDCANCEL,0);
					}
				}
			break;
			}
		case IDOK:
		case IDCANCEL:
			GetDlgItemTextA(m_hwnd,IDC_PININPUT,&m_buffer[0],(int)m_buffer.size());
			EndDialog (m_hwnd,wParam );
			return TRUE;
		}
	return FALSE;
	}

LRESULT pinDialogPriv::on_message(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
  switch (msg) {
	case WM_INITDIALOG:
		return on_init_dlg(wParam);
	case WM_COMMAND:
		return on_command(wParam,lParam);
	case WM_SYSCOMMAND:
		if (wParam == SC_CLOSE) EndDialog (hwnd, IDCANCEL );
	}
  return FALSE;
}

LRESULT CALLBACK pinDialogPriv::dialogProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
	pinDialogPriv *dlg = NULL;

	if (message == WM_INITDIALOG) {
		SetWindowLongPtr(hwnd, DWLP_USER, (LONG)lParam);
		dlg = (pinDialogPriv *)lParam;
		if (!dlg) return TRUE;
		dlg->m_hwnd = hwnd;
		}
	dlg = (pinDialogPriv *)GetWindowLongPtr(hwnd, DWLP_USER);
	if (!dlg)
	    return FALSE;
	return dlg->on_message(hwnd, message, wParam, lParam);
	}

bool pinDialogPriv::doDialog() {
	if (IDOK == DialogBoxParam(params.m_hInst,MAKEINTRESOURCE(params.m_resourceID)
		,GetForegroundWindow(),
		(DLGPROC)dialogProc, (LPARAM) this)) return true;
	return false;
	}

bool pinDialogPriv::showPrompt(std::string prompt,bool allowRetry) {
	int code = MessageBoxA(GetForegroundWindow(),prompt.c_str(),
		"PIN message", (allowRetry ? MB_RETRYCANCEL : MB_OK ) | MB_ICONHAND );
	return (IDRETRY == code  || IDOK == code);
	}

PinString pinDialogPriv::getPin() {
	return PinString(&m_buffer[0],&m_buffer[0]+strlen(&m_buffer[0]));
	}

LRESULT CALLBACK pinDialogPriv::nonmodalDialogProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam ) {
	switch(msg) {
		case WM_CREATE: {
			NONCLIENTMETRICSA ncMetrics;
			ZeroMemory(&ncMetrics,sizeof(ncMetrics));
			ncMetrics.cbSize = sizeof(ncMetrics);
			SystemParametersInfoA(SPI_GETNONCLIENTMETRICS,sizeof(ncMetrics),&ncMetrics,0);
			HFONT hFont = CreateFontIndirectA(&ncMetrics.lfCaptionFont);
			CREATESTRUCT * ptr = (CREATESTRUCT *) lParam;
			pinDialogPriv *dlg = (pinDialogPriv *) ptr->lpCreateParams;

			std::string tmp = dlg->m_dlg.m_displayName + " ID-card PIN" + (dlg->m_dlg.m_key == EstEidCard::AUTH ? "1" : "2") ;
			HWND w1 = CreateWindowExA(0, "STATIC", tmp.c_str(), 
					WS_VISIBLE | WS_CHILD, 50,  5, 240, 25, hwnd, (HMENU)1, NULL, NULL);
			HWND w2 = CreateWindowExA(0, "STATIC", dlg->m_dlg.m_PAD_prompt.c_str(), 
					WS_VISIBLE | WS_CHILD, 50, 30, 240, 25, hwnd, (HMENU)1, NULL, NULL);
			SendMessage(w1, WM_SETFONT, (WPARAM)hFont, TRUE);
			SendMessage(w2, WM_SETFONT, (WPARAM)hFont, TRUE);

			iconHandle *dlgIcon = new iconHandle(iconSet[dlg->m_dlg.m_key].module,iconSet[dlg->m_dlg.m_key].id);
			HWND b = CreateWindowExA(0, "STATIC","",	SS_CENTERIMAGE | SS_REALSIZEIMAGE | SS_ICON | WS_CHILD | WS_VISIBLE,
				5,5,40,40,hwnd,(HMENU)-1,	NULL,	NULL);
			SendMessage(b, STM_SETIMAGE,(WPARAM)IMAGE_ICON, (LPARAM)*dlgIcon);
			break;
			}
		case WM_COMMAND:
		case WM_CLOSE:
			DestroyWindow(hwnd);
			break;       
			}
	return (DefWindowProc(hwnd, msg, wParam, lParam));
}

bool pinDialogPriv::doNonmodalNotifyDlg(bool messageLoop) {
	MSG  msg ;    
	HWND hwnd = GetForegroundWindow();

	WNDCLASSEXA wc = {0};
	wc.cbSize           = sizeof(WNDCLASSEX);
	wc.lpfnWndProc      = (WNDPROC) nonmodalDialogProc;
	wc.hInstance        = params.m_hInst;
	wc.hbrBackground    = GetSysColorBrush(COLOR_3DFACE);
	wc.lpszClassName    = "DialogClass";
	RegisterClassExA(&wc);

	m_hwnd = CreateWindowExA(WS_EX_DLGMODALFRAME | WS_EX_TOPMOST,  "DialogClass", "PIN message", 
		WS_VISIBLE | WS_SYSMENU | WS_CAPTION , 100, 100, 380, 80, 
		NULL, NULL, params.m_hInst,  this);
	int counter = 20; //always pump some messages, like dialog init
	while( GetMessage(&msg, NULL, 0, 0) && counter--) {
		DispatchMessage(&msg);
		if (messageLoop) counter = 20; 
		}

	return true;
	}

#endif

#ifdef linux
#include <gtkmm/inputdialog.h>
#include <gtkmm/entry.h>
#include <gtkmm/stock.h>
#include <string.h>

struct pinDialogPriv : public Gtk::Dialog {
    Gtk::Entry m_textInput;
    Gtk::Label m_label;
    pinDialogPriv(pinDialog &,const void *opsysParam) :
        Gtk::Dialog("inputDialog",true), m_label("pin entry")
        {
        m_textInput.set_activates_default(true);
        get_vbox()->pack_start(m_label);
        m_label.set_alignment(0.1,0.5);
        get_vbox()->pack_start(m_textInput);
        m_textInput.set_visibility(false);
        set_has_separator(true);
        add_button(Gtk::Stock::OK,Gtk::RESPONSE_OK);
        add_button(Gtk::Stock::CANCEL ,Gtk::RESPONSE_CANCEL);
        set_default_response(Gtk::RESPONSE_CANCEL);
        show_all_children();
        }
    PinString getPin() {
        return m_textInput.get_text().c_str();
        }
    std::string m_prompt;
    char m_buffer[20];
    bool doDialog();
	bool showPrompt(std::string prompt,bool allowRetry=false) {
		}
};

bool pinDialogPriv::doDialog() {
    if (run()==Gtk::RESPONSE_OK) return true;
    return false;
    }

#endif

pinDialog::pinDialog(const void * opsysParam,std::string prompt) : m_minLen(4),
	m_key((EstEidCard::KeyType)0) {
	d = new pinDialogPriv(*this,opsysParam);
	m_prompt = prompt;
	}

pinDialog::pinDialog(const void * opsysParam,EstEidCard::KeyType key) : m_key(key) {
	d = new pinDialogPriv(*this,opsysParam);
	if (m_key == EstEidCard::AUTH) {
		m_prompt = "Enter ID-card (PIN1)";
		m_minLen = 4;
		}
	else if (m_key == EstEidCard::SIGN) {
		m_prompt = "Enter ID-card (PIN2)";
		m_minLen = 5;
		}
	else
		throw std::invalid_argument("pinDialog:Invalid keytype specified");
	m_PAD_prompt = m_prompt + " from PINpad";
	}

pinDialog::~pinDialog() {
	delete d;
	}

bool pinDialog::doDialog() {
	return d->doDialog();
	}

bool pinDialog::showPrompt(std::string prompt,bool allowRetry) {
	return d->showPrompt(prompt,allowRetry);
	}

bool pinDialog::doDialogInloop(pinOpInterface &operation,PinString &authPinCache) {
	for(;;) {
		byte retries = 0;
		try {
				PinString pin;
				if (authPinCache.empty()) {
					if (!doDialog())
						throw std::runtime_error("User cancelled");
					pin = getPin();
				} else
					pin = authPinCache;
				mutexObjLocker lock(operation.m_mutex);
				if (m_key == EstEidCard::AUTH)
					operation.m_card.validateAuthPin(pin,retries);
				else
					operation.m_card.validateSignPin(pin,retries);
				operation.call(operation.m_card,pin,m_key);
				authPinCache = pin;
				return true;
		} catch(AuthError &auth) {
			authPinCache.clear();
			if (auth.m_blocked) {
				showPrompt("Wrong pin entered, PIN is blocked");
				throw std::runtime_error("PIN is blocked");
				}
			std::stringstream buf;
			buf << "Wrong pin entered, " << (int)retries << " retries left";
			if (!showPrompt(buf.str(),true))
				throw std::runtime_error("User cancelled");
			}
		}
	}

PinString pinDialog::getPin() {
	return d->getPin();
	}

void pinDialog::SetDisplayName(std::string name) {
	m_displayName = name;
	}

bool pinDialog::doNonmodalNotifyDlg(bool messageLoop) {
	return d->doNonmodalNotifyDlg(messageLoop);
	}