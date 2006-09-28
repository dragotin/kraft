


####### kdevelop will overwrite this part!!! (begin)##########

lib_LTLIBRARIES = libbruns.la

libbruns_la_SOURCES = katalog.cpp katalogman.cpp  brunskatalog.cpp brunsrecord.cpp \
kataloglistview.cpp katalogview.cpp brunskataloglistview.cpp brunskatalogview.cpp \
filterheader.cpp catalogchapteredit.cpp docposition.cpp kraftdb.cpp \
geld.cpp einheit.cpp katalogsettings.kcfgc

libbruns_la_LDFLAGS = $(KDE_RPATH) $(all_libraries) -version-info 1:0:0 -no-undefined
libbruns_la_LIBADD = $(LIB_KIO) $(all_libraries)

bin_PROGRAMS = kraft kplant

## INCLUDES were found outside kdevelop specific part

kraft_SOURCES = templkatalog.cpp matkatalog.cpp zeitcalcpart.cpp \
	fixcalcpart.cpp floskeltemplate.cpp floskel.cpp calcpart.cpp kraftview.cpp \
	kraftdoc.cpp main.cpp unitmanager.cpp stockmaterial.cpp \
	stockmaterialman.cpp materialcalcpart.cpp flostab.ui flostempldialog.cpp \
	stdsatzman.cpp zeitpartui.ui fixpartui.ui matpartui.ui inserttmplbase.ui \
	zeitcalcdialog.cpp fixcalcdialog.cpp mateditor.cpp matdatatable.cpp matcalcdialog.h \
	matcalcdialog.cpp templatesaverbase.cpp templatesaverdb.cpp kraftsettings.kcfgc \
	prefsdialog.cpp templkatalogview.cpp templkataloglistview.cpp katalogsettings.kcfgc \
	docheader.ui positionwidget.ui positionviewwidget.cpp \
	documentsaverbase.cpp documentsaverdb.cpp docfooter.ui documentman.cpp docdigest.cpp \
	docdigestview.cpp portal.cpp portalview.cpp archiveman.cpp reportgenerator.cpp \
	docoverviewwidget.cpp htmlview.cpp docpostcard.cpp catalogselection.cpp \
	addressselection.cpp kraftdocheaderedit.cpp kraftdocfooteredit.cpp \
	inserttempldialog.cpp

kraft_LDADD   =  libbruns.la $(LIB_KIO) $(LIB_KABC) -lkhtml 

kplant_SOURCES = brunsviewer.cpp brunsviewmain.cpp
kplant_LDADD = libbruns.la $(LIB_KIO)

EXTRA_DIST = kraftdoc.h kraftview.h kraftui.rc katalogview.rc kraft.desktop lo32-app-kange.png lo16-app-kange.png calcpart.cpp calcpart.h floskel.cpp floskel.h kraftglobals.h einheit.cpp einheit.h floskeltemplate.cpp floskeltemplate.h fixcalcpart.cpp fixcalcpart.h zeitcalcpart.cpp zeitcalcpart.h kraftdb.cpp kraftdb.h katalog.cpp templkatalog.cpp matkatalog.cpp matkatalog.h katalog.h templkatalog.h kataloglistview.cpp kataloglistview.h katalogview.h dbids.h unitmanager.h stockmaterial.h stockmaterialman.h materialcalcpart.h flostempldialog.h geld.h katalogman.h stdsatzman.h zeitcalcdialog.h fixcalcdialog.h mateditor.cpp mateditor.h matdatatable.cpp matdatatable.h matcalcdialog.h matcalcdialog.cpp templatesaverbase.cpp templatesaverbase.h templatesaverdb.cpp templatesaverdb.h 

KDE_ICON = kraft

xdg_apps_DATA = kraft.desktop

kde_kcfg_DATA = kraftsettings.kcfg katalogsettings.kcfg

####### kdevelop will overwrite this part!!! (end)############
# These paths are KDE specific. Use them:
# kde_appsdir         Where your application's menu entry (.desktop) should go to.
# kde_icondir         Where your icon should go to - better use KDE_ICON.
# kde_sounddir        Where your sounds should go to.
# kde_htmldir         Where your docs should go to. (contains lang subdirs)
# kde_datadir         Where you install application data. (Use a subdir)
# kde_locale          Where translation files should go to. (contains lang subdirs)
# kde_cgidir          Where cgi-bin executables should go to.
# kde_confdir         Where config files should go to (system-wide ones with default values).
# kde_mimedir         Where mimetypes .desktop files should go to.
# kde_servicesdir     Where services .desktop files should go to.
# kde_servicetypesdir Where servicetypes .desktop files should go to.
# kde_toolbardir      Where general toolbar icons should go to (deprecated, use KDE_ICON).
# kde_wallpaperdir    Where general wallpapers should go to.
# kde_templatesdir    Where templates for the "New" menu (Konqueror/KDesktop) should go to.
# kde_bindir          Where executables should go to. Use bin_PROGRAMS or bin_SCRIPTS.
# kde_libdir          Where shared libraries should go to. Use lib_LTLIBRARIES.
# kde_moduledir       Where modules (e.g. parts) should go to. Use kde_module_LTLIBRARIES.
# kde_styledir        Where Qt/KDE widget styles should go to (new in KDE 3).
# kde_designerdir     Where Qt Designer plugins should go to (new in KDE 3).

# set the include path for X, qt and KDE
INCLUDES= $(all_includes)

METASOURCES = AUTO

SUBDIRS = pics

# the library search path. 
kraft_LDFLAGS = $(all_libraries) $(KDE_RPATH)

rcdir = $(kde_datadir)/kraft
rc_DATA = kraftui.rc katalogview.rc

cssdir = $(kde_datadir)/kraft
css_DATA = docoverview.css

messages: rc.cpp
	LIST=`find . -name \*.h -o -name \*.hh -o -name \*.H -o -name \*.hxx -o -name \*.hpp -o -name \*.cpp -o -name \*.cc -o -name \*.cxx -o -name \*.ecpp -o -name \*.C`; \
	if test -n "$$LIST"; then \
	 $(XGETTEXT) $$LIST -o $(podir)/kraft.pot; \
	fi

noinst_HEADERS = positionviewwidget.h
