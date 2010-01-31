#include "MozillaUI.h"

#include "xpcom-config.h"

#include "nsIServiceManager.h"
#include "nsIEstEIDPrivate.h"

#include <stdio.h>
#define ESTEID_DEBUG printf

typedef std::basic_string<PRUnichar,std::char_traits<PRUnichar> > PRUStr;

/** String composed of 16bit PRUnichars */
class PRUString : public PRUStr {
public:
    PRUString(std::string utf8str) : PRUStr(utf8str.length(), 0) {
        // FIXME: Proper UTF-8 to UTF-16 conversion required
        std::copy(utf8str.begin(), utf8str.end(), this->begin());
    }
};

MozillaUI::MozillaUI(nsISupports *smp, nsISupports *dwp)
{
    nsIEstEIDPrivate  * eidp = NULL;
    nsIServiceManager * sm = NULL;
    nsIDOMWindow      * dw = NULL;

    if(!smp ||  !dwp)
        throw std::runtime_error("MozillaUI(): invalid arguments");

    /* Get nsIServiceManager */
    smp->QueryInterface(NS_GET_IID(nsIServiceManager), (void**)&sm);
    NS_RELEASE(smp);
    if(!sm) {
        NS_RELEASE(dwp);
        throw std::runtime_error("MozillaUI(): QI(nsIServiceManager) failed");
    }

    /* Get nsIDOMWindow */
    dwp->QueryInterface(NS_GET_IID(nsIDOMWindow), (void**)&dw);
    NS_RELEASE(dwp);
    if(!dw) {
        NS_RELEASE(sm);
        throw std::runtime_error("MozillaUI(): QI(nsIDOMWindow) failed");
    }

    /* Get access to Mozilla UI XPCOM component */
    sm->GetServiceByContractID(NS_ESTEIDPRIVATE_CONTRACTID,
        NS_GET_IID(nsIEstEIDPrivate), (void **)&eidp);
    NS_RELEASE(sm);

    if(!eidp) {
        NS_RELEASE(dw);
        throw std::runtime_error("MozillaUI(): unable to access " NS_ESTEIDPRIVATE_CONTRACTID " service");
    }

    m_eid = eidp;
    m_dw = dw;
    ESTEID_DEBUG("MozillaUI intialized\n");
}

MozillaUI::~MozillaUI()
{
    NS_IF_RELEASE(m_dw);
    NS_IF_RELEASE(m_eid);
    ESTEID_DEBUG("~MozillaUI()\n");
}


std::string MozillaUI::PromptForSignPIN(std::string subject,
        std::string docUrl, std::string docHash,
        std::string pageUrl, int pinPadTimeout, bool retry, int tries)
{
    PRUString t_subject(subject);
    PRUString t_pageUrl(pageUrl);
    char *pin;

    m_eid->PromptForSignPIN(m_dw, t_subject.c_str(), docUrl.c_str(),
                            docHash.c_str(), t_pageUrl.c_str(),
                            pinPadTimeout, retry, tries, (char **)&pin);

    return (pin) ? pin : "";
}

void MozillaUI::ClosePinPrompt()
{
    m_eid->ClosePinPrompt(m_dw);
}

void MozillaUI::ShowPinBlockedMessage(int pin)
{
    m_eid->ShowPinBlockedMessage(m_dw, pin);
}

void MozillaUI::ShowSettings(std::string pageUrl)
{
    PRUString tmp(pageUrl);
    m_eid->ShowSettings(m_dw, tmp.c_str());
}

#if 0
    // Bitrot!

    //std::wstring str = L"https://www.pähh.com/";
    std::string str = "https://www.pähh.com/";
    std::basic_string<PRUnichar,std::char_traits<PRUnichar> > buf(str.length(),0);
    std::copy(str.begin(), str.end(), buf.begin());

    //std::string str = "Hello";
    //std::wstring str2(str.length(), L' ');
    //const wchar_t *buf = L"https://www.pähh.com/";
    //eidp->ShowSettings(dw, reinterpret_cast<const PRUnichar *>(buf));
    eidp->ShowSettings(dw, buf.c_str());

    char pin[1024];
    eidp->PromptForSignPIN(dw,
        reinterpret_cast<const PRUnichar *>(L"Peeter Pakiraam"),
        (const char *)"http://www.kala.ee", (const char *)"hash: 1234567890",
        reinterpret_cast<const PRUnichar *>(L"https://www.pähh.com"),
        0, false, 3, (char **)&pin);

    ESTEID_DEBUG("getMozUI: Got PIN: %s\n", pin);
#endif
