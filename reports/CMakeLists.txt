add_subdirectory(pics)

########### install files ###############

install(FILES
	allvars.gtmpl
	invoice.gtmpl
        delivery_note.gtmpl
	pagestyle.css
	kraft.css
	xrechnung.xrtmpl
	DESTINATION ${KDE_INSTALL_DATADIR}/kraft/reports)

