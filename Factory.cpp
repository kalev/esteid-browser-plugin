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

#include "FactoryBase.h"
#include "EsteidPlugin.h"
#include <boost/make_shared.hpp>

class PluginFactory : public FB::FactoryBase
{
public:
    FB::PluginCorePtr createPlugin(const std::string& mimetype)
    {
        return boost::make_shared<EsteidPlugin>(mimetype);
    }

    void globalPluginInitialize()
    {
        EsteidPlugin::StaticInitialize();
    }

    void globalPluginDeinitialize()
    {
        EsteidPlugin::StaticDeinitialize();
    }

    void getLoggingMethods(FB::Log::LogMethodList& outMethods)
    {
#ifndef NDEBUG
        outMethods.push_back(std::make_pair(FB::Log::LogMethod_Console, std::string()));
#endif
    }
};

FB::FactoryBasePtr getFactoryInstance()
{
    static boost::shared_ptr<PluginFactory> factory = boost::make_shared<PluginFactory>();
    return factory;
}

