/**
 * This class helps to implement Card Services.
 * It encapsulates platform specific locking, card event polling
 * thread and synchronized access to cards via a singleton instance.
 *
 * Typical usage:
 *
 * EstEIDService *service = EstEIDService::getInstance();
 * service->AddObserver(this);
 *
 */

#ifndef ESTEIDSERVICE_H_
#define ESTEIDSERVICE_H_

#include <smartcardpp/smartcardpp.h>

#define PDATA_MIN EstEidCard::SURNAME
#define PDATA_MAX EstEidCard::COMMENT4

/* Use mutex and thread abstraction in threadObj */
#include "threadObj.h"
typedef mutexObj idLockObj;
typedef mutexObjLocker idAutoLock;
typedef threadObj idThread;

typedef unsigned int readerID;


class EstEIDService : public idThread {
public:
    /**
     * Get service instance
     */
    static EstEIDService* getInstance();
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
    std::string signSHA1(std::string hash, EstEidCard::KeyType keyId,
    		std::string pin);
    /**
     * Sign a SHA1 hash with the specified card
     */
    std::string signSHA1(std::string hash, EstEidCard::KeyType keyId,
    		std::string pin, readerID);

    /* Message observer interface */
    enum msgType {
	CARD_INSERTED,
	CARD_REMOVED,
	READERS_CHANGED,
	CARD_ERROR
    };
    class messageObserver {
        friend class EstEIDService;

        virtual void onMessage(msgType e, readerID i) = 0;
    };
    virtual void AddObserver(messageObserver *obs);
    virtual void RemoveObserver(messageObserver *obs);

protected:
    EstEIDService();
    virtual ~EstEIDService();

    /* Get access to smartcard manager instance */
    virtual ManagerInterface & getManager();

    /* Singleton instance variable */
    static EstEIDService* sEstEIDService;

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
    //! Copy constructor.
    EstEIDService(const EstEIDService& source) : 
        m_lock(source.m_lock), idThread(source) {};

    void findEstEID();
    void Poll();
    bool readerHasCard(EstEidCard &card,readerID i);

    idLockObj m_lock;
    ManagerInterface *m_manager;

    /* Card monitor thread implementation */
    virtual void execute();
};

#endif /* ESTEIDSERVICE_H_ */
