/***************************************************************************
                        htmlview.cpp  - show a html page
                             -------------------
    begin                : Aug 2006
    copyright            : (C) 2006 Klaas Freitag <freitag@kde.org>
                           (C) 2006 Cornelius Schumacher <schumacher@kde.org>
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "htmlview.h"

#include <QFileInfo>

#include <klocale.h>
#include <kdebug.h>
#include <kaction.h>
#include <kactioncollection.h>
#include <kstandarddirs.h>
#include <krun.h>
#include <kdirwatch.h>

#include <QFile>

HtmlView::HtmlView( QWidget *parent )
  : KHTMLPart( parent ), mZoomStep( 10 )
{
  connect( this, SIGNAL( setWindowCaption( const QString & ) ),
           SLOT( setTitle( const QString & ) ) );

  setJScriptEnabled(false);
  setJavaEnabled(false);
  setMetaRefreshEnabled(false);
  setPluginsEnabled(false);
}

void HtmlView::clearView()
{
  begin();
  write( QString::null );
  end();

  setTitle( QString::null );
}

void HtmlView::setInternalUrl( const QString &url )
{
  mInternalUrl = url;
}

QString HtmlView::internalUrl() const
{
  return mInternalUrl;
}

void HtmlView::setTitle( const QString &title )
{
  mTitle = title;
}

void HtmlView::setStylesheetFile( const QString &style )
{
  QString prjPath = QString::fromUtf8(qgetenv( "KRAFT_HOME" ));
  if( !prjPath.isEmpty() ) {
    mStyleSheetFile = QString( "%1/styles/%2" ).arg( prjPath ).arg( style );
  } else {
    mStyleSheetFile = KStandardDirs::locate( "appdata", style );
  }
  QFileInfo fi(mStyleSheetFile);
  bool ok = fi.exists();
  kDebug() << "found this stylefile: " << mStyleSheetFile << ok;
}

void HtmlView::setupActions( KActionCollection *actionCollection )
{
//  mZoomInAction = new KAction( i18n( "Increase Font Sizes" ), "viewmag+",
//                               KShortcut( "Qt::CTRL++" ), this,
//                               SLOT( zoomIn() ), actionCollection, "view_zoom_in" );
  actionCollection->addAction( "view_zoom_in", this, SLOT( zoomIn() ) );

//  mZoomOutAction = new KAction( i18n( "Decrease Font Sizes" ), "viewmag-",
//                                KShortcut( "Qt::CTRL+-" ), this,
//                                SLOT( zoomOut() ), actionCollection, "view_zoom_out" );
  actionCollection->addAction( "view_zoom_out", this, SLOT( zoomOut() ) );
  updateZoomActions();
}

void HtmlView::zoomIn()
{
  setZoomFactor( zoomFactor() + mZoomStep );
  updateZoomActions();
}

void HtmlView::zoomOut()
{
  setZoomFactor( zoomFactor() - mZoomStep );
  updateZoomActions();
}

void HtmlView::updateZoomActions()
{
  mZoomInAction->setEnabled( zoomFactor() + mZoomStep <= 300 );
  mZoomOutAction->setEnabled( zoomFactor() - mZoomStep > 100 );

  // Prefs::self()->setZoomFactor( zoomFactor() );
}

QString HtmlView::locateCSSImages( const QByteArray& line )
{
    QString l = QString::fromUtf8(line);

    QRegExp reg( "\\{\\{CSS_IMG_PATH\\}\\}/(\\S+)\\s*\\);");
    if( l.contains(reg) ) {
        QString fName = reg.cap(1);
        QString p;

        if( !fName.isEmpty() ) {
            QByteArray kraftHome = qgetenv("KRAFT_HOME");
            if( !kraftHome.isEmpty() ) {
                p = QString("%1/src/pics/%2").arg(QString::fromUtf8(kraftHome)).arg(fName);
            } else {
                KStandardDirs dirs;
                QString find;
                find = QString("kraft/pics/%1").arg(fName);
                p = dirs.findResource("data", find);
                if( p.isEmpty() ) {
                    kDebug() << "ERR: Unable to find resource " << fName;
                }
            }
        }
        if( !p.isEmpty() ) {
            p += ");";
            l.replace(reg, p);
        }
    }
    return l;
}

void HtmlView::writeTopFrame( )
{
  QString t = QString( "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01//EN\">"
                       "<html><head><title>%1</title>" ).arg( mTitle );

  if ( ! mStyleSheetFile.isEmpty() ) {
      QString style;
      QFile file(mStyleSheetFile);
      if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
          style = QLatin1String("<style type=\"text/css\">");
          while (!file.atEnd()) {
              QString line = locateCSSImages(file.readLine());
              style += line;
          }
          file.close();
          style += QLatin1String("</style>");
      }
      t += style;
  }

  write( t );
}

void HtmlView::writeContent( const QString& c )
{
  write( c );
}

void HtmlView::writeBottomFrame()
{
  QString t = "</body>";

  write( t );
}

void HtmlView::displayContent( const QString& content )
{
  // kDebug() << "BASE URL: " << mBaseUrl.prettyUrl();
  // kDebug() << "Stylesheet URL: " << mStyleSheetFile;
  begin( mBaseUrl );

  // kDebug() << "Show content: " << content;

  writeTopFrame();
  writeContent( content );
  writeBottomFrame();
  end();

}

void HtmlView::showWelcomePage()
{
  QString t;
  displayContent( t );
}

void HtmlView::setBaseUrl( const QString& base )
{

  mBaseUrl = KUrl( base );
  kDebug() << "Setting base url: " << mBaseUrl.prettyUrl();
}
