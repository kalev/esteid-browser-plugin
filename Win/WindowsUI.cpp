#include "WindowsUI.h"

#include <stdio.h>
#include <vector>

#include "Win/PluginWindowWin.h"
#include "utility/pinDialog.h"

#include "debug.h"

WindowsUI::WindowsUI()
{
    ESTEID_DEBUG("WindowsUI intialized");
}

WindowsUI::~WindowsUI()
{
    ESTEID_DEBUG("~WindowsUI()");
}


std::string WindowsUI::PromptForSignPIN(std::string subject,
        std::string docUrl, std::string docHash,
        std::string pageUrl, int pinPadTimeout, bool retry, int tries)
{
    ESTEID_DEBUG("WindowsUI::PromptForSignPIN()");

    PinString pin;

    pinDialogPriv_l params = {
	    ATL::_AtlBaseModule.GetResourceInstance(),
            IDD_PIN_DIALOG_ENG
    };

    pinDialog dlg(&params, EstEidCard::SIGN);
    if (!dlg.doDialog())
        return "";

    pin = dlg.getPin();
    return pin.c_str();
}


void WindowsUI::ClosePinPrompt()
{
    ESTEID_DEBUG("WindowsUI::ClosePinPrompt()");
}


void WindowsUI::ShowPinBlockedMessage(int pin)
{
    ESTEID_DEBUG("WindowsUI::ShowPinBlockedMessage()");

    pinDialogPriv_l params = { NULL, NULL };
    pinDialog dlg(&params, "");
    dlg.showPrompt("PIN2 blocked.\nPlease run ID-card Utility to unlock the PIN.", false);
}


void WindowsUI::ShowSettings(PluginSettings &conf, std::string pageUrl)
{
    ESTEID_DEBUG("WindowsUI::ShowSettings()");
}
