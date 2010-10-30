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
#include <boost/make_shared.hpp>

/* Singleton instance variable */
boost::weak_ptr<CardService> CardService::sCardService;

/**
 * Class constructor.
 * Right now we are making this a singleton.
 * It's generally a good idea to have only a single connection
 * to SmartCard manager service.
 * We might reconsider when times change :P
 */
CardService::CardService()
    : m_manager(new SmartCardManager()),
      m_thread(boost::bind(&CardService::monitor, this))
{
}

CardService::~CardService()
{
    m_thread.interrupt();
    m_thread.join();
}

/**
 * Singleton "constructor". Will return an existing instance
 * or create a new one if none exists.
 */

boost::shared_ptr<CardService> CardService::getInstance()
{
    boost::shared_ptr<CardService> p = sCardService.lock();
    if (!p) {
        p.reset(new CardService());
        sCardService = p;
    }
    return p;
}

void CardService::findEstEid(vector<ReaderID>& readers)
{
    readers.clear();

    for (ReaderID i = 0; i < m_cache.size(); i++ ) {
        if (m_cache[i].cardPresent)
            readers.push_back(i);
    }
}

ReaderID CardService::findFirstEstEid()
{
    vector<ReaderID> readers;
    findEstEid(readers);

    // FIXME: Define a more sane exception to throw from here
    if (readers.size() <= 0)
        throw std::runtime_error("No cards found");
    else
        return readers[0];
}

/* Card monitor thread */
void CardService::monitor()
{
    while (!boost::this_thread::interruption_requested()) {
        try {
            poll();
            boost::this_thread::sleep(boost::posix_time::milliseconds(500));
        } catch(const std::runtime_error&) { }
    }
}

void CardService::addObserver(MessageObserver *obs)
{
    boost::mutex::scoped_lock l(m_mutex); // TODO: Maybe use a different lock?

    m_observers.push_back(obs);
}

void CardService::removeObserver(MessageObserver *obs)
{
    boost::mutex::scoped_lock l(m_mutex);

    vector<MessageObserver *>::iterator i;
    for (i = m_observers.begin(); i != m_observers.end(); i++) {
        if (*i == obs) {
            m_observers.erase(i);
            break;
        }
    }
}

void CardService::postMessage(MsgType m, ReaderID r)
{
    vector<MessageObserver *>::iterator i;
    for (i = m_observers.begin(); i != m_observers.end(); i++)
        (*i)->onMessage(m, r);
}

void CardService::poll()
{
    size_t nReaders;

    {
        boost::mutex::scoped_lock l(m_mutex);
        nReaders = m_manager->getReaderCount();
    }

    /* See if the list of readers has been changed */
    if (m_cache.size() != nReaders) {
        /* We have no way of knowing which reader was
           removed, so it's safe to send card removed event to
           all readers and purge all cached data */
        for (unsigned int i = 0; i < m_cache.size(); i++ ) {
            if (m_cache[i].cardPresent) {
                m_cache[i].purge();
                postMessage(CARD_REMOVED, i);
            }
        }
        m_cache.resize(nReaders);
        postMessage(READERS_CHANGED, nReaders);
    }

    /* Check for card status changes */
    EstEidCard card(*m_manager);
    for (unsigned int i = 0; i < m_cache.size(); i++ ) {
        bool inReader = readerHasCard(card, i);

        if (inReader && !m_cache[i].cardPresent) {
            m_cache[i].cardPresent = true;
            postMessage(CARD_INSERTED, i);
        } else if (!inReader && m_cache[i].cardPresent) {
            m_cache[i].purge();
            postMessage(CARD_REMOVED, i);
        }
    }
}

bool CardService::readerHasCard(EstEidCard& card, ReaderID i)
{
    boost::mutex::scoped_lock l(m_mutex);

    /* Ask manager if a token is inserted into that slot */
    std::string state = m_manager->getReaderState(i);
    if (state.find("PRESENT") == std::string::npos)
        return false;

    /* TODO: Investigate if this caching is actually needed */
    if (m_cache[i].cardPresent)
        return true;

    /* See if it's EstEID */
    return card.isInReader(i);
}

void CardService::readPersonalData(vector<std::string>& data)
{
    readPersonalData(data, findFirstEstEid());
}

void CardService::readPersonalData(vector<std::string>& data,
                                   ReaderID reader)
{
    /* Populate cache if needed */
    if (m_cache[reader].m_pData.size() <= 0) {
        boost::mutex::scoped_lock l(m_mutex);
        EstEidCard card(*m_manager, reader);
        card.readPersonalData(m_cache[reader].m_pData, PDATA_MIN, PDATA_MAX);
    }
    data = m_cache[reader].m_pData;
}

#define ESTEIDSERVICE_GETCERTIMPL(id) \
    ByteVec CardService::get##id##Cert() { \
        return get##id##Cert(findFirstEstEid()); \
    }\
    \
    ByteVec CardService::get##id##Cert(ReaderID reader) { \
        if (m_cache[reader].m_##id##Cert.size() <= 0) { \
            boost::mutex::scoped_lock l(m_mutex); \
            EstEidCard card(*m_manager, reader); \
            m_cache[reader].m_##id##Cert = card.get##id##Cert(); \
        } \
        return m_cache[reader].m_##id##Cert; \
    }

ESTEIDSERVICE_GETCERTIMPL(Auth)
ESTEIDSERVICE_GETCERTIMPL(Sign)

std::string CardService::signSHA1(const std::string& hash,
                                  EstEidCard::KeyType keyId,
                                  const std::string& pin)
{
    return signSHA1(hash, keyId, pin, findFirstEstEid());
}

std::string CardService::signSHA1(const std::string& hash,
                                  EstEidCard::KeyType keyId,
                                  const std::string& pin,
                                  ReaderID reader)
{
    ByteVec bhash = fromHex(hash);
    if (bhash.size() != 20) {
        throw std::runtime_error("Invalid SHA1 hash");
    }

    boost::mutex::scoped_lock l(m_mutex);
    EstEidCard card(*m_manager, reader);

    // FIXME: Ugly, ugly hack! This needs to be implemented correctly
    //        in order to protect PIN codes in program memory.
    return toHex(card.calcSignSHA1(bhash, keyId, PinString(pin.c_str())));
}

bool CardService::getRetryCounts(byte& puk, byte& pinAuth, byte& pinSign)
{
    return getRetryCounts(puk, pinAuth, pinSign, findFirstEstEid());
}

bool CardService::getRetryCounts(byte& puk, byte& pinAuth, byte& pinSign, ReaderID reader)
{
    boost::mutex::scoped_lock l(m_mutex);
    EstEidCard card(*m_manager, reader);
    return card.getRetryCounts(puk, pinAuth, pinSign);
}

bool CardService::hasSecurePinEntry()
{
    return hasSecurePinEntry(findFirstEstEid());
}

bool CardService::hasSecurePinEntry(ReaderID reader)
{
    boost::mutex::scoped_lock l(m_mutex);
    EstEidCard card(*m_manager, reader);
    return card.hasSecurePinEntry();
}
