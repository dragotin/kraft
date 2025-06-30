########### install files ###############

file(GLOB mig_scripts *_dbmigrate.sql)


install(FILES  README ${mig_scripts}
	DESTINATION ${KDE_INSTALL_DATADIR}/kraft/dbmigrate/mysql)

