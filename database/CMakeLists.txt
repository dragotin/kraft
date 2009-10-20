
include_directories(${KDE4_INCLUDES} ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} )

add_subdirectory(migration)


########### install files ###############

install(FILES  create_schema.sql fill_schema_de.sql fill_schema_en.sql  DESTINATION ${DATA_INSTALL_DIR}/kraft/dbinit)



#original Makefile.am contents follow:

#dbinitdir = $(kde_datadir)/kraft/dbinit
#dbinit_DATA = create_schema.sql fill_schema_de.sql fill_schema_en.sql 
#
#SUBDIRS = migration
#
