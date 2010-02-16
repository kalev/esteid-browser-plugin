# Make life easier :P
set(MKDIR ${CMAKE_COMMAND} -E make_directory)
set(RM ${CMAKE_COMMAND} -E remove)
set(CP ${CMAKE_COMMAND} -E copy)

configure_file(install.rdf.in "${CMAKE_CURRENT_BINARY_DIR}/install.rdf")

set(COMP_DEST "${CMAKE_CURRENT_BINARY_DIR}/components")
add_custom_command(
    OUTPUT ${COMP_DEST}/nsEstEIDPrivate.js
    COMMAND ${CP} nsEstEIDPrivate.js ${COMP_DEST}
    DEPENDS components/nsEstEIDPrivate.js
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/components
)

# Create directories
add_custom_target( xpi-stage
        COMMAND ${MKDIR} chrome
        COMMAND ${MKDIR} components
)

set(CHROME_JAR "${CMAKE_CURRENT_BINARY_DIR}/chrome/esteid.jar")
add_custom_target( xpi-jar
        COMMAND ${RM} ${CHROME_JAR}
        COMMAND zip -pr ${CHROME_JAR} content/* locale/* skin/* -x '*/.svn/*'
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/chrome
)

add_dependencies( xpi-jar xpi-stage)

# Build the XPI
add_custom_target( xpi ALL
        DEPENDS ${PROJNAME}
        DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/components/nsEstEIDPrivate.js
        DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/install.rdf
        DEPENDS chrome.manifest
        COMMAND ${RM} chrome.manifest
        COMMAND ${CP} ${CMAKE_CURRENT_SOURCE_DIR}/chrome.manifest .
        COMMAND ${RM} esteid-${FBSTRING_PLUGIN_VERSION}.xpi
        COMMAND zip -pr esteid-${FBSTRING_PLUGIN_VERSION}.xpi
                chrome/ components/ install.rdf chrome.manifest
        VERBATIM
)
add_dependencies( xpi xpi-jar)
