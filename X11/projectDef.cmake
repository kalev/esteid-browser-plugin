#/**********************************************************\ 
# Auto-generated X11 project definition file for the
# esteid project
#\**********************************************************/

# Windows template platform definition CMake file
# Included from ../CMakeLists.txt

if (ENABLE_RUN_IN_SOURCE_TREE)
    set(GLADE_INSTALL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/X11)
else()
    set(GLADE_INSTALL_DIR ${SHARE_INSTALL_PREFIX}/esteid-browser-plugin)
endif()

set(WHITELISTDIALOG_UI ${GLADE_INSTALL_DIR}/whitelistdialog.ui)
set(PININPUTDIALOG_UI ${GLADE_INSTALL_DIR}/pininputdialog.ui)
set(PINPADDIALOG_UI ${GLADE_INSTALL_DIR}/pinpaddialog.ui)

# remember that the current source dir is the project root; this file is in X11/
file (GLOB PLATFORM RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    X11/*.cpp
    X11/*.h
    X11/*.cmake
    )

SOURCE_GROUP(X11 FILES ${PLATFORM})

# use this to add preprocessor definitions
add_definitions(
    -DFB_X11=1
    -DWHITELISTDIALOG_UI="${WHITELISTDIALOG_UI}"
    -DPININPUTDIALOG_UI="${PININPUTDIALOG_UI}"
    -DPINPADDIALOG_UI="${PINPADDIALOG_UI}"
)

if(BUILD_TESTGTKUI)
    set(testgtkui_SRCS
        X11/pininputdialog.cpp
        X11/whitelistdialog.cpp
        X11/test/main.cpp
        X11/test/testgtkui.cpp
    )

    add_executable(testgtkui ${testgtkui_SRCS})
    target_link_libraries(testgtkui ${GTKMM_LIBRARIES})
endif()

set (SOURCES
    ${SOURCES}
    ${PLATFORM}
    )

# use custom version script
string(REGEX REPLACE " -Wl,--version-script=[^ ]*" "" ESTEID_LINK_FLAGS "${NPAPI_LINK_FLAGS}")
set(ESTEID_LINK_FLAGS "${ESTEID_LINK_FLAGS} -Wl,--version-script=${CMAKE_CURRENT_SOURCE_DIR}/X11/esteid_version_script.txt")

add_library(${PROJECT_NAME} SHARED ${SOURCES})

set_target_properties (${PROJECT_NAME} PROPERTIES
    OUTPUT_NAME np${PLUGIN_NAME}
    PROJECT_LABEL ${PROJECT_NAME}
    LINK_FLAGS "${ESTEID_LINK_FLAGS}"
    PREFIX ""
    RUNTIME_OUTPUT_DIRECTORY "${FB_BIN_DIR}/${PLUGIN_NAME}"
    LIBRARY_OUTPUT_DIRECTORY "${FB_BIN_DIR}/${PLUGIN_NAME}"
    )

# add library dependencies here; leave ${PLUGIN_INTERNAL_DEPS} there unless you know what you're doing!
target_link_libraries(${PROJECT_NAME}
    ${PLUGIN_INTERNAL_DEPS}
    ${Boost_LIBRARIES}
    ${GTK_LIBRARIES}
    ${GTKMM_LIBRARIES}
    ${ICONV_LIBRARIES}
    ${OPENSSLCRYPTO_LIBRARIES}
    ${SMARTCARDPP_LIBRARIES}
    )

install(TARGETS ${PROJECT_NAME} DESTINATION ${LIB_INSTALL_DIR}/mozilla/plugins)
install(FILES X11/whitelistdialog.ui DESTINATION ${GLADE_INSTALL_DIR})
install(FILES X11/pininputdialog.ui DESTINATION ${GLADE_INSTALL_DIR})
install(FILES X11/pinpaddialog.ui DESTINATION ${GLADE_INSTALL_DIR})
install(FILES X11/pinpad.png DESTINATION ${GLADE_INSTALL_DIR})
