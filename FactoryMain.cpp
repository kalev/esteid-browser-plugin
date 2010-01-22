/**********************************************************\

  Auto-generated FactoryMain.cpp

  This file contains the auto-generated factory methods
  for the esteid project

\**********************************************************/

#include "FactoryDefinitions.h"
#include "esteid.h"

FB::PluginCore *_getMainPlugin()
{
    return new esteid();
}

void GlobalPluginInitialize()
{
    esteid::StaticInitialize();
}

void GlobalPluginDeinitialize()
{
    esteid::StaticDeinitialize();
}
