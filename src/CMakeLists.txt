# include_directories(${KDE4_INCLUDES} ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_BINARY_DIR} ${KDEPIMLIBS_INCLUDE_DIRS} external)

include_directories(${QT_INCLUDES} src)
add_subdirectory(pics)

########### next target ###############

set(kraft_SRCS
    attribute.cpp
    einheit.cpp
    doctype.cpp
    numbercycle.cpp
    katalogman.cpp
    stdsatzman.cpp
    katalog.cpp
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
    catalogchapter.cpp
    addeditchapterdialog.cpp
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
    katalogview.cpp
    filterheader.cpp
    kataloglistview.cpp
    templkatalogview.cpp
    templkataloglistview.cpp
    positionviewwidget.cpp
    documentsaverbase.cpp
    documentsaverdb.cpp
    documentman.cpp
    docdigest.cpp
    docdigestview.cpp
    doctypeedit.cpp
    doctext.cpp
    portal.cpp
    portalview.cpp
    archiveman.cpp
    reportgenerator.cpp
    htmlview.cpp
    docpostcard.cpp
    catalogtemplate.cpp
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
    itemtagdialog.cpp
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
    addressprovider.cpp
    addressselectorwidget.cpp
    addressprovider_akonadi.cpp
    addressselectordialog.cpp
    models/globalcontactmodel.cpp
    quicksearchwidget.cpp
    kraftdb.cpp
    geld.cpp
    defaultprovider.cpp
)

kconfig_add_kcfg_files(kraft_SRCS
    databasesettings.kcfgc
    kraftsettings.kcfgc
    databasesettings.kcfgc
    kraftsettings.kcfgc
)

ki18n_wrap_ui(kraft_SRCS
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
    addressselectorwidget.ui
)

# kde4_add_executable(kraft ${kraft_SRCS})
add_executable(kraft ${kraft_SRCS})

set_target_properties(kraft PROPERTIES OUTPUT_NAME kraft)

target_link_libraries(kraft
PUBLIC
    Qt5::Core
    Qt5::Widgets
    Qt5::Sql
    Qt5::WebEngine
    Qt5::WebEngineWidgets
    
    KF5::CoreAddons
    KF5::GlobalAccel
    KF5::I18n
    KF5::NotifyConfig
    KF5::WidgetsAddons
    KF5::XmlGui
    KF5::Contacts
    KF5::AkonadiCore
    KF5::AkonadiContact
    KF5::AkonadiAgentBase 
    KF5::AkonadiWidgets 
    KF5::AkonadiXml
    ${CTEMPLATE_LIBRARIES} pthread
)

install(TARGETS kraft ${INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############

install(FILES  kraft.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
install(FILES  kraftsettings.kcfg databasesettings.kcfg DESTINATION ${KCFG_INSTALL_DIR})
install(FILES  kraftui.rc katalogview.rc DESTINATION ${DATA_INSTALL_DIR}/kraft)
