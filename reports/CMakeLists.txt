add_subdirectory(pics)

########### install files ###############

install(FILES
        delivery_receipt.trml
        invoice.trml
	DESTINATION ${DATA_INSTALL_DIR}/kraft/reports)

