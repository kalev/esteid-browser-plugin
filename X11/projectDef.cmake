#/**********************************************************\ 
# Auto-generated X11 project definition file for the
# esteid project
#\**********************************************************/

# Windows template platform definition CMake file
# Included from ../CMakeLists.txt

set(GLADE_INSTALL_DIR ${SHARE_INSTALL_PREFIX}/esteid-browser-plugin)
set(WHITELISTDIALOG_UI ${GLADE_INSTALL_DIR}/whitelistdialog.ui)
set(PININPUTDIALOG_UI ${GLADE_INSTALL_DIR}/pininputdialog.ui)

# remember that the current source dir is the project root; this file is in ${PLATFORM_NAME}/
file (GLOB PLATFORM RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    ${PLATFORM_NAME}/*.cpp
    ${PLATFORM_NAME}/*.h
    ${PLATFORM_NAME}/*.cmake
    )

SOURCE_GROUP(${PLATFORM_NAME} FILES ${PLATFORM})

# use this to add preprocessor definitions
add_definitions(
    -DFB_X11=1
    -DWHITELISTDIALOG_UI="${WHITELISTDIALOG_UI}"
    -DPININPUTDIALOG_UI="${PININPUTDIALOG_UI}"
)

if(BUILD_TESTGTKUI)
    set(testgtkui_SRCS
        ${PLATFORM_NAME}/pininputdialog.cpp
        ${PLATFORM_NAME}/whitelistdialog.cpp
        ${PLATFORM_NAME}/test/main.cpp
        ${PLATFORM_NAME}/test/testgtkui.cpp
    )

    add_executable(testgtkui ${testgtkui_SRCS})
    target_link_libraries(testgtkui ${GTKMM_LIBRARIES})
endif()

set (SOURCES
    ${SOURCES}
    ${PLATFORM}
    )

add_library(${PROJNAME} SHARED ${SOURCES})

set_target_properties (${PROJNAME} PROPERTIES
    OUTPUT_NAME np${PLUGIN_NAME}
    PROJECT_LABEL ${PROJNAME}
    LINK_FLAGS "${NPAPI_LINK_FLAGS}"
    PREFIX ""
    RUNTIME_OUTPUT_DIRECTORY "${BIN_DIR}/${PLUGIN_NAME}"
    LIBRARY_OUTPUT_DIRECTORY "${BIN_DIR}/${PLUGIN_NAME}"
    )

# add library dependencies here; leave ${PLUGIN_INTERNAL_DEPS} there unless you know what you're doing!
target_link_libraries(${PROJNAME}
    ${PLUGIN_INTERNAL_DEPS}
    ${GTK_LIBRARIES}
    ${GTKMM_LIBRARIES}
    ${ICONV_LIBRARIES}
    ${OPENSSLCRYPTO_LIBRARIES}
    ${SMARTCARDPP_LIBRARIES}
    )

add_dependencies(${PROJNAME}
    ${PLUGIN_INTERNAL_DEPS}
    )

install(TARGETS ${PROJNAME} DESTINATION ${LIB_INSTALL_DIR}/mozilla/plugins)
install(FILES ${PLATFORM_NAME}/whitelistdialog.ui DESTINATION ${GLADE_INSTALL_DIR})
install(FILES ${PLATFORM_NAME}/pininputdialog.ui DESTINATION ${GLADE_INSTALL_DIR})
