add_subdirectory(pics)

########### install files ###############

install(FILES
        delivery_receipt.trml
        invoice.trml
	DESTINATION ${DATA_INSTALL_DIR}/kraft/reports)

# localised document templates:
install(FILES de/invoice.trml de/lieferschein.trml
	DESTINATION ${DATA_INSTALL_DIR}/kraft/reports/de)
