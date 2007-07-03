/***************************************************************************
                 DocPostCard - a postcard version of the document
                             -------------------
    begin                : Aug 2006
    copyright            : (C) 2006 by Klaas Freitag
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

#include "docpostcard.h"
#include "kraftdoc.h"

#include <qstylesheet.h>

#include <klocale.h>
#include <kglobal.h>
#include <kurl.h>
#include <khtmlview.h>

#include <kdebug.h>

DocPostCard::DocPostCard( QWidget *parent )
  :HtmlView( parent ),  mMode( Full )
{
  setZoomFactor( 70 );
  loadCss( "docoverview.css", "docoverviewbg.png" );
}

void DocPostCard::setHeaderData( const QString& type,  const QString& date,
                                 const QString& address, const QString& id,
                                 const QString& pretext )
{
  mType = type;
  mDate = date;
  mAddress = address;
  mPreText = htmlify( pretext );
  mId = id;
}

QString DocPostCard::htmlify( const QString& str ) const
{
  QStringList li = QStringList::split( "\n", QStyleSheet::escape( str ) );
  return "<p>" + li.join( "</p><p>" ) + "</p>";
}

void DocPostCard::setPositions( DocPositionList posList )
{
  mPositions = "<table border=\"0\" width=\"99%\">";
  DocPositionBase *dpb;
  for( dpb = posList.first(); dpb; dpb = posList.next() ) {
     if( dpb->type() == DocPositionBase::Position ) {
      DocPosition *dp = static_cast<DocPosition*>(dpb);
      mPositions += "<tr><td width=\"20px\" align=\"right\" valign=\"top\">";

      if ( dp->toDelete() ) mPositions += "<strike>";
      mPositions += posList.posNumber( dpb ) + ". ";
      if ( dp->toDelete() ) mPositions += "</strike>";
      mPositions += "</td>";
      mPositions += "<td>";
      if ( dp->toDelete() ) mPositions += "<strike>";
      mPositions += dp->text();
      if ( dp->toDelete() ) mPositions += "</strike>";
      mPositions += "</td>";
      mPositions += "<td width=\"50px\" align=\"right\">";
      if ( dp->toDelete() ) mPositions += "<strike>";
      mPositions += dp->overallPrice().toHtmlString();
      if ( dp->toDelete() ) mPositions += "</strike>";
      mPositions += "</td></tr>";
    }
  }
  mPositionCount = posList.count();
  mTotal = posList.sumPrice().toHtmlString();
  mPositions += QString( "<tr><td colspan=\"3\" align=\"right\"><b>Total: %1</b></td></tr>" ).arg( mTotal );
  mPositions += "</table>";
  // kdDebug() << "Positions-HTML: " << mPositions << endl;
}

void DocPostCard::setFooterData( const QString& postText,  const QString& goodbye )
{
  mPostText = htmlify( postText );
  mGoodbye = goodbye;
}

void DocPostCard::renderDoc( int id )
{
  QString t;
  kdDebug() << "rendering postcard for active id " << id <<
    ( mMode == Full ? " (full) " : " (mini) " ) << endl;
  if ( mMode == Full ) {
    t = renderDocFull( id );
  } else if ( mMode == Mini ) {
    t = renderDocMini( id );
  } else {
    kdDebug() << "Unknown postcard mode" << endl;
  }

  // kdDebug () << t << endl;
  displayContent( t );
}

QString DocPostCard::renderDocFull( int id )
{
  QString t;
  if ( id == KraftDoc::Header ) {
    // header is the selected doc part
    t += QString( "<div class=\"headerlink_selected\">" );
    t += QString( i18n( "Header" ) );
  } else {
    t += QString( "<div class=\"headerlink\">" );
    t += linkBit( "kraftdoc://header", i18n( "Header" ) );
  }
  t += "</div>\n" ;

  t += "<div class=\"head\">\n";
  QString h = mAddress;
  h.replace( '\n', "<br/>" );
  t += "<table border=\"0\" width=\"99%\">";
  t += "<tr><td>";
  t += QString( "%1\n" ).arg( h );
  t += "</td><td align=\"right\" valign=\"top\">";
  t += QString( "%1<br />%2\n" ).arg( mType ).arg( mDate );
  t += "</td></tr></table>";

  t += "<p class=\"longtext\">" + mPreText + "</p>\n";
  t += "</div>\n";

  if ( id == KraftDoc::Positions ) {
    // header is the selected doc part
    t += QString( "<div class=\"bodylink_selected\">" );
    t += QString( i18n( "Positions" ) );
  } else {
    t += QString( "<div class=\"bodylink\">" );
    t += linkBit( "kraftdoc://positions", i18n( "Positions" ) );
  }
  t += "</div>\n" ;

  t += "<div class=\"body\">\n";
  t += mPositions;
  t += "\n</div>\n";

  if ( id == KraftDoc::Footer ) {
    // header is the selected doc part
    t += QString( "<div class=\"footerlink_selected\">" );
    t += QString( i18n( "Footer" ) );
  } else {
    t += QString( "<div class=\"footerlink\">" );
    t += linkBit( "kraftdoc://footer", i18n( "Footer" ) );
  }
  t += "</div>\n" ;
  t += "<div class=\"footer\">\n";
  t += "<p class=\"longtext\">" + mPostText + "</p>\n";
  if ( ! mGoodbye.isEmpty() )
    t += "<p>" + mGoodbye + "</p>\n";
  t += "</div>\n";

  // kdDebug() << "\n\n" << t << "\n\n" << endl;

  return t;
}

QString DocPostCard::renderDocMini( int id ) const
{
  QString t;
  QString c = "headerlink";
  if ( id == KraftDoc::Header ) c = "headerlink_selected";
  t += QString( "<div class=\"%1\">" ).arg( c );
  t += linkBit( "kraftdoc://header",  i18n( "Header" ) );
  t += QString( "%1 from %2" ).arg( mType ).arg( mDate );
  t += "</div>";

  c = "bodylink";
  if ( id == KraftDoc::Positions ) c = "bodylink_selected";
  t += QString( "<div class=\"%1\">" ).arg( c );
  t += linkBit( "kraftdoc://positions", i18n( "Positions" ) );
  t += QString( " %1 Positions, total %2" ).arg( mPositionCount ).arg( mTotal );
  t += "</div>";

  c = "footerlink";
  if ( id == KraftDoc::Footer ) c = "footerlink_selected";
  t += QString( "<div class=\"%1\">" ).arg( c );
  t += linkBit( "kraftdoc://footer", i18n( "Footer" ) );
#if 0
  QString h( mPostText );
  if ( h.length() > 45 ) {
    h = h.left( 42 ) +  "...";
  }
  t += h;
#endif
  t += "</div>";

return t;
}

QString DocPostCard::linkBit( const QString& url, const QString& display ) const
{
  return QString( "<a href=\"%1\">[%2]</a> " ).arg( url ).arg( display );
}

void DocPostCard::urlSelected( const QString &url, int, int,
  const QString &, KParts::URLArgs  )
{
  kdDebug() << "DocPostCard::urlSelected(): " << url << endl;

  KURL kurl( url );

  KraftDoc::Part id = KraftDoc::Header;

  if ( kurl.protocol() == "kraftdoc" ) {
    if ( kurl.host() == "header" ) {
      kdDebug() << "Header selected!" << endl;
      id = KraftDoc::Header;
    } else if ( kurl.host() == "positions" ) {
      kdDebug() << "Positions selected!" << endl;
      id = KraftDoc::Positions;
    } else if ( kurl.host() == "footer" ) {
      kdDebug() << "Footer selected!" << endl;
      id = KraftDoc::Footer;
    }
    emit selectPage( id );
  }
}

void DocPostCard::writeTopFrame()
{

}

void DocPostCard::slotSetMode( DisplayMode mode, int id ) {
  mMode = mode;
  renderDoc( id );
}
#include "docpostcard.moc"
