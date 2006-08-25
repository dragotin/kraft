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
#include <kstandarddirs.h>
#include <krun.h>
#include <kdirwatch.h>

#include <qfile.h>
#include <qtextstream.h>

HtmlView::HtmlView( QWidget *parent )
  : KHTMLPart( parent ), mZoomStep( 10 )
{
  connect( this, SIGNAL( setWindowCaption( const QString & ) ),
           SLOT( setTitle( const QString & ) ) );

  setZoomFactor( 100 );
#if 0
  mCssFile = new MetaFile( "css-fate", this );
  mCssFile->setLocalFile( "feature.css" );

#endif

  showWelcomePage();
}

bool HtmlView::loadCss()
{
  KStandardDirs stdDirs;
  QString filename = stdDirs.findResource( "data", "kraft/help.css" );

  if ( filename.isEmpty() ) {
    return false;
  }

  QFile f( filename );

  if ( !f.open( IO_ReadOnly ) ) {
    kdWarning() << "Unable to read stylesheet '" << filename << "'" << endl;
    return false;
  } else {
    kdDebug() << "Loading stylesheet " << filename << endl;
    QTextStream ts( &f );
    QString css = ts.read();
    mCss = css;
    return true;
  }
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

void HtmlView::setupActions( KActionCollection *actionCollection )
{
  mZoomInAction = new KAction( i18n( "Increase Font Sizes" ), "viewmag+",
    KShortcut( "CTRL++" ), this,
    SLOT( zoomIn() ), actionCollection, "view_zoom_in" );
  mZoomOutAction = new KAction( i18n( "Decrease Font Sizes" ), "viewmag-",
    KShortcut( "CTRL+-" ), this,
    SLOT( zoomOut() ), actionCollection, "view_zoom_out" );

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
  mZoomOutAction->setEnabled( zoomFactor() - mZoomStep >= 20 );

  // Prefs::self()->setZoomFactor( zoomFactor() );
}

void HtmlView::writeTopFrame()
{
  QString t = "<html>";
  t += "<head><title>" + i18n( "Kraft Assistance" ) + "</title></head>";
  t += "<body>";
  t += "<h1>" + i18n( "Kraft Help System" ) + "</h1>";
  write( t );
}

void HtmlView::writeContent( const QString& c )
{
  write( "<div class=\"content\">" );
  write( c );
  write( "</div>" );
}

void HtmlView::writeBottomFrame()
{
  QString t = "</body>";

  write( t );
}

void HtmlView::displayContent( const QString& content )
{
  begin();

  setStyleSheet();

  writeTopFrame();
  writeContent( content );
  writeBottomFrame();
  end();

}

void HtmlView::showWelcomePage()
{
  QString t;
  t = "Kraft helps you with your small business. It aids you to write ";
  t += "offers, order acceptances and invoices in an easy to use, simple ";
  t += "yet powerful way.";
  displayContent( t );
}

void HtmlView::setStyleSheet()
{
  if ( loadCss() )
    setUserStyleSheet( mCss );
}

#include "htmlview.moc"
