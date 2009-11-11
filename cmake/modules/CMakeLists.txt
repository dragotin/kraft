# explicitely install the "public" cmake files

set(cmakeFiles
  findCtemplate.cmake
)

install(FILES ${cmakeFiles} DESTINATION ${DATA_INSTALL_DIR}/cmake/modules)

