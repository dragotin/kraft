
include_directories(${KDE4_INCLUDES} ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} )


########### install files ###############


install(FILES lurchie.png DESTINATION ${DATA_INSTALL_DIR}/kraft/reports/images)
install(FILES postit.png kraft_customer.png DESTINATION ${DATA_INSTALL_DIR}/kraft/reports/images/docdigestdetailview)
install(FILES identity.png DESTINATION ${DATA_INSTALL_DIR}/kraft/reports/images)
