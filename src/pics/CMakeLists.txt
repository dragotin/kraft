add_subdirectory(global)

########### install files ###############

install(FILES kraftsplash.png kraftapp_logo.png DESTINATION ${DATA_INSTALL_DIR}/kraft/pics)

ecm_install_icons( ICONS
	16-action-kraft_alternative.png
	16-action-kraft_demand.png
	16-action-kraft_fulltax.png
	16-action-kraft_notax.png
	16-action-kraft_redtax.png
	22-action-kraftdice.png
	sc-apps-kraft_catalogue.svg
	DESTINATION share/icons)

