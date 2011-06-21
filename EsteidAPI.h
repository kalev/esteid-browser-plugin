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

#ifndef ESTEIDAPI_H
#define ESTEIDAPI_H

#include <string>
#include <sstream>

#include "localize.h"
#include "JSAPIAuto.h"
#include "DOM/Element.h"
#include "BrowserHost.h"
#include "NpapiBrowserHost.h"
#include "PluginUI.h"
#include "CardService.h"
#include "CallbackAPI.h"
#include "urlparser.h"

// FIXME: Find a sane way to deal with error messages
#define CANCEL_MSG _("User cancelled operation")

class EsteidAPI : public FB::JSAPIAuto, CardService::MessageObserver
{
public:
    EsteidAPI(FB::BrowserHostPtr host, const std::string& mimetype);
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

    /** Personal data file (read-only properties) */
    FB::JSAPIPtr get_personalData();

    /** Last error code (not implemented yet)
      * @returns int error code
      */
    int get_errorCode();

    /** Last error message (not implemented yet)
      * @returns string error message
      */
    std::string get_errorMessage();

    void onPinEntered(const std::string& hash);

    /**  Will fire event handlers
    *        - CardInserted
    *        - CardRemoved
    *        - ReadersChanged
    */
    virtual void onMessage(CardService::MsgType, ReaderID);

#ifdef SUPPORT_OLD_APIS
    struct sign_method_wrapper {
        typedef FB::variant result_type;
        sign_method_wrapper() {}
        FB::variant operator()(EsteidAPI* instance, const FB::VariantList& in)
        {
            /* Sign method with two arguments
             * (legacy Mozilla plugin, or SK leakplugin) */
            if(in.size() == 2) {
                return instance->sign(
                    FB::detail::methods::convertArgumentSoft<std::string>(in, 1),
                    FB::detail::methods::convertLastArgument<std::string>(in, 2)
                );
            }
            /* Sign method with 3 arguments (SK leakplugin) */
            if(in.size() == 3) {
                return instance->signSK("",
                    FB::detail::methods::convertArgumentSoft<std::string>(in, 2));
            }
            /* Sign method with 6 arguments (legacy XMLSignApplet) */
            if(in.size() == 6) {
                instance->signXML(
                    FB::detail::methods::convertArgumentSoft<std::string>(in, 1),
                    FB::detail::methods::convertArgumentSoft<std::string>(in, 2),
                    FB::detail::methods::convertArgumentSoft<std::string>(in, 3),
                    FB::detail::methods::convertArgumentSoft<std::string>(in, 4),
                    FB::detail::methods::convertArgumentSoft<std::string>(in, 5),
                    FB::detail::methods::convertLastArgument<std::string>(in, 6)
                );
                return ""; // signXML does not return anything
            }
            // FIXME: Throw error
            return FB::variant();
        }
    };
    std::string sign(const std::string&, const std::string&);
    std::string get_version(); // SK leakplugin
    FB::JSAPIPtr getCertificate(); // SK leakplugin
    FB::VariantList getCertificatesSK(); // SK leakplugin
    std::string signSK(const std::string&, const std::string&,
                       FB::variant = FB::variant()); // SK leakplugin
    std::string getCertificatesMoz(); // Old Mozilla plugin
    FB::variant getCertificates(); // SK leakplugin, Old Mozilla plugin
    void signXML(const std::string&, const std::string&,
        const std::string&, const std::string&, const std::string&,
        const std::string&);
    void throwIfSignFailure();
    std::string askPinAndSign(const std::string& hash, const std::string& url);
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
    boost::shared_ptr<CardService> m_service;
    std::string m_subject;
    std::string m_hash;
    std::string m_url;
    std::string m_mimeType; /* This is here for the future, current Firebreath
                             * (2010/03/11) always passes an empty string */
    bool m_pinpad;
    volatile bool m_stoprequested;
    std::string m_signedHash;
    std::string m_signFailure;
    PluginSettings m_settings;

    class SettingsCallback : public CallbackAPI {
    public:
        SettingsCallback(FB::BrowserHostPtr host, EsteidAPI &eidp) :
            CallbackAPI(host), m_eidp(eidp) { }
        virtual bool eventHandler()
            { m_eidp.settingsDialog(); return true; };
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
    void prepareSign(const std::string& hash, const std::string& url);
    void askPin(bool retrying = false);
    std::string signSHA1(std::string hash, const std::string& pin);
    int getPin2RetryCount();
    void settingsDialog();
    void DisplayNotification(const std::string& msg);
    void CreateNotificationBar();
    void OpenNotificationBar();
    void CloseNotificationBar();
    bool IsSecure();
    bool IsLocal();
    bool IsWhiteListed();
    void whitelistRequired();
    void invokeSignCallback(const std::string& callback, const std::string& data);
    void returnSignedData(const std::string& data);
    void returnSignFailure(const std::string& msg);

    std::string EstEIDNotificationBarScript;
    static void filterWhitespace(std::string& s);
    static std::string subjectToHumanReadable(const std::string& subject);
};

#endif //ESTEIDAPI_H
