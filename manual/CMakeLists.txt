
FIND_PACKAGE(Asciidoctor)

SET( manual_src ${PROJECT_SOURCE_DIR}/manual/kraft.adoc )
SET( out_file kraft_en.html )

ADD_CUSTOM_COMMAND(
    OUTPUT "${out_file}"
    COMMAND ${ASCIIDOCTOR_EXECUTABLE} -a VERSION=${APPLICATION_VERSION} -o ${out_file} ${manual_src}
    DEPENDS ${manual_src}
    COMMENT "asciidoctor ${manual_src}"
)

ADD_CUSTOM_TARGET(manual ALL echo
    DEPENDS "${out_file}"
)

INSTALL(FILES ${CMAKE_BINARY_DIR}/manual/${out_file} DESTINATION ${DATA_INSTALL_DIR}/kraft/manual)
INSTALL(DIRECTORY images DESTINATION ${DATA_INSTALL_DIR}/kraft/manual)
