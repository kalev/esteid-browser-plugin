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

#include "pinentry.h"


PinEntry::PinEntry(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& refGlade)
    : Gtk::Entry(cobject)
{
}

PinEntry::~PinEntry()
{
}

bool PinEntry::notDigit(char c)
{
    return !isdigit(c);
}

bool PinEntry::containsOnlyNumbers(const std::string& str)
{
    if (std::find_if(str.begin(), str.end(), notDigit) == str.end())
        return true;
    else
        return false;
}

// override base class signal handler
void PinEntry::on_insert_text(const Glib::ustring& text, int* position)
{
    // allow only numbers to be entered
    if (containsOnlyNumbers(text))
        Gtk::Entry::on_insert_text(text, position);
}
