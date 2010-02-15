#include <string>
#include <sstream>
#include "JSAPIAuto.h"
#include "DOM/JSAPI_DOMElement.h"
#include "BrowserHostWrapper.h"
#include "NpapiBrowserHost.h"
#include "CertificateAPI.h"
#include "PluginUI.h"
#include "EstEIDService.h"
#include "CallbackAPI.h"

class esteidAPI : public FB::JSAPIAuto, EstEIDService::messageObserver
{
public:
    esteidAPI(FB::BrowserHostWrapper *host);
    virtual ~esteidAPI();

    /** get plugin version 
      * @returns string version number
      */
    std::string getVersion();

    /** Sign hash with digital signature certificate
     * Browser will automatically prompt for PIN2
     * @param hash HEX encoded document hash to sign
     * @param url an URL to the document itself
     */
    std::string sign(std::string hash, std::string url);

    /** Certificates (read-only properties) */
    FB::JSOutObject get_authCert();
    FB::JSOutObject get_signCert();

    /** Personal data file attributes (read-only properties) */
    std::string get_lastName();
    std::string get_firstName();
    std::string get_middleName();
    std::string get_sex();
    std::string get_citizenship();
    std::string get_birthDate();
    std::string get_personalID();
    std::string get_documentID();
    std::string get_expiryDate();
    std::string get_placeOfBirth();
    std::string get_issuedDate();
    std::string get_residencePermit();
    std::string get_comment1();
    std::string get_comment2();
    std::string get_comment3();
    std::string get_comment4();

   /**  Will fire event handlers
   *        - OnCardInserted
   *        - OnCardRemoved
   *        - OnReadersChanged
   */
   virtual void onMessage(EstEIDService::msgType, readerID);

private:
    FB::AutoPtr<FB::BrowserHostWrapper> m_host;
    FB::AutoPtr<PluginUI> m_UI;
    FB::JSOutObject m_authCert;
    FB::JSOutObject m_signCert;
    FB::JSOutObject m_settingsCallback;
    FB::JSOutObject m_closeCallback;
    FB::JSObject m_msgElement;
    FB::JSObject m_barElement;
    EstEIDService *m_service;
    vector <std::string> m_pdata;
    std::string m_pageURL;
    PluginSettings m_conf; // TODO: Optimize this?

    class SettingsCallback : public CallbackAPI {
    public:
        SettingsCallback(FB::BrowserHostWrapper *host, esteidAPI &eidp) : 
            CallbackAPI(host), m_eidp(eidp) { }
        virtual bool eventHandler()
            { m_eidp.ShowSettings(); return true; };
    private:
        esteidAPI &m_eidp;
    };

    class CloseCallback : public CallbackAPI {
    public:
        CloseCallback(FB::BrowserHostWrapper *host, esteidAPI &eidp) : 
            CallbackAPI(host), m_eidp(eidp) { }
        virtual bool eventHandler()
            { m_eidp.CloseNotificationBar(); return true; };
    private:
        esteidAPI &m_eidp;
    };

    std::string GetHostName(void);
    std::string GetPageURL(void);
    PluginUI* GetMozillaUI(void);
    void UpdatePersonalData(void);
    int getPin2RetryCount();
    void ShowSettings(void);
    void DisplayNotification(std::string msg);
    void CreateNotificationBar(void);
    void OpenNotificationBar(void);
    void CloseNotificationBar(void);
    bool IsSecure(void);
    bool IsLocal(void);
    bool IsWhiteListed(void);

    static std::string subjectToHumanReadable(std::string& subject);
    static std::vector<std::string> stringSplit(std::string str, std::string separator);
    static std::string iconvConvert(const std::string&, const char*, const char*);
    static std::string CP1252_to_UTF8(const std::string&);
};
