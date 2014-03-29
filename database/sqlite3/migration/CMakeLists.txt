########### install files ###############

file(GLOB mig_scripts *_dbmigrate.sql)
install(FILES ${mig_scripts} 
	DESTINATION ${DATA_INSTALL_DIR}/kraft/dbmigrate/sqlite3)

