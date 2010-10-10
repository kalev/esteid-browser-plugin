/*
 * esteid-browser-plugin - a browser plugin for Estonian EID card
 *
 * Copyright (C) 2010  Estonian Informatics Centre
 * Copyright (C) 2010  Smartlink OÜ
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
#include "DOM/Element.h"
#include "BrowserHost.h"
#include "NpapiBrowserHost.h"
#include "CertificateAPI.h"
#include "PluginUI.h"
#include "EstEIDService.h"
#include "CallbackAPI.h"
#include "urlparser.h"

// FIXME: Find a sane way to deal with error messages
#define CANCEL_MSG "User cancelled operation"

class EsteidAPI : public FB::JSAPIAuto, EstEIDService::messageObserver
{
public:
    EsteidAPI(FB::BrowserHostPtr host);
    virtual ~EsteidAPI();

    void setWindow(FB::PluginWindow*);

    /** get plugin version 
      * @returns string version number
      */
    std::string getVersion();

    /** Sign hash with digital signature certificate
     * Browser will automatically prompt for PIN2
     * @param hash HEX encoded document hash to sign
     * @param url an URL to the document itself
     */
    void signAsync(const std::string& hash, const std::string& url, const FB::JSObjectPtr& callback);

    /** Open settings window.
      * Only available for file:// and chrome:// URL-s
      */
    void showSettings();

    /** Certificates (read-only properties) */
    FB::JSAPIPtr get_authCert();
    FB::JSAPIPtr get_signCert();

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

    void onPinEntered(const std::string& hash);

    /**  Will fire event handlers
    *        - CardInserted
    *        - CardRemoved
    *        - ReadersChanged
    */
    virtual void onMessage(EstEIDService::msgType, readerID);

#ifdef SUPPORT_OLD_APIS
    void deprecatedCall();
    std::string sign(const std::string&, const std::string&);
    std::string promptForPin(bool retrying = false);
    std::string askPinAndSign(const std::string& hash, const std::string& url);
    std::string getCertificates();
    std::string getInfo();
    std::string getSigningCertificate();
    std::string getSignedHash(const std::string&, const std::string&);
    std::string get_selectedCertNumber();
    void prepare(const std::string&, const std::string&, const std::string&);
    void finalize(const std::string&, const std::string&, const std::string&, const std::string&, const std::string&);
    bool isActive();
#endif


private:
    FB::BrowserHostPtr m_host;
    boost::shared_ptr<PluginUI> m_UI;
    UrlParser m_pageURL;
    FB::JSAPIPtr m_authCert;
    FB::JSAPIPtr m_signCert;
    FB::JSAPIPtr m_settingsCallback;
    FB::JSAPIPtr m_closeCallback;
    FB::JSObjectPtr m_barJSO;
    FB::JSObjectPtr m_signCallback;
    EstEIDService *m_service;
    vector <std::string> m_pdata;
    std::string m_subject;
    std::string m_hash;
    std::string m_url;
    bool m_pinpad;
    PluginSettings m_conf; // TODO: Optimize this?

    class SettingsCallback : public CallbackAPI {
    public:
        SettingsCallback(FB::BrowserHostPtr host, EsteidAPI &eidp) :
            CallbackAPI(host), m_eidp(eidp) { }
        virtual bool eventHandler()
            { m_eidp.ShowSettings(); return true; };
    private:
        EsteidAPI &m_eidp;
    };

    class CloseCallback : public CallbackAPI {
    public:
        CloseCallback(FB::BrowserHostPtr host, EsteidAPI &eidp) :
            CallbackAPI(host), m_eidp(eidp) { }
        virtual bool eventHandler()
            { m_eidp.CloseNotificationBar(); return true; };
    private:
        EsteidAPI &m_eidp;
    };

    class UICallback : public PluginUI::UICallbacks {
    public:
        UICallback(EsteidAPI &eidp) : m_eidp(eidp) { }
        virtual void onPinEntered(const std::string& p) { m_eidp.onPinEntered(p); }
        virtual void onPinCancelled() { m_eidp.returnSignFailure(CANCEL_MSG); }

    private:
        EsteidAPI &m_eidp;
    };
    boost::shared_ptr<PluginUI::UICallbacks> m_uiCallback;

    std::string pageURL();
    void UpdatePersonalData();
    void prepareSign(const std::string& hash, const std::string& url);
    void promptForPinAsync(bool retrying = false);
    std::string signSHA1(const std::string& hash, const std::string& pin);
    int getPin2RetryCount();
    void ShowSettings();
    void DisplayNotification(const std::string& msg);
    void DisplayError(const std::string& msg);
    void CreateNotificationBar();
    void OpenNotificationBar();
    void CloseNotificationBar();
    bool IsSecure();
    bool IsLocal();
    bool IsWhiteListed();
    void whitelistRequired();
    void returnSignedData(const std::string& data);
    void returnSignFailure(const std::string& msg);

    std::string EstEIDNotificationBarScript;
    static std::string subjectToHumanReadable(const std::string& subject);
};
