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

#include "CardService.h"
#include "converters.h"
#include <boost/bind.hpp>

/* Zero out our singleton instance variable */
CardService* CardService::sCardService = NULL;

/**
 * Class constructor.
 * Right now we are making this a singleton.
 * It's generally a good idea to have only a single connection
 * to SmartCard manager service.
 * We might reconsider when times change :P
 */
CardService::CardService()
    : m_manager(NULL),
      m_thread(boost::bind(&CardService::monitor, this))
{
    if(sCardService)
        throw std::runtime_error(">1 CardService object created");

    sCardService = this;
}

CardService::~CardService()
{
    if(m_manager) delete m_manager;
    sCardService = NULL;
}

/**
 * Singleton "constructor". Will return an existing instance
 * or create a new one if none exists.
 */

CardService* CardService::getInstance()
{
    if(!sCardService) {
        CardService* ni = new CardService();
        if (!ni)
            return NULL;
        if(!sCardService)
            throw std::runtime_error("Memory corrupt?");
    }
    return sCardService;
}

void CardService::FindEstEID(vector <readerID>& readers)
{
    readers.clear();

    for (readerID i = 0; i < m_cache.size(); i++ )
        if(m_cache[i].cardPresent) readers.push_back(i);
}

readerID CardService::findFirstEstEID()
{
    vector <readerID> readers;
    FindEstEID(readers);

    // FIXME: Define a more sane exception to throw from here
    if(readers.size() <= 0)
        throw std::runtime_error("No cards found");
    else
        return readers[0];
}

/* Card monitor thread */
void CardService::monitor()
{
    for(;;) {
        try {
            boost::this_thread::sleep(boost::posix_time::milliseconds(500));
            Poll();
        } catch(std::runtime_error) { }
    }
}

ManagerInterface &CardService::getManager()
{
    if(!m_manager)
        m_manager = new SmartCardManager();

    return *m_manager;
}

void CardService::AddObserver(messageObserver *obs)
{
    boost::mutex::scoped_lock l(m_mutex); // TODO: Maybe use a different lock?

    m_observers.push_back(obs);
}

void CardService::RemoveObserver(messageObserver *obs)
{
    boost::mutex::scoped_lock l(m_mutex);

    vector <messageObserver *>::iterator i;
    for (i = m_observers.begin(); i != m_observers.end(); i++)
        if(*i == obs) { m_observers.erase(i); break; }
}

void CardService::PostMessage(msgType m, readerID r)
{
    vector <messageObserver *>::iterator i;
    for (i = m_observers.begin(); i != m_observers.end(); i++ )
        (*i)->onMessage(m, r);
}

void CardService::Poll()
{
    size_t nReaders;

    ManagerInterface &mgr = getManager();

    {
        boost::mutex::scoped_lock l(m_mutex);
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

bool CardService::readerHasCard(EstEidCard& card, readerID i)
{
    boost::mutex::scoped_lock l(m_mutex);
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
    boost::mutex::scoped_lock l(m_mutex); \
    ManagerInterface &mgr = getManager(); \
    EstEidCard card(mgr, reader);
    
void CardService::readPersonalData(vector <std::string>& data)
{
    readPersonalData(data, findFirstEstEID());
}

void CardService::readPersonalData(vector <std::string>& data,
                                   readerID reader)
{
    /* Populate cache if needed */
    if(m_cache[reader].mPData.size() <= 0) {
        CREATE_LOCKED_ESTEID_INSTANCE
        card.readPersonalData(m_cache[reader].mPData, PDATA_MIN, PDATA_MAX);
    }
    data = m_cache[reader].mPData;
}

#define ESTEIDSERVICE_GETCERTIMPL(id) \
    ByteVec CardService::get##id##Cert() { \
        return get##id##Cert(findFirstEstEID()); \
    }\
    \
    ByteVec CardService::get##id##Cert(readerID reader) { \
        if(m_cache[reader].m##id##Cert.size() <= 0) { \
            CREATE_LOCKED_ESTEID_INSTANCE \
            m_cache[reader].m##id##Cert = card.get##id##Cert(); \
        } \
        return m_cache[reader].m##id##Cert; \
    }

ESTEIDSERVICE_GETCERTIMPL(Auth)
ESTEIDSERVICE_GETCERTIMPL(Sign)

std::string CardService::signSHA1(const std::string& hash,
                                  EstEidCard::KeyType keyId,
                                  const std::string& pin)
{
    return signSHA1(hash, keyId, pin, findFirstEstEID());
}

std::string CardService::signSHA1(const std::string& hash,
                                  EstEidCard::KeyType keyId,
                                  const std::string& pin,
                                  readerID reader)
{
    ByteVec bhash = fromHex(hash);
    if (bhash.size() != 20) {
        throw std::runtime_error("Invalid SHA1 hash");
    }

    CREATE_LOCKED_ESTEID_INSTANCE

    // FIXME: Ugly, ugly hack! This needs to be implemented correctly
    //        in order to protect PIN codes in program memory.
    return toHex(card.calcSignSHA1(bhash, keyId, PinString(pin.c_str())));
}

bool CardService::getRetryCounts(byte& puk,
    byte &pinAuth,byte &pinSign)
{
    return getRetryCounts(puk, pinAuth, pinSign, findFirstEstEID());
}

bool CardService::getRetryCounts(byte& puk,
    byte &pinAuth,byte &pinSign, readerID reader)
{

    CREATE_LOCKED_ESTEID_INSTANCE
    return card.getRetryCounts(puk, pinAuth, pinSign);
}

bool CardService::hasSecurePinEntry()
{
    return hasSecurePinEntry(findFirstEstEID());
}

bool CardService::hasSecurePinEntry(readerID reader)
{
    CREATE_LOCKED_ESTEID_INSTANCE
    return card.hasSecurePinEntry();
}
