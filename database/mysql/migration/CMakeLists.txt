########### install files ###############

file(GLOB mig_scripts *_dbmigrate.sql)


install(FILES  README ${mig_scripts}
        DESTINATION ${DATA_INSTALL_DIR}/kraft/dbmigrate/mysql)

