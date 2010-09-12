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

/**********************************************************\ 
  Default Windows factory methods

  These are used to create the Npapi plugin type and
  the PluginWindow type on windows; if you want to extend
  these classses, do so and update these functions to
  instantiate the correct types.
\**********************************************************/

#include "Win/FactoryDefinitionsWin.h"
#include "Win/NpapiPluginWin.h"
#include "Win/PluginWindowWin.h"

FB::Npapi::NpapiPluginPtr _getNpapiPlugin(FB::Npapi::NpapiBrowserHostPtr &host)
{
    return FB::Npapi::NpapiPluginPtr(new FB::Npapi::NpapiPluginWin(host));
}

FB::PluginWindowWin *_createPluginWindow(HWND hWnd)
{
    return new FB::PluginWindowWin(hWnd);
}
