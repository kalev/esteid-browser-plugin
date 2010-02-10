#ifndef WINDOWSUI_H
#define WINDOWSUI_H

#include "PluginUI.h"
#include "Win/win_common.h"

struct pinDialogPriv_l {
    HINSTANCE m_hInst;
    WORD m_resourceID;
};


class WindowsUI : public PluginUI {
public:
    WindowsUI();
    virtual ~WindowsUI();

    virtual std::string PromptForSignPIN(std::string subject,
        std::string docUrl, std::string docHash,
        std::string pageUrl, int pinPadTimeout,
        bool retry, int tries);
    virtual void ClosePinPrompt();
    virtual void ShowSettings(PluginSettings &conf, std::string pageUrl = "");
    virtual void ShowPinBlockedMessage(int pin);
};

#endif /* WINDOWSUI_H */
