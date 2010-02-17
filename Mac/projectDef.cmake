#/**********************************************************\ 
# Auto-generated Mac project definition file for the
# esteid project
#\**********************************************************/

# Mac template platform definition CMake file
# Included from ../CMakeLists.txt

find_library(FOUNDATION_LIBRARY Foundation)
find_library(APPKIT_LIBRARY AppKit)

set(EXTRA_LIBRARIES ${FOUNDATION_LIBRARY} ${APPKIT_LIBRARY})
    
# remember that the current source dir is the project root; this file is in Mac/
file (GLOB PLATFORM RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    Mac/*.cpp
    Mac/*.h
    Mac/*.m
    Mac/*.mm
    Mac/*.cmake
    )

# use this to add preprocessor definitions
add_definitions(
    
)

# Bundle resources
    file(GLOB_RECURSE RESOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}
    	Mac/bundle_template/*.png
        Mac/bundle_template/**/*.nib
        Mac/bundle_template/**/*.strings)

    foreach(_file ${RESOURCE_FILES})
        get_filename_component(_file_dir ${_file} PATH)
        file(RELATIVE_PATH _file_dir ${CMAKE_CURRENT_SOURCE_DIR}/Mac/bundle_template ${_file_dir})
        
        set_source_files_properties(${_file} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources/${_file_dir}" )
    endforeach(_file)

SOURCE_GROUP(Mac FILES ${PLATFORM})

set (SOURCES
    ${SOURCES}
    ${PLATFORM}
    ${RESOURCE_FILES}
    )

set(PLIST "Mac/bundle_template/Info.plist")
set(STRINGS "Mac/bundle_template/InfoPlist.strings")
set(LOCALIZED "Mac/bundle_template/Localized.r")

add_mac_plugin(${PROJNAME} ${PLIST} ${STRINGS} ${LOCALIZED} SOURCES)
    
# add library dependencies here; leave ${PLUGIN_INTERNAL_DEPS} there unless you know what you're doing!
target_link_libraries(${PROJNAME}
    ${PLUGIN_INTERNAL_DEPS}
    ${ICONV_LIBRARIES}
    ${OPENSSLCRYPTO_LIBRARIES}
    ${SMARTCARDPP_LIBRARIES}
    ${EXTRA_LIBRARIES}
    )

add_dependencies(${PROJNAME}
    ${PLUGIN_INTERNAL_DEPS}
    )
