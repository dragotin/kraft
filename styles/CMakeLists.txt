########### install files ###############
add_subdirectory(pics)

install(FILES templcatalog.style DESTINATION ${KDE_INSTALL_DATADIR}/kraft/styles)
install(FILES docdigestview.css docoverview.css docoverview_ro.css catalogview.css systemview.css DESTINATION ${KDE_INSTALL_DATADIR}/kraft/styles)

