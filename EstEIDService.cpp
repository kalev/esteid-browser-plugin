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
EstEIDService::EstEIDService() : m_lock("EstEIDService"), m_manager(NULL),
                                 idThread("EstEIDService card monitor")
{
    if(sEstEIDService)
        throw std::runtime_error(">1 EstEIDService object created");

    sEstEIDService = this;
    this->start();
}

EstEIDService::~EstEIDService() {
    if(m_manager) delete m_manager;
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

/* Card monitor thread implementation */
void EstEIDService::execute() {
    for(;;) {
        try {
            threadObj::wait(500);
            Poll();
        } catch(std::runtime_error &e) { }
    }
}

ManagerInterface &EstEIDService::getManager() {
    if(!m_manager)
        m_manager = new SmartCardManager();

    return *m_manager;
}

void EstEIDService::AddObserver(messageObserver *obs) {
    idAutoLock lock(m_lock); // TODO: Maybe use a different lock?

    m_observers.push_back(obs);
}

void EstEIDService::RemoveObserver(messageObserver *obs) {
    idAutoLock lock(m_lock);

    vector <messageObserver *>::iterator i;
    for (i = m_observers.begin(); i != m_observers.end(); i++)
        if(*i == obs) { m_observers.erase(i); break; }
}

void EstEIDService::PostMessage(msgType m, readerID r) {
    vector <messageObserver *>::iterator i;
    for (i = m_observers.begin(); i != m_observers.end(); i++ )
        (*i)->onMessage(m, r);
}

void EstEIDService::Poll() {
    size_t nReaders;

    ManagerInterface &mgr = getManager();

    {   idAutoLock lock(m_lock);
        nReaders = mgr.getReaderCount();
    }

    /* See if the list of readers has been changed */
    if(m_cache.size() != nReaders) {
        /* We have no way of knowing which reader was
           removed, so it's safe to send card removed event to
           all readers and purge all cached data */
        for (unsigned int i = 0; i < m_cache.size(); i++ ) {
            if(m_cache[i].cardPresent) {
                m_cache[i].purge();
                PostMessage(CARD_REMOVED, i);
            }
        }
        m_cache.resize(nReaders);
        PostMessage(READERS_CHANGED, nReaders);
    }

    /* Check for card status changes */
    EstEidCard card(mgr);
    for (unsigned int i = 0; i < m_cache.size(); i++ ) {
        bool inReader = readerHasCard(card, i);

        if (inReader && !m_cache[i].cardPresent) {
            m_cache[i].cardPresent = true;
            PostMessage(CARD_INSERTED, i);
        }
        else if (!inReader && m_cache[i].cardPresent) {
            m_cache[i].purge();
            PostMessage(CARD_REMOVED, i);
        }
    }
}

bool EstEIDService::readerHasCard(EstEidCard &card,readerID i) {
    idAutoLock lock(m_lock);
    ManagerInterface &mgr = getManager();

    /* Ask manager if a token is inserted into that slot */
    std::string state = mgr.getReaderState(i);
    if (state.find("PRESENT") == std::string::npos ) return false;

    /* TODO: Investigate if this caching is actually needed */
    if (m_cache[i].cardPresent)
        return true;

    /* See if it's EstEID */
    return card.isInReader(i);
}

#define CREATE_LOCKED_ESTEID_INSTANCE \
    idAutoLock lock(m_lock); \
    ManagerInterface &mgr = getManager(); \
    EstEidCard card(mgr, reader);
    
void EstEIDService::readPersonalData(vector <std::string> & data) {
    readPersonalData(data, findFirstEstEID());
}

void EstEIDService::readPersonalData(vector <std::string> & data,
                                           readerID reader) {
    /* Populate cache if needed */
    if(m_cache[reader].mPData.size() <= 0) {
        CREATE_LOCKED_ESTEID_INSTANCE
        card.readPersonalData(m_cache[reader].mPData, PDATA_MIN, PDATA_MAX);
    }
    data = m_cache[reader].mPData;
}

#define ESTEIDSERVICE_GETCERTIMPL(id) \
    ByteVec EstEIDService::get##id##Cert() { \
        return get##id##Cert(findFirstEstEID()); \
    }\
    \
    ByteVec EstEIDService::get##id##Cert(readerID reader) { \
        if(m_cache[reader].m##id##Cert.size() <= 0) { \
            CREATE_LOCKED_ESTEID_INSTANCE \
            m_cache[reader].m##id##Cert = card.get##id##Cert(); \
        } \
        return m_cache[reader].m##id##Cert; \
    }

ESTEIDSERVICE_GETCERTIMPL(Auth)
ESTEIDSERVICE_GETCERTIMPL(Sign)

std::string EstEIDService::signSHA1(std::string hash,
                EstEidCard::KeyType keyId, std::string pin) {
    return signSHA1(hash, keyId, pin, findFirstEstEID());
}

std::string EstEIDService::signSHA1(std::string hash,
                EstEidCard::KeyType keyId, std::string pin, readerID reader) {

    ByteVec bhash = fromHex(hash);
    if (bhash.size() != 20) {
        throw std::runtime_error("Invalid SHA1 hash");
    }

    CREATE_LOCKED_ESTEID_INSTANCE

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

    CREATE_LOCKED_ESTEID_INSTANCE
    return card.getRetryCounts(puk, pinAuth, pinSign);
}

bool EstEIDService::hasSecurePinEntry() {
    return hasSecurePinEntry(findFirstEstEID());
}

bool EstEIDService::hasSecurePinEntry(readerID reader) {
    CREATE_LOCKED_ESTEID_INSTANCE
    return card.hasSecurePinEntry();
}
