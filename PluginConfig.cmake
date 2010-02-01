#/**********************************************************\ 
# Auto-Generated Plugin Configuration file
# for esteid
#\**********************************************************/

set(PLUGIN_NAME "esteid")
set(PLUGIN_PREFIX "EE")
set(COMPANY_NAME "RIA")

# ActiveX constants:
set(FBTYPELIB_NAME esteidLib)
set(FBTYPELIB_DESC "esteid 1.0 Type Library")
set(IFBControl_DESC "esteid Control Interface")
set(FBControl_DESC "esteid Control Class")
set(IFBComJavascriptObject_DESC "esteid IComJavascriptObject Interface")
set(FBComJavascriptObject_DESC "esteid ComJavascriptObject Class")
set(IFBComEventSource_DESC "esteid IFBComEventSource Interface")
set(AXVERSION_NUM "1")

# NOTE: THESE GUIDS *MUST* BE UNIQUE TO YOUR PLUGIN/ACTIVEX CONTROL!  YES, ALL OF THEM!
set(FBTYPELIB_GUID b2c0ed58-945a-53ea-bbcf-51af4ad4f81d)
set(IFBControl_GUID 8ceb9c14-aa9c-5a2e-be64-727f7c24bd5f)
set(FBControl_GUID 29ce3405-f2fd-5402-a971-cb7b9fc0abac)
set(IFBComJavascriptObject_GUID 4662cd17-0b44-535a-91e5-1753bb2f1666)
set(FBComJavascriptObject_GUID 4eee5d8d-2c87-50b7-bde3-fe242d062ae5)
set(IFBComEventSource_GUID 60eb3e65-2938-5533-9491-4596e4acc5e6)

# these are the pieces that are relevant to using it from Javascript
set(ACTIVEX_PROGID "EE.esteid")
set(MOZILLA_PLUGINID "id.eesti.ee/esteid")

# strings
set(FBSTRING_CompanyName "Estonian Informatics Centre")
set(FBSTRING_FileDescription "Estonian ID-Card Plugin")
set(_VERSION "1.0.0")
if(SVN_REV)
   set(FBSTRING_PLUGIN_VERSION "${_VERSION}.${SVN_REV}")
else()
   set(FBSTRING_PLUGIN_VERSION "${_VERSION}")
endif()
set(FBSTRING_LegalCopyright "Copyright 2010 Estonian Informatics Centre")
set(FBSTRING_PluginFileName "np${PLUGIN_NAME}.dll")
set(FBSTRING_ProductName "esteid")
set(FBSTRING_FileExtents "")
set(FBSTRING_PluginName "esteid")
set(FBSTRING_MIMEType "application/x-esteid")

