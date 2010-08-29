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

#include "X509Certificate.h"

#define THROW_API_ERROR(a) { \
    std::string msg(a); \
    msg += ": "; \
    msg += getError(); \
    throw std::runtime_error(msg); \
}

#ifdef USE_OPENSSL
#include <openssl/bio.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <openssl/x509v3.h>

X509Certificate::X509Certificate(ByteVec bv) : m_cert(NULL) {
    if(bv.empty())
        throw std::runtime_error("Invalid certificate: no data");

#if OPENSSL_VERSION_NUMBER >= 0x0090800fL
    const unsigned char* buf = reinterpret_cast<const unsigned char*>(&bv[0]);
#else
    unsigned char* buf = &bv[0];
#endif

    d2i_X509(&m_cert, &buf, bv.size());
    if(m_cert == NULL) THROW_API_ERROR("Invalid certificate");
}

X509Certificate::~X509Certificate() {
    if(m_cert) X509_free(m_cert);
}

std::string X509Certificate::getError() {
    return ERR_reason_error_string(ERR_get_error());
}
std::string X509Certificate::getSubjectCN() {
    return X509NameToUTF8(X509_get_subject_name(m_cert), NID_commonName);
}
std::string X509Certificate::getIssuerCN() {
    return X509NameToUTF8(X509_get_issuer_name(m_cert), NID_commonName);
}
std::string X509Certificate::getValidFrom() {
    return X509TimeConvert(X509_get_notBefore(m_cert));
}
std::string X509Certificate::getValidTo() {
    return X509TimeConvert(X509_get_notAfter(m_cert));
}
long X509Certificate::getSerial() {
    long serial = ASN1_INTEGER_get(X509_get_serialNumber(m_cert));
    if(serial <= 0) THROW_API_ERROR("Failed to read certificate serial");

    return serial;
}
std::string X509Certificate::getPEM() {
    BIO *bio = BIO_new(BIO_s_mem());
    char *data;
    int len;

    if(!PEM_write_bio_X509(bio, m_cert))
        THROW_API_ERROR("Error converting certificate to PEM format");

    len = BIO_get_mem_data(bio, &data);
    std::string result(data, len);
    BIO_free(bio);

    return result;
}
bool X509Certificate::isValid() {
    ASN1_TIME *t1 = X509_get_notBefore(m_cert);
    ASN1_TIME *t2 = X509_get_notAfter(m_cert);
    if(!t1 || !t2) THROW_API_ERROR("Failed to parse certificate");

    return X509_cmp_current_time(t1) < 0 && X509_cmp_current_time(t2) > 0;
}

std::string X509Certificate::getKeyUsage() {
    int pos = X509_get_ext_by_NID(m_cert, NID_key_usage, -1);
    if(pos < 0) THROW_API_ERROR("Failed to parse certificate");

    X509_EXTENSION *keyUsage = X509_get_ext(m_cert, pos);
    if(!keyUsage) THROW_API_ERROR("Failed to parse certificate");

    char *data;
    BIO *bio = BIO_new(BIO_s_mem());
    X509V3_EXT_print(bio, keyUsage, 0, 0);
    size_t len = BIO_get_mem_data(bio, &data);
    std::string result(data, len);
    BIO_free(bio);

    return result;
}

std::string X509Certificate::X509TimeConvert(ASN1_TIME *date) {
    char *data;
    int len;

    // FIXME: Return something more sane
    if(!date || !date->data) THROW_API_ERROR("Failed to parse certificate");

    BIO *bio = BIO_new(BIO_s_mem());
    ASN1_TIME_print(bio, date);
    len = BIO_get_mem_data(bio, &data);
    std::string result(data, len);
    BIO_free(bio);

    return result;
    // return std::string((char *)date->data);
}

std::string X509Certificate::X509NameToUTF8(X509_NAME *name, int nid) {
    if(!name) THROW_API_ERROR("Failed to parse certificate");
    int idx = X509_NAME_get_index_by_NID(name, nid, -1);
    if(idx < 0) THROW_API_ERROR("Failed to parse certificate");
    X509_NAME_ENTRY *entry = X509_NAME_get_entry(name, idx);
    if(!entry) THROW_API_ERROR("Failed to parse certificate");
    ASN1_STRING *asnstr = X509_NAME_ENTRY_get_data(entry);

    unsigned char *data = NULL;
    int len = ASN1_STRING_to_UTF8(&data, asnstr);
    if(len < 0) THROW_API_ERROR("Failed to convert certificate data");
    std::string result(reinterpret_cast<const char *>(data));
    OPENSSL_free(data);

    return result;
}
#else
#error Not implemented yet
#endif
