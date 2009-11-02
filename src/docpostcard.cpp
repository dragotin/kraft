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

#include <klocale.h>
#include <kglobal.h>
#include <kurl.h>
#include <khtmlview.h>

#include <kdebug.h>
#include <kstandarddirs.h>
#include <QTextDocument>

DocPostCard::DocPostCard( QWidget *parent )
  :HtmlView( parent ),  mMode( Full )
{

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
  QStringList li = Qt::escape(str).split( "\n" );
  return "<p>" + li.join( "</p><p>" ) + "</p>";
}

void DocPostCard::setPositions( DocPositionList posList, DocPositionBase::TaxType taxType,
                                double tax, double reducedTax )
{
  mPositions = "<table border=\"0\" width=\"99%\">";

  DocPositionListIterator it(posList);
  while( it.hasNext() ) {
    DocPositionBase *dpb = it.next();
    if( dpb->type() != DocPositionBase::Header ) {
      DocPosition *dp = static_cast<DocPosition*>(dpb);
      mPositions += "<tr><td width=\"20px\" align=\"right\" valign=\"top\">";

      if ( dp->toDelete() ) mPositions += "<strike>";
      mPositions += posList.posNumber( dpb ) + ". ";
      if ( dp->toDelete() ) mPositions += "</strike>";
      mPositions += "</td>";
      mPositions += "<td>";
      if ( dp->toDelete() ) mPositions += "<strike>";

      if ( dp->attributes().contains( DocPosition::Kind ) ) {
        mPositions += "<i>" + dp->text() + "</i>";
      } else {
        mPositions += dp->text();
      }

      if ( dp->toDelete() ) mPositions += "</strike>";
      mPositions += "</td>";
      mPositions += "<td width=\"50px\" align=\"right\">";
      if ( dp->toDelete() ) mPositions += "<strike>";
      mPositions += dp->overallPrice().toHtmlString( posList.locale() );
      if ( dp->toDelete() ) mPositions += "</strike>";
      mPositions += "</td></tr>";
    }
  }
  mPositions += "</table>";
  // Create the sum table
  mPositions += "<div class=\"alignright\" align=\"right\"><table border=\"0\">";
  mPositionCount = posList.count();
  mTotal  = posList.nettoPrice().toHtmlString( posList.locale() );
  QString brutto = posList.bruttoPrice( tax, reducedTax ).toHtmlString( posList.locale() );
  QString taxStr = posList.taxSum( tax, reducedTax ).toHtmlString( posList.locale() );
  mPositions += QString( "<tr><td colspan=\"2\" class=\"baseline\"></td></tr>" );

  if ( taxType != DocPositionBase::TaxInvalid && taxType != DocPositionBase::TaxNone ) {
    mPositions += QString( "<tr><td>&nbsp;&nbsp;&nbsp;" ) + i18n( "Netto:" )+
                  QString( "</td><td align=\"right\">%1</td></tr>" ).arg( mTotal );

    QString curTax;
    curTax.setNum( tax, 'f', 1 );
    if ( taxType == DocPositionBase::TaxReduced ) {
      curTax.setNum( reducedTax, 'f', 1 );
    }

    mPositions += QString( "<tr><td>" ) + i18n( "+ %1% Tax:" ).arg( curTax ) +
                  QString( "</td><td align=\"right\">%1</td></tr>" ).arg( taxStr );
  }
  mPositions += QString( "<tr><td><b>" ) + i18n( "Total:" )+
                QString( "</b></td><td align=\"right\"><b>%1</b></td></tr>" ).arg( brutto );
  mPositions += "</table></div>";
  // kDebug() << "Positions-HTML: " << mPositions << endl;
}

void DocPostCard::setFooterData( const QString& postText,  const QString& goodbye )
{
  mPostText = htmlify( postText );
  mGoodbye = goodbye;
}

void DocPostCard::renderDoc( int id )
{
  QString t;
  // kDebug() << "rendering postcard for active id " << id <<
    //( mMode == Full ? " (full) " : " (mini) " ) << endl;
  if ( mMode == Full ) {
    t = renderDocFull( id );
  } else if ( mMode == Mini ) {
    t = renderDocMini( id );
  } else {
    kDebug() << "Unknown postcard mode" << endl;
  }

  // kDebug () << t << endl;
  displayContent( t );
}

QString DocPostCard::renderDocFull( int id )
{
  QString rethtml;
  QString t;
  QString selString;

  setStylesheetFile( "docoverview.css" );
  setTitle( i18n( "Document Overview" ) );

  rethtml = QString::fromLatin1( "<body>" );

  if ( id == KraftDoc::Header ) selString = QString::fromLatin1( "_selected" );
  t += QString( "<div class=\"head%1\">\n" ).arg( selString );

  t += header( id == KraftDoc::Header, "headerlink", i18n( "Header" ), "header" );

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
  rethtml += linkBit( "kraftdoc://header", t );


  // the Body section showing the positions
    selString= QString();
  if ( id == KraftDoc::Positions ) selString = QString::fromLatin1( "_selected" );
  t = QString( "<div class=\"body%1\">\n" ).arg( selString );
  t += header( id == KraftDoc::Positions, "bodylink", KraftDoc::partToString(KraftDoc::Positions ), "positions" );

  t += mPositions;
  t += "\n</div>\n";
  rethtml += linkBit( "kraftdoc://positions", t );

  selString = QString();
  if ( id == KraftDoc::Footer ) selString = QString::fromLatin1( "_selected" );
  t = QString( "<div class=\"foot%1\">\n" ).arg( selString );
  t += header( id == KraftDoc::Footer, "footerlink", i18n( "Footer" ), "footer" );

  t += "<p class=\"longtext\">" + mPostText + "</p>\n";
  if ( ! mGoodbye.isEmpty() )
    t += "<p>" + mGoodbye + "</p>\n";
  t += "</div>\n";

  rethtml += linkBit( "kraftdoc://footer", t );
  rethtml += "</body>";

  return rethtml;
}

QString DocPostCard::renderDocMini( int id ) const
{
  QString t;
  QString rethtml = QString::fromLatin1( "<body>" );
  QString selString;

  if ( id == KraftDoc::Header ) selString = QString::fromLatin1( "_selected" );
  t = QString( "<div class=\"head%1\">\n" ).arg( selString );
  t += header( id == KraftDoc::Header, "headerlink", i18n( "Header" ), "header",
               QString( "%1 from %2" ).arg( mType ).arg( mDate ) );
  t += "</div>";
  rethtml += linkBit( "kraftdoc://header", t );

  selString = QString();
  if ( id == KraftDoc::Positions ) selString = QString::fromLatin1( "_selected" );
  t = QString( "<div class=\"body%1\">\n" ).arg( selString );
  t += header( id == KraftDoc::Positions, "bodylink", i18n( "Positions" ), "positions",
               i18n( " %1 Positions, netto %2" ).arg( mPositionCount ).arg( mTotal ) );
  t += "</div>";
  rethtml += linkBit( "kraftdoc://positions", t );

  selString = QString();
  if ( id == KraftDoc::Footer ) selString = QString::fromLatin1( "_selected" );
  t = QString( "<div class=\"foot%1\">\n" ).arg( selString );
  t += header( id == KraftDoc::Footer, "footerlink", i18n( "Footer" ), "footer" );
  t += "</div>";
  rethtml += linkBit( "kraftdoc://footer", t );
  rethtml += "</body>";

  return rethtml;
}

QString DocPostCard::header( bool selected,
                             const QString& styleName,
                             const QString& displayName,
                             const QString& /* protocol */,
                             const QString& addons ) const
{
  QString t;
  if ( selected ) {
    t += QString( "<div class=\"%1_selected\">" ).arg( styleName );
    t += displayName;
  } else {
    t += QString( "<div class=\"%1\">" ).arg( styleName );
    t += displayName;

    // t += linkBit( QString( "kraftdoc://%1" ).arg( protocol ), "["+displayName+"]" );
  }
  if ( ! addons.isEmpty() ) {
    t += "-&nbsp;";
    t += addons;
  }
  t += "</div>\n";

  return t;
}

QString DocPostCard::linkBit( const QString& url, const QString& display ) const
{
  return QString( "<a href=\"%1\">%2</a> " ).arg( url ).arg( display );
}

bool DocPostCard::urlSelected (const QString &url, int, int, const QString &,
                    const KParts::OpenUrlArguments &,
                    const KParts::BrowserArguments &)
{
  kDebug() << "DocPostCard::urlSelected(): " << url << endl;

  KUrl kurl( url );

  KraftDoc::Part id = KraftDoc::Header;

  if ( kurl.protocol() == "kraftdoc" ) {
    if ( kurl.host() == "header" ) {
      kDebug() << "Header selected!" << endl;
      id = KraftDoc::Header;
    } else if ( kurl.host() == "positions" ) {
      kDebug() << "Positions selected!" << endl;
      id = KraftDoc::Positions;
    } else if ( kurl.host() == "footer" ) {
      kDebug() << "Footer selected!" << endl;
      id = KraftDoc::Footer;
    }
    emit selectPage( id );
  }
  return true;
}

void DocPostCard::slotSetMode( DisplayMode mode, int id ) {
  mMode = mode;
  renderDoc( id );
}
#include "docpostcard.moc"
