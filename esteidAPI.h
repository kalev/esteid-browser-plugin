/*
 * esteid-browser-plugin - a browser plugin for Estonian EID card
 *
 * Copyright (C) 2010  Estonian Informatics Centre
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

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

// FIXME: Find a sane way to deal with error messages
#define CANCEL_MSG "User cancelled operation"

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
    void signAsync(std::string hash, std::string url, const FB::JSObject callback);

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

    void onPinEntered(std::string hash);

    /**  Will fire event handlers
    *        - CardInserted
    *        - CardRemoved
    *        - ReadersChanged
    */
    virtual void onMessage(EstEIDService::msgType, readerID);

#ifdef SUPPORT_OLD_APIS
    std::string sign(std::string, std::string);
    std::string promptForPin(bool retrying = false);
    std::string askPinAndSign(const std::string& hash, const std::string& url);
    std::string getCertificates();
    std::string getInfo();
    std::string getSigningCertificate();
    std::string getSignedHash(std::string, std::string);
    std::string get_selectedCertNumber();
    void prepare(std::string, std::string, std::string);
    void finalize(std::string, std::string, std::string, std::string, std::string);
#endif


private:
    FB::AutoPtr<FB::BrowserHostWrapper> m_host;
    boost::shared_ptr<PluginUI> m_UI;
    FB::JSOutObject m_authCert;
    FB::JSOutObject m_signCert;
    FB::JSOutObject m_settingsCallback;
    FB::JSOutObject m_closeCallback;
    FB::JSObject m_barJSO;
    FB::JSObject m_signCallback;
    EstEIDService *m_service;
    vector <std::string> m_pdata;
    std::string m_pageURL;
    std::string m_subject;
    std::string m_hash;
    std::string m_url;
    bool m_pinpad;
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

    class UICallback : public PluginUI::UICallbacks {
    public:
        UICallback(esteidAPI &eidp) : m_eidp(eidp) { }
        virtual void onPinEntered(const std::string& p) { m_eidp.onPinEntered(p); }
        virtual void onPinCancelled() { m_eidp.returnSignFailure(CANCEL_MSG); }

    private:
        esteidAPI &m_eidp;
    };
    boost::shared_ptr<PluginUI::UICallbacks> m_uiCallback;

    std::string GetHostName(void);
    std::string GetPageURL(void);
    PluginUI* GetMozillaUI(void);
    void UpdatePersonalData(void);
    void prepareSign(const std::string& hash, const std::string& url);
    void promptForPinAsync(bool retrying = false);
    std::string signSHA1(const std::string& hash, const std::string& pin);
    int getPin2RetryCount();
    void ShowSettings(void);
    void DisplayNotification(std::string msg);
    void DisplayError(std::string msg);
    void CreateNotificationBar(void);
    void OpenNotificationBar(void);
    void CloseNotificationBar(void);
    bool IsSecure(void);
    bool IsLocal(void);
    bool IsWhiteListed(void);
    void returnSignedData(const std::string& data);
    void returnSignFailure(const std::string& msg);

    std::string EstEIDNotificationBarScript;
    static std::string subjectToHumanReadable(std::string& subject);
    static std::vector<std::string> stringSplit(std::string str, std::string separator);
    static std::string iconvConvert(const std::string&, const char*, const char*);
    static std::string CP1252_to_UTF8(const std::string&);
};
