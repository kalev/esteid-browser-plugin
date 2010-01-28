#/**********************************************************\ 
# Auto-generated CMakeLists.txt for the esteid project
#\**********************************************************/

# Written to work with cmake 2.6
cmake_minimum_required (VERSION 2.6)
set (CMAKE_BACKWARDS_COMPATIBILITY 2.6)

include(${CMAKE_DIR}/common.cmake)

set (PROJNAME ${PLUGIN_NAME})

browserplugin_project(${PROJNAME})

# Custom cmake modules
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")
find_package(SmartCardpp REQUIRED)
find_package(Gecko REQUIRED)

# Utility classes (FIXME: Move those to smartcardpp)
set(UTIL_DIR utility)
set(UTILITY_SOURCES
    ${UTIL_DIR}/threadObj.cpp
    ${UTIL_DIR}/converters.cpp
)

# Pthread is needed to link utility classes on UNIX-like OS-es
if(UNIX)
  link_libraries (pthread)
endif(UNIX)

include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${SMARTCARDPP_INCLUDE_DIR}
    ${UTIL_DIR}
)

# Need to include Gecko directories before Firebreath's bundled headers
include_directories(
    BEFORE
    ${GECKO_SDK_DIR}
    ${GECKO_INCLUDE_DIR}
)


set(XPIDL_FLAGS -I${GECKO_IDL_DIR} -I${CMAKE_CURRENT_SOURCE_DIR}/Mozilla/components)

macro(xpidl_generate_header idl header)
    add_custom_command(
        OUTPUT ${header}
        COMMAND ${GECKO_XPIDL_EXECUTABLE} ${XPIDL_FLAGS} -m header -e ${header} ${idl}
        DEPENDS ${idl}
    )
endmacro(xpidl_generate_header)

macro(xpidl_generate_typelib idl xpt)
    add_custom_command(
        OUTPUT ${xpt}
        COMMAND ${GECKO_XPIDL_EXECUTABLE} ${XPIDL_FLAGS} -m typelib -e ${xpt} ${idl}
        DEPENDS ${idl}
    )
endmacro(xpidl_generate_typelib)


# Generate header and typelib from IDL files
file(GLOB idlFiles ${CMAKE_CURRENT_SOURCE_DIR}/Mozilla/components/*.idl)
foreach(idlFile ${idlFiles})
    get_filename_component(basename ${idlFile} NAME_WE) # name without extension
    set(hFile ${basename}.h)
    set(xptFile ${basename}.xpt)

    xpidl_generate_header(${idlFile} ${hFile})
    xpidl_generate_typelib(${idlFile} ${xptFile})
endforeach(idlFile)

set(mozilla_ui_SRCS
    ${CMAKE_CURRENT_BINARY_DIR}/nsIEstEIDPrivate.h
    ${CMAKE_CURRENT_BINARY_DIR}/nsIEstEIDPrivate.xpt
    Mozilla/MozillaUI.cpp
)
set_source_files_properties(${mozilla_ui_SRCS} PROPERTIES
    COMPILE_FLAGS "${GECKO_DEFINITIONS}"
    )
    

file (GLOB GENERAL RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    *.cpp
    *.h
    *.cmake
    )

# Generated files are stored in ${GENERATED} by the project configuration
SET_SOURCE_FILES_PROPERTIES(
    ${GENERATED}
    PROPERTIES
        GENERATED 1
    )

SOURCE_GROUP(Generated FILES
    ${GENERATED}
    )

SET( SOURCES
    ${mozilla_ui_SRCS}
    ${GENERAL}
    ${GENERATED}
    ${UTILITY_SOURCES}
    )

# This will include Win/projectDef.cmake, Linux/projectDef.cmake, etc
include_platform()
