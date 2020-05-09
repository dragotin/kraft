add_subdirectory(pics)

########### install files ###############

install(FILES
        delivery_receipt.trml
        invoice.trml
	kraft.css
	invoice.gtmpl
	DESTINATION ${DATA_INSTALL_DIR}/kraft/reports)

