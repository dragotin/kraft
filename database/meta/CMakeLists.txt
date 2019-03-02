########### install files ###############

file(GLOB meta_scripts *_meta.xml)


install(FILES ${meta_scripts}
        DESTINATION ${DATA_INSTALL_DIR}/kraft/meta)

