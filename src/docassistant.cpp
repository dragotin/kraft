/***************************************************************************
                          docassistant.cpp  - Assistant widget
                             -------------------
    begin                : April 2007
    copyright            : (C) 2007 by Klaas Freitag
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
#include <QtGui>

#include <QToolTip>
#include <QTimer>
#include <QTreeWidgetItem>

#include <kiconloader.h>
#include <kpushbutton.h>
#include <kmessagebox.h>
#include <khbox.h>
#include <kvbox.h>
#include <khtml_part.h>
#include <khtmlview.h>

#include "docassistant.h"
#include "docpostcard.h"
#include "catalogselection.h"
#include "headerselection.h"
#include "textselection.h"
#include "kraftsettings.h"
#include "kataloglistview.h"
#include "doctext.h"
#include "defaultprovider.h"
#include "headertemplateprovider.h"
#include "footertemplateprovider.h"
#include "catalogtemplateprovider.h"
#include "addresstemplateprovider.h"
#include "addressselection.h"

DocAssistant::DocAssistant( QWidget *parent ):
  QSplitter( parent ), mFullPreview( true ),
  mActivePage( KraftDoc::Header )
{
  setOrientation( Qt::Vertical );

  QWidget *w = new QWidget;
  addWidget( w );
  QVBoxLayout *topVBox = new QVBoxLayout;
  w->setLayout( topVBox );

  QHBoxLayout *buttonLayout = new QHBoxLayout;
  topVBox->addLayout( buttonLayout );
  buttonLayout->setMargin( KDialog::marginHint()/2 );

  KPushButton *pb = new KPushButton( i18n( "Show Templates" ) );
  buttonLayout->addWidget( pb );
  connect( pb, SIGNAL( toggled( bool ) ),
           this, SLOT( slotToggleShowTemplates( bool ) ) );
  pb->setCheckable( true );
  pb->setToolTip( i18n( "Show mask to create or select templates to be used in the document" ) );

  buttonLayout->addStretch();

  mPostCard = new DocPostCard;

  mPostCard->slotSetMode( DocPostCard::Full, KraftDoc::Header );
  // setResizeMode( vb /* mPostCard->view() */, KeepSize );

  connect( mPostCard, SIGNAL( completed() ),
           this,  SLOT( slotRenderCompleted() ) );

  topVBox->addWidget( mPostCard->view() );

  // KVBox *stackVBox = new KVBox( this );
  mTemplatePane = new QWidget;
  QVBoxLayout *bottomVBox = new QVBoxLayout;
  mTemplatePane->setLayout( bottomVBox );
  addWidget( mTemplatePane );

  mWidgetStack = new QStackedWidget;

  bottomVBox->addWidget( mWidgetStack );
  mWidgetStack->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );

  /* Selections are the gui reprenentations of the template providing catalogs
   * like header- and footer texts and catalogs.
   */
  mCatalogSelection = new CatalogSelection;
  mWidgetStack->addWidget( mCatalogSelection );
  connect( mCatalogSelection, SIGNAL( selectionChanged(QTreeWidgetItem*,QTreeWidgetItem*) ),
           this,  SLOT( slotCatalogSelectionChanged(QTreeWidgetItem*,QTreeWidgetItem*) ) );

  mHeaderSelection = new HeaderSelection;
  mWidgetStack->addWidget( mHeaderSelection );

  connect( mHeaderSelection, SIGNAL( addressSelectionChanged() ),
           this, SLOT( slotAddressSelectionChanged() ) );
  connect( mHeaderSelection, SIGNAL(textSelectionChanged( QTreeWidgetItem* ) ),
           this, SLOT( slotTextsSelectionChanged( QTreeWidgetItem* ) ) );
  connect( mHeaderSelection->textSelection(), SIGNAL( actionCurrentTextToDoc() ),
           this,  SLOT( slotAddToDocument() ) );
  connect( mHeaderSelection, SIGNAL( doubleClickedOnItem() ),
           this, SLOT( slotAddToDocument() ) );

  mFooterSelection = new TextSelection( 0, KraftDoc::Footer );
  mWidgetStack->addWidget( mFooterSelection );

  connect( mFooterSelection, SIGNAL( textSelectionChanged( QTreeWidgetItem* ) ),
           this, SLOT( slotTextsSelectionChanged( QTreeWidgetItem* ) ) );
  connect( mFooterSelection, SIGNAL( actionCurrentTextToDoc() ),
           this,  SLOT( slotAddToDocument() ) );

  mWidgetStack->setCurrentWidget( mHeaderSelection );
  connect( mPostCard, SIGNAL( selectPage( int ) ),
           this,  SLOT( slotSelectDocPart( int ) ) );

  QHBoxLayout *butHBox2 = new QHBoxLayout;
  bottomVBox->addLayout( butHBox2 );

  butHBox2->setSpacing( KDialog::spacingHint() );
  KIcon icons = KIcon( "go-previous" ); // KDE 4 icon name: go-previous
  mPbAdd  = new KPushButton( icons, QString() );
  mPbAdd->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum );
  connect( mPbAdd, SIGNAL( clicked() ), this, SLOT( slotAddToDocument() ) );
  butHBox2->addWidget( mPbAdd );
  mPbAdd->setToolTip( i18n( "Add a template to the document" ) );

  icons = KIcon( "document-new" );
  mPbNew  = new KPushButton( icons, QString() ); // KDE 4 icon name: document-new
  mPbNew->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum );
  connect( mPbNew, SIGNAL( clicked() ), this, SLOT( slotNewTemplate() ) );
  mPbNew->setToolTip( i18n( "Create a new template (type depending)" ) );
  butHBox2->addWidget( mPbNew );

  icons = KIcon( "document-properties" );
  mPbEdit  = new KPushButton( icons, QString() ); // KDE 4 icon name: document-properties
  mPbEdit->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum );
  connect( mPbEdit, SIGNAL( clicked() ), this, SLOT( slotEditTemplate() ) );
  mPbEdit->setToolTip( i18n( "Edit the selected template (type depending)" ) );
  butHBox2->addWidget( mPbEdit );

  icons = KIcon( "edit-delete" );
  mPbDel  = new KPushButton( icons, QString() ); // KDE 4 icon name: edit-delete
  mPbDel->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum );
  connect( mPbDel, SIGNAL( clicked() ), this, SLOT( slotDeleteTemplate() ) );
  mPbDel->setToolTip( i18n( "Delete the selected template (type depending)" ) );
  butHBox2->addWidget( mPbDel );

  butHBox2->addStretch();

  mPbAdd->setEnabled( false );
  mPbEdit->setEnabled( false );
  mPbDel->setEnabled( false );

  /* Template Provider initialisations */
  mHeaderTemplateProvider = new HeaderTemplateProvider( parent );

  /* get a new header text from the default provider */
  connect( mHeaderTemplateProvider, SIGNAL( newHeaderText( const DocText& ) ),
           this,  SLOT( slotNewHeaderDocText( const DocText& ) ) );
  connect( mHeaderTemplateProvider, SIGNAL( updateHeaderText( const DocText& ) ),
           this,  SLOT( slotUpdateHeaderDocText( const DocText& ) ) );
  connect( mHeaderTemplateProvider, SIGNAL( headerTextToDocument( const DocText& ) ),
           this,  SLOT( slotHeaderTextToDocument( const DocText& ) ) );
  connect( mHeaderTemplateProvider, SIGNAL( deleteHeaderText( const DocText& ) ),
           this,  SLOT( slotHeaderTextDeleted( const DocText& ) ) );

  connect( mHeaderSelection, SIGNAL( switchedToHeaderTab( HeaderSelection::HeaderTabType ) ),
           this, SLOT( slSetHeaderTemplateProvider( HeaderSelection::HeaderTabType ) ) );

  mHeaderTemplateProvider->setSelection( mHeaderSelection->textSelection() );

  mFooterTemplateProvider = new FooterTemplateProvider( parent );

  /* get a new Footer text from the default provider */
  connect( mFooterTemplateProvider, SIGNAL( newFooterText( const DocText& ) ),
           this,  SLOT( slotNewFooterDocText( const DocText& ) ) );
  connect( mFooterTemplateProvider, SIGNAL( updateFooterText( const DocText& ) ),
           this,  SLOT( slotUpdateFooterDocText( const DocText& ) ) );
  connect( mFooterTemplateProvider, SIGNAL( footerTextToDocument( const DocText& ) ),
           this,  SLOT( slotFooterTextToDocument( const DocText& ) ) );
  connect( mFooterTemplateProvider, SIGNAL( deleteFooterText( const DocText& ) ),
           this,  SLOT( slotFooterTextDeleted( const DocText& ) ) );
  mFooterTemplateProvider->setSelection( mFooterSelection );

  /* Catalog Template Provider */
  mCatalogTemplateProvider = new CatalogTemplateProvider( parent );
  mCatalogTemplateProvider->setCatalogSelection( mCatalogSelection );
  connect( mCatalogTemplateProvider,  SIGNAL( positionSelected( Katalog*, void* ) ),
           this, SIGNAL( positionSelected( Katalog*, void* ) ) );

  mAddressTemplateProvider = new AddressTemplateProvider( parent );
  connect( mHeaderSelection->addressSelection(), SIGNAL( addressSelected( const Addressee& ) ),
           mAddressTemplateProvider, SLOT( slotSetCurrentAddress( const Addressee& ) ) );

  connect( mAddressTemplateProvider, SIGNAL( addressToDocument( const Addressee& ) ),
           this, SLOT( slotAddressToDocument( const Addressee& ) ) );

  mCurrTemplateProvider = mHeaderTemplateProvider;

  // mMainSplit->setSizes( KraftSettings::self()->self()->assistantSplitterSetting() );
  mTemplatePane->hide();
}

void DocAssistant::slotAddToDocument()
{
  kDebug() << "SlotAddToDocument called!" << endl;
  if ( mCurrTemplateProvider ) {
    mCurrTemplateProvider->slotTemplateToDocument();
  }
}

void DocAssistant::slotAddressSelectionChanged()
{
  kDebug() << "A address template was selected!" << endl;
  if ( mHeaderSelection->textSelection()->textsListView()->selectedItems().count() ) {
    mPbAdd->setEnabled( true );
    mPbEdit->setEnabled( true );
    mPbDel->setEnabled( false );

  }
}

void DocAssistant::slotTextsSelectionChanged( QTreeWidgetItem *item )
{
  if ( item ) {
    mPbAdd->setEnabled( true );
    mPbEdit->setEnabled( true );
    mPbDel->setEnabled( true );
  } else {
    mPbAdd->setEnabled( false );
    mPbEdit->setEnabled( false );
    mPbDel->setEnabled( false );
  }
}

void DocAssistant::slotCatalogSelectionChanged(QTreeWidgetItem *current ,QTreeWidgetItem *)
{
  // enable the move-to-document button.
  kDebug() << "catalog position selection changed!" << endl;
  if ( current ) {
    mPbAdd->setEnabled( true );
  } else {
    mPbAdd->setEnabled( false );
  }
}

void DocAssistant::slotNewTemplate()
{
  /* always set the doc type in case the provider benefits from that */
  mCurrTemplateProvider->slotSetDocType( mDocType );
  mCurrTemplateProvider->slotNewTemplate();
}

/* a new header doc text was created and should go to the document */
void DocAssistant::slotNewHeaderDocText( const DocText& dt )
{
  /* show in list of texts in the GUI */
  mHeaderSelection->textSelection()->addNewDocText( dt );
}

/* called with a changed text that needs to be updated in the view */
void DocAssistant::slotUpdateHeaderDocText( const DocText& dt )
{
  mHeaderSelection->textSelection()->updateDocText( dt );
}

/* the user hit "add to document" to use a header text template */
void DocAssistant::slotHeaderTextToDocument( const DocText& dt )
{
  emit headerTextTemplate( dt.text() );
}

/* a new header doc text was created and should go to the document */
void DocAssistant::slotNewFooterDocText( const DocText& dt )
{
  /* show in list of texts in the GUI */
  mFooterSelection->addNewDocText( dt );
}

/* called with a changed text that needs to be updated in the view */
void DocAssistant::slotUpdateFooterDocText( const DocText& dt )
{
  mFooterSelection->updateDocText( dt );
}

/* the user hit "add to document" to use a header text template */
void DocAssistant::slotFooterTextToDocument( const DocText& dt )
{
  emit footerTextTemplate( dt.text() );
}

void DocAssistant::slotAddressToDocument( const Addressee& adr )
{
  emit addressTemplate( adr );
}

/* Slot that initiates an edit */
void DocAssistant::slotEditTemplate()
{
  kDebug() << "Editing a template using the currentTemplProvider" << endl;
  if ( mCurrTemplateProvider ) {
    mCurrTemplateProvider->slotSetDocType( mDocType );
    mCurrTemplateProvider->slotEditTemplate();
  }
}

/* slot that initialises a delete, called from the delete button */
void DocAssistant::slotDeleteTemplate()
{
  if ( KMessageBox::warningYesNo( this, i18n( "Do you really want to delete the "
                                            "template permanently? There is no way "
                                              "to recover!" ) )
       == KMessageBox::No  )
  {
    return;
  }

  if ( mCurrTemplateProvider ) {
    mCurrTemplateProvider->slotDeleteTemplate();
  }
}

void DocAssistant::slSetHeaderTemplateProvider( HeaderSelection::HeaderTabType t )
{

  // go out here if it is not the header doc part, sometimes the tab widget
  // seems to throw the signal a bit unwanted what results in a current template
  // provider that points to header however we're on the footer page
  if ( mActivePage != KraftDoc::Header ) {
    return;
  }

  if ( t == HeaderSelection::AddressTab ) {
    mCurrTemplateProvider = mAddressTemplateProvider;
  } else if ( t == HeaderSelection::TextTab ) {
    mCurrTemplateProvider = mHeaderTemplateProvider;
  } else {
    kDebug() << "Unknown HeaderSelection type" << endl;
  }
}

void DocAssistant::slotHeaderTextDeleted( const DocText& /* dt */)
{
  mHeaderSelection->textSelection()->deleteCurrentText();
}

void DocAssistant::slotFooterTextDeleted( const DocText& /* dt */)
{
  mFooterSelection->deleteCurrentText();
}

/* slot that opens the template details in case on == true */
void DocAssistant::slotToggleShowTemplates( bool on )
{
  if ( on ) {
    // setFullPreview is set in the subslots called from here, that
    // makes mFullPreview truly reflecting the state of the toggle button
    if ( mActivePage == KraftDoc::Header ) {
      slotShowHeaderTemplates();
    } else if ( mActivePage == KraftDoc::Positions ) {
      slotShowCatalog();
    } else if ( mActivePage == KraftDoc::Footer ) {
      slotShowFooterTemplates();
    }
  } else {
    // hide the details
    setFullPreview( true, mActivePage );
  }
  emit toggleShowTemplates( on );
}

void DocAssistant::slotRenderCompleted()
{
  // kDebug() << "Render completed: " << mPostCard->view()->contentsHeight() << endl;

#if 0
  /* This is unfortunately not working because contentsHeight is always as
     heigh as the viewport is. Must be fixed in khtmlpart. */
  QList<int> sizes;
  sizes << mPostCard->view()->contentsHeight();
  setSizes( sizes );
#endif

}

DocPostCard *DocAssistant::postCard()
{
  return mPostCard;
}

CatalogSelection* DocAssistant::catalogSelection()
{
  return mCatalogSelection;
}

/* sets the Part of the doc, eg. Header, Footer */
void DocAssistant::slotSelectDocPart( int p )
{
  mActivePage = p;
  // change the currentTemplateProvider variable.
  mPbEdit->setEnabled( false );
  mPbDel->setEnabled( false );

  if ( p == KraftDoc::Header ) {
    if(mHeaderSelection->currentIndex() == 0)
      slSetHeaderTemplateProvider( HeaderSelection::AddressTab );
    else
      slSetHeaderTemplateProvider( HeaderSelection::TextTab );

    mPbNew->setEnabled( true );
    if ( mHeaderSelection->itemSelected() ) {
      kDebug() << "Enabling Edit and Del for Header" << endl;
      mPbEdit->setEnabled( true );
      mPbDel->setEnabled( true );
    }

  } else if ( p == KraftDoc::Positions ) {
    mCurrTemplateProvider = mCatalogTemplateProvider;
    mPbNew->setEnabled( false );
  } else if ( p == KraftDoc::Footer ) {
    mCurrTemplateProvider = mFooterTemplateProvider;
    mPbNew->setEnabled( true );

    if ( mFooterSelection->textsListView()->selectedItems().count() ) {
      mPbEdit->setEnabled( true );
      mPbDel->setEnabled( true );
    }
  }

  emit selectPage( p );
  slotToggleShowTemplates( !mFullPreview );
}

/* Doc Type like offer, invoice etc. */
void DocAssistant::slotSetDocType( const QString& type )
{
  mDocType = type;
  mHeaderSelection->slotSelectDocType( type );
  mFooterSelection->slotSelectDocType( type );

  // nothing is selected.
  bool selector = false;
  if ( mActivePage == KraftDoc::Header && mHeaderSelection->itemSelected() ) {
    selector = true;
  }
  if ( mActivePage == KraftDoc::Footer && mFooterSelection->textsListView()->selectedItems().count() ) {
    selector = true;
  }

  mPbAdd->setEnabled( true );
  mPbEdit->setEnabled( selector );
  mPbDel->setEnabled( selector );
}

void DocAssistant::slotShowCatalog( )
{
  setFullPreview( false, KraftDoc::Positions );
  mWidgetStack->setCurrentWidget( mCatalogSelection );
}

void DocAssistant::slotShowHeaderTemplates()
{
  setFullPreview( false, KraftDoc::Header );
  mWidgetStack->setCurrentWidget( mHeaderSelection );
}

void DocAssistant::slotShowFooterTemplates()
{
  setFullPreview( false, KraftDoc::Footer );
  mWidgetStack->setCurrentWidget( mFooterSelection );
}

void DocAssistant::setFullPreview( bool setFull, int id )
{
  if ( setFull ) {
    /* remember the sizes used before */
    if ( mTemplatePane->isVisible() ) {
      // kDebug() << "Writing mSplitterSizes: " << mMainSplit->sizes() << endl;
      // KraftSettings::self()->self()->setAssistantSplitterSetting( mMainSplit->sizes() );
    //  KraftSettings::self()->self()->writeConfig();
    }

    mTemplatePane->hide();
    mPostCard->slotSetMode( DocPostCard::Full, id );
    mFullPreview = true;
  } else {
    mTemplatePane->show();
    mPostCard->slotSetMode( DocPostCard::Mini, id );

    if ( KraftSettings::self()->self()->assistantSplitterSetting().size() == 2 ) {
      // mMainSplit->setSizes( KraftSettings::self()->self()->assistantSplitterSetting() );
    }
    mFullPreview = false;
  }
}

#include "docassistant.moc"
