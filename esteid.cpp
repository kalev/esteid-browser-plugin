/**********************************************************\

  Auto-generated esteid.cpp

  This file contains the auto-generated main plugin object
  implementation for the esteid project

\**********************************************************/

#include "NpapiTypes.h"
#include "esteidAPI.h"

#include "esteid.h"

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
{
}

esteid::~esteid()
{
}

FB::JSAPI* esteid::createJSAPI()
{
    // m_host is the BrowserHostWrapper
    return new esteidAPI(m_host);
}

bool esteid::onMouseDown(FB::MouseDownEvent *evt, FB::PluginWindow *)
{
    //printf("Mouse down at: %d, %d\n", evt->m_x, evt->m_y);
    return false;
}

bool esteid::onMouseUp(FB::MouseUpEvent *evt, FB::PluginWindow *)
{
    //printf("Mouse up at: %d, %d\n", evt->m_x, evt->m_y);
    return false;
}

bool esteid::onMouseMove(FB::MouseMoveEvent *evt, FB::PluginWindow *)
{
    //printf("Mouse move at: %d, %d\n", evt->m_x, evt->m_y);
    return false;
}
bool esteid::onWindowAttached(FB::AttachedEvent *evt, FB::PluginWindow *)
{
    // The window is attached; act appropriately
    return false;
}

bool esteid::onWindowDetached(FB::DetachedEvent *evt, FB::PluginWindow *)
{
    // The window is about to be detached; act appropriately
    return false;
}
