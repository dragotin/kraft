

include_directories(${KDE4_INCLUDES} ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} )


########### install files ###############

install(FILES  kplant_splash.png DESTINATION ${DATA_INSTALL_DIR}/kplant/pics)
install(FILES  kraftsplash.png docoverviewbg.png kraftapp_logo.png DESTINATION ${DATA_INSTALL_DIR}/kraft/pics)

kde4_install_icons(${ICON_INSTALL_DIR})



#original Makefile.am contents follow:

#kplantsplashdir = $(kde_datadir)/kplant/pics
#kplantsplash_DATA = kplant_splash.png
#
#kraftsplashdir = $(kde_datadir)/kraft/pics
#kraftsplash_DATA = muckilogo_oS.png mucki_en_oS.png muckilogo_oS.png docoverviewbg.png
#
#KDE_ICON = kraft alternative demand 
#
