add_subdirectory(global)

########### install files ###############

install(FILES kraftapp_logo.png DESTINATION ${DATA_INSTALL_DIR}/kraft/pics)

ecm_install_icons( ICONS
	16-actions-kraft_alternative.png
	16-actions-kraft_demand.png
	16-actions-kraft_fulltax.png
	16-actions-kraft_notax.png
	16-actions-kraft_redtax.png
	22-actions-kraftdice.png
	sc-apps-kraft_catalogue.svg
	DESTINATION share/icons)

