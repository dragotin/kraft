add_subdirectory(pics)

########### install files ###############

install(FILES identity.trml invoice.trml kraftdoc_ro.trml docdigest.trml delivery_receipt.trml
	DESTINATION ${DATA_INSTALL_DIR}/kraft/reports)

# localised document templates:
install(FILES de/invoice.trml de/lieferschein.trml
	DESTINATION ${DATA_INSTALL_DIR}/kraft/reports/de)
