########### install files ###############
add_subdirectory(pics)

install(FILES templcatalog.style DESTINATION ${DATA_INSTALL_DIR}/kraft/styles)
install(FILES docdigestview.css docoverview.css docoverview_ro.css catalogview.css systemview.css DESTINATION ${DATA_INSTALL_DIR}/kraft/styles)

