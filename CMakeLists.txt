project(${PLUGIN_NAME})

# Written to work with cmake 2.6
cmake_minimum_required (VERSION 2.6)
set (CMAKE_BACKWARDS_COMPATIBILITY 2.6)

# Options which affect the way the project is built
set(ENABLE_DEBUG FALSE
    CACHE BOOL "Set to TRUE to enable debug print statements (default: FALSE)"
)
option(ENABLE_OLD_APIS "Support OLD API-s (default: TRUE)" TRUE)
option(ENABLE_RUN_IN_SOURCE_TREE "Enable search for the config file and UI files in local directory" FALSE)


# FIXME: Merge with libdigidocpp/CMakeLists.txt and create a proper
#        cmake module that is used by qesteidutil as well
if(WIN32)
    set(PKCS11_MODULE_NAME "onepin-opensc-pkcs11.dll")
else()
    set(PKCS11_MODULE_NAME "onepin-opensc-pkcs11.so")
endif()
IF(PKCS11_MODULE_DIR)
    SET(PKCS11_MODULE "${PKCS11_MODULE_DIR}/${PKCS11_MODULE_NAME}")
ENDIF(PKCS11_MODULE_DIR)

find_package(Subversion)
if(Subversion_FOUND AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/.svn)
    Subversion_WC_INFO(${CMAKE_CURRENT_SOURCE_DIR} PROJECT)
    set(SVN_REV ${PROJECT_WC_LAST_CHANGED_REV})
    message(STATUS "Current subversion revision is ${SVN_REV}")
else()
    message( "Unable to determine subversion revision")
endif()

# Custom cmake modules
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")
include(InstallSettings)

find_package(Gettext)
find_package(Iconv REQUIRED)
find_package(OpenSSLCrypto REQUIRED)
find_package(SmartCardpp REQUIRED)
if (UNIX)
    find_package(PCSCLite)
endif()

if (WITH_SYSTEM_BOOST)
    # FindBoost.cmake isn't up to date wrt new boost versions
    set(Boost_ADDITIONAL_VERSIONS "1.44" "1.44.0" "1.43" "1.43.0" "1.42" "1.42.0")

    if (WIN32)
        # boost::signals is only needed for Windows UI code
        set(EXTRA_BOOST_COMPONENTS "signals")
    endif()
    find_package(Boost 1.41.0 COMPONENTS filesystem system thread ${EXTRA_BOOST_COMPONENTS} REQUIRED)
else()
    message(STATUS "Using bundled boost")
    if (WIN32)
        # boost::signals is only needed for Windows UI code
        add_boost_library(signals)
    endif()
    add_boost_library(filesystem)
    add_boost_library(system)
    add_boost_library(thread)
endif()

if(UNIX AND NOT APPLE)
    if(NOT GTKMM_INCLUDE_DIRS)
        pkg_check_modules(GTKMM REQUIRED gtkmm-2.4)
    endif()
endif(UNIX AND NOT APPLE)

if(ICONV_SECOND_ARGUMENT_IS_CONST)
    add_definitions( -DICONV_SECOND_ARGUMENT_IS_CONST )
endif(ICONV_SECOND_ARGUMENT_IS_CONST)

if(ENABLE_DEBUG)
    message(STATUS "Debug print statements are enabled")
else()
    add_definitions(-DNDEBUG)
endif()

if(ENABLE_OLD_APIS)
    message(STATUS "Support for OLD API-s is enabled")
    add_definitions(-DSUPPORT_OLD_APIS)
endif(ENABLE_OLD_APIS)


include_directories(
    ${PLUGIN_INCLUDE_DIRS}
    ${Boost_INCLUDE_DIRS}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${GTKMM_INCLUDE_DIRS}
    ${ICONV_INCLUDE_DIR}
    ${OPENSSLCRYPTO_INCLUDE_DIR}
    ${PCSCLITE_INCLUDE_DIR}
    ${SMARTCARDPP_INCLUDE_DIRS}
    ${UTIL_DIR}
)

if( APPLE )
    include_directories( /usr/local/include/../include )
endif( APPLE )

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
    ${GENERAL}
    ${GENERATED}
    )

if (ENABLE_RUN_IN_SOURCE_TREE)
    set(ESTEID_CONFIGDIR "${CMAKE_CURRENT_SOURCE_DIR}")
else()
    set(ESTEID_CONFIGDIR "${SYSCONF_INSTALL_DIR}")
endif()

include(CheckIncludeFiles)
check_include_files(libintl.h HAVE_LIBINTL_H)

set(ESTEID_LOCALEDIR ${SHARE_INSTALL_PREFIX}/locale)
if(GETTEXT_FOUND AND HAVE_LIBINTL_H)
    # Generate translations
    file(GLOB PO_FILES po/*.po)
    gettext_create_translations(po/esteid-browser-plugin.pot ALL ${PO_FILES})
else()
    message(STATUS "Gettext or libintl not found, skipping translations")
endif()

configure_file(esteid-config.h.in ${CMAKE_CURRENT_BINARY_DIR}/esteid-config.h)

# This will include Win/projectDef.cmake, Linux/projectDef.cmake, etc
include_platform()

add_subdirectory(Mozilla)

install(FILES esteid-browser-plugin.conf DESTINATION ${ESTEID_CONFIGDIR})
