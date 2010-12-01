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

#include <algorithm>
#include <boost/bind.hpp>

#include "whitelist.h"

bool inWhitelist(const Whitelist& wl, const std::string& site)
{
    if (site.empty())
        return false;

    return find(wl.begin(), wl.end(), site) != wl.end();
}

void removeDuplicateEntries(Whitelist& wl)
{
    std::sort(wl.begin(), wl.end());
    wl.erase(std::unique(wl.begin(), wl.end()), wl.end());
}

// Remove from wl entries which are duplicate with ref_wl
void removeDuplicateEntries(Whitelist& wl, const Whitelist& ref_wl)
{
    wl.erase(remove_if(wl.begin(), wl.end(), boost::bind(&inWhitelist, ref_wl, _1)), wl.end());
}
