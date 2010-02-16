# Make life easier :P
set(MKDIR ${CMAKE_COMMAND} -E make_directory)
set(RM ${CMAKE_COMMAND} -E remove)
set(CP ${CMAKE_COMMAND} -E copy)


set(XPI_OUT ${CMAKE_CURRENT_BINARY_DIR}/esteid-${FBSTRING_PLUGIN_VERSION}.xpi)
set(INSTALLRDF ${CMAKE_CURRENT_BINARY_DIR}/install.rdf)

configure_file(install.rdf.in ${INSTALLRDF})

# Build the XPI
add_custom_target( xpi ALL
        DEPENDS ${PROJNAME}
        DEPENDS ${INSTALLRDF}
        DEPENDS chrome.manifest
        DEPENDS chrome
        COMMAND ${RM} ${XPI_OUT}
        COMMAND zip -pr ${XPI_OUT} chrome chrome.manifest 
                    -j ${INSTALLRDF} -x */.svn/*
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        VERBATIM
)
