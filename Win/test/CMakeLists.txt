cmake_minimum_required(VERSION 2.6)
project(testwindowsui)

set(SRCS
    ../whitelistdialog.cpp
    ../../PluginSettings.cpp
    ../../converter.cpp
    main.cpp
)

add_definitions(
    -DUNICODE -D_UNICODE
    -D_WIN32_WINNT=0x501
    -D_WIN32_IE=0x0600
    -DWIN32_LEAN_AND_MEAN
    -DFBSTRING_PluginName="shut_up"
)

set(WINDRES "i686-pc-mingw32-windres")

if (MINGW)
    add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/test_rc.o
                       COMMAND ${WINDRES} -I ${CMAKE_CURRENT_SOURCE_DIR}/../..
                                          -i ${CMAKE_CURRENT_SOURCE_DIR}/test.rc
                                          -o ${CMAKE_CURRENT_BINARY_DIR}/test_rc.o)
    set(WINRC ${CMAKE_CURRENT_BINARY_DIR}/test_rc.o)
else()
    set(WINRC resources/test.rc)
endif()

include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}/../..
    ${CMAKE_CURRENT_SOURCE_DIR}/../../..
    ${CMAKE_CURRENT_SOURCE_DIR}/../../utility
)

add_executable(testwindowsui WIN32
    ${SRCS}
    ${WINRC}
)

target_link_libraries(testwindowsui iconv comctl32)
