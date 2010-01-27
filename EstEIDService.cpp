#include "EstEIDService.h"
#include "converters.h"

#include <stdio.h> //FIXME: remove!

/* Zero out our singleton instance variable */
EstEIDService* EstEIDService::sEstEIDService = NULL;

/**
 * Class constructor.
 * Right now we are making this a singleton.
 * It's generally a good idea to have only a single connection
 * to SmartCard manager service.
 * We might reconsider when times change :P
 */
EstEIDService::EstEIDService() : m_lock("EstEIDService")
{
    if(sEstEIDService)
        throw std::runtime_error(">1 EstEIDService object created");

    sEstEIDService = this;
}

EstEIDService::~EstEIDService() {
    sEstEIDService = NULL;
}

/**
 * Singleton "constructor". Will return an existing instance
 * or create a new one if none exists.
 */

EstEIDService* EstEIDService::getInstance() {
    if(!sEstEIDService) {
        EstEIDService* ni = new EstEIDService();
        if (!ni)
            return NULL;
        if(!sEstEIDService)
            throw std::runtime_error("Memory corrupt?");
    }
    return sEstEIDService;
}

void EstEIDService::FindEstEID(vector <readerID> & readers) {
    readers.clear();
    Poll();

    for (readerID i = 0; i < m_cache.size(); i++ )
        if(m_cache[i].cardPresent) readers.push_back(i);
}

readerID EstEIDService::findFirstEstEID() {
    vector <readerID> readers;
    FindEstEID(readers);

    // FIXME: Define a more sane exception to throw from here
    if(readers.size() <= 0)
        throw std::runtime_error("No cards found");
    else
        return readers[0];
}

void EstEIDService::Worker() {
    try {
        Poll();
    }
    catch(std::runtime_error &err) {
        PostMessage(MSG_CARD_ERROR, 0, err.what());
        return;
    }
}

void EstEIDService::Poll() {
    size_t nReaders;

    SmartCardManager mgr;

    {   idAutoLock lock(m_lock);
        nReaders = mgr.getReaderCount();
    }

    EstEidCard card(mgr);
    if(m_cache.size() != nReaders) {
        /* We have no way of knowing which reader was
           removed, so it's safe to send card removed event to
           all readers and purge all cached data */
        for (unsigned int i = 0; i < m_cache.size(); i++ ) {
            PostMessage(MSG_CARD_REMOVED, i);
            m_cache[i].pdata.clear();
        }
        m_cache.resize(nReaders);
        PostMessage(MSG_READERS_CHANGED, nReaders);
    }
    _Poll(card);
}

void EstEIDService::_Poll(EstEidCard & card) {
    for (unsigned int i = 0; i < m_cache.size(); i++ ) {
        try {
            bool inReader;

            {   idAutoLock lock(m_lock);
                inReader = card.isInReader(i);
            }

            if (inReader && !m_cache[i].cardPresent) {
                m_cache[i].cardPresent = true;
                PostMessage(MSG_CARD_INSERTED, i);
            }
            else if (!inReader && m_cache[i].cardPresent) {
                m_cache[i].cardPresent = false;
                PostMessage(MSG_CARD_REMOVED, i);
            }
        }
        catch(std::runtime_error &err) {
            PostMessage(MSG_CARD_ERROR, i, err.what());
        }
    }
}

void EstEIDService::readPersonalData(vector <std::string> & data) {
    readPersonalData(data, findFirstEstEID());
}

void EstEIDService::readPersonalData(vector <std::string> & data,
                                           readerID reader) {
    /* Populate cache if needed */
    printf("kala: %d\n", m_cache[reader].pdata.size());

    if(m_cache[reader].pdata.size() <= 0) {
        printf("Reading ....\n");

        idAutoLock lock(m_lock);

        SmartCardManager mgr;
        EstEidCard card(mgr, reader);
        card.readPersonalData(m_cache[reader].pdata, PDATA_MIN, PDATA_MAX);
    }
    data = m_cache[reader].pdata;
}

#define ESTEIDSERVICEBASE_GETCERTIMPL(id) \
    ByteVec EstEIDService::get##id##Cert() { \
        return get##id##Cert(findFirstEstEID()); \
    }\
    \
    ByteVec EstEIDService::get##id##Cert(readerID reader) { \
        idAutoLock lock(m_lock); \
        SmartCardManager mgr; \
        EstEidCard card(mgr, reader); \
        return card.get##id##Cert(); \
    }

ESTEIDSERVICEBASE_GETCERTIMPL(Auth)
ESTEIDSERVICEBASE_GETCERTIMPL(Sign)

std::string EstEIDService::signSHA1(std::string hash,
                EstEidCard::KeyType keyId, std::string pin) {
    return signSHA1(hash, keyId, pin, findFirstEstEID());
}

std::string EstEIDService::signSHA1(std::string hash,
                EstEidCard::KeyType keyId, std::string pin, readerID reader) {
    idAutoLock lock(m_lock);

    ByteVec bhash = fromHex(hash);
    if (bhash.size() != 20) {
        throw std::runtime_error("Invalid SHA1 hash");
    }

    SmartCardManager mgr;
    EstEidCard card(mgr, reader);

    // FIXME: Ugly, ugly hack! This needs to be implemented correctly
    //        in order to protect PIN codes in program memory.
    return toHex(card.calcSignSHA1(bhash, keyId, PinString(pin.c_str())));
}

bool EstEIDService::getRetryCounts(byte &puk,
    byte &pinAuth,byte &pinSign) {
    return getRetryCounts(puk, pinAuth, pinSign, findFirstEstEID());
}
bool EstEIDService::getRetryCounts(byte &puk,
    byte &pinAuth,byte &pinSign, readerID reader) {

    idAutoLock lock(m_lock);

    SmartCardManager mgr;
    EstEidCard card(mgr, reader);
    return card.getRetryCounts(puk, pinAuth, pinSign);
}

bool EstEIDService::hasSecurePinEntry() {
    return hasSecurePinEntry(findFirstEstEID());
}

bool EstEIDService::hasSecurePinEntry(readerID reader) {
    idAutoLock lock(m_lock);

    SmartCardManager mgr;
    EstEidCard card(mgr, reader);
    return card.hasSecurePinEntry();
}
