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
#include <QApplication>
#include <QCursor>
#include <QLayout>
#include <QTextDocument>
#include <QHBoxLayout>
#include <QBoxLayout>

// include files for KDE
#include <klocale.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <kstandarddirs.h>
#include <kurl.h>
#include <kcalendarsystem.h>
#include <khtmlview.h>

#include "version.h"
#include "kraftdb.h"
#include "portalview.h"
#include "portalhtmlview.h"
#include "katalogman.h"
#include "docdigestview.h"
#include "documentman.h"
#include "defaultprovider.h"
#include "reportgenerator.h"

PortalView::PortalView(QWidget *parent, const char*)
    : KPageWidget( parent ),
      mDocDigestView( 0 ),
      mCatalogBrowser( 0 ),
      mSystemBrowser( 0 ),
      mSysPage(0),
      mDocsPage(0)
{
  documentDigests();
  katalogDetails();
  systemDetails();
}

void PortalView::katalogDetails()
{
  QWidget *w = new QWidget;
  mCatalogPage = addPage( w, i18n("Catalogs" ) );
  mCatalogPage->setIcon(KIcon("server-database"));

  QBoxLayout *b = new QHBoxLayout;
  w->setLayout( b );
  mCatalogBrowser = new PortalHtmlView( w );
  mCatalogBrowser->setTitle( i18n( "Kraft Document Overview" ) );
  mCatalogBrowser->setStylesheetFile( "catalogview.css" );
  

  b->addWidget( mCatalogBrowser->view() );
  b->addSpacing( KDialog::marginHint() );

  QString html;

  html = "<html><h2>" + i18n("Available Catalogs") + "</h2>";
  html += "<div>\n";
  html += i18n( "No catalogs available." );
  html += "</div>";
  mCatalogBrowser->displayContent( html );

  connect( mCatalogBrowser, SIGNAL( openCatalog( const QString& ) ),
           SIGNAL( openKatalog( const QString& ) ) );
}

void PortalView::fillCatalogDetails()
{
  if ( ! mCatalogBrowser ) return;

    QStringList katalogNamen = KatalogMan::self()->allKatalogNames();
    QString html;

    html = "<html><h2>" + i18n("Available Catalogs") + "</h2>";
    html += "<div>\n";
    html += "<table border=\"0\">";

    int cnt = 0;
    for(QStringList::ConstIterator namesIt = katalogNamen.begin();
        namesIt != katalogNamen.end(); ++namesIt )
    {
        QString katName = *namesIt;
        html += printKatLine( katName, cnt++ );
    }

    html += "</table></div></html>\n";

    mCatalogBrowser->displayContent( html );
}

QString PortalView::printKatLine( const QString& name, int cnt ) const
{
    QString urlName = Qt::escape( name );

    kDebug() << "Converted Katalog name: " << urlName << endl;
    QString html;

    html += "<tr";
    if ( cnt % 2 ) {
      html += " class=\"odd\"";
    }
    html += ">\n";

    html += "<td><b>"+urlName+"</b></td>";
    html += "<td align=\"center\"><a href=\"http://localhost/katalog.cgi?kat="+
            name+"&action=open\">";
    html += i18n("Open");
    html += "</td></tr>";

    KatalogMan::CatalogDetails details = KatalogMan::self()->catalogDetails(name);
    html += "<tr";
    if ( cnt % 2 ) {
      html += " class=\"odd\"";
    }
    html += ">\n";

    if( details.countEntries == 0 ) {
        html += "<td colspan=\"2\"><span style=\"font-size:75%;\">No templates yet.</span></td>";
    } else {
        KLocale *locale = DefaultProvider::self()->locale();
        QString dateStr = locale->formatDateTime( details.maxModDate );
        html += QString("<td colspan=\"2\"><span style=\"font-size:75%;\">%1 templates in %2 chapters, last modified at %3</span></td>").
                arg(details.countEntries).arg(details.countChapters).arg(dateStr);
    }
#if 0
    html += "<td align=\"center\"><a href=\"http://localhost/katalog.cgi?kat="+
            name+"&action=xml\">";
    html += i18n("XML Export");
    html += "</td>";

    html += "<td align=\"center\"><a href=\"http://localhost/katalog.cgi?kat="+
            name+"&action=delete\">";
    html += i18n("Remove");
    html += "</td>";
#endif
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

void PortalView::systemDetails()
{
  QWidget *w = new QWidget;
  mSysPage = addPage( w, i18n("System Details" ) );
  mSysPage->setIcon(KIcon("dialog-information"));
  QBoxLayout *b = new QHBoxLayout;
  w->setLayout( b );
  mSystemBrowser = new PortalHtmlView( w );
  b->addWidget( mSystemBrowser->view() );
  b->addSpacing( KDialog::marginHint() );
  mSystemBrowser->setStylesheetFile( "catalogview.css" ); //, "mucki_en_oS.png",

  // browser->setNotifyClick(false);
}

QString PortalView::systemViewHeader() const
{

  QString html( "" );

  KStandardDirs stdDirs;
  QString logoFile = stdDirs.findResource( "data",  "kraft/pics/kraftapp_logo.png" );
  html += i18n( "<h2>Welcome to Kraft</h2>" );
  html += "<div><table width=\"100%\" border=\"0\"><tr><td>";
  html += i18n("Kraft Version: %1</td>").arg( KRAFT_VERSION );
  html += "<td align=\"right\" rowspan=\"3\">";
  if ( ! logoFile.isEmpty() ) {
    html += QString( "<img src=\"%1\"/>" ).arg( logoFile );
  } else {
    html += "&nbsp;";
  }
  html += "</td></tr>";
  html += QString( "<tr><td>Codename: <i>%1</i></td></tr>" ).arg( KRAFT_CODENAME );
  QString h1 = DefaultProvider::self()->locale()->country();
  html += QString( "<tr><td>" ) + i18n( "Country Setting: " ) +
          QString( "<i>%1 (%2)</i></td></tr>" ).arg( h1 ).arg( DefaultProvider::self()->locale()->country() );
  h1 = DefaultProvider::self()->locale()->languageCodeToName(
    DefaultProvider::self()->locale()->language() );
  html += QString( "<tr><td>" ) + i18n( "Language Setting: " ) +
          QString( "<i>%1 (%2)</i></td></tr>" ).arg( h1 ).arg( DefaultProvider::self()->locale()->language() );
  html += "</table></div>";

  return html;
}

void PortalView::fillSystemDetails()
{
  QString html;
  if ( ! mSystemBrowser ) return;

  html = systemViewHeader();

  html += "<h2>" + i18n("Database Information") + "</h2>";
  html += "<div><table>";
  html += "<tr><td>" + i18n( "Kraft database name:" ) + "</td>";
  html += "<td>" + KraftDB::self()->databaseName() + "</td></tr>";

  html += "<tr><td>" + i18n( "Database schema version:" ) + "</td>";
  html += "<td>" + QString::number( KraftDB::self()->currentSchemaVersion() );
  if ( KraftDB::self()->currentSchemaVersion() != KRAFT_REQUIRED_SCHEMA_VERSION ) {
    html += "&nbsp;-&nbsp;" + QString( "<font color=\"red\">Required Version: %1</font>" )
            .arg( KRAFT_REQUIRED_SCHEMA_VERSION );
  }
  html += "</td></tr>";

  html += "<tr><td>" + i18n( "Qt database driver:" ) + "</td>";
  html += "<td>" +  KraftDB::self()->qtDriver() + "</td></tr>";

  html += "<tr><td>" + i18n( "Database connection:" ) + "</td>";
  html += "<td>";

  bool dbOk = false;
  if( KraftDB::self()->getDB()->isOpen() ) {
    dbOk = true;
    html += i18n("established");
  } else {
    html += i18n("<font color=\"red\">NOT AVAILABLE!</font>");
  }
  html += "</td></tr>";

  if( dbOk ) {
    QSqlQuery q("SHOW VARIABLES like 'version';");
    if( q.isActive() ) {
      q.next();
      QString version = q.value(1).toString();
      html += "<tr><td>" + i18n( "Database Version:" ) + "</td>";
      html += "<td>" +  version + "</td></tr>";
    }
  }
  html += "</table></div>";
  // external tools
  html += "<h2>" + i18n( "External Tools" ) + "</h2>";
  html += "<div><table>";
  html += "<tr><td>" + i18n( "RML to PDF conversion tool:" ) + "</td><td>";
  QStringList trml2pdf = ReportGenerator::self()->findTrml2Pdf();
  if( trml2pdf.count() ) {
    html += trml2pdf.join(" ")+ "</td></tr>";
  } else {
    html += i18n("not found!") + "</td></tr>";
  }
  html += "<tr><td>" + i18n( "iconv tool for text import:" ) + "</td><td>";
  html += DefaultProvider::self()->iconvTool() + "</td></tr>";

  html += "</table></div>";

  mSystemBrowser->displayContent( html );
}

void PortalView::systemInitError( const QString& htmlMsg )
{
  QString html = systemViewHeader(); // "<h2>" + i18n("Kraft System Information") + "</h2>";

  html += "<h2 class=\"error\">Kraft Initialisation Problem:</h2>";
  html += "<div class=\"error\">";
  html += ptag( i18n( "There is a initialisation error on your system. "
                       "Kraft will not work that way." ) );
  html += ptag( htmlMsg );
  html += "</div>";

  mSystemBrowser->displayContent( html ); // , "error" );

  mDocsPage->setEnabled( false );
  mCatalogPage->setEnabled( false );

  setCurrentPage( mSysPage );
}

void PortalView::documentDigests()
{
  QWidget *w = new QWidget;
  mDocsPage = addPage( w, i18n("Documents" ) );
  mDocsPage->setIcon(KIcon("folder-documents"));

  QBoxLayout *b = new QHBoxLayout;
  b->setContentsMargins( 0, 0, 0, 0 );
  w->setLayout( b );

  mDocDigestView = new DocDigestView( w );
  b->addWidget( mDocDigestView );
  b->addSpacing( KDialog::marginHint() );

  connect( mDocDigestView, SIGNAL( createDocument() ),
           this, SLOT( slotCreateDocument() ) );
  connect( mDocDigestView, SIGNAL( openDocument( const QString& ) ),
           SIGNAL( openDocument( const QString& ) ) );
  connect( mDocDigestView, SIGNAL( viewDocument( const QString& ) ),
           SIGNAL( viewDocument( const QString& ) ) );
  connect( mDocDigestView, SIGNAL( copyDocument( const QString& ) ),
           SIGNAL( copyDocument( const QString& ) ) );
  connect( mDocDigestView, SIGNAL( openArchivedDocument( const ArchDocDigest& ) ),
           SIGNAL( openArchivedDocument( const ArchDocDigest& ) ) );
  // connect( mDocDigestView, SIGNAL( printDocument( const QString& ) ),
  //         SIGNAL( printDocument( const QString& ) ) );
  connect( mDocDigestView, SIGNAL( docSelected( const QString& ) ),
           SIGNAL( documentSelected( const QString& ) ) );
  connect( mDocDigestView, SIGNAL( openArchivedDocument( const ArchDocDigest& ) ),
           SIGNAL( archivedDocSelected( const ArchDocDigest& ) ) );
 }

void PortalView::slotCreateDocument()
{
  // this slot is called if the user wants to initiate the creation of a new doc
  // It is routed to higher layers.
  emit createDocument();
}

void PortalView::slotBuildView()
{

  QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );
  mDocDigestView->slotBuildView();
  QApplication::restoreOverrideCursor();
}

PortalView::~PortalView( )
{
  delete mDocDigestView;
}

/* END */


#include "portalview.moc"
