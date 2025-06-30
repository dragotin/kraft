########### install files ###############

file(GLOB meta_scripts *_meta.xml)


install(FILES ${meta_scripts}
	DESTINATION ${KDE_INSTALL_DATADIR}/kraft/meta)

