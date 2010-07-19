cmake_minimum_required(VERSION 2.6)
project(TestWindowsUI)

#find_package(Boost COMPONENTS signals REQUIRED)

# XXX
find_package(Boost REQUIRED)
set(Boost_LIBRARIES boost_signals-gcc44-1_41)

include_directories(${Boost_INCLUDE_DIRS})

set(testwhitelist_SRCS
    ../basedialog.cpp
    ../whitelistdialog.cpp
    ../../PluginSettings.cpp
    ../../converter.cpp
    testwhitelist.cpp
)

set(testpininput_SRCS
    ../basedialog.cpp
    ../pininputdialog.cpp
    ../../PluginSettings.cpp
    ../../converter.cpp
    testpininput.cpp
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

add_executable(testwhitelist WIN32
    ${testwhitelist_SRCS}
    ${WINRC}
)
target_link_libraries(testwhitelist iconv comctl32)

add_executable(testpininput WIN32
    ${testpininput_SRCS}
    ${WINRC}
)
target_link_libraries(testpininput iconv comctl32 ${Boost_LIBRARIES})
