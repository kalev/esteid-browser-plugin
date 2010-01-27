#ifndef H_ESTEID_PLUGINUI
#define H_ESTEID_PLUGINUI

#include <string>

/** A base class for different UI implementations */
class PluginUI
{
public:
    /** Prompt for Signature PIN */
    virtual std::string PromptForSignPIN(std::string subject,
        std::string docUrl, std::string docHash,
        std::string pageUrl, int pinPadTimeout,
        bool retry, int tries) = 0;

    /** Close Signature PIN prompt (used when user hits cancel on pinpad) */
    virtual void ClosePinPrompt() = 0;

    /** Inform user that the PIN has been blocked */
    virtual void ShowPinBlockedMessage(int pin) = 0;

    /** Open Settings dialog */
    //FIXME: how to pass whitelist?
    virtual void ShowSettings(std::string pageUrl = "") = 0;

    PluginUI(void);
    virtual ~PluginUI(void);

    // Support Reference counting
    void AddRef();
    unsigned int Release();

protected:
    unsigned int m_refCount;
};
#endif // H_ESTEID_PLUGINUI
