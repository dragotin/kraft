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
#include <qwidgetstack.h>
#include <qtooltip.h>
#include <qasciidict.h>
#include <qtimer.h>
#include <kiconloader.h>
#include <kpushbutton.h>
#include <kmessagebox.h>

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
  setOrientation( Vertical );
  QVBox *vb = new QVBox( this );

  QHBox *hb = new QHBox( vb );
  hb->setFrameStyle( Box + Sunken );
  hb->setMargin( KDialog::marginHint()/2 );

  KPushButton *pb = new KPushButton( i18n( "show Templates" ),  hb );
  connect( pb, SIGNAL( toggled( bool ) ),
           this, SLOT( slotToggleShowTemplates( bool ) ) );
  pb->setToggleButton( true );
  QToolTip::add( pb, i18n( "Show mask to create or select templates to be used in the document" ) );

  QWidget *w = new QWidget( hb );
  w->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Minimum );

  mPostCard =  new DocPostCard( vb );

  mPostCard->slotSetMode( DocPostCard::Full, KraftDoc::Header );
  setResizeMode( vb /* mPostCard->view() */, KeepSize );

  connect( mPostCard, SIGNAL( completed() ),
           this,  SLOT( slotRenderCompleted() ) );

  QVBox *stackVBox = new QVBox( this );
  mTemplatePane = stackVBox;
  stackVBox->setSpacing( KDialog::spacingHint() );
  mWidgetStack = new QWidgetStack( stackVBox );
  mWidgetStack->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );

  /* Selections are the gui reprenentations of the template providing catalogs
   * like header- and footer texts and catalogs.
   */
  mCatalogSelection = new CatalogSelection( mWidgetStack );
  connect( mCatalogSelection, SIGNAL( selectionChanged( QListViewItem* ) ),
           this,  SLOT( slotCatalogSelectionChanged( QListViewItem* ) ) );

  mHeaderSelection = new HeaderSelection( mWidgetStack );
  connect( mHeaderSelection, SIGNAL( addressSelectionChanged() ),
           this, SLOT( slotAddressSelectionChanged() ) );
  connect( mHeaderSelection, SIGNAL( textSelectionChanged( QListViewItem* ) ),
           this, SLOT( slotTextsSelectionChanged( QListViewItem* ) ) );
  connect( mHeaderSelection->textSelection(), SIGNAL( actionCurrentTextToDoc() ),
           this,  SLOT( slotAddToDocument() ) );

  mFooterSelection = new TextSelection( mWidgetStack, KraftDoc::Footer );
  connect( mFooterSelection, SIGNAL( textSelectionChanged( QListViewItem* ) ),
           this, SLOT( slotTextsSelectionChanged( QListViewItem* ) ) );
  connect( mFooterSelection, SIGNAL( actionCurrentTextToDoc() ),
           this,  SLOT( slotAddToDocument() ) );

  mWidgetStack->raiseWidget( mHeaderSelection );
  connect( mPostCard, SIGNAL( selectPage( int ) ),
           this,  SLOT( slotSelectDocPart( int ) ) );

  QHBox *butBox = new QHBox( stackVBox );
  butBox->setSpacing( KDialog::spacingHint() );
  QIconSet icons = BarIconSet( "back" );
  mPbAdd  = new KPushButton( icons, i18n(""), butBox );
  mPbAdd->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum );
  connect( mPbAdd, SIGNAL( clicked() ), this, SLOT( slotAddToDocument() ) );

  QToolTip::add( mPbAdd, i18n( "Add a template to the document" ) );

  w = new QWidget( butBox );
  w->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Maximum );

  mPbNew  = new KPushButton( BarIconSet( "filenew" ), i18n(""),  butBox );
  mPbNew->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum );
  connect( mPbNew, SIGNAL( clicked() ), this, SLOT( slotNewTemplate() ) );
  QToolTip::add( mPbNew, i18n( "Create a new template (type depending)" ) );

  mPbEdit  = new KPushButton( BarIconSet( "edit" ), i18n(""),  butBox );
  mPbEdit->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum );
  connect( mPbEdit, SIGNAL( clicked() ), this, SLOT( slotEditTemplate() ) );
  QToolTip::add( mPbEdit, i18n( "Edit the selected template (type depending)" ) );

  mPbDel  = new KPushButton( BarIconSet( "editdelete" ), i18n(""),  butBox );
  mPbDel->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum );
  connect( mPbDel, SIGNAL( clicked() ), this, SLOT( slotDeleteTemplate() ) );
  QToolTip::add( mPbDel, i18n( "Delete the selected template (type depending)" ) );

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
           this,  SLOT( slotTextDeleted( const DocText& ) ) );

  connect( mHeaderSelection, SIGNAL( switchedToHeaderTab( HeaderSelection::HeaderTabType ) ),
           this, SLOT( slSetHeaderTemplateProvider( HeaderSelection::HeaderTabType ) ) );

  mFooterTemplateProvider = new FooterTemplateProvider( parent );

  /* get a new Footer text from the default provider */
  connect( mFooterTemplateProvider, SIGNAL( newFooterText( const DocText& ) ),
           this,  SLOT( slotNewFooterDocText( const DocText& ) ) );
  connect( mFooterTemplateProvider, SIGNAL( updateFooterText( const DocText& ) ),
           this,  SLOT( slotUpdateFooterDocText( const DocText& ) ) );
  connect( mFooterTemplateProvider, SIGNAL( footerTextToDocument( const DocText& ) ),
           this,  SLOT( slotFooterTextToDocument( const DocText& ) ) );
  connect( mFooterTemplateProvider, SIGNAL( deleteFooterText( const DocText& ) ),
           this,  SLOT( slotTextDeleted( const DocText& ) ) );

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

  // mCurrTemplateProvider = mHeaderTemplateProvider;

  setSizes( KraftSettings::self()->assistantSplitterSetting() );
  mTemplatePane->hide();
}

void DocAssistant::slotAddToDocument()
{
  kdDebug() << "SlotAddToDocument called!" << endl;
  if ( mCurrTemplateProvider ) {
    mCurrTemplateProvider->slotTemplateToDocument();
  }
}

void DocAssistant::slotAddressSelectionChanged()
{
  kdDebug() << "A address template was selected!" << endl;
  if ( mHeaderSelection->textSelection()->textsListView()->currentItem() ) {
    mPbAdd->setEnabled( true );
    mPbEdit->setEnabled( true );
    mPbDel->setEnabled( false );

  }
}

void DocAssistant::slotTextsSelectionChanged( QListViewItem *item )
{
  mHeaderTemplateProvider->slotSetCurrentDocText( mHeaderSelection->currentDocText() );
  mFooterTemplateProvider->slotSetCurrentDocText( mFooterSelection->currentDocText() );

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

void DocAssistant::slotCatalogSelectionChanged( QListViewItem* item )
{
  // enable the move-to-document button.
  kdDebug() << "catalog position selection changed!" << endl;
  if ( item ) {
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
  kdDebug() << "Editing a template using the currentTemplProvider" << endl;
  if ( mCurrTemplateProvider ) {
    mCurrTemplateProvider->slotSetDocType( mDocType );
    mCurrTemplateProvider->slotEditTemplate();
  }
}

/* slot that initialises a delete, called from the delete button */
void DocAssistant::slotDeleteTemplate()
{
  if ( KMessageBox::warningYesNo( this, i18n( "Do you really want to delete the "
                                            "Template permanently? There is no way "
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
    kdDebug() << "Unknown HeaderSelection type" << endl;
  }
}

void DocAssistant::slotTextDeleted( const DocText& /* dt */)
{
  mHeaderSelection->textSelection()->deleteCurrentText();
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
  // kdDebug() << "Render completed: " << mPostCard->view()->contentsHeight() << endl;

#if 0
  /* This is unfortunately not working because contentsHeight is always as
     heigh as the viewport is. Must be fixed in khtmlpart. */
  QValueList<int> sizes;
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
    slSetHeaderTemplateProvider( mHeaderSelection->currentPageIndex() ?
                                 HeaderSelection::AddressTab : HeaderSelection::TextTab );
    mPbNew->setEnabled( true );

  } else if ( p == KraftDoc::Positions ) {
    mCurrTemplateProvider = mCatalogTemplateProvider;
    mPbNew->setEnabled( false );
  } else if ( p == KraftDoc::Footer ) {
    mCurrTemplateProvider = mFooterTemplateProvider;
    mPbNew->setEnabled( true );
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
}

void DocAssistant::slotShowCatalog( )
{
  setFullPreview( false, KraftDoc::Positions );
  mWidgetStack->raiseWidget( mCatalogSelection );
}

void DocAssistant::slotShowHeaderTemplates()
{
  setFullPreview( false, KraftDoc::Header );
  mWidgetStack->raiseWidget( mHeaderSelection );
}

void DocAssistant::slotShowFooterTemplates()
{
  setFullPreview( false, KraftDoc::Footer );
  mWidgetStack->raiseWidget( mFooterSelection );
}

void DocAssistant::setFullPreview( bool setFull, int id )
{
  if ( setFull ) {
    /* remember the sizes used before */
    if ( mTemplatePane->isVisible() ) {
      kdDebug() << "Writing mSplitterSizes: " << sizes() << endl;
      KraftSettings::self()->setAssistantSplitterSetting( sizes() );
      KraftSettings::self()->writeConfig();
    }

    mTemplatePane->hide();
    mPostCard->slotSetMode( DocPostCard::Full, id );
    mFullPreview = true;
  } else {
    mTemplatePane->show();
    mPostCard->slotSetMode( DocPostCard::Mini, id );

    if ( KraftSettings::self()->assistantSplitterSetting().size() == 2 ) {
      setSizes( KraftSettings::self()->assistantSplitterSetting() );
    }
    mFullPreview = false;
  }
}

#include "docassistant.moc"
