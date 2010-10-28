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
 * CardService *service = CardService::getInstance();
 * service->AddObserver(this);
 *
 */

#ifndef ESTEIDSERVICE_H_
#define ESTEIDSERVICE_H_

#include <smartcardpp/smartcardpp.h>
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>

#define PDATA_MIN EstEidCard::SURNAME
#define PDATA_MAX EstEidCard::COMMENT4

typedef unsigned int readerID;


class CardService
{
public:
    /**
     * Get service instance
     */
    static CardService* getInstance();
    /**
     * Find readers with valid card
     */
    void FindEstEID(vector <readerID> & readers);
    /**
     * Find the reader with first valid card
     */
    readerID findFirstEstEID();
    /**
     * Read personal data file off the first card
     */
    void readPersonalData(vector <std::string> & data);
    /**
     * Read personal data file off the card in specified reader
     */
    void readPersonalData(vector <std::string> & data, readerID);
    /**
     * Read authentication certificate off the first card
     */
    ByteVec getAuthCert();
    /**
     * Read authentication certificate off the card in specified reader
     */
    ByteVec getAuthCert(readerID);
    /**
     * Read signature certificate off the first card
     */
    ByteVec getSignCert();
    /**
     * Read signature certificate off the card in specified reader
     */
    ByteVec getSignCert(readerID);
    /**
     * Retrieve retry counts from the first card
     */
    bool getRetryCounts(byte &puk, byte &pinAuth,byte &pinSign);
    /**
     * Retrieve retry counts from the card in specified reader
     */
    bool getRetryCounts(byte &puk, byte &pinAuth,byte &pinSign, readerID);
    /**
     * Check if first card is in a reader with a PinPAD
     */
    bool hasSecurePinEntry();
    /**
     * Check if the specified card is in a reader with a PinPAD
     */
    bool hasSecurePinEntry(readerID);
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
                         readerID);

    /* Message observer interface */
    enum msgType {
        CARD_INSERTED,
        CARD_REMOVED,
        READERS_CHANGED,
        CARD_ERROR
    };
    class messageObserver {
        friend class CardService;

        virtual void onMessage(msgType e, readerID i) = 0;
    };
    virtual void AddObserver(messageObserver *obs);
    virtual void RemoveObserver(messageObserver *obs);

protected:
    CardService();
    virtual ~CardService();

    /* Get access to smartcard manager instance */
    virtual ManagerInterface & getManager();

    /* Singleton instance variable */
    static CardService* sCardService;

    class idCardCacheEntry {
    public:
        bool cardPresent;
        vector <std::string> mPData;
        ByteVec mAuthCert;
        ByteVec mSignCert;
        void purge() { cardPresent = false; mPData.clear(); mAuthCert.clear(); mSignCert.clear(); }
        idCardCacheEntry() : cardPresent(false) {}
    };
    typedef vector <idCardCacheEntry> idCardCache;

    idCardCache m_cache;
    vector <messageObserver *> m_observers;
    virtual void PostMessage(msgType e, readerID i);

private:
    // Declarations only for copy constructor and assignment operator
    CardService(const CardService&);
    CardService& operator=(const CardService&);

    void findEstEID();
    void monitor();
    void Poll();
    bool readerHasCard(EstEidCard &card,readerID i);

    ManagerInterface *m_manager;

    boost::mutex m_mutex;
    boost::thread m_thread;
};

#endif /* ESTEIDSERVICE_H_ */
