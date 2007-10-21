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
#include <qdom.h>
#include <klocale.h>
#include <kdebug.h>
#include <qstring.h>

#include "templatesaverbase.h"
#include "templatesaverdb.h"
#include "floskeltemplate.h"
#include "unitmanager.h"
#include "kraftdb.h"
#include "calcpart.h"
#include "materialcalcpart.h"
#include "fixcalcpart.h"
#include "zeitcalcpart.h"
#include "stockmaterial.h"

FloskelTemplate::FloskelTemplate()
    : CatalogTemplate(),
      m_einheitID(0),
      m_templID(-1),
      m_chapter(0),
      m_gewinn(0),
      m_zeitbeitrag(true),
      m_calcType(Calculation),
      m_listViewItem(0),
      m_saver(0)
{
    m_calcType = Calculation;
    m_calcParts.setAutoDelete(true);
}

FloskelTemplate::FloskelTemplate( int tID, const QString& text,
                                  int einheit, int chapter, int calcKind,
                                  const QDateTime& modDate,
                                  const QDateTime& createDate )
 : CatalogTemplate(),
   m_text(text),
   m_einheitID(einheit),
   m_templID(tID),
   m_chapter(chapter),
   m_gewinn(0),
   m_modifyDate(modDate),
   m_createDate(createDate),
   m_calcType(Unknown),
   m_preis(long(0)),
   m_listViewItem(0),
   m_saver(0)
{
    if( calcKind == 1 )
    {
        m_calcType = ManualPrice;
    }
    else if( calcKind == 2 )
    {
        m_calcType = Calculation;
    }

    m_calcParts.setAutoDelete(true);
}

FloskelTemplate::FloskelTemplate( FloskelTemplate& templ )
    : CatalogTemplate(),
      m_text( templ.m_text ),
      m_einheitID( templ.m_einheitID ),
      m_templID( templ.m_templID ),
      m_chapter( templ.m_chapter ),
      m_modifyDate( templ.m_modifyDate ),
      m_createDate( templ.m_createDate ),
      m_preis( templ.m_preis ),
      m_listViewItem(templ.m_listViewItem ),
      m_saver( 0 )
{
  deepCopyCalcParts( templ );
  m_calcParts.setAutoDelete(true);
}

FloskelTemplate& FloskelTemplate::operator= ( FloskelTemplate& src )
{
  if ( this == &src ) return *this;

  m_text = src.m_text;
  m_einheitID = src.m_einheitID;
  m_templID = src.m_templID;
  m_chapter = src.m_chapter;
  m_modifyDate = src.m_modifyDate;
  m_createDate = src.m_createDate;
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
  CalcPart *cp = templ.m_calcParts.first();
  CalcPart *ncp = 0;

  m_calcParts.clear();

  for( ; cp; cp = templ.m_calcParts.next() )
  {
    if( cp->getType() == KALKPART_TIME ) {
      ncp = new ZeitCalcPart( *( static_cast<ZeitCalcPart*>(cp) ) );
    } else if( cp->getType() == KALKPART_FIX ) {
      ncp = new FixCalcPart( *( static_cast<FixCalcPart*>(cp) ) );
    } else if( cp->getType() == KALKPART_MATERIAL ) {
      ncp = new MaterialCalcPart( *( static_cast<MaterialCalcPart*>(cp) ) );
    } else {
      kdDebug() << "ERROR: Unbekannter Kalkulations-Anteil-Typ!" << endl;
    }
    m_calcParts.append( ncp );
  }
}

Einheit FloskelTemplate::einheit() const
{
    return UnitManager::getUnit( m_einheitID );
}

void FloskelTemplate::setEinheitId(int id)
{
    m_einheitID = id;
}

void FloskelTemplate::setGewinn( double g )
{
    m_gewinn = g;
    CalcPart *cp;
    /*  jede teilkalkulation hat einen eigenen Gewinn fuer spaetere Erweiterung.
     *  Hier werden alle mit dem reinkommenden Wert beschrieben, spaeter kann
     *  jeder einzeln einen Wert haben...
     */
    for( cp = m_calcParts.first(); cp; cp = m_calcParts.next() )
    {
        cp->setProzentPlus(g);
    }
}

double FloskelTemplate::getGewinn( )
{
  return m_gewinn;
}

void FloskelTemplate::setTemplID( int newID )
{
  m_templID = newID;
}

void FloskelTemplate::setChapterID(int id)
{
  // FIXME: ggf. Umh�gen im Feature-listview
  m_chapter = id;
}

/** der Preis pro einer Einheit */
Geld FloskelTemplate::einheitsPreis()
{
  return calcPreis();
}

CalculationType FloskelTemplate::calcKind()
{
  return m_calcType;
}

void FloskelTemplate::setCalculationType( CalculationType t )
{
  m_calcType = t;
}

QString FloskelTemplate::calcKindString() const
{
  if( m_calcType == ManualPrice )
    return i18n("Manual Price");
  else if( m_calcType == Calculation )
    return i18n("Calculated");
  else return i18n( "Err: Unknown type %d").arg(m_calcType);
}

/** No descriptions */
Geld FloskelTemplate::calcPreis()
{
  Geld g;

    if( calcKind() == ManualPrice )
    {
        g = m_preis;
    }
    else
    {
        g = m_calcParts.calcPrice();
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
  if( cpart) {// m_calcParts.removeRef(cpart);
    cpart->setToDelete(true);
    cpart->setDirty(true);
  }
}

Geld FloskelTemplate::kostenPerKalcPart( const QString& part )
{
  return m_calcParts.costPerCalcPart( part );
}

TemplateSaverBase* FloskelTemplate::getSaver()
{
  /* Hier k�nten andere Save-Engines ausgew�lt werden */
  if( ! m_saver )
  {
    kdDebug() << "Erzeuge neuen DB-Saver" << endl;
    m_saver = new TemplateSaverDB();
  }
  return m_saver;
}

bool FloskelTemplate::save()
{
    TemplateSaverBase *saver = getSaver();
    kdDebug() << "Saver is " << saver << endl;
    if( saver ) {
        return saver->saveTemplate( this );
    } else {
        kdDebug() << "ERR: No saver available!" << endl;
        return false;
    }
}


QDomElement FloskelTemplate::toXML( QDomDocument& doc)
{
    QDomElement templ = doc.createElement("template");

    templ.appendChild( createDomNode(doc, "unit",   UnitManager::getUnit(m_einheitID).einheitSingular()));
    templ.appendChild( createDomNode(doc, "text", getText()));
    templ.appendChild( createDomNode(doc, "id", QString::number(getTemplID())));
    templ.appendChild( createDomNode(doc, "benefit", QString::number(getGewinn())));
    templ.appendChild( createDomNode(doc, "timecount", hasTimeslice() ? "yes": "no" ));

    QDomElement calcParts = doc.createElement( "calcParts" );
    templ.appendChild(calcParts);
    fixPartsToXML(doc, calcParts);
    timePartsToXML(doc, calcParts);
    materialPartsToXML(doc, calcParts);

#if 0
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
#endif

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
        calcPart.appendChild(createDomNode(doc, "price", g.toString()));
    }
}

void FloskelTemplate::timePartsToXML( QDomDocument& doc, QDomElement& calcParts )
{
    CalcPartList tpList = getCalcPartsList(KALKPART_TIME);

    ZeitCalcPart *tc = 0;
    tc = static_cast<ZeitCalcPart*>(tpList.first());
    for( ; tc; tc = static_cast<ZeitCalcPart*>(tpList.next()) ) {
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


