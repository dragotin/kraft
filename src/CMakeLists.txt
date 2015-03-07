include_directories(${KDE4_INCLUDES} ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_BINARY_DIR} ${KDEPIMLIBS_INCLUDE_DIRS} external)

add_subdirectory(pics)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}" )

########### next target ###############

set(kraft_SRCS
    catalogtemplate.cpp
    catalogchapter.cpp
    katalog.cpp
    katalogman.cpp
    brunskatalog.cpp
    brunsrecord.cpp
    defaultprovider.cpp
    doctext.cpp
    doctype.cpp
    kataloglistview.cpp
    katalogview.cpp
    brunskataloglistview.cpp
    brunskatalogview.cpp
    filterheader.cpp
    kraftdb.cpp
    geld.cpp
    einheit.cpp
    attribute.cpp
    numbercycle.cpp
    addeditchapterdialog.cpp
    templkatalog.cpp
    docposition.cpp
    matkatalog.cpp
    timecalcpart.cpp
    fixcalcpart.cpp
    floskeltemplate.cpp
    floskel.cpp
    calcpart.cpp
    kraftview.cpp
    kraftdoc.cpp
    main.cpp
    unitmanager.cpp
    stockmaterial.cpp
    stockmaterialman.cpp
    materialcalcpart.cpp
    flostempldialog.cpp
    stdsatzman.cpp
    materialtempldialog.cpp
    timecalcdialog.cpp
    fixcalcdialog.cpp
    matcalcdialog.h
    matcalcdialog.cpp
    templatesaverbase.cpp
    templatesaverdb.cpp
    materialsaverbase.cpp
    materialsaverdb.cpp
    prefsdialog.cpp
    prefswages.cpp
    prefsunits.cpp
    templkatalogview.cpp
    templkataloglistview.cpp
    positionviewwidget.cpp
    documentsaverbase.cpp
    documentsaverdb.cpp
    documentman.cpp
    docdigest.cpp
    docdigestview.cpp
    doctypeedit.cpp
    portal.cpp
    portalview.cpp
    archiveman.cpp
    reportgenerator.cpp
    htmlview.cpp
    docpostcard.cpp
    catalogselection.cpp
    kraftdocheaderedit.cpp
    kraftdocfooteredit.cpp
    inserttempldialog.cpp
    archdocposition.cpp
    archdoc.cpp
    materialkataloglistview.cpp
    materialkatalogview.cpp
    materialselectdialog.cpp
    kraftdocedit.cpp
    kraftdocpositionsedit.cpp
    portalhtmlview.cpp
    templtopositiondialogbase.cpp
    insertplantdialog.cpp
    textselection.cpp
    newdocassistant.cpp
    docassistant.cpp
    texteditdialog.cpp
    templateprovider.cpp
    headertemplateprovider.cpp
    footertemplateprovider.cpp
    catalogtemplateprovider.cpp
    texttemplate.cpp
    doclocaledialog.cpp
    tagman.cpp
    positiontagdialog.cpp
    tagtemplatesdialog.cpp
    importitemdialog.cpp
    importfilter.cpp
    kraftview_ro.cpp
    taxeditdialog.cpp
    numbercycledialog.cpp
    impviewwidgets.cpp
    setupassistant.cpp
    models/documentmodel.cpp
    models/documentproxymodels.cpp
    docdigestdetailview.cpp
    external/ktreeviewsearchline.cpp
    addressprovider.cpp
    akonadiaddressselector.cpp
    akonadiaddressselectordialog.cpp
    models/globalcontactmodel.cpp
    quicksearchwidget.cpp
)

kde4_add_kcfg_files(kraft_SRCS
    databasesettings.kcfgc
    kraftsettings.kcfgc
    databasesettings.kcfgc
    kraftsettings.kcfgc)

kde4_add_ui_files(kraft_SRCS
    calctemplate.ui
    dbinit.ui
    createdb.ui
    dbselect.ui
    mysqldetails.ui
    sqlitedetails.ui
    statuspage.ui
    upgradedb.ui
    timepart.ui
    fixpartui.ui
    matpartui.ui
    inserttmplbase.ui
    materialdialog.ui
    insertplantbase.ui
    docheader.ui
    positionwidget.ui
    docfooter.ui
    texteditbase.ui
    positionwidget.ui
    doctypeeditbase.ui
    taxeditbase.ui
    wageseditbase.ui
    unitseditbase.ui
    importtodocbase.ui
    numbercycleseditbase.ui

)

kde4_add_executable(kraft ${kraft_SRCS})

target_link_libraries(kraft ${QT_QTSQL_LIBRARY} ${KDE4_KHTML_LIBS} ${KDE4_KDEUI_LIBS}
	${KDEPIMLIBS_KABC_LIBS} ${KDEPIMLIBS_AKONADI_CONTACT_LIBS} ${KDE4_KIO_LIBS}
        ${CTEMPLATE_LIBRARIES} pthread)

install(TARGETS kraft ${INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############

install(FILES  kraft.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
install(FILES  kraftsettings.kcfg databasesettings.kcfg DESTINATION ${KCFG_INSTALL_DIR})
install(FILES  kraftui.rc katalogview.rc DESTINATION ${DATA_INSTALL_DIR}/kraft)
