
FIND_PACKAGE(Asciidoctor)
SET (install_files kraft-en.html kraft-de.html)

if (ASCIIDOCTOR_FOUND)
  SET( manual_src ${PROJECT_SOURCE_DIR}/manual/kraft.adoc )
  SET( out_file kraft-en.html )

  ADD_CUSTOM_COMMAND(
    OUTPUT "${out_file}"
    COMMAND "${PROJECT_SOURCE_DIR}/manual/makeman.sh" ARGS "${PROJECT_SOURCE_DIR}/manual"
    DEPENDS ${manual_src}
    COMMENT "Make the manual"
  )
  message( "Asciidoctor found")
  ADD_CUSTOM_TARGET(manual ALL echo
    DEPENDS "${out_file}"
  )

  INSTALL(FILES ${CMAKE_BINARY_DIR}/manual/kraft-en.html ${CMAKE_BINARY_DIR}/manual/kraft-de.html
          DESTINATION ${DATA_INSTALL_DIR}/kraft/manual)

else(ASCIIDOCTOR_FOUND)
  # Maybe there are preinstalled html files in the tarball
  message("Asciidoctor not found, installing pre built manual.")
  install(FILES "kraft-de.html" "kraft-en.html" DESTINATION ${DATA_INSTALL_DIR}/kraft/manual)
endif(ASCIIDOCTOR_FOUND)

INSTALL(FILES kraftmanual.css DESTINATION ${DATA_INSTALL_DIR}/kraft/manual)
INSTALL(DIRECTORY images DESTINATION ${DATA_INSTALL_DIR}/kraft/manual)
