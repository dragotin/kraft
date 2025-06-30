add_subdirectory(migration)

########### install files ###############

install(FILES create_schema.sql fill_schema_de.sql fill_schema_en.sql  DESTINATION ${KDE_INSTALL_DATADIR}/kraft/dbinit/sqlite3)
