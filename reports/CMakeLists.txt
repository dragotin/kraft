add_subdirectory(pics)

########### install files ###############

install(FILES identity.trml invoice.trml kraftdoc_ro.trml docdigest.trml DESTINATION ${DATA_INSTALL_DIR}/kraft/reports)

# localised document templates:
install(FILES de/invoice.trml DESTINATION ${DATA_INSTALL_DIR}/kraft/reports/de)
