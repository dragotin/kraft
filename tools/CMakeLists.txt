
include_directories(${KDE4_INCLUDES} ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} )

########### install files ###############
install(PROGRAMS erml2pdf erml2pdf.py DESTINATION ${KDE4_BIN_INSTALL_DIR} )
