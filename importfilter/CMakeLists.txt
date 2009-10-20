
include_directories(${KDE4_INCLUDES} ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} )


########### install files ###############

install(FILES woerlein_txt.ftr standard_txt.ftr DESTINATION ${DATA_INSTALL_DIR}/kraft/importfilter/positions/)



#original Makefile.am contents follow:

#
#migrationsdir = $(kde_datadir)/kraft/dbmigrate
#migrations_DATA = README 2_dbmigrate.sql 3_dbmigrate.sql 4_dbmigrate.sql \
#5_dbmigrate.sql 6_dbmigrate.sql 7_dbmigrate.sql 8_dbmigrate.sql \
#9_dbmigrate.sql 10_dbmigrate.sql 11_dbmigrate.sql
