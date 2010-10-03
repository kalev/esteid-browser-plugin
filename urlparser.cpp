/*
 * esteid-browser-plugin - a browser plugin for Estonian EID card
 *
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

#include "urlparser.h"
#include <algorithm>
#include <string>

UrlParser::UrlParser(const std::string& url)
{
    parse(url);
}

UrlParser::~UrlParser()
{
}

std::string UrlParser::hostname()
{
    return m_hostname;
}

std::string UrlParser::protocol()
{
    return m_protocol;
}

bool hostname_end_char(char c)
{
    return (c == '/' ||
            c == ':');
}

/* Parse an URL in protocol://username:password@hostname:port/... format */
void UrlParser::parse(const std::string& url)
{
    static const std::string sep("://");

    typedef std::string::const_iterator iter;

    // Find the beginning of protocol separator
    iter sep_beg = std::search(url.begin(), url.end(), sep.begin(), sep.end());

    // Return immediately if the url doesn't contain protocol separator
    if (sep_beg == url.end())
        return;

    m_protocol = std::string(url.begin(), sep_beg);

    // Skip over username:password@ part
    iter it;
    iter host_beg;
    if ((it = find(sep_beg + sep.size(), url.end(), '@')) != url.end())
        host_beg = it + 1;
    else
        host_beg = sep_beg + sep.size();

    // Find the end of the hostname part
    iter host_end = find_if(host_beg, url.end(), hostname_end_char);

    m_hostname = std::string(host_beg, host_end);
}
