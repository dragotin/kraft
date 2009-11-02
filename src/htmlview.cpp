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

#include <klocale.h>
#include <kdebug.h>
#include <kaction.h>
#include <kactioncollection.h>
#include <kstandarddirs.h>
#include <krun.h>
#include <kdirwatch.h>

#include <qfile.h>
#include <q3textstream.h>

HtmlView::HtmlView( QWidget *parent )
  : KHTMLPart( parent ), mZoomStep( 10 )
{
  connect( this, SIGNAL( setWindowCaption( const QString & ) ),
           SLOT( setTitle( const QString & ) ) );

  setJScriptEnabled(false);
  setJavaEnabled(false);
  setMetaRefreshEnabled(false);
  setPluginsEnabled(false);

  showWelcomePage();
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
  mStyleSheetFile = style;
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

void HtmlView::writeTopFrame( )
{
  KStandardDirs stdDirs;
  QString filename = stdDirs.findResource( "data", QString( "kraft/%1" ) .arg( mStyleSheetFile ) );
  filename = KStandardDirs::locate( "appdata", mStyleSheetFile  );
  kDebug() << "found this stylefile: " << filename << " out of " << mStyleSheetFile;
  QString t = QString( "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01//EN\">"
                       "<html><head><title>%1</title>" ).arg( mTitle );
  if ( ! filename.isEmpty() ) {
    t += QString( "<link rel=\"stylesheet\" type=\"text/css\" href=\"%1\">"
                  "<style type=\"text/css\">"
                  "</style></head>\n\n" ).arg( filename );
  }
  t += "<body>";
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
  begin();

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

#include "htmlview.moc"
