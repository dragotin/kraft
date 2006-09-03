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

#include <klocale.h>
#include <kglobal.h>
#include <kurl.h>

#include <kdebug.h>

DocPostCard::DocPostCard( QWidget *parent )
  :HtmlView( parent ),  mMode( Full )
{
  setZoomFactor( 60 );
}

void DocPostCard::setHeaderData( const QString& type,  const QString& date,
                                 const QString& address, const QString& pretext )
{
  mType = type;
  mDate = date;
  mAddress = address;
  mPreText = pretext;
}

void DocPostCard::setPositions( DocPositionList posList )
{
  mPositions = "<table border=\"0\" width=\"99%\">";
  DocPositionBase *dpb;
  for( dpb = posList.first(); dpb; dpb = posList.next() ) {
     if( dpb->type() == DocPositionBase::Position ) {
      DocPosition *dp = static_cast<DocPosition*>(dpb);
      mPositions += "<tr><td width=\"20px\" align=\"right\" valign=\"top\">" + dp->position() + ".</td>";
      mPositions += "<td>" + dp->text() + "</td>";
      mPositions += "<td width=\"50px\" align=\"right\">" + dp->overallPrice().toString() + "</td></tr>";
    }
  }
  mPositions += "</table>";
}

void DocPostCard::setFooterData( const QString& postText,  const QString& goodbye )
{
  mPostText = postText;
  mGoodbye = goodbye;
}

void DocPostCard::renderDoc()
{
  QString t;
  if ( mMode == Full ) {
    t = renderDocFull();
  } else if ( mMode == Mini ) {
    t = renderDocMini();
  } else {
    kdDebug() << "Unknown postcard mode" << endl;
  }

  // kdDebug () << t << endl;
  displayContent( t );
}

QString DocPostCard::renderDocFull() const
{

  QString t = "<div class=\"head\">";
  t += "<a href=\"kraftdoc://header\">" + i18n( "Header:" ) + "</a>" ;
  QString h = mAddress;
  h.replace( '\n', "<br/>" );
  t += "<table border=\"0\" width=\"99%\">";
  t += "<tr><td>";
  t += QString( "%1\n" ).arg( h );
  t += "</td><td align=\"right\" valign=\"top\">";
  t += QString( "%1<br />%2\n" ).arg( mType ).arg( mDate );
  t += "</td></tr></table>";

  t += "<p class=\"longtext\">" + mPreText + "</p>\n";
  t += "</div>";

  t += "<div class=\"body\">";
  t += "<a href=\"kraftdoc://positions\">" + i18n( "Positions:" ) + "</a>\n" ;
  t += mPositions;
  t += "\n</div>";

  t += "<div class=\"footer\">";
  t += "<a href=\"kraftdoc://footer\">" + i18n( "Footer:" ) + "</a>\n" ;

  t += "<p class=\"longtext\">" + mPostText + "</p>\n";
  t += "<p>" + mGoodbye + "</p>\n";
  t += "</div>\n";

  t += "</body></html>";
  return t;
}

QString DocPostCard::renderDocMini() const
{
  QString t = "<div class=\"head\">";
  t += "<a href=\"kraftdoc://header\">" + i18n( "Header:" ) + "</a>" ;
  t += "</div>";

  t += "<div class=\"body\">";
  t += "<a href=\"kraftdoc://positions\">" + i18n( "Positions:" ) + "</a>\n" ;
  t += "so many positions";
  t += "</div>";

  t += "<div class=\"footer\">";
  t += "<a href=\"kraftdoc://footer\">" + i18n( "Footer:" ) + "</a>\n" ;
  t += "</div>";

return t;
}

void DocPostCard::urlSelected( const QString &url, int, int,
  const QString &, KParts::URLArgs  )
{
  kdDebug() << "DocPostCard::urlSelected(): " << url << endl;

  KURL kurl( url );

  PageId id = HeaderId;

  if ( kurl.protocol() == "kraftdoc" ) {
    if ( kurl.host() == "header" ) {
      kdDebug() << "Header selected!" << endl;
      id = HeaderId;
    } else if ( kurl.host() == "positions" ) {
      kdDebug() << "Positions selected!" << endl;
      id = PositionId;
    } else if ( kurl.host() == "footer" ) {
      kdDebug() << "Footer selected!" << endl;
      id = FooterId;
    }
    emit selectPage( id );
  }
}

void DocPostCard::writeTopFrame()
{

}

void DocPostCard::slotSetMode( DisplayMode mode ) {
  mMode = mode;
}
#include "docpostcard.moc"
