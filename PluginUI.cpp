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

#include "PluginUI.h"

PluginUI::PluginUI(FB::AutoPtr<UICallbacks> cb)
    : m_refCount(0),
      m_callbacks(cb)
{
}

PluginUI::~PluginUI(void)
{
}

void PluginUI::AddRef()
{
    ++m_refCount;
}

unsigned int PluginUI::Release()
{
    if (--m_refCount == 0) {
        delete this;
        return 0;
    } else {
        return m_refCount;
    }
}

void PluginUI::UICallbacks::AddRef()
{
    ++m_refCount;
}

unsigned int PluginUI::UICallbacks::Release()
{
    if (--m_refCount == 0) {
        delete this;
        return 0;
    } else {
        return m_refCount;
    }
}
