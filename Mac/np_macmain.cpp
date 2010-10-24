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
  Default auto-generated np_macmain.cpp

  This file contains the entrypoints for a Mac NPAPI
  plugin.  Don't change this unless you know what you're
  doing!
\**********************************************************/

#include "NpapiTypes.h"
#include "NpapiPluginModule.h"

using namespace FB::Npapi;
FB::Npapi::NpapiPluginModule *module = NULL;

typedef void (*NPP_ShutdownProcPtr)(void);

#pragma GCC visibility push(default)

extern "C" {
    NPError NP_Initialize(NPNetscapeFuncs *browserFuncs);
    NPError NP_GetEntryPoints(NPPluginFuncs *pluginFuncs);
    NPError NP_Shutdown(void);

#ifndef _NO_MAIN
    // For compatibility with CFM browsers.
    int main(NPNetscapeFuncs *browserFuncs, NPPluginFuncs *pluginFuncs, NPP_ShutdownProcPtr *shutdown);
#endif
}

#pragma GCC visibility pop

#ifndef _NO_MAIN
int main(NPNetscapeFuncs *browserFuncs, NPPluginFuncs *pluginFuncs, NPP_ShutdownProcPtr *shutdown)
{
    NP_Initialize(browserFuncs);
    NP_GetEntryPoints(pluginFuncs);
    *shutdown = (NPP_ShutdownProcPtr)&NP_Shutdown;
}
#endif

void initPluginModule()
{
    if (module == NULL) {
        module = new NpapiPluginModule();
        NpapiPluginModule::Default = module;
    }
}

NPError OSCALL NP_GetEntryPoints(NPPluginFuncs* pFuncs)
{
    initPluginModule();
    module->getPluginFuncs(pFuncs);
    return NPERR_NO_ERROR;
}

NPError OSCALL NP_Initialize(NPNetscapeFuncs* pFuncs)
{
    initPluginModule();
    module->setNetscapeFuncs(pFuncs);

    return NPERR_NO_ERROR;
}

NPError OSCALL NP_Shutdown()
{
    delete module;
    module = NULL;
    return NPERR_NO_ERROR;
}
