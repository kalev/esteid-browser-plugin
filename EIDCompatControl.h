// EIDCompatControl.h : Declaration of the CEIDCompatControl
#ifndef H_EIDCOMPATCONTROL
#define H_EIDCOMPATCONTROL

#include "FBControl.h"
#include "EIDCompatControl_res.h"
#include "debug.h"

class ATL_NO_VTABLE CEIDCompatControl : 
    public CFBControl
{
public:
    CEIDCompatControl() : CFBControl() {
	    ESTEID_DEBUG("Loaded ActiveX control via compatibility CLSID / PROGID");
	}
	
DECLARE_REGISTRY_RESOURCEID_EX(IDR_EIDCOMPATCONTROL)
};

OBJECT_ENTRY_AUTO(__uuidof(EIDCompatControl), CEIDCompatControl)
#endif