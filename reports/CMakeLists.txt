add_subdirectory(pics)

########### install files ###############

install(FILES
        delivery_receipt.trml
        invoice.trml
	kraft.css
	invoice.gtmpl
	xrechnung.xrtmpl
	DESTINATION ${DATA_INSTALL_DIR}/kraft/reports)

