/***************************************************************************
                          templkatalogview.cpp
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

// include files for QT
#include <qdir.h>
#include <qprinter.h>
#include <qpainter.h>
#include <qlayout.h>
// include files for KDE
#include <kiconloader.h>
#include <kmessagebox.h>
#include <kfiledialog.h>
#include <kmenubar.h>
#include <kstatusbar.h>
#include <klocale.h>
#include <kconfig.h>
#include <kstdaction.h>
#include <kdebug.h>

// application specific includes
#include "katalogview.h"
#include "templkatalogview.h"
#include "floskeltemplate.h"
#include "kataloglistview.h"
#include "flostempldialog.h"
#include "templkatalog.h"
#include "templkataloglistview.h"
#include "katalogman.h"
#include "filterheader.h"
#include "documentman.h"
#
#define ID_STATUS_MSG 1

TemplKatalogView::TemplKatalogView(QWidget* parent, const char* name)
    : KatalogView(parent, name),
    m_flosDialog(0)
{
}

TemplKatalogView::~TemplKatalogView()
{

}

Katalog* TemplKatalogView::getKatalog( const QString& name )
{
    Katalog *k = KatalogMan::getKatalog( name );
    if( ! k ) {
        k = new TemplKatalog( name );
        KatalogMan::registerKatalog( k );
    }
    return k;
}

/////////////////////////////////////////////////////////////////////
// SLOT IMPLEMENTATION
/////////////////////////////////////////////////////////////////////


void TemplKatalogView::slEditVorlage()
{
    TemplKatalogListView* listview = static_cast<TemplKatalogListView*>(getListView());
    
    if( listview )
    {
        FloskelTemplate *currTempl = static_cast<FloskelTemplate*> (listview->currentItemData());
        if( currTempl ) {
            KListViewItem *item = (KListViewItem*) listview->currentItem();
            openDialog(item, currTempl);
        }
    }
}

void TemplKatalogView::slNeueVorlage()
{
    KatalogListView *listview = getListView();
    if( !listview ) return;
    TemplKatalogListView *templListView = static_cast<TemplKatalogListView*>(listview);

    // Anlegen eines neuen Templates
    FloskelTemplate *flosTempl = new FloskelTemplate();

    // Eltern = Katalogitem rausfinden
    KListViewItem *parentItem = static_cast<KListViewItem*>(listview->currentItem());
    if( parentItem )
    {
        // Wenn es kein chapter ist, nehmen wir den Parent
        if( ! (templListView->isRoot(parentItem) || templListView->isChapter(parentItem)) )
        {
            parentItem = (KListViewItem*) parentItem->parent();
        }
    }

    if( parentItem ) {
      // try to find out which catalog is open/actual
      QString name = parentItem->text(0);
      Katalog *k = KatalogMan::getKatalog(m_katalogName);
      if( k ) {
        kdDebug() << "setting catalog name " << name << endl;
        flosTempl->setChapterID(k->chapterID(name));
      }
    }
    
    KListViewItem *newItem = templListView->addFlosTemplate(parentItem, flosTempl);
    openDialog(newItem, flosTempl);
}

bool TemplKatalogView::currentItemToDocPosition( DocPosition& pos )
{
  TemplKatalogListView* listview = static_cast<TemplKatalogListView*>(getListView());
  bool res = false;
  
  if( listview )
  {
    FloskelTemplate *currTempl = static_cast<FloskelTemplate*> (listview->currentItemData());
    if( currTempl ) {
      // create a new position and offer it to the document manager
      pos.setText( currTempl->getText() );
      pos.setUnit( currTempl->einheit() );
      pos.setUnitPrice( currTempl->einheitsPreis() );
      pos.setAmount( 1.0 );
      res = true;
    }
  }
  return res;
}

void TemplKatalogView::slChangeChapter(int newID)
{
    KatalogListView *listview = getListView();
    if( !listview ) return;
    TemplKatalogListView *templListView = static_cast<TemplKatalogListView*>(listview);

    if( m_editListViewItem ) {
        templListView->slChangeChapter( m_editListViewItem, newID);
    }
}

void TemplKatalogView::openDialog( KListViewItem *listitem, FloskelTemplate *tmpl )
{
    if( ! m_flosDialog )
    {
        m_flosDialog = new FlosTemplDialog(this, "VORLAGE_EDIT", false);
        connect( m_flosDialog, SIGNAL(editAccepted( FloskelTemplate* )),
                 this, SLOT( slEditOk(FloskelTemplate*)));
        connect( m_flosDialog, SIGNAL(chapterChanged(int)),
                 this, SLOT(slChangeChapter(int)));
    }
    m_flosDialog->setVorlage(tmpl,m_katalogName);
    m_editListViewItem = listitem;
    m_flosDialog->refreshPrices();
    m_flosDialog->show();
}

void TemplKatalogView::slEditOk(FloskelTemplate* templ)
{
    KatalogListView *listview = getListView();
    if( !listview ) return;
    TemplKatalogListView *templListView = static_cast<TemplKatalogListView*>(listview);

    if(m_flosDialog )
    {

    }
    if( m_editListViewItem ) {
        kdDebug() << "Edit was ok, refreshing item in list " << templ << endl;
        m_editListViewItem->setSelected(true);
      templListView->slFreshupItem( m_editListViewItem, templ, true );
    }
    m_editListViewItem = 0;
}


void TemplKatalogView::slListviewExecuted(QListViewItem* qItem)
{
    KatalogListView *listview = getListView();
    if( !listview ) return;
    TemplKatalogListView *templListView = static_cast<TemplKatalogListView*>(listview);

    if( ! qItem ) return;
    KListViewItem *item = static_cast<KListViewItem*>(qItem);

    bool itemEdit = true;

    if( templListView->isRoot(item) ) {
        // we have the root item, not editable
        itemEdit = false;
    } else if( templListView->isChapter(item) ) {
        itemEdit = false;
    }
    m_acEditItem->setEnabled(itemEdit);
}

void TemplKatalogView::createCentralWidget(QBoxLayout*box, QWidget *w)
{
    kdDebug() << "Creating new Listview" << endl;
    m_listview = new TemplKatalogListView(w);
    box->addWidget(m_listview);
}

#include "templkatalogview.moc"
