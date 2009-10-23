/***************************************************************************
                          flostempllist.cpp  -
                             -------------------
    begin                : Son Feb 8 2004
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

#include <q3sqlcursor.h>
#include <QSqlQuery>
#include <QStringList>
#include <qdom.h>
#include <QFile>
#include <q3textstream.h>
#include <QDir>

#include <kdebug.h>
#include <kfiledialog.h>
#include <klocale.h>

#include "floskeltemplate.h"
#include "dbids.h"
#include "templkatalog.h"
#include "kraftdb.h"
#include "unitmanager.h"
#include "zeitcalcpart.h"
#include "fixcalcpart.h"
#include "materialcalcpart.h"
#include "geld.h"
#include "katalog.h"

/** constructor of a katalog, which is only a list of Floskel templates.
 *  A name must be given, which is displayed for the root element in the
 *
 */

TemplKatalog::TemplKatalog( const QString& name )
    : Katalog( name )
{

}

TemplKatalog::~TemplKatalog()
{

}

void TemplKatalog::reload( dbID )
{
  kDebug() << "**** RELOADING katalog ****" << endl;
  m_flosList.clear();
  load();
}

int TemplKatalog::load()
{
    Katalog::load();
    int cnt = 0;

    Q3SqlCursor cur( "Catalog" ); // Specify the table/view name
    cur.setMode( Q3SqlCursor::ReadOnly );
    cur.select(); // We'll retrieve every record
    while ( cur.next() ) {
        cnt++;
        int einheit = cur.value("unitID").toInt();
        int templID = cur.value("TemplID").toInt();
        kDebug() << "Loading template number " << templID << endl;
        int chapID = cur.value("chapterID").toInt();
        // int sortID = cur.value( "sortKey" ).toInt();
        int calcKind = cur.value("Preisart").toInt();
        double g = cur.value("EPreis").toDouble();

        Geld preis(g);
        /* Only for debugging: */
        if( templID == 272 ) {
            kDebug() << "Geld ist " << preis.toString( *( &mLocale ) ) << " from g-value " << g << endl;
        }

        QDateTime modDt;
        QString modDate = cur.value("modifyDatum").toString();
        /* modifyDatum ist TIMESTAMP und den gibt mysql offensichtlich mit einem T im
         * String zurck */
        if( modDate[10] == 'T' ) {
            modDate = modDate.replace( 10, 1, QChar(' ') );
            modDt = QDateTime::fromString(modDate, Qt::ISODate );
        }

        QDateTime enterDt = cur.value("enterDatum").toDateTime();

        kDebug() << "Chapter ID is " << chapID << endl;

        FloskelTemplate *flos = new FloskelTemplate( templID,
                                                     QString::fromUtf8(cur.value("Floskel").toByteArray()),
                                                     einheit, chapID, calcKind,
                                                     modDt, enterDt );
        // flos->setSortKey( sortID );
        flos->setGewinn( cur.value("Gewinn").toDouble());
        flos->setManualPrice( preis );
        bool tslice = cur.value("zeitbeitrag").toInt() > 0;
        flos->setHasTimeslice( tslice );

        loadCalcParts( flos );

        // FIXME KDE4: Append to list sorted.
        m_flosList.append(flos);
    }
    return cnt;
}

int TemplKatalog::addNewTemplate( FloskelTemplate *tmpl )
{
  int re = -1;

  if ( tmpl ) {
    m_flosList.append( tmpl );
    re = m_flosList.count();
  }
  return re;
}

int TemplKatalog::loadCalcParts( FloskelTemplate *flos )
{
    int cnt = 0;

    cnt = loadTimeCalcParts( flos );
    cnt += loadFixCalcParts( flos );
    cnt += loadMaterialCalcParts(flos);
    return cnt;
}

int TemplKatalog::loadTimeCalcParts( FloskelTemplate *flos )
{
    if( ! flos ) return(0);
    int cnt = 0;

    Q3SqlCursor cur("CalcTime");

    cur.select( "TemplID=" + QString::number( flos->getTemplID()));

    while( cur.next() )
    {
        cnt++;
        int tcalcid = cur.value("TCalcID").toInt();
        int templid = cur.value("TemplID").toInt();

        QString name = QString::fromUtf8(cur.value("name").toByteArray());
        int minutes = cur.value("minutes").toInt();
        int prozent = cur.value("percent").toInt();
        int hourSet = cur.value("stdHourSet").toInt();
        bool globAllowed = cur.value("allowGlobal").toInt() > 0;

        ZeitCalcPart *zcp = new ZeitCalcPart( name, minutes, prozent );
        zcp->setGlobalStdSetAllowed( globAllowed );
        zcp->setStundensatz( StdSatzMan::self()->getStdSatz(hourSet) );

        zcp->setDbID( dbID(tcalcid));
        zcp->setTemplID( dbID(templid));
	zcp->setDirty( false );
        flos->addCalcPart( zcp );
    }

    return cnt;
}

int TemplKatalog::loadMaterialCalcParts( FloskelTemplate *flos )
{
    if( ! flos ) return(0);
    int cnt = 0;

    Q3SqlCursor cur("CalcMaterials");

    cur.select( "TemplID=" + QString::number( flos->getTemplID()));

    while( cur.next() )
    {
        cnt++;
        QString name = QString::fromUtf8(cur.value("name").toByteArray());
        int prozent = cur.value("percent").toInt();
        long mcalcID = cur.value("MCalcID").toLongLong();
        int templid = cur.value("TemplID").toInt();

        MaterialCalcPart *mPart = new MaterialCalcPart( mcalcID, name, prozent );
        mPart->setDbID( dbID(mcalcID));
        mPart->setTemplID( dbID(templid));
	mPart->setDirty( false );
        flos->addCalcPart( mPart );
        loadMaterialDetails( mcalcID, mPart );
    }

    return cnt;
}

int TemplKatalog::loadMaterialDetails( long calcID, MaterialCalcPart* mcp )
{
    if( ! mcp ) return 0;

    Q3SqlCursor cur("CalcMaterialDetails");
    cur.select("CalcID=" + QString::number(calcID));

    int cnt = 0;
    while( cur.next())
    {
        cnt ++;

        long   matID  = cur.value("materialID").toLongLong();
        double amount = cur.value("amount").toDouble();

        mcp->addMaterial( amount, matID );
    }

    return cnt;
}


int TemplKatalog::loadFixCalcParts( FloskelTemplate *flos )
{
    if( ! flos ) return(0);
    int cnt = 0;

    Q3SqlCursor cur("CalcFixed");

    cur.select( "TemplID=" + QString::number( flos->getTemplID()));

    while( cur.next() )
    {
        cnt++;
        QString name  = QString::fromUtf8(cur.value("name").toByteArray());
        double amount = cur.value("amount").toDouble();
        int percent   = cur.value("percent").toInt();
        int tcalcid = cur.value("FCalcID").toInt();
        int templid = cur.value("TemplID").toInt();

        double g      = cur.value("price").toDouble();
        Geld price(g); //     = (int) g; // FIXME: proper handling of money here.

        FixCalcPart *fcp = new FixCalcPart( name, price, percent );
        fcp->setMenge( amount );
        fcp->setDbID( dbID(tcalcid));
        fcp->setTemplID( dbID(templid));
	fcp->setDirty( false );
        flos->addCalcPart( fcp );
    }

    return cnt;
}


FloskelTemplateList TemplKatalog::getFlosTemplates( const QString& chapter )
{
    FloskelTemplateList resultList;
    int chap = chapterID(chapter);

    if( m_flosList.count() == 0 )
    {
        kDebug() << "Empty katalog list - loading!" << endl;
        load();
    }

    FloskelTemplateListIterator it(m_flosList);
    FloskelTemplate *tmpl;

    while( it.hasNext() )
    {
      tmpl = it.next();

      int haveChap = tmpl->getChapterID();

      // kDebug() << "Searching for chapter " << chapter << " with ID " << chap << " and have " << haveChap << endl;
      if( haveChap == chap )
      {
        resultList.append( tmpl );
      }
    }
    return resultList;
}


int TemplKatalog::load( const QString& /* chapter */ )
{
    return 0;
}


void TemplKatalog::writeXMLFile()
{
    QString filename = KFileDialog::getSaveFileName( QDir::homePath(),
            "*.xml", 0, i18n("Export XML Katalog"));
    if(filename.isEmpty()) return;

    QDomDocument doc = toXML();

    QFile file( filename );
    if( file.open( QIODevice::WriteOnly ) )
    {
        Q3TextStream ts( &file );
        ts << doc.toString();

        file.close();
    }

}

QDomDocument TemplKatalog::toXML()
{

    QDomDocument doc("catalog");
    QDomElement root = doc.createElement("catalog");
    doc.appendChild(root);
    QDomElement elem = doc.createElement("catalogname");
    QDomText text = doc.createTextNode(m_name);
    elem.appendChild(text);
    root.appendChild(elem);

    QStringList allSets = StdSatzMan::self()->allStdSaetze();
    for ( QStringList::Iterator it = allSets.begin(); it != allSets.end(); ++it ) {
        QDomElement set = doc.createElement("hourset");
        QDomElement elem = doc.createElement("name");
        QDomText tname = doc.createTextNode(*it);
        elem.appendChild(tname);
        set.appendChild(elem);

        QDomElement rateelem = doc.createElement("rate");
        StdSatz satz = StdSatzMan::self()->getStdSatz(*it);
        Geld g = satz.getPreis();
        QDomText rname = doc.createTextNode(g.toString( mLocale ));
        rateelem.appendChild(rname);
        set.appendChild(rateelem);

        root.appendChild(set);
    }

    QStringList chaps = getKatalogChapters();

    for ( QStringList::Iterator it = chaps.begin(); it != chaps.end(); ++it ) {
        QString chapter = *it;
        QDomElement chapElem = doc.createElement("chapter");
        QDomElement chapName = doc.createElement("chaptername");
        text = doc.createTextNode(chapter);
        chapName.appendChild(text);
        chapElem.appendChild(chapName);
        root.appendChild(chapElem);

        FloskelTemplateList templs = getFlosTemplates(chapter);
        FloskelTemplateListIterator it(templs);
        // FloskelTemplate *tmpl = 0;

        // while( ( it.hasNext()) {
        //  tmpl = it.next();
          // chapElem.appendChild( tmpl->toXML(doc));

        // }
    }
    return doc;
}


int TemplKatalog::getEntriesPerChapter( const QString& chapter)
{
    int cnt = 0;

    QString q( "SELECT count(*) FROM katalog" );
    if( !chapter.isEmpty() ) {
        int id = chapterID( chapter );
        if( id > 0 ) {
            q += " WHERE chapterID=" + QString::number( id );
        } else {
            return cnt;
        }
    }
    QSqlQuery query( q );

    while ( query.next() ) {
        cnt = query.value(0).toInt();
    }
    return cnt;
}
