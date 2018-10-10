/***************************************************************************
                          floskeltemplate.cpp  -
                             -------------------
    begin                : Don Jan 1 2004
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

#include <QString>
#include <QObject>
#include <QDebug>

#include "kraftdb.h"
#include "templatesaverbase.h"
#include "templatesaverdb.h"
#include "floskeltemplate.h"
#include "unitmanager.h"
#include "calcpart.h"
#include "materialcalcpart.h"
#include "fixcalcpart.h"
#include "timecalcpart.h"
#include "stockmaterial.h"


FloskelTemplate::FloskelTemplate()
    : CatalogTemplate(),
      mTemplId(-1),
      m_chapter(0),
      mTimeAdd(true),
      m_listViewItem(0),
      m_saver(0)
{
    m_calcType = Calculation;
}

FloskelTemplate::FloskelTemplate( int tID, const QString& text,
                                  int einheit, int chapter, int calcKind )
 : CatalogTemplate(),
   mTemplId(tID),
   m_chapter(chapter),
   mTimeAdd(true),
   m_preis(long(0)),
   m_listViewItem(0),
   m_saver(0)
{
  if( calcKind == 1 ) {
    setCalculationType( ManualPrice );
  } else if( calcKind == 2 ) {
    setCalculationType( Calculation );
  } else if ( calcKind == 3 ) {
    setCalculationType( AutoCalc );
  }

  setText( text );
  this->setUnitId(einheit);
  setChapterId( dbID(chapter), false );
}

FloskelTemplate::FloskelTemplate( FloskelTemplate& templ )
    : CatalogTemplate( templ ),
      mTemplId( templ.mTemplId ),
      m_preis( templ.m_preis ),
      m_listViewItem(templ.m_listViewItem ),
      m_saver( 0 )
{
  deepCopyCalcParts( templ );
  setModifyDate( templ.modifyDate() );
  setEnterDate( templ.enterDate() );
  setText( templ.getText() );
  setUnitId(templ.unit().id());
  // m_calcParts.setAutoDelete(true);
}

FloskelTemplate& FloskelTemplate::operator= ( FloskelTemplate& src )
{
  if ( this == &src ) return *this;

  mText = src.mText;
  setUnitId(src.unit().id());
  mTemplId = src.mTemplId;
  mChapterId = src.mChapterId;
  m_preis = src.m_preis;
  m_listViewItem = src.m_listViewItem;
  m_saver = 0; // src.m_saver;

  deepCopyCalcParts( src );

  return *this;
}

FloskelTemplate::~FloskelTemplate()
{
  delete m_saver;
}

void FloskelTemplate::deepCopyCalcParts( FloskelTemplate& templ )
{
  CalcPart *cp = 0;

  m_calcParts.clear();

  QListIterator<CalcPart*> i( templ.m_calcParts );
  while( i.hasNext()) {
    cp = i.next();
    CalcPart *ncp = 0;

    if( cp->getType() == KALKPART_TIME ) {
      ncp = new TimeCalcPart( *( static_cast<TimeCalcPart*>(cp) ) );
    } else if( cp->getType() == KALKPART_FIX ) {
      ncp = new FixCalcPart( *( static_cast<FixCalcPart*>(cp) ) );
    } else if( cp->getType() == KALKPART_MATERIAL ) {
      ncp = new MaterialCalcPart( *( static_cast<MaterialCalcPart*>(cp) ) );
    } else {
      // qDebug () << "ERROR: Unknown Calculation-Type!" << endl;
    }
    m_calcParts.append( ncp );
  }
}

void FloskelTemplate::setBenefit( double g )
{
    /* Every calc part has an value for benefit. Set the benefit value for
       each calc part, later on each can have its own value
     */
    for( auto *cp: m_calcParts) {
        cp->setProzentPlus(g);
    }
}

double FloskelTemplate::getBenefit( )
{
    bool first = true;
    double b = 0.0;

    for( auto *cp: m_calcParts) {
        if( first ) {
            b = cp->getProzentPlus();
            first = false;
        }
        // all benefits are the same atm, thus this ASSERT.
        Q_ASSERT( fabs(b - cp->getProzentPlus()) < std::numeric_limits<double>::epsilon());
    }
    return b;
}

void FloskelTemplate::setTemplID( int newID )
{
  mTemplId = newID;
}

Geld FloskelTemplate::unitPrice()
{
  return calcPreis();
}


Geld FloskelTemplate::calcPreis()
{
  Geld g;

    if( calcKind() == ManualPrice ) {
        g = m_preis;
    } else {
        g = m_calcParts.calcPrice();
       double b = getBenefit();
       g += g.percent(b);
    }
    return g;
}

CalcPartList FloskelTemplate::getCalcPartsList()
{
    return getCalcPartsList(ALL_KALKPARTS);
}

// Returns a calcpartlist where all calcparts have lost their connection
// to the database from the dbID POV. That's needed for the transition
// from template -> document calculations.
CalcPartList FloskelTemplate::decoupledCalcPartsList()
{
  return m_calcParts.decoupledCalcPartsList();
}

CalcPartList FloskelTemplate::getCalcPartsList( const QString& calcPart )
{
  return m_calcParts.getCalcPartsList( calcPart );
}

void FloskelTemplate::addCalcPart( CalcPart* cpart )
{
    m_calcParts.append(cpart);
}

void FloskelTemplate::removeCalcPart( CalcPart *cpart )
{
  if( cpart) {
    cpart->setToDelete(true);
    cpart->setDirty(true);

  }
}

void FloskelTemplate::clearCalcParts()
{
  for(int i=0; i<m_calcParts.count(); ++i)
  {
    delete m_calcParts[i];
  }

  m_calcParts.clear();
}

Geld FloskelTemplate::costsByCalcPart( const QString& part )
{
  return m_calcParts.costPerCalcPart( part );
}

TemplateSaverBase* FloskelTemplate::getSaver()
{
  if( ! m_saver )
  {
    m_saver = new TemplateSaverDB();
  }
  return m_saver;
}

bool FloskelTemplate::save()
{
    TemplateSaverBase *saver = getSaver();
    // qDebug () << "Saver is " << saver << endl;
    if( saver ) {
        return saver->saveTemplate( this );
    } else {
        // qDebug () << "ERR: No saver available!" << endl;
        return false;
    }
}

void FloskelTemplate::saveChapterId()
{
  TemplateSaverBase *saver = getSaver();
  if( saver ) {
    saver->saveTemplateChapter( this );
  }
}

#if 0
QDomElement FloskelTemplate::toXML( QDomDocument& doc)
{
    QDomElement templ = doc.createElement("template");

    templ.appendChild( createDomNode(doc, "unit", getUnit().einheitSingular()));
    templ.appendChild( createDomNode(doc, "text", getText()));
    templ.appendChild( createDomNode(doc, "id", QString::number(getTemplID())));
    templ.appendChild( createDomNode(doc, "benefit", QString::number(getBenefit())));
    templ.appendChild( createDomNode(doc, "timecount", hasTimeslice() ? "yes": "no" ));

    QDomElement calcParts = doc.createElement( "calcParts" );
    templ.appendChild(calcParts);
    fixPartsToXML(doc, calcParts);
    timePartsToXML(doc, calcParts);
    materialPartsToXML(doc, calcParts);

    /* Material Calculation Parts */
    materialPartsToXML(doc);
    CalcPartList tpList = getCalcPartsList(KALKPART_MATERIAL);
    MaterialCalcPart *mc = 0;
    mc = static_cast<MaterialCalcPart*>(tpList.first());
    for( ; mc; mc = static_cast<MaterialCalcPart*>(tpList.next()) )
    {
        QDomElement calcPart = doc.createElement( "MaterialCalcpart" );
        calcParts.appendChild(calcPart);
        calcPart.appendChild(createDomNode(doc, "name", mc->getName()));
        calcPart.appendChild(createDomNode(doc, "dbid", mc->getDbID().toString()));

        StockMaterialList materials = mc->getCalcMaterialList();
        StockMaterialListIterator it( materials );

        StockMaterial *mat=0;
        while ( (mat = it.current()) != 0 )
        {
          ++it;
          QDomElement matElem = doc.createElement("Material");
          matElem.appendChild(createDomNode(doc, "MatName", mat->getName()));
          QString h;
          h = h.setNum(mc->getCalcAmount(mat));
          matElem.appendChild(createDomNode(doc, "Amount", h));
          matElem.appendChild(createDomNode(doc, "PriceSum",   mc->getPriceForMaterial(mat).toString()));
          matElem.appendChild(createDomNode(doc, "CostSum",   mc->getCostsForMaterial(mat).toString()));
          matElem.appendChild(createDomNode(doc, "Price",   mat->salesPrice().toString()));
          matElem.appendChild(createDomNode(doc, "Cost",   mat->purchPrice().toString()));
          Einheit e = mat->getUnit();
          h = e.einheitSingular();
          matElem.appendChild(createDomNode(doc, "Unit", h));
          calcPart.appendChild(matElem);
        }
    }
    return templ;
}

void FloskelTemplate::fixPartsToXML( QDomDocument& doc, QDomElement& calcParts )
{
    CalcPartList tpList = getCalcPartsList(KALKPART_FIX);

    FixCalcPart *fc = 0;
    fc = static_cast<FixCalcPart*>(tpList.first());
    for( ; fc; fc = static_cast<FixCalcPart*>(tpList.next()) ) {
        QDomElement calcPart = doc.createElement("FixCalcPart");
        calcParts.appendChild(calcPart);
        calcPart.appendChild(createDomNode(doc, "name", fc->getName()));
        calcPart.appendChild(createDomNode(doc, "dbid", fc->getDbID().toString()));

        QString h;
        h.setNum(fc->getMenge());
        calcPart.appendChild(createDomNode(doc, "amount", h));

        Geld g = fc->unitPreis();
        calcPart.appendChild(createDomNode(doc, "price", g.toString( mLocale )));
    }
}

void FloskelTemplate::timePartsToXML( QDomDocument& doc, QDomElement& calcParts )
{
    CalcPartList tpList = getCalcPartsList(KALKPART_TIME);

    TimeCalcPart *tc = 0;
    tc = static_cast<TimeCalcPart*>(tpList.first());
    for( ; tc; tc = static_cast<TimeCalcPart*>(tpList.next()) ) {
        QDomElement calcPart = doc.createElement("TimeCalcPart");
        calcParts.appendChild(calcPart);
        calcPart.appendChild(createDomNode(doc, "name", tc->getName()));
        calcPart.appendChild(createDomNode(doc, "dbid", tc->getDbID().toString()));

        QString h;
        h.setNum( tc->getMinuten());
        calcPart.appendChild(createDomNode(doc, "minutes", h));
        StdSatz ss = tc->getStundensatz();
        calcPart.appendChild(createDomNode(doc, "stundensatz", ss.getName()));
        calcPart.appendChild(createDomNode(doc, "globalHourSetup",
                             tc->globalStdSetAllowed() ? "yes" : "no"));
    }
}

void FloskelTemplate::materialPartsToXML( QDomDocument& doc, QDomElement& calcParts )
{
    /* Material Calculation Parts */
    CalcPartList tpList = getCalcPartsList(KALKPART_MATERIAL);
    MaterialCalcPart *mc = 0;
    mc = static_cast<MaterialCalcPart*>(tpList.first());
    for( ; mc; mc = static_cast<MaterialCalcPart*>(tpList.next()) )
    {
        QDomElement calcPart = doc.createElement( "MaterialCalcpart" );
        calcParts.appendChild(calcPart);
        calcPart.appendChild(createDomNode(doc, "name", mc->getName()));
        calcPart.appendChild(createDomNode(doc, "dbid", mc->getDbID().toString()));

        StockMaterialList materials = mc->getCalcMaterialList();
        StockMaterialListIterator it( materials );

        StockMaterial *mat=0;
        while ( (mat = it.current()) != 0 )
        {
            ++it;
            QDomElement matElem = doc.createElement("Material");
            matElem.appendChild(createDomNode(doc, "MaterialName", mat->name()));
            QString h;
            h = h.setNum(mc->getCalcAmount(mat));
            matElem.appendChild(createDomNode(doc, "Amount", h));
            matElem.appendChild(createDomNode(doc, "Price",   mc->getCostsForMaterial(mat).toString()));
            Einheit e = mat->getUnit();
            h = e.einheitSingular();
            matElem.appendChild(createDomNode(doc, "Unit", h));
            calcPart.appendChild(matElem);
        }
    }
}

QDomElement FloskelTemplate::createDomNode( QDomDocument doc,
                                            const QString& name, const QString& value)
{
  QDomElement elem = doc.createElement(name);
  QDomText text = doc.createTextNode(value);
  elem.appendChild(text);
  return elem;
}
#endif

