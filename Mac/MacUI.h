#ifndef H_ESTEID_MacUI
#define H_ESTEID_MacUI

#include "../PluginUI.h"
#include "npapi.h"
#include <stdexcept>

class MacUI : public PluginUI {
public:
    MacUI();
    virtual ~MacUI();
	
    virtual std::string PromptForSignPIN(std::string subject,
										 std::string docUrl, std::string docHash,
										 std::string pageUrl, int pinPadTimeout,
										 bool retry, int tries);
    virtual void ClosePinPrompt();
    virtual void ShowPinBlockedMessage(int pin);
	virtual void ShowSettings(PluginSettings &conf, std::string pageUrl = "");
	
private:
    /* Pointers to Safari interfaces. */
    void *m_internal;
};

#endif /* H_ESTEID_MacUI */
