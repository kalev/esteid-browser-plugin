#include "PluginUI.h"

PluginUI::PluginUI(void) : m_refCount(0)
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
