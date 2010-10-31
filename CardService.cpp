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
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

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
    : m_thread(boost::bind(&CardService::monitor, this))
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
    boost::mutex::scoped_lock l(m_mutex);

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
        } catch(const std::runtime_error&) { }

        boost::this_thread::sleep(boost::posix_time::milliseconds(500));
    }
}

/*
 * SmartCardManager constructor throws if the pcsc daemon isn't running.
 * CardService class however needs to start polling even if the pcsc daemon
 * isn't currently running. As a workaround, all calls to SmartCardManager are
 * proxied through cardManager() which attempts to start SmartCardManager if
 * needed.
 */
ManagerInterface& CardService::cardManager()
{
    if (!m_manager)
        m_manager.reset(new SmartCardManager());

    return *m_manager;
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
    boost::mutex::scoped_lock l(m_mutex);

    /* See if the list of readers has been changed */
    size_t nReaders = cardManager().getReaderCount();
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
    EstEidCard card(cardManager());
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
    /* Ask manager if a token is inserted into that slot */
    std::string state = cardManager().getReaderState(i);
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
    boost::mutex::scoped_lock l(m_mutex);

    /* Populate cache if needed */
    if (m_cache[reader].m_pData.size() <= 0) {
        EstEidCard card(cardManager(), reader);
        card.readPersonalData(m_cache[reader].m_pData, PDATA_MIN, PDATA_MAX);
    }
    data = m_cache[reader].m_pData;
}

ByteVec CardService::getAuthCert()
{
    return getAuthCert(findFirstEstEid());
}

ByteVec CardService::getAuthCert(ReaderID reader)
{
    boost::mutex::scoped_lock l(m_mutex);

    /* Populate cache if needed */
    if (m_cache[reader].m_authCert.size() <= 0) {
        EstEidCard card(cardManager(), reader);
        m_cache[reader].m_authCert = card.getAuthCert();
    }
    return m_cache[reader].m_authCert;
}

ByteVec CardService::getSignCert()
{
    return getSignCert(findFirstEstEid());
}

ByteVec CardService::getSignCert(ReaderID reader)
{
    boost::mutex::scoped_lock l(m_mutex);

    /* Populate cache if needed */
    if (m_cache[reader].m_signCert.size() <= 0) {
        EstEidCard card(cardManager(), reader);
        m_cache[reader].m_signCert = card.getSignCert();
    }
    return m_cache[reader].m_signCert;
}

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
    EstEidCard card(cardManager(), reader);

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
    EstEidCard card(cardManager(), reader);
    return card.getRetryCounts(puk, pinAuth, pinSign);
}

bool CardService::hasSecurePinEntry()
{
    return hasSecurePinEntry(findFirstEstEid());
}

bool CardService::hasSecurePinEntry(ReaderID reader)
{
    boost::mutex::scoped_lock l(m_mutex);
    EstEidCard card(cardManager(), reader);
    return card.hasSecurePinEntry();
}
