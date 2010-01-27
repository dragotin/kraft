//
// C++ Implementation: catalogchapteredit
//
// Description:
//
//
// Author: Klaas Freitag <freitag@kde.org>, (C) 2005
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include <QStringList>
#include <QListView>
#include <QPushButton>

#include <klocale.h>
#include <kdialog.h>
#include <kdebug.h>
#include <klineedit.h>



#include "catalogchapteredit.h"
#include "katalog.h"
#include "katalogman.h"


CatalogChapterEdit::CatalogChapterEdit(QWidget *parent)
    : KEditListBox(i18n("Edit Catalog Chapters"), parent)
{
}


CatalogChapterEdit::~CatalogChapterEdit()
{
}

//
// This is another class: The dialog
//
CatalogChapterEditDialog::CatalogChapterEditDialog(QWidget *parent,
                                                  const QString& katName)
    : KDialog(parent),
    m_katalog(0),
    mDirty( false )
{
    setCaption(i18n("Edit Catalog Chapters" ));
    m_chapEdit = new CatalogChapterEdit(this);
    connect( m_chapEdit->listView(), SIGNAL( activated(QModelIndex) ),
               this, SLOT( slotSelectionChanged(QModelIndex) ) );
    connect( m_chapEdit, SIGNAL( added( const QString& ) ),
               this, SLOT( slotAdded( const QString& ) ) );
    connect( m_chapEdit, SIGNAL( removed( const QString& ) ),
               this, SLOT( slotRemoved( const QString& ) ) );
    connect( m_chapEdit, SIGNAL( changed() ),
               this, SLOT( slotTextChanged() ) );

    setMainWidget( m_chapEdit );

    m_katalog = KatalogMan::self()->getKatalog( katName );
    if( ! m_katalog ) return;

    const QStringList li = m_katalog->getKatalogChapters();

    for ( QStringList::ConstIterator it = li.begin(); it != li.end(); ++it ) {
        QString entry = *it;
        m_chapEdit->insertItem( entry );
        int id = m_katalog->chapterID( entry );
        mEntryDict.insert( entry, dbID( id ) );
    }
}

void CatalogChapterEditDialog::accept()
{

    // First delete entries from the 'to remove' chain.
    QStringList::ConstIterator strIt;

    for( strIt = m_removedItems.begin(); strIt != m_removedItems.end(); ++strIt ) {
      m_katalog->removeChapter( *strIt );
      mDirty = true;
    }

    dbIdDictIterator it( mEntryDict );
    while( it.hasNext() ) {
      it.next();
      /* if the dbID of the dict entry is not ok it means that the entry was
       * added in the dialog and needs to go into the database.
       * If the id is ok, we need to compare if the entry that the catalog
       * knows is different from the entry we have from the dialog. If it
       * differs, it was changed and must be updated.
      */

      if( ! it.value().isOk() ) {
          kDebug() << it.key() << " is new and must be added" << endl;
          m_katalog->addChapter( it.key(), 1 );
      } else {
        QString stored = m_katalog->chapterName( it.value() );
        QString curr = it.key();
        kDebug() << "Comparing edited <" << curr << "> with stored <" << stored << ">" << endl;
        if( curr != stored ) {
          kDebug() << "Renaming " << stored << " to " << curr << endl;
          m_katalog->renameChapter( stored, curr );
          mDirty = true;
        }
      }
    }

    QStringList newChapList = m_chapEdit->items();
    int pos = 1;
    int testie = 0;
    for ( strIt = newChapList.begin(); strIt != newChapList.end(); ++strIt ) {
      const QString current = *strIt;
      kDebug() << "Setting entry " << current << " to sortkey " << pos << endl;
      if( pos != m_katalog->chapterSortKey( current ) ) {
        testie = m_katalog->chapterSortKey(current);
        m_katalog->setChapterSortKey( current, pos );
        mDirty = true;
      }
      pos++;
    }

    KDialog::accept();
}

void CatalogChapterEditDialog::slotAdded( const QString& item )
{
    m_newItems << item;
    kDebug() << "adding item " << item << endl;
    dbID ndb;
    mEntryDict.insert(item, ndb); //
}

void CatalogChapterEditDialog::slotRemoved( const QString& item )
{
    m_removedItems << item;
    kDebug() << "Removing item " << item << endl;
    if( mEntryDict.contains(item) ) {
      mEntryDict.remove( item );
    } else {
      kDebug() << "Can not remove item " << item << " from dict" << endl;
    }
}

void CatalogChapterEditDialog::slotTextChanged()
{
    kDebug() << "Text changed" << endl;

    if( m_chapEdit->currentItem() >= 0 ) {
      QString current = mLastSelection;
      QString newCurrent = m_chapEdit->lineEdit()->text();
      kDebug() << "Current ="<< current << " and new=" << newCurrent << endl;
      if( current != newCurrent ) {
        dbID id = mEntryDict[current];

        if( mEntryDict.contains(current) && !(mEntryDict.contains( newCurrent)) ) {
          mEntryDict.insert( newCurrent, id );
          mEntryDict.remove( current );
        }
        mLastSelection = newCurrent;
      }
    }
}

void CatalogChapterEditDialog::slotSelectionChanged(const QModelIndex & /* index */)
{
    QString current = m_chapEdit->currentText();
    mLastSelection = current;

    int cnt = m_katalog->getEntriesPerChapter( current );

    bool mayRemove = false;
    if( cnt == 0 ) {
        // can remove the chapter because it is empty
        mayRemove = true;
    }

    m_chapEdit->removeButton()->setEnabled( mayRemove );
}

