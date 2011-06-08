#/**********************************************************\ 
# Auto-generated Windows project definition file for the
# esteid project
#\**********************************************************/

# Windows template platform definition CMake file
# Included from ../CMakeLists.txt

# remember that the current source dir is the project root; this file is in Win/
file (GLOB PLATFORM RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    Win/*.cpp
    Win/*.h
    Win/*.cmake
    )

# use this to add preprocessor definitions
add_definitions(
    /D "_ATL_STATIC_REGISTRY"
    /D ISOLATION_AWARE_ENABLED=1
)

SOURCE_GROUP(Win FILES ${PLATFORM})

set (SOURCES
    ${SOURCES}
    ${PLATFORM}
    )

add_windows_plugin(${PROJECT_NAME} SOURCES)

# add library dependencies here; leave ${PLUGIN_INTERNAL_DEPS} there unless you know what you're doing!
target_link_libraries(${PROJECT_NAME}
    ${PLUGIN_INTERNAL_DEPS}
    ${Boost_LIBRARIES}
    ${ICONV_LIBRARIES}
    ${OPENSSLCRYPTO_LIBRARIES}
    ${SMARTCARDPP_LIBRARIES}
    comctl32
    ws2_32
    )

install(TARGETS ${PROJECT_NAME} DESTINATION ${LIB_INSTALL_DIR}/mozilla/plugins)
