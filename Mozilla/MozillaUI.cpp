#include "MozillaUI.h"

#include "xpcom-config.h"

#include "nsIServiceManager.h"
#include "nsIEstEIDPrivate.h"

#include <stdio.h>
#define ESTEID_DEBUG printf

/*class PRUString : public std::basic_string<PRUnichar,std::char_traits<PRUnichar> >
{
} */
typedef std::basic_string<PRUnichar,std::char_traits<PRUnichar> > PRUString;

MozillaUI::MozillaUI(void *smp, void *dwp)
{
    nsIEstEIDPrivate  * eidp = NULL;
    nsIServiceManager * sm = NULL;
    nsIDOMWindow      * dw = NULL;
    nsISupports       * smiip = static_cast<nsISupports*>(smp);
    nsISupports       * dwiip = static_cast<nsISupports*>(dwp);

    if(!smp ||  !dwp)
        throw std::runtime_error("MozillaUI(): invalid arguments");

    /* Get nsIServiceManager */
    smiip->QueryInterface(NS_GET_IID(nsIServiceManager), (void**)&sm);
    NS_RELEASE(smiip);
    if(!sm) {
        NS_RELEASE(dwiip);
        throw std::runtime_error("MozillaUI(): QI(nsIServiceManager) failed");
    }

    /* Get nsIDOMWindow */
    dwiip->QueryInterface(NS_GET_IID(nsIDOMWindow), (void**)&dw);
    NS_RELEASE(dwiip);
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
    nsISupports *tmp;

    tmp = static_cast<nsISupports *>(m_dw);
    NS_IF_RELEASE(tmp);
    tmp = static_cast<nsISupports *>(m_eid);
    NS_IF_RELEASE(tmp);
    ESTEID_DEBUG("~MozillaUI()\n");
}


std::string MozillaUI::PromptForSignPIN(std::string subject,
        std::string docUrl, std::string docHash,
        std::string pageUrl, int pinPadTimeout, bool retry, int tries)
{
    nsIEstEIDPrivate *eidp = static_cast<nsIEstEIDPrivate *>(m_eid);
    nsIDOMWindow     *dw   = static_cast<nsIDOMWindow *>(m_dw);
#if 0
    eidp->PromptForSignPIN(dw,
        reinterpret_cast<const PRUnichar *>(L"Peeter Pakiraam"),
        (const char *)"http://www.kala.ee", (const char *)"hash: 1234567890",
        reinterpret_cast<const PRUnichar *>(L"https://www.pähh.com"),
        0, false, 3, (char **)&pin);
#endif
}

void MozillaUI::ClosePinPrompt()
{
    nsIEstEIDPrivate *eidp = static_cast<nsIEstEIDPrivate *>(m_eid);
    nsIDOMWindow     *dw   = static_cast<nsIDOMWindow *>(m_dw);

    eidp->ClosePinPrompt(dw);
}

void MozillaUI::ShowPinBlockedMessage(int pin)
{
    nsIEstEIDPrivate *eidp = static_cast<nsIEstEIDPrivate *>(m_eid);
    nsIDOMWindow     *dw   = static_cast<nsIDOMWindow *>(m_dw);

    eidp->ShowPinBlockedMessage(dw, pin);
}

void MozillaUI::ShowSettings(std::string pageUrl)
{
    nsIEstEIDPrivate *eidp = static_cast<nsIEstEIDPrivate *>(m_eid);
    nsIDOMWindow     *dw   = static_cast<nsIDOMWindow *>(m_dw);

    // FIXME: Proper UTF-8 to UTF-16 conversion required
    PRUString tmp(pageUrl.length(),0);
    std::copy(pageUrl.begin(), pageUrl.end(), tmp.begin());
    eidp->ShowSettings(dw, tmp.c_str());
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
