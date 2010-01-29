/**
 * This class helps to implement Card Services.
 * It encapsulates platform specific locking, card event polling
 * thread and synchronized access to cards via a singleton instance.
 *
 * Typical usage:
 *
 * EstEIDService *service = EstEIDService::getInstance();
 * service->addEventListener(...);
 *
 */

#ifndef ESTEIDSERVICE_H_
#define ESTEIDSERVICE_H_

#include <smartcardpp/smartcardpp.h>

#define PDATA_MIN EstEidCard::SURNAME
#define PDATA_MAX EstEidCard::COMMENT4

/* Use mutex implementation in threadObj */
#include "monitorThread.h"
//#include "threadObj.h"
typedef mutexObj idLockObj;
typedef mutexObjLocker idAutoLock;

typedef unsigned int readerID;

class EstEIDService : public monitorObserver {
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
protected:
    EstEIDService();
    virtual ~EstEIDService();

    /* Singleton instance variable */
    static EstEIDService* sEstEIDService;

    enum msgType {
	MSG_CARD_INSERTED,
	MSG_CARD_REMOVED,
	MSG_READERS_CHANGED,
	MSG_CARD_ERROR };

    /** Must be called from a background Thread once
     * in every 500ms for card events to work */
    void Worker();

    /** Callback for the watcher thread. Will be called when
     * a card event (insertion, removal) is detected. */
    virtual void PostMessage(msgType type, readerID reader,
                             std::string msg = "") {};

    /** Callback defined in monitorObserver to be executed by monitorThread */
    virtual void onEvent(monitorEvent eType,int param);

private:
    //! Copy constructor.
    EstEIDService(const EstEIDService& source) : m_lock(source.m_lock) {};

    void findEstEID();
    void Poll();
    void _Poll(EstEidCard & card);

    struct CardCacheEntry {
        bool cardPresent;
        vector <std::string> pdata;
    };

    vector <CardCacheEntry> m_cache;
    idLockObj m_lock;
    monitorThread *m_monitorThread;
};

#endif /* ESTEIDSERVICE_H_ */
