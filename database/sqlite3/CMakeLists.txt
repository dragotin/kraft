add_subdirectory(migration)

########### install files ###############

install(FILES create_schema.sql fill_schema_de.sql fill_schema_en.sql  DESTINATION ${DATA_INSTALL_DIR}/kraft/dbinit/sqlite3)
