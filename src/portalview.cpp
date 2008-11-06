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
#include <qvbox.h>
#include <qsqlquery.h>
#include <qsqldatabase.h>
#include <qstylesheet.h>
#include <qapplication.h>
#include <qcursor.h>
#include <qlayout.h>

// include files for KDE
#include <klocale.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <kstandarddirs.h>
#include <kurl.h>
#include <klistview.h>
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

PortalView::PortalView(QWidget *parent, const char *name, int face)
    : KJanusWidget( parent, name, face ),
      m_docBox(0),
      m_katalogBox(0),
      mArchiveBox( 0 ),
      mCatalogBrowser( 0 ),
      mSystemBrowser( 0 )
{
  m_docBox     = addVBoxPage( i18n("Documents"),
                              i18n("Document List"),
                              DesktopIcon("folder_outbox"));
  mDocDigestIndex = pageIndex( m_docBox );
  documentDigests( m_docBox );

  m_katalogBox = addVBoxPage( i18n("Catalogs"),
                              i18n("Available Catalogs"),
                              DesktopIcon("folder_green"));
  mCatalogIndex = pageIndex( m_katalogBox );
  katalogDetails(m_katalogBox);

  m_sysBox     = addVBoxPage( i18n("System"),
                              i18n("Information about the Kraft System"),
                              DesktopIcon("server"));
  mSystemIndex = pageIndex( m_sysBox );
  systemDetails( m_sysBox );
}

void PortalView::katalogDetails(QWidget *parent)
{
  QWidget *w = new QWidget( parent );
  QBoxLayout *b = new QHBoxLayout( w );

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

  connect( mCatalogBrowser, SIGNAL( urlClick(const QString&) ),
           this, SLOT( slUrlClicked( const QString& ) ) );
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

void PortalView::archiveDetails( QWidget *  )
{

}

QString PortalView::printKatLine( const QString& name, int cnt ) const
{
    QString urlName = QStyleSheet::escape( name );

    kdDebug() << "Converted Katalog name: " << urlName << endl;
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
    html += "</td>";
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

void PortalView::slUrlClicked( const QString& urlStr )
{
    KURL url( urlStr );

    kdDebug() << "URL: " << url.path() << endl;
    if( url.path().startsWith( "/katalog.cgi") )
    {
        QString action = url.queryItem("action");
        QString kat    = url.queryItem("kat");
        // Katalog editieren
        if( action == "open" )
        {
            emit openKatalog(kat);
        }
        else if( action == "delete" )
        {
            emit deleteKatalog(kat);
        }
        else if( action == "xml" )
        {
            emit( katalogToXML(kat));
        }
        else
        {
            // unknown query
            kdDebug() << "Can not handle Query: " << url.query() << endl;
        }
    }
    else
    {
        // weitere
    }
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

void PortalView::systemDetails(QWidget *parent)
{
  QWidget *w = new QWidget( parent );
  QBoxLayout *b = new QHBoxLayout( w );
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
  QString logoFile = stdDirs.findResource( "data",  "kraft/pics/muckilogo_oS.png" );
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
  html += QString( "<tr><td>Codename <i>%1</i></td></tr>" ).arg( KRAFT_CODENAME );
  QString h1 = DefaultProvider::self()->locale()->twoAlphaToCountryName(
    DefaultProvider::self()->locale()->country() );
  html += QString( "<tr><td>" ) + i18n( "Country Setting: " ) +
          QString( "<i>%1 (%2)</i></td></tr>" ).arg( h1 ).arg( DefaultProvider::self()->locale()->country() );
  h1 = DefaultProvider::self()->locale()->twoAlphaToLanguageName(
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

  html = systemViewHeader(); // "<h2>" + i18n("Kraft System Information") + "</h2>";

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
  html += "</div></table>";
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

  pageWidget( mDocDigestIndex )->setEnabled( false );
  pageWidget( mCatalogIndex )->setEnabled( false );

  showPage( mSystemIndex );
}

void PortalView::documentDigests( QWidget *parent )
{
  mDocDigestView = new DocDigestView( parent );

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
  connect( mDocDigestView, SIGNAL( archivedDocSelected( const ArchDocDigest& ) ),
           SIGNAL( archivedDocSelected( const ArchDocDigest& ) ) );
  connect( mDocDigestView->listview(), SIGNAL( currentChanged( QListViewItem* ) ),
           this,  SLOT( slotDigestItemSelected( QListViewItem* ) ) );
}

void PortalView::slotCreateDocument()
{
  // this slot is called if the user wants to initiate the creation of a new doc
  // It is routed to higher layers.
  emit createDocument();
}

void PortalView::slotDocumentCreated( DocGuardedPtr doc )
{
  // the new doc is now created and can be inserted into the doc digest view
  mDocDigestView->slotNewDoc( doc );
}

void PortalView::slotDocumentUpdate( DocGuardedPtr doc )
{
  mDocDigestView->slotUpdateDoc( doc );
}

void PortalView::slotBuildView()
{

  QApplication::setOverrideCursor( QCursor( BusyCursor ) );
  mDocDigestView->slotBuildView();
  QApplication::restoreOverrideCursor();
}

void PortalView::slotDigestItemSelected( QListViewItem *item )
{
  kdDebug() << "Digest Item Selected " << item << endl;

}

PortalView::~PortalView( )
{

}

/* END */


#include "portalview.moc"
