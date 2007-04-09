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

#include <kiconloader.h>
#include <kpushbutton.h>

#include "docassistant.h"
#include "docpostcard.h"
#include "catalogselection.h"
#include "headerselection.h"
#include "kraftsettings.h"
#include "kataloglistview.h"


DocAssistant::DocAssistant( QWidget *parent ):
  QSplitter( parent ), mFullPreview( true ),
  mActivePage( DocPostCard::HeaderId )
{
  setOrientation( Vertical );
  QVBox *vb = new QVBox( this );

  QHBox *hb = new QHBox( vb );
  hb->setFrameStyle( Box + Sunken );
  hb->setMargin( KDialog::marginHint()/2 );

  KPushButton *pb = new KPushButton( i18n( "show Templates" ),  hb );
  connect( pb, SIGNAL( toggled( bool ) ),
           this,  SLOT( slotToggleShowTemplates( bool ) ) );
  pb->setToggleButton( true );
  QToolTip::add( pb, i18n( "Show mask to create or select templates to be used in the document" ) );

  QWidget *w = new QWidget( hb );
  w->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Minimum );

  mPostCard =  new DocPostCard( vb );

  mPostCard->slotSetMode( DocPostCard::Full, DocPostCard::HeaderId );
  setResizeMode( vb /* mPostCard->view() */, KeepSize );

  connect( mPostCard, SIGNAL( completed() ),
           this,  SLOT( slotRenderCompleted() ) );

  QVBox *stackVBox = new QVBox( this );
  mTemplatePane = stackVBox;
  stackVBox->setSpacing( KDialog::spacingHint() );
  mWidgetStack = new QWidgetStack( stackVBox );
  mWidgetStack->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );

  mCatalogSelection = new CatalogSelection( mWidgetStack );
  connect( mCatalogSelection,  SIGNAL( positionSelected( Katalog*, void* ) ),
           this,  SIGNAL( positionSelected( Katalog*, void* ) ) );

  mHeaderSelection = new HeaderSelection( mWidgetStack );
  connect( mHeaderSelection, SIGNAL( addressSelectionChanged() ),
           this, SLOT( slotAddressSelectionChanged() ) );
  connect( mHeaderSelection, SIGNAL( textSelectionChanged() ),
           this, SLOT( slotTextsSelectionChanged() ) );

  mWidgetStack->raiseWidget( mHeaderSelection );
  connect( mPostCard, SIGNAL( selectPage( int ) ),
           this,  SLOT( slotSelectPage( int ) ) );

  QHBox *butBox = new QHBox( stackVBox );
  butBox->setSpacing( KDialog::spacingHint() );
  QIconSet icons = BarIconSet( "back" );
  mPbAdd  = new KPushButton( icons, i18n(""), butBox );
  mPbAdd->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum );
  connect( mPbAdd, SIGNAL( clicked() ), this, SLOT( slotAddToDocument() ) );

  QToolTip::add( mPbAdd, i18n( "Add a template to the document" ) );

  mPbNew  = new KPushButton( BarIconSet( "filenew" ), i18n(""),  butBox );
  mPbNew->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum );
  connect( mPbNew, SIGNAL( clicked() ), this, SLOT( slotNewTemplate() ) );
  QToolTip::add( mPbNew, i18n( "Create a new template (type depending)" ) );
  mPbAdd->setEnabled( false );

  w = new QWidget( butBox );
  w->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Maximum );

  setSizes( KraftSettings::self()->assistantSplitterSetting() );
  mTemplatePane->hide();
}

void DocAssistant::slotAddToDocument()
{
  kdDebug() << "SlotAddToDocument called!" << endl;

  if ( mWidgetStack->visibleWidget() == mHeaderSelection ) {
    /* Header page */
    if ( mHeaderSelection->textPageActive() ) {
      kdDebug() << "Text Page active" << endl;
    } else if ( mHeaderSelection->addressPageActive() ) {
      kdDebug() << "Address Page active" << endl;
      KABC::Addressee adr = mHeaderSelection->currentAddressee();
      emit addressTemplate( adr );
    }

  } else if ( mWidgetStack->visibleWidget() == mCatalogSelection ) {


  } else if ( mWidgetStack->visibleWidget() == mFooterSelection ) {

  }
}

void DocAssistant::slotAddressSelectionChanged()
{
  kdDebug() << "A address template was selected!" << endl;
  mPbAdd->setEnabled( true );
}

void DocAssistant::slotTextsSelectionChanged()
{
  kdDebug() << "A text template was selected!" << endl;
  mPbAdd->setEnabled( true );
}

void DocAssistant::slotNewTemplate()
{
  kdDebug() << "SlotNewTemplate called!" << endl;

}

void DocAssistant::slotSelectPage( int p )
{
  mActivePage = p;
  emit selectPage( p ) ;
}

/* slot that opens the template details in case on == true */
void DocAssistant::slotToggleShowTemplates( bool on )
{
  if ( on ) {
    if ( mActivePage == DocPostCard::HeaderId ) {
      slotShowAddresses();
    } else if ( mActivePage == DocPostCard::PositionId ) {
      slotShowCatalog();
    } else if ( mActivePage == DocPostCard::FooterId ) {

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

void DocAssistant::slotShowCatalog( )
{
  setFullPreview( false, DocPostCard::PositionId );
  mWidgetStack->raiseWidget( mCatalogSelection );
}

void DocAssistant::slotShowAddresses()
{
  setFullPreview( false, DocPostCard::HeaderId );
  mWidgetStack->raiseWidget( mHeaderSelection );
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
