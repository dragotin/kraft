
include_directories(${KDE4_INCLUDES} ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} )

########### install files ###############

install(FILES templcatalog.style DESTINATION ${DATA_INSTALL_DIR}/kraft/styles)
install(FILES docoverview.css docoverview_ro.css catalogview.css DESTINATION ${DATA_INSTALL_DIR}/kraft)

