set(MKDIR ${CMAKE_COMMAND} -E make_directory)
set(RM    ${CMAKE_COMMAND} -E remove)
set(RMDIR ${CMAKE_COMMAND} -E remove_directory)
set(CP    ${CMAKE_COMMAND} -E copy)
set(CPDIR ${CMAKE_COMMAND} -E copy_directory)

set(XPI_STAGE_DIR ${CMAKE_CURRENT_BINARY_DIR}/xpi)
set(XPI_OUT ${PROJECT_BINARY_DIR}/esteid-${FBSTRING_PLUGIN_VERSION}.xpi)

configure_file(install.rdf.in ${CMAKE_CURRENT_BINARY_DIR}/install.rdf)

# Stage all files which are going to end up in the .xpi
add_custom_target(xpi-stage
    COMMENT "Staging xpi content"

    COMMAND ${RMDIR} ${XPI_STAGE_DIR}
    COMMAND ${CPDIR} ${CMAKE_CURRENT_SOURCE_DIR}/chrome ${XPI_STAGE_DIR}/chrome
    COMMAND ${CP}    ${CMAKE_CURRENT_SOURCE_DIR}/chrome.manifest ${XPI_STAGE_DIR}
    COMMAND ${CP}    ${CMAKE_CURRENT_BINARY_DIR}/install.rdf ${XPI_STAGE_DIR}
    VERBATIM
)

# Compress the XPI
add_custom_target(zip-xpi ALL
    DEPENDS ${PROJNAME}
    COMMENT "Compressing xpi file"

    WORKING_DIRECTORY ${XPI_STAGE_DIR}
    COMMAND ${RM}  ${XPI_OUT}
    COMMAND zip -r ${XPI_OUT} . -x *.svn/*
    VERBATIM
)
add_dependencies(zip-xpi xpi-stage)

add_custom_target(runff
        DEPENDS run-in-ff.sh
        COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/run-in-ff.sh ${XPI_OUT} "${BIN_DIR}/${PLUGIN_NAME}"
)
add_dependencies(runff zip-xpi)

add_custom_target(runtb
        DEPENDS run-in-tb.sh
        COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/run-in-tb.sh ${XPI_OUT} "${BIN_DIR}/${PLUGIN_NAME}"
)
add_dependencies(runtb zip-xpi)
