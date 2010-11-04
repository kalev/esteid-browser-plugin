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

#include "NpapiTypes.h"
#include "EsteidAPI.h"
#include "EsteidPlugin.h"

#if defined(SUPPORT_OLD_APIS) && defined(_WIN32)
#include "EIDCompatControl.h" // Support loading by old ActiveX CLSID
#endif

logger eidlog("npesteid");

void EsteidPlugin::StaticInitialize()
{
    // Place one-time initialization stuff here; note that there isn't an absolute guarantee that
    // this will only execute once per process, just a guarantee that it won't execute again until
    // after StaticDeinitialize is called
}

void EsteidPlugin::StaticDeinitialize()
{
    // Place one-time deinitialization stuff here
}


EsteidPlugin::EsteidPlugin(const std::string& mimetype)
    : m_pluginAPI(),
      m_window(NULL)
{
      m_mimeType = mimetype;
}

EsteidPlugin::~EsteidPlugin()
{
}

FB::JSAPIPtr EsteidPlugin::createJSAPI()
{
    // m_host is the BrowserHostPtr
    m_pluginAPI = boost::shared_ptr<EsteidAPI>(new EsteidAPI(m_host, m_mimeType));
    m_pluginAPI->setWindow(m_window);
    return m_pluginAPI;
}

bool EsteidPlugin::onMouseDown(FB::MouseDownEvent *evt, FB::PluginWindow *)
{
    return false;
}

bool EsteidPlugin::onMouseUp(FB::MouseUpEvent *evt, FB::PluginWindow *)
{
    return false;
}

bool EsteidPlugin::onMouseMove(FB::MouseMoveEvent *evt, FB::PluginWindow *)
{
    return false;
}
bool EsteidPlugin::onWindowAttached(FB::AttachedEvent *evt, FB::PluginWindow *win)
{
    m_window = win;
    if (m_pluginAPI)
        m_pluginAPI->setWindow(win);
    return true;
}

bool EsteidPlugin::onWindowDetached(FB::DetachedEvent *evt, FB::PluginWindow *)
{
    m_window = NULL;
    if (m_pluginAPI)
        m_pluginAPI->setWindow(NULL);
    return true;
}
