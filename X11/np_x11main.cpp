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
  Default auto-generated np_x11main.cpp

  This file contains the entrypoints for a *nix NPAPI
  plugin.  Don't change this unless you know what you're
  doing!
\**********************************************************/

#include "config.h"
#include "NpapiTypes.h"
#include "NpapiPluginModule.h"

using namespace FB::Npapi;
FB::Npapi::NpapiPluginModule *module = NULL;

void initPluginModule()
{
    if (module == NULL) {
        module = new NpapiPluginModule();
        NpapiPluginModule::Default = module;
    }
}

extern "C" char * NP_GetPluginVersion()
{
    return (char *)FBSTRING_PLUGIN_VERSION;
}

extern "C" char * NP_GetMIMEDescription()
{
// FIXME: Fix FireBreath!!!!
#ifdef SUPPORT_OLD_APIS
    return (char *)"application/x-esteid::Estonian ID Card Plugin;application/x-digidoc::Estonian ID Card Plugin;application/x-idcard-plugin::Estonian ID Card Plugin";
#else
    return (char *)FBSTRING_X11MIMEType;
#endif
}

extern "C" NPError NP_GetValue(void *future, NPPVariable variable, void *value)
{
    initPluginModule();
    return module->NPP_GetValue((NPP_t *)future, variable, value);
}

extern "C" NPError NP_Initialize(NPNetscapeFuncs* pFuncs
                             , NPPluginFuncs *pluginFuncs)
{
    initPluginModule();
    module->getPluginFuncs(pluginFuncs);
    module->setNetscapeFuncs(pFuncs);

    return NPERR_NO_ERROR;
}

extern "C" NPError NP_Shutdown()
{
    delete module;
    module = NULL;
    return NPERR_NO_ERROR;
}
