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
  :HtmlView( parent )
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
  mPositions = "<table border=\"1\" width=\"99%\">";
  DocPositionBase *dpb;
  for( dpb = posList.first(); dpb; dpb = posList.next() ) {
     if( dpb->type() == DocPositionBase::Position ) {
      DocPosition *dp = static_cast<DocPosition*>(dpb);
      mPositions += "<tr><td width=\"15px\">" + dp->position() + "</td>";
      mPositions += "<td>" + dp->text() + "</td>";
      mPositions += "<td width=\"45px\" align=\"right\">" + dp->overallPrice().toString() + "</td></tr>";
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
    QString t = "<div class=\"head\">";

    t += "<a href=\"kraftdoc://header\">" + i18n( "Header:" ) + "</a>" ;
    QString h = mAddress;
    h.replace( '\n', "<br/>" );
    t += QString( "<p class=\"address\">%1</p>\n" ).arg( h );
    t += "<p>" + mType + i18n( " from " ) + mDate + "</p>\n";
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
    kdDebug () << t << endl;
    displayContent( t );
}

void DocPostCard::showDocument( DocGuardedPtr ptr )
{
  mDoc = ptr;

  if( ptr ) {
    // t += i18n( "Document to show: " ) + ptr->ident();
    QString t = "<div class=\"head\">";

    t += "<a href=\"kraftdoc://header\">" + i18n( "Header:" ) + "</a>" ;
    QString h = ptr->address();
    h.replace( '\n', "<br/>" );
    t += QString( "<p class=\"address\">%1</p>" ).arg( h );
    t += "<p>" + ptr->docType() + i18n( " from " ) +
         KGlobal().locale()->formatDate( ptr->date() ) + "</p>";
    t += "<p class=\"longtext\">" + ptr->preText() + "</p>";
    t += "</div>";

    t += "<div class=\"body\">";
    t += "<a href=\"kraftdoc://positions\">" + i18n( "Positions:" ) + "</a>" ;

    DocPositionBase *dpb;
    DocPositionList posList = ptr->positions();

    t += "<table border=\"1\" width=\"99%\">";
    for( dpb = posList.first(); dpb; dpb = posList.next() ) {
      if( dpb->type() == DocPositionBase::Position ) {
        DocPosition *dp = static_cast<DocPosition*>(dpb);
        t += "<tr><td width=\"15px\">" + dp->position() + "</td>";
        t += "<td>" + dp->text() + "</td>";
        t += "<td width=\"45px\" align=\"right\">" + dp->overallPrice().toString() + "</td></tr>";
      }
    }
    t += "</table>";
    t += "</div>";

    t += "<div class=\"footer\">";
    t += "<a href=\"kraftdoc://footer\">" + i18n( "Footer:" ) + "</a>" ;

    t += "<p class=\"longtext\">" + ptr->postText() + "</p>";
    t += "<p>" + ptr->goodbye() + "</p>";
    t += "</div>";

    t += "</body></html>";
    kdDebug () << t << endl;
    displayContent( t );

  }

}

void DocPostCard::urlSelected( const QString &url, int button, int state,
  const QString &_target, KParts::URLArgs args )
{
  kdDebug() << "DocPostCard::urlSelected(): " << url << endl;

  KURL kurl( url );

  if ( kurl.protocol() == "kraftdoc" ) {
    if ( kurl.host() == "header" ) {
      kdDebug() << "Header selected!" << endl;
      emit selectPage( HeaderId );
    } else if ( kurl.host() == "positions" ) {
      kdDebug() << "Positions selected!" << endl;
      emit selectPage( PositionId );
    } else if ( kurl.host() == "footer" ) {
      kdDebug() << "Footer selected!" << endl;
      emit selectPage( FooterId );
    }
  }
}

void DocPostCard::writeTopFrame()
{

}
#include "docpostcard.moc"
