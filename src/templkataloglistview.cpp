
/***************************************************************************
             templkataloglistview  - template katalog listview.
                             -------------------
    begin                : 2005-07-09
    copyright            : (C) 2005 by Klaas Freitag
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

#include <QDebug>
#include <QMenu>
#include <QHeaderView>
#include <QByteArray>

#include <klocalizedstring.h>

#include "templkataloglistview.h"
#include "portal.h"
#include "kraftglobals.h"
#include "katalog.h"
#include "katalogman.h"
#include "kataloglistview.h"
#include "materialcalcpart.h"
#include "stockmaterial.h"
#include "templkatalog.h"
#include "timecalcpart.h"
#include "docposition.h"
#include "defaultprovider.h"
#include "kraftsettings.h"

TemplKatalogListView::TemplKatalogListView(QWidget *w)
    : KatalogListView(w),
      mShowCalcParts( true )
{
    QStringList labels;
    labels << i18n("Template");
    labels << i18n("Price");
    labels << i18n("Calc. Type");

    setHeaderLabels(labels);

    QByteArray headerState = QByteArray::fromBase64( KraftSettings::self()->templateCatViewHeader().toLatin1() );
    header()->restoreState(headerState);

    contextMenu()->setTitle( i18n("Template Catalog"));
}

/*
 * This class adds a complete catalog and fills the view. It gets the
 * catalog from KatalogMan, iterates over the catalog chapters and
 * fills in the templates.
 */
void TemplKatalogListView::addCatalogDisplay( const QString& katName )
{
    KatalogListView::addCatalogDisplay(katName);

    TemplKatalog* catalog = static_cast<TemplKatalog*>(KatalogMan::self()->getKatalog(katName));

    if ( !catalog ) {
        qCritical() << "Could not load catalog " << katName;
        return;
    }

    setupChapters();

    const QList<CatalogChapter> chapters = catalog->getKatalogChapters();
    foreach( CatalogChapter chap, chapters ) {
        if( mChapterDict.contains( chap.id().toInt() ) ) {
            int chapId = chap.id().toInt();
            QTreeWidgetItem *katItem = mChapterDict[chapId];
            FloskelTemplateList katList = catalog->getFlosTemplates(chapId);
            FloskelTemplateListIterator flosIt( katList );

            while( flosIt.hasNext() ) {
                FloskelTemplate *tmpl = flosIt.next();

                /* create a ew item as the child of katalog entry */
                addFlosTemplate( katItem, tmpl );
                if ( mShowCalcParts )
                    addCalcParts( tmpl );
            }
        }
    }
    // ... and all what is zero is going to the top level
    FloskelTemplateList katList = catalog->getFlosTemplates(0);
    for( FloskelTemplate *tmpl : katList) {
        addFlosTemplate(nullptr, tmpl);
        if ( mShowCalcParts )
            addCalcParts( tmpl );
    }
}
/*
 * add a single template to the view with setting icon etc.
 */
QTreeWidgetItem* TemplKatalogListView::addFlosTemplate( QTreeWidgetItem *parentItem, FloskelTemplate *tmpl )
{
    if( ! parentItem ) parentItem = m_root;
    QTreeWidgetItem *listItem = new QTreeWidgetItem( parentItem );
    slFreshupItem( listItem, tmpl);
    tmpl->setListViewItem( listItem );

    if( tmpl->calcKind() == CatalogTemplate::ManualPrice )
    {
        listItem->setIcon(0, DefaultProvider::self()->icon( "dice" ) );
    }
    else
    {
        listItem->setIcon(0, DefaultProvider::self()->icon("calculator"));
    }

    if ( mCheckboxes ) {
        listItem->setCheckState(0, Qt::Unchecked);
    }

    // store the connection between the listviewitem and the template in a dict.
    m_dataDict.insert( listItem, tmpl );

    return listItem;
}

void TemplKatalogListView::slFreshupItem( QTreeWidgetItem *item, FloskelTemplate *tmpl, bool remChildren )
{
    if( !(item && tmpl) ) return;

    Geld g     = tmpl->unitPrice();
    const QString ck = tmpl->calcKindString();
    const QString t  = Portal::textWrap(tmpl->getText(), 72, 4);

    item->setText( 0, t );
    if( t.endsWith(QStringLiteral("…"))) {
        item->setToolTip(0, Portal::textWrap(tmpl->getText(), 72, 22));
    }
    QString h;
    h = QString( "%1 / %2" ).arg( g.toLocaleString() )
            .arg( tmpl->unit().einheitSingular() );
    item->setText( 1,  h );
    item->setText( 2, ck );
    // item->setText( 4, QString::number(tmpl->getTemplID()));

    if( remChildren ) {
        /* remove all children and insert them again afterwards.
        * That updates the view
      */
        for( int i = 0; i < item->childCount(); i++ ) {
            QTreeWidgetItem *it = item->child(i);
            if( it )  {
                item->removeChild( it );
                delete it;
            }
        }

        addCalcParts(tmpl); // Insert to update the view again.
    }
}


void TemplKatalogListView::addCalcParts( FloskelTemplate *tmpl )
{
    QTreeWidgetItem *item = tmpl->getListViewItem();

    if( ! item ) return;

    CalcPartList parts = tmpl->getCalcPartsList();
    CalcPartListIterator it(parts);

    while( it.hasNext() ) {
        CalcPart *cp = it.next();
        QString title = cp->getName();
        QString type = cp->getType();
        // qDebug () << "Type is " << type;
        if( type  == KALKPART_TIME ) {
            TimeCalcPart *zcp = static_cast<TimeCalcPart*>(cp);
            StdSatz stdsatz = zcp->getStundensatz();
            title = QString( "%1, %2 %3 %4" )
                    .arg( cp->getName() )
                    .arg( QString::number(zcp->duration()))
                    .arg( TimeCalcPart::timeUnitString(zcp->timeUnit()))
                    .arg( stdsatz.getName() );
        }

        QStringList list;
        list << title;
        list << cp->basisKosten().toLocaleString();
        list << cp->getType();
        QTreeWidgetItem *cpItem =  new QTreeWidgetItem( item, list );
        cpItem->setDisabled(true);
    }
}

void TemplKatalogListView::setShowCalcParts( bool on )
{
    mShowCalcParts = on;
}

bool TemplKatalogListView::showCalcParts()
{
    return mShowCalcParts;
}

TemplKatalogListView::~TemplKatalogListView()
{
}

DocPosition TemplKatalogListView::itemToDocPosition( QTreeWidgetItem *it )
{
    DocPosition pos;
    if ( ! it ) {
        it = currentItem();
    }

    if ( ! it ) return pos;

    FloskelTemplate *flos = static_cast<FloskelTemplate*>( m_dataDict[ it ] );

    if ( flos ) {
        pos.setText( flos->getText() );
        pos.setUnit( flos->unit() );
        pos.setUnitPrice( flos->unitPrice() );
    } else {
        // qDebug () << "Can not find a template for the item";
    }

    return pos;
}

CalcPartList TemplKatalogListView::itemsCalcParts( QTreeWidgetItem* it )
{
    CalcPartList cpList;

    if ( ! it ) {
        it = currentItem();
    }

    if ( ! it ) return cpList;

    FloskelTemplate *flos = static_cast<FloskelTemplate*>( m_dataDict[ it ] );
    if ( flos ) {
        // qDebug () << "We have calc parts: " << flos->getCalcPartsList().count();
        cpList = flos->getCalcPartsList();
    }
    return cpList;
}

// Updates the sequence of items below a parent item stored in the inherited
// variable mSortChapterItem

void TemplKatalogListView::startUpdateItemSequence()
{
    Q_ASSERT(_query == nullptr);

    _query = new QSqlQuery;
    _query->prepare("UPDATE Catalog SET sortKey=? WHERE TemplID=?");
}

void TemplKatalogListView::updateItemSequence(QTreeWidgetItem *item, int seqNo)
{
    FloskelTemplate *flos = static_cast<FloskelTemplate*>( itemData(item) );
    // qDebug () << "Updating item " << flos->getTemplID() << " to sort key " << sequenceCnt;
    if( _query && flos ) {
        _query->bindValue( 0, seqNo );
        _query->bindValue( 1, flos->getTemplID() );
        _query->exec();
    }
}

void TemplKatalogListView::saveState()
{
    const QByteArray state = this->header()->saveState();

    KraftSettings::self()->setTemplateCatViewHeader(state.toBase64());
    KraftSettings::self()->save();
}
