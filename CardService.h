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

/**
 * This class helps to implement Card Services.
 * It encapsulates platform specific locking, card event polling
 * thread and synchronized access to cards via a singleton instance.
 *
 * Typical usage:
 *
 * boost::shared_ptr<CardService> service = CardService::getInstance();
 * service->addObserver(this);
 *
 */

#ifndef CARDSERVICE_H_
#define CARDSERVICE_H_

#include <smartcardpp/smartcardpp.h>
#include <boost/scoped_ptr.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/function.hpp>

#define PDATA_MIN EstEidCard::SURNAME
#define PDATA_MAX EstEidCard::COMMENT4

typedef unsigned int ReaderID;

enum SignError {
    SIGN_ERROR_BLOCKED,
    SIGN_ERROR_WRONG_PIN,
    SIGN_ERROR_ABORTED,
    SIGN_ERROR_CARD_ERROR
};

class CardService
{
public:
    // Constructor is protected
    virtual ~CardService();

    /**
     * Get service instance
     */
    static boost::shared_ptr<CardService> getInstance();
    /**
     * Find readers with valid card
     */
    void findEstEid(vector<ReaderID>& readers);
    /**
     * Find the reader with first valid card
     */
    ReaderID findFirstEstEid();
    /**
     * Read personal data file off the first card
     */
    void readPersonalData(vector<std::string>& data);
    /**
     * Read personal data file off the card in specified reader
     */
    void readPersonalData(vector<std::string>& data, ReaderID);
    /**
     * Read authentication certificate off the first card
     */
    ByteVec getAuthCert();
    /**
     * Read authentication certificate off the card in specified reader
     */
    ByteVec getAuthCert(ReaderID);
    /**
     * Read signature certificate off the first card
     */
    ByteVec getSignCert();
    /**
     * Read signature certificate off the card in specified reader
     */
    ByteVec getSignCert(ReaderID);
    /**
     * Retrieve retry counts from the first card
     */
    bool getRetryCounts(byte& puk, byte& pinAuth, byte& pinSign);
    /**
     * Retrieve retry counts from the card in specified reader
     */
    bool getRetryCounts(byte& puk, byte& pinAuth, byte& pinSign, ReaderID);
    /**
     * Check if first card is in a reader with a PinPAD
     */
    bool hasSecurePinEntry();
    /**
     * Check if the specified card is in a reader with a PinPAD
     */
    bool hasSecurePinEntry(ReaderID);
    /**
     * Sign a SHA1 hash with the first card
     */
    std::string signSHA1(const std::string& hash,
                         EstEidCard::KeyType keyId,
                         const std::string& pin);
    /**
     * Sign a SHA1 hash with the specified card
     */
    std::string signSHA1(const std::string& hash,
                         EstEidCard::KeyType keyId,
                         const std::string& pin,
                         ReaderID);

    typedef boost::function<void (const std::string&)> SignCompletedFunc;
    typedef boost::function<void (SignError, const std::string&)> SignFailedFunc;

    void setSignCompletedCallback(SignCompletedFunc f);
    void setSignFailedCallback(SignFailedFunc f);

    void signSHA1Async(const std::string& hash,
                       EstEidCard::KeyType keyId,
                       const std::string& pin,
                       ReaderID reader);

    void signSHA1Async(const std::string& hash,
                       EstEidCard::KeyType keyId,
                       const std::string& pin);

    /* Message observer interface */
    enum MsgType {
        CARD_INSERTED,
        CARD_REMOVED,
        READERS_CHANGED,
        CARD_ERROR
    };
    class MessageObserver {
        friend class CardService;

        virtual void onMessage(MsgType e, ReaderID i) = 0;
    };
    virtual void addObserver(MessageObserver *obs);
    virtual void removeObserver(MessageObserver *obs);

protected:
    CardService();

    /* Get access to smartcard manager instance */
    ManagerInterface& cardManager();

    /* Singleton instance variable */
    static boost::weak_ptr<CardService> sCardService;

    class IdCardCacheEntry {
    public:
        bool cardPresent;
        vector<std::string> m_pData;
        ByteVec m_authCert;
        ByteVec m_signCert;
        void purge() { cardPresent = false; m_pData.clear(); m_authCert.clear(); m_signCert.clear(); }
        IdCardCacheEntry() : cardPresent(false) {}
    };
    typedef vector<IdCardCacheEntry> IdCardCache;

    IdCardCache m_cache;
    vector<MessageObserver *> m_observers;
    virtual void postMessage(MsgType e, ReaderID i);

private:
    // Declarations only for copy constructor and assignment operator
    CardService(const CardService&);
    CardService& operator=(const CardService&);

    void findEstEid();
    void monitor();
    void poll();
    bool readerHasCard(EstEidCard& card, ReaderID i);
    void runSignSHA1(const std::string& hash,
                     EstEidCard::KeyType keyId,
                     const std::string& pin,
                     ReaderID reader);
    static SignError decodeAuthError(const AuthError& e);

    boost::scoped_ptr<ManagerInterface> m_manager;

    SignCompletedFunc signCompletedFunc;
    SignFailedFunc signFailedFunc;

    boost::mutex m_cardMutex;
    boost::mutex m_messageMutex;
    boost::thread m_thread;
    boost::thread m_signThread;
};

#endif /* CARDSERVICE_H_ */
