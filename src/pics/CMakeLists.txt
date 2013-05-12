add_subdirectory(global)

########### install files ###############

install(FILES  kraftsplash.png docoverviewbg.png kraftapp_logo.png DESTINATION ${DATA_INSTALL_DIR}/kraft/pics)

kde4_install_icons(${DATA_INSTALL_DIR}/kraft/icons)
