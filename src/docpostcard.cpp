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

#include <KLocalizedString>
#include <QLocale>
#include <QUrl>
#include <QStandardPaths>
#include <QDebug>
#include <QTextDocument>

#define QL1(X) QLatin1String(X)

DocPostCard::DocPostCard( QWidget *parent )
    :HtmlView( parent ),  mMode( Full ), mShowPrices(true)
{
  setStylesheetFile( "docoverview.css" );
  setTitle( i18n( "Document Overview" ) );

  connect( this, SIGNAL(openUrl(QUrl)), this, SLOT(slotUrlSelected(QUrl)) );
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
  return QL1("<p>") + li.join( "</p><p>" ) + QL1("</p>");
}

#define REDUCED_TAX_MARK "&#xB2;"
#define NO_TAX_MARK "&#xB9;"

void DocPostCard::setPositions( DocPositionList posList, DocPositionBase::TaxType taxType,
                                double tax, double reducedTax )
{
  mPositions = "<div  align=\"right\"><table border=\"0\" width=\"99%\">";

  DocPositionListIterator it(posList);
  while( it.hasNext() ) {
      DocPositionBase *dpb = it.next();
      DocPosition *dp = static_cast<DocPosition*>(dpb);
      mPositions += "<tr><td valign=\"top\" width=\"20\" class=\"itemnums\">";

      if ( dp->toDelete() ) mPositions += "<strike>";
      mPositions += posList.posNumber( dpb ) + ". ";
      if ( dp->toDelete() ) mPositions += "</strike>";
      mPositions += "</td>";
      mPositions += "<td class=\"itemtexts\">";
      if ( dp->toDelete() ) mPositions += "<strike>";

      if ( dp->attributes().contains( DocPosition::Kind ) ) {
          mPositions += "<i>" + dp->text() + "</i>";
      } else {
          mPositions += htmlify(dp->text());
      }

      if ( dp->toDelete() ) mPositions += "</strike>";
      mPositions += "</td>";

      if( mShowPrices ) {
          mPositions += "<td align=\"right\" valign=\"bottom\" class=\"prices\">";
          if ( dp->toDelete() ) mPositions += "<strike>";
          mPositions += dp->overallPrice().toHtmlString();
          if ( dp->toDelete() ) mPositions += "</strike>";
          mPositions += "</td>";

          mPositions += "<td align=\"right\" valign=\"bottom\" width=\"12\">";
          if( taxType == DocPositionBase::TaxIndividual && (dp->taxType() == DocPositionBase::TaxReduced) ) {
              if ( dp->toDelete() ) mPositions += "<strike>";
              mPositions += QString(REDUCED_TAX_MARK);
              if ( dp->toDelete() ) mPositions += "</strike>";
          }

          if( taxType == DocPositionBase::TaxIndividual && (dp->taxType() == DocPositionBase::TaxNone) ) {
              if ( dp->toDelete() ) mPositions += "<strike>";
              mPositions += QString(NO_TAX_MARK);
              if ( dp->toDelete() ) mPositions += "</strike>";
          }
          mPositions += "</td>";
      }
      mPositions += "</tr>";

  }
  mPositions += "</table></div>";

  // Create the sum table
  mPositionCount = posList.count();
  if( mShowPrices ) {
      mPositions += "<div align=\"right\"><table border=\"0\" width=\"66%\">";
      mPositionCount = posList.count();
      mTotal  = posList.nettoPrice().toHtmlString();
      QString brutto = posList.bruttoPrice( tax, reducedTax ).toHtmlString();
      mPositions += QString( "<tr><td align=\"right\" colspan=\"2\" class=\"baseline\">______________________________</td><td width=\"12\" align=\"right\"></td></tr>" );

      if ( taxType != DocPositionBase::TaxInvalid && taxType != DocPositionBase::TaxNone ) {
          mPositions += QString( "<tr><td align=\"right\">" ) + i18n( "Netto:" )+
                  QString( "</td><td align=\"right\">%1</td><td width=\"12\" align=\"right\"></td></tr>" ).arg( mTotal );

          QString curTax;
          curTax.setNum( tax, 'f', 1 );
          QString taxStr;

          if( taxType == DocPositionBase::TaxReduced || taxType == DocPositionBase::TaxIndividual ) {
              curTax.setNum( reducedTax, 'f', 1 );
              taxStr = posList.reducedTaxSum( reducedTax ).toHtmlString();
              mPositions += QString( "<tr><td align=\"right\">" );
              mPositions += i18n( "+ %1% Tax:", curTax ) +
                      QString( "</td><td align=\"right\">%1</td><td width=\"12\" align=\"right\">%2</td></tr>" ).arg( taxStr ).arg(REDUCED_TAX_MARK);
          }

          if( taxType == DocPositionBase::TaxFull || taxType == DocPositionBase::TaxIndividual ) {
              curTax.setNum( tax, 'f', 1 );
              taxStr = posList.fullTaxSum( tax ).toHtmlString();
              mPositions += QString( "<tr><td align=\"right\">" ) + i18n( "+ %1% Tax:", curTax ) +
                      QString( "</td><td align=\"right\">%1</td><td width=\"12\" align=\"right\"></td></tr>" ).arg( taxStr );
          }

          if( taxType == DocPositionBase::TaxIndividual ) {
              taxStr = posList.taxSum( tax, reducedTax ).toHtmlString();
              mPositions += QString( "<tr><td align=\"right\">" ) + i18n( "Sum Tax:" ) +
                      QString( "</td><td align=\"right\">%1</td><td width=\"12\" align=\"right\"></td></tr>" ).arg( taxStr );
          }

      }
      mPositions += QString( "<tr><td align=\"right\"><b>" ) + i18n( "Total:" )+
              QString( "</b></td><td align=\"right\"><b>%1</b></td><td width=\"12\" align=\"right\"></td></tr>" ).arg( brutto );
  } // showPrices
  mPositions += "</table></div>";
  // qDebug() << "Positions-HTML: " << mPositions << endl;
}

void DocPostCard::setFooterData( const QString& postText,  const QString& goodbye )
{
  mPostText = htmlify( postText );
  mGoodbye = goodbye;
}

void DocPostCard::renderDoc( int id )
{
  QString t;
  // qDebug() << "rendering postcard for active id " << id <<
    //( mMode == Full ? " (full) " : " (mini) " ) << endl;
  if ( mMode == Full ) {
    t = renderDocFull( id );
  } else if ( mMode == Mini ) {
    t = renderDocMini( id );
  } else {
    // qDebug () << "Unknown postcard mode" << endl;
  }

  // qDebug() << t << endl;
  displayContent( t );
}

#define SEL_STRING(X) ( id == X ? QL1("_selected"): QL1(""))

QString DocPostCard::renderDocFull( int id )
{
  QString rethtml;
  QString t;

  rethtml = QL1( "<body>" );

  t += QL1("<a href=\"kraftdoc://header\">");
  t += QString( "<div class=\"head%1\">\n" ).arg( SEL_STRING(KraftDoc::Header) );

  t += header( id == KraftDoc::Header, "headerlink", KraftDoc::partToString(KraftDoc::Header), "kraftdoc://header" );

  QString h = mAddress;
  h.replace( '\n', "<br/>" );
  t += "<table border=\"0\" width=\"99%\">";
  t += "<tr><td>";
  t += QString( "%1\n" ).arg( h );
  t += "</td><td align=\"right\" valign=\"top\">";
  t += QString( "<b>%1</b><br />%2\n" ).arg( mType ).arg( mDate );
  t += "</td></tr></table>";

  t += "<p class=\"longtext\">" + mPreText + "</p>\n";
  t += "</div></a>\n";
    rethtml += t;

  // the Body section showing the positions
  t = QL1("<a href=\"kraftdoc://positions\">");
  t += QString( "<div class=\"body%1\">\n" ).arg( SEL_STRING(KraftDoc::Positions ) );
  t += header( id == KraftDoc::Positions, "bodylink", KraftDoc::partToString(KraftDoc::Positions), "kraftdoc://positions" );

  t += mPositions;
  t += "\n</div></a>\n";
  rethtml += t;

  t = QL1("<a href=\"kraftdoc://footer\">");
  t += QString( "<div class=\"foot%1\">\n" ).arg( SEL_STRING(KraftDoc::Footer) );
  t += header( id == KraftDoc::Footer, "footerlink", KraftDoc::partToString(KraftDoc::Footer), "kraftdoc://footer" );

  t += "<p class=\"longtext\">" + mPostText + "</p>\n";
  if ( ! mGoodbye.isEmpty() )
    t += "<p>" + mGoodbye + "</p>\n";
  t += "</div></a>\n";

  rethtml += t + "</body>";

  return rethtml;
}

QString DocPostCard::renderDocMini( int id ) const
{
  QString t;
  QString rethtml = QL1( "<body>" );

  t = QString( "<div class=\"head%1\">\n" ).arg( SEL_STRING(KraftDoc::Header) );
  t += header( id == KraftDoc::Header, "headerlink", KraftDoc::partToString(KraftDoc::Header), "kraftdoc://header",
               QString( "<b>%1</b>, %2" ).arg( mType ).arg( mDate ) );
  t += QL1("</div>");
  rethtml += t;

  t = QString( "<div class=\"body%1\">\n" ).arg( SEL_STRING(KraftDoc::Positions));
  QString d = i18n("%1 Items", mPositionCount);
  if( mShowPrices )
      d = i18n("%1 Items, netto %2", mPositionCount, mTotal);

  // do not add another "Items" string to the header to not bloat
  t += header( id == KraftDoc::Positions, "bodylink", QString(), "kraftdoc://positions",
               d );
  t += QL1("</div>");
  rethtml += t;

  t = QString( "<div class=\"foot%1\">\n" ).arg(SEL_STRING(KraftDoc::Footer));
  t += header( id == KraftDoc::Footer, "footerlink", KraftDoc::partToString(KraftDoc::Footer), "kraftdoc://footer" );
  t += QL1("</div>");
  rethtml += t;
  rethtml += QL1("</body>");

  return rethtml;
}

QString DocPostCard::header( bool selected,
                             const QString& styleName,
                             const QString& displayName,
                             const QString& protocol,
                             const QString& addons ) const
{
  const QString content = QString("<p class=\"%1\">%2&nbsp;&nbsp;%3</p>")
    .arg( styleName + (selected ? QL1("_selected") : QL1("")))
        .arg(displayName).arg(addons);

  // These colors do the frame around the header boxes
  QString bgCol("#aaaaaa");
  if( !selected ) bgCol = QL1("#cccccc");

  return QString( "<table width=\"99%\" bgcolor=\"%1\" cellpadding=\"3\"><tr>"
                  "<td>< a href=\"%2\">%3</a></td>"
                  "</tr></table>").arg(bgCol).arg(protocol).arg(content);
}

void DocPostCard::slotUrlSelected( const QUrl& kurl)
{
    KraftDoc::Part id = KraftDoc::Header;

    if ( kurl.scheme() == "kraftdoc" ) {
        if ( kurl.host() == "header" ) {
            // qDebug () << "Header selected!" << endl;
            id = KraftDoc::Header;
        } else if ( kurl.host() == "positions" ) {
            // qDebug () << "Positions selected!" << endl;
            id = KraftDoc::Positions;
        } else if ( kurl.host() == "footer" ) {
            // qDebug () << "Footer selected!" << endl;
            id = KraftDoc::Footer;
        }
        emit selectPage( id );
    }
}

void DocPostCard::slotSetMode( DisplayMode mode, int id ) {
  mMode = mode;
  renderDoc( id );
}

void DocPostCard::slotShowPrices( bool showIt )
{
    mShowPrices = showIt;
}
