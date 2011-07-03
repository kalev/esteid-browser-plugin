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

#include "converter.h"
#include "debug.h"
#include <iconv.h>
#include <iomanip>
#include <sstream>

#ifdef ICONV_SECOND_ARGUMENT_IS_CONST
    #define ICONV_CONST const
#else
    #define ICONV_CONST
#endif

#if defined (__APPLE__) && defined (__LP64__)
#define LIBICONV_PLUG 1
#endif

std::string Converter::iconvConvert(const std::string & str_in, const char *tocode, const char *fromcode)
{
    iconv_t iso_utf;
    ICONV_CONST char *inptr;
    char *outptr;
    size_t inbytes, outbytes, result;
    char outbuf[128];
    std::string out;

    iso_utf = iconv_open(tocode, fromcode);
    if (iso_utf == (iconv_t)-1) {
        ESTEID_DEBUG("error in iconv_open");
        return str_in;
    }

    inptr = (ICONV_CONST char*)str_in.c_str();
    inbytes = str_in.size();

    outptr = outbuf;
    outbytes = sizeof(outbuf) - 1;

    result = iconv(iso_utf, &inptr, &inbytes, &outptr, &outbytes);
    if (result == (size_t)-1) {
        ESTEID_DEBUG("error converting with iconv");
        return str_in;
    }
    *outptr = '\0';

    iconv_close(iso_utf);

    out += outbuf;
    return out;
}


std::string Converter::CP1252_to_UTF8(const std::string & str_in)
{
    return iconvConvert(str_in, "UTF-8", "CP1252");
}


std::string Converter::bytes_to_hex(const std::vector<unsigned char>& v)
{
    typedef std::vector<unsigned char>::const_iterator iter;

    std::ostringstream ret;
    ret << std::setw(2) << std::setfill('0') << std::hex;
    for (iter it = v.begin(); it != v.end(); ++it)
        ret << static_cast<int>(*it);

    return ret.str();
}


std::vector<unsigned char> Converter::hex_to_bytes(const std::string& hex)
{
    std::vector<unsigned char> bytes;
    bytes.reserve(hex.size() / 2);
    for (std::string::size_type i = 0; i < hex.size(); i += 2) {
        int byte;
        std::istringstream hex_byte(hex.substr(i, 2));
        hex_byte >> std::hex >> byte;
        bytes.push_back(static_cast<unsigned char>(byte));
    }

    return bytes;
}
