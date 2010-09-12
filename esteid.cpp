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

/**********************************************************\

  Auto-generated esteid.cpp

  This file contains the auto-generated main plugin object
  implementation for the esteid project

\**********************************************************/

#include "NpapiTypes.h"
#include "esteidAPI.h"
#include "esteid.h"

#if defined(SUPPORT_OLD_APIS) && defined(_WIN32)
#include "EIDCompatControl.h" // Support loading by old ActiveX CLSID
#endif

logger eidlog("npesteid");

void esteid::StaticInitialize()
{
    // Place one-time initialization stuff here; note that there isn't an absolute guarantee that
    // this will only execute once per process, just a guarantee that it won't execute again until
    // after StaticDeinitialize is called
}

void esteid::StaticDeinitialize()
{
    // Place one-time deinitialization stuff here
}


esteid::esteid()
    : m_esteidAPI(),
      m_window(NULL)
{
}

esteid::~esteid()
{
}

FB::JSAPIPtr esteid::createJSAPI()
{
    // m_host is the BrowserHostWrapper
    m_esteidAPI = boost::shared_ptr<esteidAPI>(new esteidAPI(m_host));
    m_esteidAPI->setWindow(m_window);
    return m_esteidAPI;
}

bool esteid::onMouseDown(FB::MouseDownEvent *evt, FB::PluginWindow *)
{
    return false;
}

bool esteid::onMouseUp(FB::MouseUpEvent *evt, FB::PluginWindow *)
{
    return false;
}

bool esteid::onMouseMove(FB::MouseMoveEvent *evt, FB::PluginWindow *)
{
    return false;
}
bool esteid::onWindowAttached(FB::AttachedEvent *evt, FB::PluginWindow *win)
{
    m_window = win;
    if (m_esteidAPI)
        m_esteidAPI->setWindow(win);
    return true;
}

bool esteid::onWindowDetached(FB::DetachedEvent *evt, FB::PluginWindow *)
{
    m_window = NULL;
    if (m_esteidAPI)
        m_esteidAPI->setWindow(NULL);
    return true;
}
