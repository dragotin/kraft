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
#include <qpushbutton.h>
#include <klocale.h>
#include <kdialogbase.h>
#include <kdebug.h>
#include <klineedit.h>

#include <qstringlist.h>

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
    : KDialogBase(parent),
    m_katalog(0),
    mDirty( false )
{
    setCaption(i18n("Edit Catalog Chapters" ));
    m_chapEdit = new CatalogChapterEdit(this);
    connect( m_chapEdit->listBox(), SIGNAL( selectionChanged() ), 
               this, SLOT( slotSelectionChanged() ) );
    connect( m_chapEdit, SIGNAL( added( const QString& ) ), 
               this, SLOT( slotAdded( const QString& ) ) );
    connect( m_chapEdit, SIGNAL( removed( const QString& ) ), 
               this, SLOT( slotRemoved( const QString& ) ) );
    connect( m_chapEdit, SIGNAL( changed() ), 
               this, SLOT( slotTextChanged() ) );

    setMainWidget( m_chapEdit );

    m_katalog = KatalogMan::getKatalog( katName );
    if( ! m_katalog ) return;

    const QStringList li = m_katalog->getKatalogChapters();

    for ( QStringList::ConstIterator it = li.begin(); it != li.end(); ++it ) {
        QString entry = *it;
        m_chapEdit->insertItem( entry );
        int id = m_katalog->chapterID( entry );
        mEntryDict.insert( entry, new dbID( id ) );
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
    
    QDictIterator<dbID> it( mEntryDict );
    for( ; it.current(); ++it ) {
      /* if the dbID of the dict entry is not ok it means that the entry was
       * added in the dialog and needs to go into the database.
       * If the id is ok, we need to compare if the entry that the catalog
       * knows is different from the entry we have from the dialog. If it
       * differs, it was changed and must be updated.
      */

      if( ! it.current()->isOk() ) {
          kdDebug() << it.currentKey() << " is new and must be added" << endl;
          m_katalog->addChapter( it.currentKey(), 1 );
      } else {
        QString stored = m_katalog->chapterName( *(it.current()) );
        QString curr = it.currentKey();
        kdDebug() << "Comparing edited <" << curr << "> with stored <" << stored << ">" << endl;
        if( curr != stored ) {
          kdDebug() << "Renaming " << stored << " to " << curr << endl;
          m_katalog->renameChapter( stored, curr );
          mDirty = true;
        }
      }
    }

    QStringList newChapList = m_chapEdit->items();
    int pos = 1;
    for ( strIt = newChapList.begin(); strIt != newChapList.end(); ++strIt ) {
      const QString current = *strIt;
      kdDebug() << "Setting entry " << current << " to sortkey " << pos << endl;
      if( pos != m_katalog->chapterSortKey( current ) ) {
        m_katalog->setChapterSortKey( current, pos );
        mDirty = true;
      }
      pos++;
    }

    KDialogBase::accept();
}

void CatalogChapterEditDialog::slotAdded( const QString& item )
{
    m_newItems << item;
    kdDebug() << "adding item " << item << endl;
    dbID *ndb = new dbID();
    mEntryDict.insert(item, ndb); // 
}

void CatalogChapterEditDialog::slotRemoved( const QString& item )
{
    m_removedItems << item;
    kdDebug() << "Removing item " << item << endl;
    if( mEntryDict[item] ) {
      mEntryDict.remove( item );
    } else {
      kdDebug() << "Can not remove item " << item << " from dict" << endl;
    }
}

void CatalogChapterEditDialog::slotTextChanged()
{
    kdDebug() << "Text changed" << endl;

    if( m_chapEdit->currentItem() >= 0 ) {
      QString current = mLastSelection;
      QString newCurrent = m_chapEdit->lineEdit()->text();
      kdDebug() << "Current ="<< current << " and new=" << newCurrent << endl;
      if( current != newCurrent ) {
        dbID *id = mEntryDict[current];
        dbID *newID = mEntryDict[newCurrent];
        if( id && !newID ) {
          mEntryDict.insert( newCurrent, id );
          mEntryDict.remove( current );
        }
        mLastSelection = newCurrent;
      }
    }
}

void CatalogChapterEditDialog::slotSelectionChanged()
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

    // FIXME: Better disable remove button instead of hiding it, but the KEditListBox
    //        controlls the buttons itself
    if( mayRemove ) 
        m_chapEdit->setButtons( KEditListBox::All );
    else
        m_chapEdit->setButtons( KEditListBox::Add | KEditListBox::UpDown );

}
#include "catalogchapteredit.moc"
