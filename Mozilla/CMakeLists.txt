set(MKDIR ${CMAKE_COMMAND} -E make_directory)
set(RM    ${CMAKE_COMMAND} -E remove)
set(CP    ${CMAKE_COMMAND} -E copy)
set(CPDIR ${CMAKE_COMMAND} -E copy_directory)

set(XPI_STAGE ${CMAKE_CURRENT_BINARY_DIR}/xpi)
set(XPI_OUT ${PROJECT_BINARY_DIR}/esteid-${FBSTRING_PLUGIN_VERSION}.xpi)

configure_file(install.rdf.in ${XPI_STAGE}/install.rdf)

# Stage all files which are going to end up in the .xpi
add_custom_target(xpi-stage
    COMMAND ${CPDIR} ${CMAKE_CURRENT_SOURCE_DIR}/chrome ${XPI_STAGE}
    COMMAND ${CP}    ${CMAKE_CURRENT_SOURCE_DIR}/chrome.manifest ${XPI_STAGE}
    VERBATIM
)

# Compress the XPI
add_custom_target(zip-xpi ALL
    DEPENDS ${PROJNAME}
    DEPENDS ${XPI_STAGE}/install.rdf
    COMMENT "Compressing xpi file"

    WORKING_DIRECTORY ${XPI_STAGE}
    COMMAND ${RM}  ${XPI_OUT}
    COMMAND zip -r ${XPI_OUT} . -x *.svn/*
    VERBATIM
)
add_dependencies(zip-xpi xpi-stage)
