/***************************************************************************
             portalview.cpp  - the main portal class
                             -------------------
    begin                : 2004-05-09
    copyright            : (C) 2004 by Klaas Freitag
    email                : freitag@kde.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

// include files for Qt
#include <QSqlQuery>
#include <QSqlDatabase>
#include <QDebug>
#include <QHBoxLayout>
#include <QtCore>
#include <QPushButton>

#include <KLocalizedString>

#include "version.h"
#include "kraftdb.h"
#include "portalview.h"
#include "portalhtmlview.h"
#include "katalogman.h"
#include "alldocsview.h"
#include "documentman.h"
#include "defaultprovider.h"
#include "reportgenerator.h"
#include "format.h"
#include "grantleetemplate.h"

PortalView::PortalView(QWidget *parent, const char*)
    : QWidget( parent ),
      _allDocsView(nullptr),
      mCatalogBrowser(nullptr),
      mSystemBrowser(nullptr)
{
    const QSize iSize{64,64};

    _contentsWidget = new QListWidget(this);
    _contentsWidget->setViewMode(QListView::IconMode);
    _contentsWidget->setIconSize(iSize);
    _contentsWidget->setMovement(QListView::Static);
    _contentsWidget->setMaximumWidth(132);
    _contentsWidget->setSpacing(0);

    _pagesWidget = new QStackedWidget(this);
    _pagesWidget->addWidget(documentDigests());
    _pagesWidget->addWidget(new QWidget());  // doc timeline
    _pagesWidget->addWidget(katalogDetails()); // catalogs
    _sysPageIndx = _pagesWidget->addWidget(systemDetails()); // system

    createIcons(iSize);
    _contentsWidget->setCurrentRow(0);

    QHBoxLayout *horizontalLayout = new QHBoxLayout;
    QVBoxLayout *vbox = new QVBoxLayout;
    vbox->addWidget(_contentsWidget);

    QPushButton *pb = new QPushButton(i18n("About Kraft"));
    pb->setIcon(DefaultProvider::self()->icon("kraft-simple"));
    vbox->addWidget(pb);

    connect(pb, &QPushButton::clicked, this, &PortalView::displaySystemsTab);
    horizontalLayout->addLayout(vbox);
    horizontalLayout->addWidget(_pagesWidget, 1);
    setLayout(horizontalLayout);
}

PortalView::~PortalView( )
{
}

void PortalView::createIcons(const QSize& iconSize)
{
    QSize sHint{128, 100};

    // Scale the icon via a pixmap, as in AppImages, for some reasons the icons are small
    // stackoverflow reports that svg icons cannot be scaled up.
    auto icon = [iconSize](const QIcon& i) {
        const QPixmap pix = i.pixmap(iconSize);
        QIcon icon(pix);
        QList<QSize> sizes = icon.availableSizes();
        qDebug() << "III" << sizes;
        return icon;
    };

    QListWidgetItem *documentsButton = new QListWidgetItem(_contentsWidget);
    documentsButton->setIcon(icon(DefaultProvider::self()->icon("file-description")));
    documentsButton->setText(i18n("Documents"));
    documentsButton->setTextAlignment(Qt::AlignHCenter);
    documentsButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    documentsButton->setSizeHint(sHint);

    QListWidgetItem *timeLineButton = new QListWidgetItem(_contentsWidget);
    timeLineButton->setIcon(icon(DefaultProvider::self()->icon("file-chart")));
    timeLineButton->setText(i18n("Timeline"));
    timeLineButton->setTextAlignment(Qt::AlignHCenter);
    timeLineButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    timeLineButton->setSizeHint(sHint);

    QListWidgetItem *catButton = new QListWidgetItem(_contentsWidget);

    catButton->setIcon(icon(DefaultProvider::self()->icon("book")));
    catButton->setText(i18n("Catalogs"));
    catButton->setTextAlignment(Qt::AlignHCenter);
    catButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    catButton->setSizeHint(sHint);

    connect( _contentsWidget, &QListWidget::itemClicked,
             this, &PortalView::changePage);
}

void PortalView::changePage(QListWidgetItem *current)
{
    if (!current)
        return;

    int indx = _contentsWidget->row(current);
    if( indx == 0 ) {
        // the flat documents list
        _allDocsView->setView( AllDocsView::FlatList );
    } else if( indx == 1 ) {
        // the timeline
        _allDocsView->setView( AllDocsView::TreeView );
        indx = 0;
    }

    _pagesWidget->setCurrentIndex(indx);
}

void PortalView::displaySystemsTab()
{
    _pagesWidget->setCurrentIndex(_sysPageIndx);
}

QWidget* PortalView::katalogDetails()
{
    QWidget *w = new QWidget;

    QBoxLayout *b = new QHBoxLayout;
    w->setLayout( b );
    mCatalogBrowser = new PortalHtmlView( w );
    mCatalogBrowser->setTitle( i18n( "Kraft Document Overview" ) );
    mCatalogBrowser->setStylesheetFile( "catalogview.css" );

    b->addWidget( mCatalogBrowser );

    QString html;

    html = "<h2>" + i18n("Available Catalogs") + "</h2>";
    html += "<div>\n";
    html += i18n( "No catalogs available." );
    html += "</div>";
    mCatalogBrowser->displayContent( html );

    connect( mCatalogBrowser, SIGNAL( openCatalog( const QString& ) ),
             SIGNAL( openKatalog( const QString& ) ) );

    return w;
}

void PortalView::fillCatalogDetails()
{
    if ( ! mCatalogBrowser ) return;

    const QStringList katalogNamen = KatalogMan::self()->allKatalogNames();
    QString html;

    html = QStringLiteral("<br/><h2>") + i18n("Available Catalogs") + QStringLiteral("</h2>");
    html += QStringLiteral("<div><table width=\"60%\" cellpadding=\"4\" cellspacing=\"0\" border=\"0\">\n");

    int cnt = 0;
    for(QStringList::ConstIterator namesIt = katalogNamen.begin();
        namesIt != katalogNamen.end(); ++namesIt )
    {
        QString katName = *namesIt;
        html += printKatLine( katName, cnt++ );
    }

    html += QStringLiteral("</table></div>\n");

    mCatalogBrowser->displayContent( html );
}

QString PortalView::printKatLine( const QString& name, int cnt ) const
{
    QString urlName = name.toHtmlEscaped();

    // qDebug () << "Converted Katalog name: " << urlName;
    QString html;

    html += "<tr";
    if ( cnt % 2 ) {
        html += " class=\"odd\"";
    }
    html += ">\n";

    html += "<td class=\"bigfont\"><div>"+urlName+"</div></td>";
    html += "<td class=\"bigfont\" align=\"right\"><a href=\"http://localhost/katalog.cgi?kat="+
            name+"&action=open\">";
    html += i18n("Open");
    html += "</a></td></tr>";

    KatalogMan::CatalogDetails details = KatalogMan::self()->catalogDetails(name);
    html += "<tr";
    if ( cnt % 2 ) {
        html += " class=\"odd\"";
    }
    html += ">\n";

    if( details.countEntries == 0 ) {
        html += QString("<td colspan=\"2\"><span style=\"font-size:75%;\">%1</span></td>").arg(i18n("No templates yet."));
    } else {
        const QString dateStr = Format::toDateString(details.maxModDate.date(), Format::DateFormatGerman);
        html += QString("<td class=\"sub\" colspan=\"2\">") +
                i18n("%1 templates in %2 chapters<br/>last modified at %3",
                     details.countEntries, details.countChapters, dateStr)
                + QLatin1String("</td>");
    }
    html += "</tr>\n";
    return html;
}

QString PortalView::ptag( const QString& content,  const QString& c ) const
{
    QString html( "<p" );
    if ( ! c.isEmpty() ) {
        html += QString( " class=\"%1\"" ).arg( c );
    }
    html += ">";
    html += content;
    html += "</p>";

    return html;
}

QWidget* PortalView::systemDetails()
{
    QWidget *w = new QWidget;
    QBoxLayout *b = new QHBoxLayout;
    w->setLayout( b );
    mSystemBrowser = new PortalHtmlView( w );
    b->addWidget( mSystemBrowser );
    mSystemBrowser->setStylesheetFile( "systemview.css" );

    return w;
}

QString PortalView::systemView( const QString& htmlMsg ) const
{
    if ( ! mSystemBrowser ) return QString ("");

    const QString templateName = ( htmlMsg.isNull() ? QString( "views/systemviewdetails.gtmpl" ) : QString ( "views/systemviewerror.gtmpl" ) );
    const QString tmplFile = DefaultProvider::self()->locateFile( templateName );

    GrantleeFileTemplate tmpl(tmplFile);
    if( !tmpl.isOk() ) {
        return QString();
    }
    bool ok;

    const QString logoFile = DefaultProvider::self()->locateFile("styles/pics/kraftapp_logo_trans.png" );
    QObject obj;

    obj.setProperty( "KRAFT_LOGO_FILE", logoFile );
    obj.setProperty("KRAFT_WEBSITE", i18n( "Kraft Website" ));
    QDate d = QDate::currentDate();
    obj.setProperty("KRAFT_COPYRIGHT_YEAR", QString::number(d.year()));
    obj.setProperty( "KRAFT_LICENSE_TEXT", i18nc("The string is followed by a link to the GPL2 text", "Kraft is free software licensed under the"));
    obj.setProperty( "KRAFT_GITHUB", i18nc("The string is followed by the link to github", "Kraft is maintained on "));
    obj.setProperty( "KRAFT_AUTHORS", i18n("Authors"));
    obj.setProperty( "KRAFT_MAINTAINER", i18n("Developer and Maintainer"));
    obj.setProperty( "KRAFT_DEVELOPER", i18n("Developer"));
    obj.setProperty( "KRAFT_GRAPHICS", i18nc("The person who provided the logo graphics", "Logo design"));
    obj.setProperty( "KRAFT_MANUAL", i18nc("The person who provided the user manual", "User Manual"));
    // kraft infos
    obj.setProperty("KRAFT_INTRO_DESC", i18n("Kraft helps you to handle documents like quotes and invoices in your small business."));
    obj.setProperty( "KRAFT_WELCOME_LABEL", i18n( "Welcome to Kraft" ) );
    obj.setProperty( "KRAFT_VERSION_LABEL", i18n( "Kraft Version" ) );
    obj.setProperty( "KRAFT_VERSION", Kraft::Version::number());
    obj.setProperty( "KRAFT_CODENAME_LABEL", i18n( "Codename" ) );
    obj.setProperty( "KRAFT_CODENAME", Kraft::Version::codeName() );

    // string like
    // git sha <sha> on branch <branch> built on <host> (<distro>)
    obj.setProperty( "GIT_BRANCH", Kraft::Version::gitBranch());
    obj.setProperty( "GIT_SHA1", Kraft::Version::gitSha());
    obj.setProperty( "BUILD_HOST", Kraft::Version::buildHost());
    obj.setProperty( "BUILD_HOST_DISTRO", Kraft::Version::buildHostDistro());
    obj.setProperty( "GIT_BUILD_LABEL", i18n("Git Information"));
    obj.setProperty( "GIT_BUILD_STRING", QString("git sha %1 on branch %2 built on %3 (%4)")
                   .arg(Kraft::Version::gitSha())
                   .arg(Kraft::Version::gitBranch())
                   .arg(Kraft::Version::buildHost())
                   .arg(Kraft::Version::buildHostDistro()));

    const QString countryName = DefaultProvider::self()->locale()->nativeTerritoryName();
    obj.setProperty( "COUNTRY_SETTING_LABEL", i18n( "Country Setting" ) );
    obj.setProperty( "COUNTRY_SETTING", QString( "%1 (%2)" ).arg( countryName ).arg( DefaultProvider::self()->locale()->territory() ));
    const QString languageName = DefaultProvider::self()->locale()->nativeLanguageName();
    obj.setProperty( "LANGUAGE_SETTING_LABEL", i18n( "Language Setting" ) );
    obj.setProperty( "LANGUAGE_SETTING", QString( "%1 (%2)" ).arg( languageName ).arg( DefaultProvider::self()->locale()->language() ));

    if ( !htmlMsg.isNull() ) {
        obj.setProperty( "ERROR_TITLE_LABEL", i18n( "Kraft Initialisation Problem" ) );
        QString errorMessage = i18n( "There is a initialisation error on your system. Kraft will not work that way." );
        errorMessage += htmlMsg;
        obj.setProperty( "ERROR_TEXT", errorMessage );
        return tmpl.render(ok);
    }

    // database infos
    obj.setProperty( "DATABASE_TITLE_LABEL", i18n( "Database Information" ) );
    obj.setProperty( "DATABASE_NAME_LABEL", i18n( "Kraft database name" ) );
    obj.setProperty( "DATABASE_NAME", KraftDB::self()->databaseName() );

    QString schemaVersion = QString::number( KraftDB::self()->currentSchemaVersion() );
    if ( KraftDB::self()->currentSchemaVersion() != Kraft::Version::dbSchemaVersion() ) {
        schemaVersion += " - " + QString( "<font color='red'>%1: %2</font>" ).arg( i18n ( "Required Version" ))
                .arg( Kraft::Version::dbSchemaVersion() );
    }
    obj.setProperty( "DATABASE_SCHEMA_VERSION_LABEL", i18n( "Database schema version" ) );
    obj.setProperty( "DATABASE_SCHEMA_VERSION", schemaVersion );
    obj.setProperty( "DATABASE_DRIVER_LABEL", i18n( "Qt database driver" ) );
    obj.setProperty( "DATABASE_DRIVER", KraftDB::self()->qtDriver() );

    bool dbOk = KraftDB::self()->getDB()->isOpen();
    const QString databaseConnection = ( dbOk ? i18n("established") : QString( "<font color='red'>%1</font>" ).arg( i18n( "NOT AVAILABLE!" ) ) );
    obj.setProperty( "DATABASE_CONNECTION_LABEL", i18n( "Database connection" ) );
    obj.setProperty( "DATABASE_CONNECTION", databaseConnection );

    if( dbOk ) {
        QString query{"select sqlite_version()"}; //
        if (KraftDB::self()->qtDriver() == "MYSQL")
            query = "SHOW VARIABLES like 'version';";

        QSqlQuery q(query);
        if( q.isActive() ) {
            q.next();
            const QString version = q.value(0).toString();
            obj.setProperty("DATABASE_VERSION_LABEL", i18n("Database Version"));
            obj.setProperty("DATABASE_VERSION", version);
        }
    }

    // Akonadi and friends
    QScopedPointer<AddressProvider> aprov;
    aprov.reset( new AddressProvider);
    obj.setProperty( "ADDRESSBOOK_BACKEND_LABEL", i18n( "Addressbook Backend" ) );
    obj.setProperty( "ADDRESSBOOK_BACKEND_TYPE_LABEL", i18n( "Backend type" ) );
    const QString backendTypeValue = QString( "%1 (%2)").arg( aprov->backendName())
            .arg(aprov->backendUp() ? i18n("running") : i18n("not running") );
    obj.setProperty( "ADDRESSBOOK_BACKEND_TYPE", backendTypeValue );

    // external tools
    obj.setProperty( "EXTERNAL_TOOLS_LABEL", i18n( "External Tools" ) );

    obj.setProperty( "RML2PDF_TOOL_LABEL", i18n( "RML to PDF conversion tool" ) );
    const QStringList trml2pdf = DefaultProvider::self()->locatePythonTool("erml2pdf.py");
    QString trml2pdfValue = (trml2pdf.count() ? trml2pdf.join(" ") : i18n("not found!") );
    obj.setProperty( "RML2PDF_TOOL", trml2pdfValue );

    obj.setProperty( "ICONV_TOOL_LABEL", i18n( "iconv tool for text import" ) );
    obj.setProperty( "ICONV_TOOL", DefaultProvider::self()->iconvTool() );

    obj.setProperty( "WEASYPRINT_TOOL_LABEL", i18n( "<a href=\"https://weasyprint.org/\">weasyprint</a> for PDF generation" ) );

    QString wp = DefaultProvider::self()->locateBinary("weasyprint");
    if (wp.isEmpty()) {
        wp = i18n("not available");
    }
    obj.setProperty( "WEASYPRINT_TOOL", wp);

    // aknowledgement
    obj.setProperty( "ICON_ACKNOWLEDGEMENT_LABEL", i18n("Some Icons are made by") );
    obj.setProperty( "ACKNOWLEGEMENT_LABEL", i18n( "Acknowledgements" ) );

    obj.setProperty( "KRAFT_V2_LABEL", i18n("Kraft Version 2"));
    obj.setProperty( "KRAFT_V2_ROOT_DIR_LABEL", i18n("Kraft Version 2 Data Directory"));
    const QString v2dir = DefaultProvider::self()->kraftV2Dir();
    obj.setProperty( "KRAFTV2_ROOT_DIR", v2dir);

    tmpl.addToObjMapping("system", &obj);

    return tmpl.render(ok);
}

void PortalView::fillSystemDetails()
{
    const QString html = systemView( QString() );
    mSystemBrowser->displayContent( html );
}

void PortalView::systemInitError( const QString& htmlMsg )
{
    const QString html = systemView( htmlMsg );
    mSystemBrowser->displayContent( html );
}

QWidget* PortalView::documentDigests()
{
    QWidget *w = new QWidget;

    QBoxLayout *b = new QHBoxLayout;
    b->setContentsMargins( 0, 0, 0, 0 );
    w->setLayout( b );

    _allDocsView = new AllDocsView( w );
    b->addWidget( _allDocsView );

    connect(_allDocsView, &AllDocsView::openDocument, this, &PortalView::openDocument);
    connect(_allDocsView, &AllDocsView::docSelected , this, &PortalView::documentSelected);

    return w;
}

void PortalView::slotBuildView()
{

    // QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );
    _allDocsView->slotBuildView();
    // QApplication::restoreOverrideCursor();
}

/* END */

