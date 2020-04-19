add_subdirectory(pics)

########### install files ###############

install(FILES
        delivery_receipt.trml
        invoice.trml
	weasyprint/kraft.css
	weasyprint/invoice.gtmpl
	DESTINATION ${DATA_INSTALL_DIR}/kraft/reports)

