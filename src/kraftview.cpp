/***************************************************************************
                          kraftview.cpp  -
                             -------------------
    begin                : Mit Dez 31 19:24:05 CET 2003
    copyright            : (C) 2003 by Klaas Freitag
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

// include files for Qt
#include <qlayout.h>
#include <qlabel.h>
#include <qscrollview.h>
#include <qsizepolicy.h>
#include <qtextedit.h>
#include <qsignalmapper.h>
#include <qhbox.h>
#include <qvbox.h>
#include <qgrid.h>
#include <qwidgetstack.h>
#include <qtabwidget.h>
#include <qcolor.h>
#include <qsplitter.h>
#include <qbuttongroup.h>
#include <qtooltip.h>
#include <qfont.h>
#include <qptrlist.h>

#include <kdebug.h>
#include <kdialogbase.h>
#include <kpushbutton.h>
#include <kcombobox.h>
#include <kdatewidget.h>
#include <knuminput.h>
#include <kactioncollection.h>
#include <kmessagebox.h>
#include <khtmlview.h>
#include <kiconloader.h>

#include <kabc/addressbook.h>
#include <kabc/stdaddressbook.h>
#include <kabc/addresseedialog.h>
#include <kabc/addressee.h>


// application specific includes
#include "kraftdb.h"
#include "kraftsettings.h"
#include "kraftview.h"
#include "kraftdoc.h"
#include "portal.h"
#include "docheader.h"
#include "documentman.h"
#include "docassistant.h"
#include "positionviewwidget.h"
#include "docfooter.h"
#include "docposition.h"
#include "unitmanager.h"
#include "docpostcard.h"
#include "kataloglistview.h"
#include "katalogman.h"
#include "templkatalog.h"
#include "templkataloglistview.h"
#include "catalogselection.h"
#include "headerselection.h"
#include "kraftdocheaderedit.h"
#include "kraftdocpositionsedit.h"
#include "kraftdocfooteredit.h"
#include "inserttempldialog.h"
#include "defaultprovider.h"
#include "stockmaterial.h"
#include "brunsrecord.h"
#include "insertplantdialog.h"
#include "templtopositiondialogbase.h"
#include "doctype.h"
#include "catalogtemplate.h"
#include "extendedcombo.h"
#include "importitemdialog.h"

#include <qtimer.h>
#include "doclocaledialog.h"


KraftViewScroll::KraftViewScroll( QWidget *parent ):
QScrollView( parent )
{

}

void KraftViewScroll::viewportResizeEvent ( QResizeEvent *ev )
{
  int w = ev->size().width(); // visibleWidth()-1;
  resizeContents( w, contentsHeight () );
  QScrollView::viewportResizeEvent( ev );
}

void KraftViewScroll::resizeContents(  int w, int h )
{
  if( w < 400 ) {
    w = 400;
  }
  QScrollView::resizeContents( w, h );
  PositionViewWidget *wid;
  for ( wid = mWidgetList.first(); wid; wid = mWidgetList.next() ) {
    wid->resize( w, wid->height() );
  }
}

void KraftViewScroll::addChild( QWidget *child, int x, int y )
{
  mWidgetList.append( static_cast<PositionViewWidget*>(child) );
  QScrollView::addChild( child, x, y );
}

void KraftViewScroll::kraftRemoveChild( PositionViewWidget *child )
{
  removeChild( child ); // from the scrollview
  mWidgetList.removeRef( child );
}

// #########################################################

KraftView::KraftView(QWidget *parent, const char *name) :
  KDialogBase( parent, name, false /* modal */, i18n("Document"),
	      Ok|Cancel, Ok, true /* separator */ ),
  m_doc( 0 ),
  mHelpLabel( 0 ), mRememberAmount( -1 ), mModified( false )
{
  mDeleteMapper = new QSignalMapper( this );
  connect( mDeleteMapper, SIGNAL( mapped(int)),
           this, SLOT( slotDeletePosition( int ) ) );

  mMoveUpMapper = new QSignalMapper( this );
  connect( mMoveUpMapper, SIGNAL( mapped(int)),
           this, SLOT( slotMovePositionUp( int  ) ) );

  mMoveDownMapper = new QSignalMapper( this );
  connect( mMoveDownMapper, SIGNAL( mapped(int)),
           this, SLOT( slotMovePositionDown( int ) ) );

  mLockPositionMapper = new QSignalMapper( this );
  connect( mLockPositionMapper, SIGNAL( mapped( int )),
           this, SLOT( slotLockPosition( int ) ) );

  mUnlockPositionMapper = new QSignalMapper( this );
  connect( mUnlockPositionMapper, SIGNAL( mapped( int )),
           this, SLOT( slotUnlockPosition( int ) ) );

  mModifiedMapper = new QSignalMapper( this );
  connect( mModifiedMapper,  SIGNAL( mapped( int ) ),
           this,  SLOT( slotPositionModified( int ) ) );

#if 0
  connect( this, SIGNAL( aboutToShowPage( QWidget* ) ),
           this, SLOT( slotAboutToShow( QWidget* ) ) );
#endif

  mGlobalVBox = makeVBoxMainWidget();
  mGlobalVBox->setMargin( 3 );

  mDetailHeader = new QLabel( mGlobalVBox );
  mDetailHeader->setSizePolicy( QSizePolicy( QSizePolicy::Minimum, QSizePolicy::Fixed ) );
  // mDetailHeader->setMargin( 4 );
  mDetailHeader->setFrameStyle( QFrame::Box + QFrame::Plain );
  mDetailHeader->setLineWidth( 1 );
  mDetailHeader->setPaletteBackgroundColor( QColor( "darkBlue" ));
  mDetailHeader->setPaletteForegroundColor( QColor( "white" ) );
  mDetailHeader->setTextFormat( Qt::PlainText );
  mDetailHeader->setFixedHeight( 40 ); // FIXME
  QFont f = mDetailHeader->font();
  f.setPointSize( qRound( 1.4 * f.pointSize() ) );
  f.setBold( true );
  mDetailHeader->setFont( f );

  mCSplit    = new QSplitter( mGlobalVBox );
  QVBox *vb  = new QVBox( mCSplit );
  vb->setMargin( 0 );
  mViewStack = new QWidgetStack( vb );
  mViewStack->setMargin( 0 );
  kdDebug() << "mViewSTack height is " << mViewStack->height() << endl;

  mAssistant = new DocAssistant( mCSplit );
  /* catalog template selection signal */
  connect( mAssistant,  SIGNAL( positionSelected( Katalog*, void* ) ),
           this,  SLOT( slotAddPosition( Katalog*, void* ) ) );

  /* signal to toggle the visibility of the template section in the assistant */
  connect(  mAssistant, SIGNAL( toggleShowTemplates( bool ) ),
            this,  SLOT( slotShowTemplates( bool ) ) );

  /* signal that brings a new address to the document */
  connect( mAssistant, SIGNAL( addressTemplate( const Addressee& ) ),
           this, SLOT( slotNewAddress( const Addressee& ) ) );

  connect( mAssistant, SIGNAL( headerTextTemplate( const QString& ) ),
           this, SLOT( slotNewHeaderText( const QString& ) ) );

  connect( mAssistant, SIGNAL( footerTextTemplate( const QString& ) ),
           this, SLOT( slotNewFooterText( const QString& ) ) );

  if ( KraftSettings::self()->docViewSplitter().count() == 2 ) {
    mCSplit->setSizes( KraftSettings::self()->docViewSplitter() );
  }
  connect( mAssistant, SIGNAL( selectPage( int ) ),
           this,  SLOT( slotSwitchToPage( int ) ) );

  QSize size = KraftSettings::self()->docViewSize();
  if ( !size.isEmpty() ) resize( size );
  QPoint pos = KraftSettings::self()->docViewPosition();
  if ( !pos.isNull() ) move( pos );

  mAssistant->slotSelectDocPart( KraftDoc::Header );

  mNewTemplates.setAutoDelete( true );
}
KraftView::~KraftView()
{

}

void KraftView::setup( DocGuardedPtr doc )
{
  m_doc = doc;

  setupDocHeaderView();
  setupPositions();
  setupFooter();
  setCaption( m_doc->docIdentifier() );
  slotSwitchToPage( KraftDoc::Header );
}


void KraftView::slotSwitchToPage( int id )
{
  // check if the wanted part is already visible
  if ( mViewStack->visibleWidget() == mViewStack->widget( id ) ) return;


  mViewStack->raiseWidget( id );

  KraftDocEdit *edit =
    static_cast<KraftDocEdit *>( mViewStack->visibleWidget() );

  mDetailHeader->setText( edit->title() );
  mDetailHeader->setPaletteBackgroundColor( edit->color() );
  // FIXME: color
  mDetailHeader->setPaletteForegroundColor( QColor( "#00008b" ) );

  mAssistant->slotSelectDocPart( mViewStack->id( mViewStack->visibleWidget() ) );
  // mAssistant->postCard()->renderDoc( mViewStack->id( mViewStack->visibleWidget() ) );
}

void KraftView::slotShowTemplates( bool )
{
}

void KraftView::setupDocHeaderView()
{
    KraftDocHeaderEdit *edit = new KraftDocHeaderEdit( mainWidget() );

    mHeaderId = mViewStack->addWidget( edit, KraftDoc::Header );

    m_headerEdit = edit->docHeaderEdit();

    m_headerEdit->m_cbType->clear();
    // m_headerEdit->m_cbType->insertStringList( DefaultProvider::self()->docTypes() );
    m_headerEdit->m_cbType->insertStringList( DocType::allLocalised() );

    if ( KraftSettings::showDocumentLocale() ) {
      m_headerEdit->mButtLang->show();
    } else {
      m_headerEdit->mButtLang->hide();
    }

    m_headerEdit->m_whiteboardEdit->setFrameStyle( QFrame::NoFrame );
    m_headerEdit->m_whiteboardEdit->setMaximumSize( 32676, 80 );
    // m_headerEdit->m_whiteboardEdit->setBackgroundColor( QColor( 243, 244, 121 ) );
    connect( m_headerEdit->m_cbType,  SIGNAL( activated( const QString& ) ),
             this, SLOT( slotDocTypeChanged( const QString& ) ) );

    connect( m_headerEdit->mButtLang, SIGNAL( clicked() ),
             this, SLOT( slotLanguageSettings() ) );
    connect( edit, SIGNAL( modified() ),
              this, SLOT( slotModifiedHeader() ) );
}

void KraftView::setupPositions()
{
    KraftDocPositionsEdit *edit = new KraftDocPositionsEdit( mainWidget() );
    mViewStack->addWidget( edit, KraftDoc::Positions );

    m_positionScroll = edit->positionScroll();

    connect( edit, SIGNAL( addPositionClicked() ), SLOT( slotAddPosition() ) );
    connect( edit, SIGNAL( addExtraClicked() ), SLOT( slotAddExtraPosition() ) );
    connect( edit, SIGNAL( importItemsClicked() ), SLOT( slotImportItems() ) );

}

void KraftView::redrawDocument( )
{
    KraftDoc *doc = getDocument();
    if( !doc ) {
      kdDebug() << "ERR: No document available in view, return!" << endl;
    } else {
      kdDebug() << "** Starting redraw of document " << doc << endl;
    }

    /* header: date and document type */
    QDate date = doc->date();
    m_headerEdit->m_dateEdit->setDate( date );
    m_headerEdit->m_cbType->setCurrentText( doc->docType() );

    /* header: address */
    mContactUid  = doc->addressUid();
    QString address = doc->address();

    kdDebug() << "Loaded address uid from database " << mContactUid << endl;
    if( ! mContactUid.isEmpty() ) {
      // FIXME - use centralised address provider
      AddressBook *adrBook =  StdAddressBook::self( );
      Addressee contact;
      if( adrBook ) {
        contact = adrBook->findByUid( mContactUid );
        if( contact.isEmpty() ) {
          // the Uid is set, but the address is not in this addressbook
          KMessageBox::sorry( this, i18n("The addressbook does not contain the address "
                                         "identified by the ID in the document.")
                              .arg( mContactUid ), i18n("Addressbook out of sync") );

          m_headerEdit->m_labName->setText( i18n("--lost--") );
        } else {
          kdDebug() << "The loaded Contact has this realname: " << contact.realName() << endl;
          slotNewAddress( contact );
        }
      }
    }
    if( !address.isEmpty() ) {
      // custom address stored in the document.
      // kdDebug() << "custom address came in: " << address << endl;
      m_headerEdit->m_postAddressEdit->setText( address );
    }

    if( !doc->salut().isEmpty() ) {
      m_headerEdit->m_letterHead->insertItem( doc->salut() );
      m_headerEdit->m_letterHead->setCurrentText( doc->salut() );
    }
    /* pre- and post text */
    m_headerEdit->m_teEntry->setText( doc->preText() );
    m_headerEdit->m_whiteboardEdit->setText( doc->whiteboard() );
    m_headerEdit->mProjectLabelEdit->setText( doc->projectLabel() );
    m_footerEdit->m_teSummary->setText( doc->postText() );

    mAssistant->slotSetDocType( doc->docType() );
    if ( !doc->goodbye().isEmpty() ) {
      m_footerEdit->m_cbGreeting->setCurrentText( doc->goodbye() );
    }

    redrawDocPositions( );
    slotDocTypeChanged( doc->docType() );
    refreshPostCard();

    mModified = false;
}

void KraftView::redrawDocPositions( )
{
  // If there is no position yet, come up with a help message.
  KraftDoc *doc = getDocument();
  if ( ! doc ) return;

  DocPositionList list = doc->positions();
  if ( list.count() == 0 ) {
    // the doc has no positions yet. Let's show a help page
    if ( ! mHelpLabel ) {
      mHelpLabel = new QLabel(0);
      mHelpLabel->setMargin( KDialog::marginHint() );
      mHelpLabel->setText( i18n( "<qt><h2>The Document Position List is still empty, but Positions "
                                 "can be added now.</h2>"
                                 "To add positions to the document either "
                                 "<ul>"
                                 "<li>Press the 'Add' button on top of this canvas.</li>"
                                 "<li>Open the template catalog clicking on the '%1' "
                                  "button on the right and select from available template positions.</li>"
                                   "</ul></qt>").arg( i18n( "show Template" ) ) );
      m_positionScroll->addChild( mHelpLabel,  0, 0 );
    }
    return;

  } else {
    if ( mHelpLabel ) {
      delete mHelpLabel;
      mHelpLabel = 0;
    }
  }

  kdDebug() << "starting to redraw " << list.count() << " positions!" << endl;

  DocPositionBase *dp = 0;

  for( dp = list.first(); dp; dp = list.next() ) {
    PositionViewWidget *w = mPositionWidgetList.widgetFromPosition( dp );
    if( !w ) {
      w = createPositionViewWidget( dp, list.at() );
    }
    kdDebug() << "now position " << dp->positionNumber() << endl;
  }

  // now go through the positionWidgetMap and check if it contains elements
  // that are not any longer in the list of positions
  PositionViewWidget *w = 0;
  for( w = mPositionWidgetList.first(); w; w = mPositionWidgetList.next() ) {
    if( list.containsRef( w->position() ) == 0 ) {
      kdDebug() << "Removing this one: " << w << endl;
      m_positionScroll->kraftRemoveChild( w );
      mPositionWidgetList.remove();
      break;
    }
  }

  // repaint everything
  m_positionScroll->updateContents();

}

void KraftView::setMappingId( QWidget *widget, int pos )
{
  mDeleteMapper->setMapping( widget, pos );
  mMoveUpMapper->setMapping( widget, pos );
  mMoveDownMapper->setMapping( widget, pos );
  mLockPositionMapper->setMapping( widget, pos );
  mUnlockPositionMapper->setMapping( widget, pos );
  mModifiedMapper->setMapping( widget, pos );
}

//
// create a new position widget.
// The position parameter comes in as list counter, starting at 0

PositionViewWidget *KraftView::createPositionViewWidget( DocPositionBase *dp, int pos )
{
  PositionViewWidget *w = new PositionViewWidget( );

  int cw = m_positionScroll->contentsWidth();
  if ( cw < 400 ) cw = 400;
  w->resize( cw, w->height() );

  connect( w, SIGNAL( deletePosition() ),  mDeleteMapper, SLOT( map() ) );
  connect( w, SIGNAL( moveUp() ),          mMoveUpMapper, SLOT( map() ) );
  connect( w, SIGNAL( moveDown() ),        mMoveDownMapper, SLOT( map() ) );
  connect( w, SIGNAL( lockPosition() ),    mLockPositionMapper, SLOT( map() ) );
  connect( w, SIGNAL( unlockPosition() ),  mUnlockPositionMapper, SLOT( map() ) );
  connect( w, SIGNAL( positionModified()), mModifiedMapper,  SLOT( map() ) );

  setMappingId( w, pos );

  QStringList units = UnitManager::allUnits();
  units.sort();

  for ( QStringList::Iterator it = units.begin(); it != units.end(); ++it ) {
    Einheit e = UnitManager::getUnit( UnitManager::getUnitIDSingular( *it ) );

    w->m_cbUnit->insertEntry( e.einheitSingular(), e.einheitSingularLong() );
  }

  if( dp->dbId().toInt() < 0 ) {
    w->slotSetState( PositionViewWidget::New );
  }

  /* do resizing and add the widget to the scrollview and move it to the final place */

  if ( mHelpLabel ) {
    mHelpLabel->hide();
  }

  mPositionWidgetList.insert( pos,  w );
  m_positionScroll->resizeContents( cw,
                                    mPositionWidgetList.count() * w->height()+1 );

  /* If the new pos is not appended, we have to move all the following on height unit down */
  if ( ( unsigned )pos != mPositionWidgetList.count() ) {
    PositionViewWidget *vw;
    for ( vw = mPositionWidgetList.at( pos+1 ); vw; vw = mPositionWidgetList.next() ) {
      m_positionScroll->moveChild( vw, 0, m_positionScroll->childY( vw ) + w->height() );
    }
  }

  m_positionScroll->addChild( w, 0, 0 );

  w->setDocPosition( dp, getDocument()->locale() );
  w->setOrdNumber( 1 + pos );
  int y = pos * w->height();
  m_positionScroll->moveChild( w, 0, y );

  w->show();

  return w;
}

DocPositionBase::TaxType KraftView::currentTaxSetting()
{
  int taxKind = m_footerEdit->mTaxCombo->currentItem();
  DocPositionBase::TaxType tt = DocPositionBase::TaxInvalid;

  if ( taxKind == 0 ) { // No Tax at all
    tt = DocPositionBase::TaxNone;
  } else if ( taxKind == 1 ) { // Reduced tax for all items
    tt = DocPositionBase::TaxReduced;
  } else if ( taxKind == 2 ) { // Full tax for all items
    tt = DocPositionBase::TaxFull;
  } else { // individual level, not yet implementend
    kdError() << "Item individual tax level is not yet implemented." << endl;
  }
  return tt;
}

void KraftView::refreshPostCard()
{
  DocPositionList positions = currentPositionList();

  if ( mAssistant->postCard() ) {
    QDate d = m_headerEdit->m_dateEdit->date();
    const QString dStr = getDocument()->locale()->formatDate( d );

    mAssistant->postCard()->setHeaderData( m_headerEdit->m_cbType->currentText(),
                                           dStr, m_headerEdit->m_postAddressEdit->text(),
                                           getDocument()->ident(),
                                           m_headerEdit->m_teEntry->text() );


    mAssistant->postCard()->setPositions( positions,  currentTaxSetting(),
                                          DocumentMan::self()->tax( d ),
                                          DocumentMan::self()->reducedTax( d ) );

    mAssistant->postCard()->setFooterData( m_footerEdit->m_teSummary->text(),
                                           m_footerEdit->m_cbGreeting->currentText() );

    mAssistant->postCard()->renderDoc( mViewStack->id( mViewStack->visibleWidget() ) );
  }

  DocPositionBase *dp;
  for( dp = positions.first(); dp; dp = positions.next() ) {
    if (  dp->type() == DocPositionBase::ExtraDiscount ) {
      PositionViewWidget *w = ( static_cast<DocPosition*>( dp ) )->associatedWidget();
      if( w ) {
        w->slotSetOverallPrice( ( static_cast<DocPosition*>( dp ) )->overallPrice() );
      } else {
        kdDebug() << "Warning: Position object has no associated widget!" << endl;
      }
    }
  }
}

void KraftView::setupFooter()
{
  KraftDocFooterEdit *edit = new KraftDocFooterEdit( mainWidget() );

  mViewStack->addWidget( edit, KraftDoc::Footer );

  m_footerEdit = edit->docFooterEdit();

  m_footerEdit->m_cbGreeting->insertStringList( KraftDB::self()->wordList( "greeting" ) );

  m_footerEdit->m_cbGreeting->setCurrentText( KraftSettings::self()->greeting() );

  // ATTENTION: If you change the following inserts, make sure to check the code
  //            in method currentPositionList!
  m_footerEdit->mTaxCombo->insertItem( i18n( "Display no tax at all" ), 0 );
  m_footerEdit->mTaxCombo->insertItem( i18n( "Calculate reduced tax for all items" ), 1);
  m_footerEdit->mTaxCombo->insertItem( i18n( "Calculate full tax for all items" ), 2 );
  // m_footerEdit->mTaxCombo->insertItem( i18n( "Calculate on individual item tax rate" ), 3 );

  // set the tax type combo correctly: If all items have the same tax type, take it.
  // If items have different, its the individual thing.
  DocPositionList list = m_doc->positions();

  int tt = -1;
  DocPositionBase *dp = 0;
  bool equality = true;

  for( dp = list.first(); dp; dp = list.next() ) {
    if ( tt == -1 )
      tt = dp->taxTypeNumeric(); // store the first entry.
    else {
      if ( tt != dp->taxTypeNumeric() ) {
        m_footerEdit->mTaxCombo->setCurrentItem( 3 );
        equality = false;
      } else {
        // old and new taxtype are the same.
      }
    }
  }
  if ( tt == -1 ) {
    // means that there is no item yet, the default tax type needs to be used.
    m_footerEdit->mTaxCombo->setCurrentItem( KraftSettings::self()->defaultTaxType() );
  } else {
    if ( equality ) {
      m_footerEdit->mTaxCombo->setCurrentItem( tt-1 );
    } else {
      kdError() << "Problem: Not all Tax-Levels are the same! (Fixed later with tax on item base)" << endl;
    }
  }

  connect( m_footerEdit->mTaxCombo, SIGNAL( activated( int ) ),
           this, SLOT( slotModifiedFooter() ) );

  connect( edit, SIGNAL( modified() ),
           this, SLOT( slotModifiedFooter() ) );
}

void KraftView::slotAboutToShow( QWidget* w )
{
  kdDebug() << "showing page " << w << endl;
}

/* This is the flow in the move up method:
         0    Bla1          0 Bla1                         0 Bla1
         1    Bla2 <- w2    1 Bla2   -> insert at pos-1 => 1 Bla3
 pos ->  2    Bla3 <- w1    2 Bla4                         2 Bla2
         3    Bla4                                         3 Bla4
 */
void KraftView::slotMovePositionUp( int pos )
{
  PositionViewWidget *w1 = 0;
  PositionViewWidget *w2 = 0;

  kdDebug() << "Moving position up: " << pos << endl;
  if( pos < 1 || (unsigned) pos > mPositionWidgetList.count() ) {
    kdDebug() << "ERR: position out of range: " << pos << endl;
    return;
  }

  w2 = mPositionWidgetList.at( pos-1 );
  w1 = mPositionWidgetList.take( pos );
  mPositionWidgetList.insert( pos-1, w1 );
  kdDebug() << "Found at pos " << pos << " the widgets " << w1 << " and " << w2 << endl;

  PositionViewWidget *vw = 0;
  for( vw = mPositionWidgetList.first(); vw; vw = mPositionWidgetList.next() ) {
    DocPositionBase* pb = vw->position();
    if( ! pb ) {
      kdDebug() << "There is no position!" << endl;
    } else {
      kdDebug() << "Pos " << vw->ordNumber() << ": " << pb->text() << endl;
    }
  }

  if( w1 && w2 ) {
    kdDebug() << "Setting ord number: " << pos << endl;
    w1->setOrdNumber( pos );  // note: ordnumbers start with 1, thus add one
    w2->setOrdNumber( pos+1 );
    setMappingId( w1, pos-1 );
    setMappingId( w2, pos );
    // int tmpX = m_positionScroll->childX( w1 );
    int tmpY = m_positionScroll->childY( w1 );

    m_positionScroll->moveChild( w1, 0, m_positionScroll->childY( w2 ) );
    m_positionScroll->moveChild( w2, 0, tmpY );
    QTimer::singleShot( 0, this, SLOT(refreshPostCard()  ) );
  } else {
    kdDebug() << "ERR: Did not find the two corresponding widgets!" << endl;
  }
}
/*
          0    Bla1          0 Bla1                         0 Bla1
  pos ->  1    Bla2 <- w1    1 Bla3                         1 Bla3
          2    Bla3 <- w2    2 Bla4   -> insert at pos+1 => 2 Bla2
          3    Bla4                                         3 Bla4
*/
void KraftView::slotMovePositionDown( int pos )
{
  PositionViewWidget *w1 = 0;
  PositionViewWidget *w2 = 0;
  kdDebug() << "Moving position down: " << pos << endl;

  if( pos < 0 || (unsigned) pos >= mPositionWidgetList.count() -1 ) {
    kdDebug() << "ERR: position out of range: " << pos << endl;
    return;
  }

  w2 = mPositionWidgetList.at( pos+1 );
  w1 = mPositionWidgetList.take( pos );
  mPositionWidgetList.insert( pos+1, w1 );

  if( w1 && w2 ) {
    w1->setOrdNumber( pos+2  );
    w2->setOrdNumber( pos+1 );

    setMappingId( w1, pos+1 );
    setMappingId( w2, pos );

    int tmpY = m_positionScroll->childY( w1 );
    m_positionScroll->moveChild( w1, 0, m_positionScroll->childY( w2 ) );
    m_positionScroll->moveChild( w2, 0, tmpY );

    QTimer::singleShot( 0, this, SLOT( refreshPostCard() ) );
  } else {
    kdDebug() << "ERR: Did not find the two corresponding widgets!" << endl;
  }
}

void KraftView::slotDeletePosition( int pos )
{
  PositionViewWidget *w1 = mPositionWidgetList.at( pos );
  if( w1 ) {
    w1->slotSetState( PositionViewWidget::Deleted );

    refreshPostCard();
  }
}

void KraftView::slotLockPosition( int pos )
{
  kdDebug() << "Locking Position " << pos << endl;

  PositionViewWidget *w1 = mPositionWidgetList.at( pos );
  if( w1 ) {
    w1->slotSetState( PositionViewWidget::Locked );
    refreshPostCard();
  }
}

void KraftView::slotUnlockPosition( int pos )
{
  kdDebug() << "Unlocking Position " << pos << endl;

  PositionViewWidget *w1 = mPositionWidgetList.at( pos );
  if( w1 ) {
    w1->slotSetState( PositionViewWidget::Active );
    refreshPostCard();
  }
}

void KraftView::slotPositionModified( int pos )
{
  kdDebug() << "Modified Position " << pos << endl;
  mModified = true;
  QTimer::singleShot( 0, this, SLOT( refreshPostCard() ) );
}

void KraftView::slotNewAddress( const Addressee& contact )
{
  Addressee adr( contact );

  if( contact.isEmpty() ) {
    return;
  }

  if( ! adr.isEmpty() ) {
    m_headerEdit->m_labName->setText( adr.realName() );

    Address address;

    Address::List addresses = adr.addresses();
    if ( addresses.count() > 1 ) {
      kdDebug() << "Have more than one address, taking the default add" << endl;
      address = adr.address( KABC::Address::Pref );
    } else if ( addresses.count() == 0 ) {
      kdDebug() << "Have no address, problem!" << endl;
    } else {
      // FIXME: Handle multiple addresses
      address = addresses.first();
    }

    mContactUid = contact.uid();
    QString adrStr;
    if( address.label().isEmpty() ) {
      adrStr = address.formattedAddress( adr.realName() );
    } else {
      adrStr = address.label();
    }
    kdDebug() << "formatted address string: " << adrStr << endl;
    m_headerEdit->m_postAddressEdit->setText( adrStr );
    m_headerEdit->m_letterHead->clear();
    QStringList li = generateLetterHead( adr );

    m_headerEdit->m_letterHead->insertStringList( li );
    m_headerEdit->m_letterHead->setCurrentItem( KraftSettings::self()->salut() );
  }
}

void KraftView::slotDocTypeChanged( const QString& newType )
{
  kdDebug() << "Doc Type changed to " << newType << endl;
  mAssistant->slotSetDocType( newType );

  DocType docType( newType );

  PositionViewWidget *w = 0;

  for ( w = mPositionWidgetList.first(); w; w = mPositionWidgetList.next() ) {
    w->slotEnableKindMenu( docType.allowAlternative() );
  }

}

void KraftView::slotLanguageSettings()
{
  kdDebug() << "Language Settings" << endl;
  DocLocaleDialog dia( this );
  KLocale *l = m_doc->locale();

  if ( m_doc ) {
    dia.setLocale( l->country(), l->language() );

    if ( dia.exec() == QDialog::Accepted  ) {
      QString c = dia.locale().country();
      if ( c != m_doc->locale()->country() ) {
        m_doc->locale()->setCountry( c );
        m_doc->locale()->setLanguage( dia.locale().language() );

        PositionViewWidget *w; //  = 0;
        for( w = mPositionWidgetList.first(); w; w = mPositionWidgetList.next() ) {
          w->setLocale( m_doc->locale() );
          w->repaint();
        }

        refreshPostCard();
      }
    }
  }
}

void KraftView::slotNewHeaderText( const QString& str )
{
  m_headerEdit->m_teEntry->setText( str );
  slotModifiedHeader();
}

void KraftView::slotNewFooterText( const QString& str )
{
  m_footerEdit->m_teSummary->setText( str );
  slotModifiedFooter();
}

void KraftView::slotAddPosition()
{
  // find the katalog
  CatalogSelection *catsel = mAssistant->catalogSelection();
  slotAddPosition( catsel->currentSelectedKat(), 0 );
}

void KraftView::slotAddPosition( Katalog *kat, void *tmpl )
{
  // newpos is a list position, starts counting at zero!
  int newpos = mPositionWidgetList.count();
  kdDebug() << "Adding Position at list position " << newpos << endl;

  TemplToPositionDialogBase *dia = 0;

  DocPosition *dp = new DocPosition();
  dp->setPositionNumber( QString::number( newpos +1 ) );
  QSize s;

  bool newTemplate = false;
  if ( !tmpl ) {
    newTemplate = true;
  }

  if ( newTemplate ) {
    // New templates may only go to the standard template catalog, FIXME
    dia = new InsertTemplDialog( this );
    dia->setCatalogChapters( kat->getKatalogChapters() );
  } else {
    // it's not a new template
    if ( kat ) {
      // For empty template in plants dialog come up with standard dialog
      if ( kat->type() == TemplateCatalog ) {
        dia = new InsertTemplDialog( this );
        FloskelTemplate *ftmpl = static_cast<FloskelTemplate*>( tmpl );
        dp->setText( ftmpl->getText() );
        dp->setUnit( ftmpl->einheit() );
        dp->setUnitPrice( ftmpl->unitPrice() );

        s = KraftSettings::self()->templateToPosDialogSize();

      } else if ( kat->type() == MaterialCatalog ) {
        dia = new InsertTemplDialog( this );
        StockMaterial *mat = static_cast<StockMaterial*>( tmpl );
        dp->setText( mat->name() );
        dp->setUnit( mat->getUnit() );
        dp->setUnitPrice( mat->salesPrice() );
        s = KraftSettings::self()->templateToPosDialogSize();

      } else if ( kat->type() == PlantCatalog ) {
        dia = new InsertPlantDialog( this );
        InsertPlantDialog *plantDia = static_cast<InsertPlantDialog*>( dia );
        BrunsRecord *bruns = static_cast<BrunsRecord*>( tmpl );
        plantDia->setSelectedPlant( bruns );
        s = KraftSettings::self()->plantTemplateToPosDialogSize();
      }
    }
  }

  if ( dia ) {
    if ( mRememberAmount > 0 ) {
      dp->setAmount( mRememberAmount );
    }

    dia->setDocPosition( dp, newTemplate );

    DocPositionList list = currentPositionList();
    dia->setPositionList( list, newpos );

    dia->setInitialSize( s );

    if ( dia->exec() ) {
      DocPosition diaPos = dia->docPosition();
      *dp = diaPos;

      // set the tax settings
      dp->setTaxType( currentTaxSetting() );

      // store the initial size of the template-to-doc-pos dialogs
      s = dia->size();

      if ( kat->type() == TemplateCatalog ) {
        KraftSettings::self()->setTemplateToPosDialogSize( s );

        // if it's a new position, create a catalog template in the incoming chapter
        if ( newTemplate ) {
          const QString chapter = dia->chapter();
          if ( !chapter.isEmpty() ) {
            FloskelTemplate *flos = new FloskelTemplate( -1, dp->text(),
                                                         dp->unit().id(),
                                                         kat->chapterID( chapter ),
                                                         1, /* CalcKind = Manual */
                                                         QDateTime::currentDateTime(),
                                                         QDateTime::currentDateTime() );
            flos->setManualPrice( dp->unitPrice() );
            flos->save(); // <- Checke das hier!
            mNewTemplates.append( flos );
          }
        }
      } else if ( kat->type() == MaterialCatalog ) {
        KraftSettings::self()->setTemplateToPosDialogSize( s );
        if ( newTemplate ) {

        }

      } else if ( kat->type() == PlantCatalog ) {
        KraftSettings::self()->setPlantTemplateToPosDialogSize( s );
        if ( newTemplate ) {

        }

      }
      KraftSettings::self()->writeConfig();

      newpos = dia->insertAfterPosition();

      mRememberAmount = dp->amount();
    } else {
      return;
    }
  }

  delete dia;

  PositionViewWidget *widget = createPositionViewWidget( dp, newpos );
  widget->slotModified();
  slotFocusPosition( widget, newpos );
  refreshPostCard();
}

void KraftView::slotImportItems()
{
  ImportItemDialog dia( this );
  DocPositionList list = currentPositionList();
  int newpos = list.count();
  dia.setPositionList( list, newpos );

  if ( dia.exec() ) {
    QValueList<DocPosition> list = dia.positionList();
    if ( list.count() > 0 ) {
      kdDebug() << "Importlist amount of entries: " << list.count() << endl;
      int cnt = 0;
      int newpos = dia.getPositionCombo()->currentItem();
      kdDebug() << "Newpos is " << newpos << endl;

      QValueList<DocPosition>::iterator posIt;
      for( posIt = list.begin(); posIt != list.end(); ++posIt ) {
        DocPosition *dp = new DocPosition( *posIt );
        dp->setTaxType( currentTaxSetting() );
        PositionViewWidget *widget = createPositionViewWidget( dp, newpos + cnt++ );
        widget->slotModified();
      }
      refreshPostCard();
    }
  }
}

void KraftView::slotAddExtraPosition()
{
  // newpos is a list position, starts counting at 0
  int newpos = mPositionWidgetList.count();
  kdDebug() << "Adding EXTRA Position at position " << newpos << endl;

  DocPosition *dp = new DocPosition( DocPosition::ExtraDiscount );
  dp->setPositionNumber( QString::number( newpos+1 ) );
  dp->setText( i18n( "Discount" ) );
  dp->setTaxType( currentTaxSetting() );

  kdDebug() << "New Extra position is " << dp << endl;

  PositionViewWidget *widget = createPositionViewWidget( dp, newpos );
  kdDebug() << "PositionViewWiget doc position is: " << widget->position() << endl;
  widget->slotModified();
  slotFocusPosition( widget, newpos );
  refreshPostCard();

}

DocPositionList KraftView::currentPositionList()
{
    DocPositionList list;
    list.setLocale( m_doc->locale() );
    PositionViewWidget *widget;
    int cnt = 1;

    PositionViewWidgetListIterator outerIt( mPositionWidgetList );

    bool progress = true;

    while ( progress && ( list.count() != mPositionWidgetList.count() ) ) {
      // the loop runs until all positions have a valid price.

      unsigned preListCnt = list.count();
      // kdDebug() << "# Pre List Count: " << preListCnt << endl;

      while ( ( widget = outerIt.current() ) != 0 ) {
        DocPositionBase *dpb = widget->position();

        ++outerIt;

        KraftDB::StringMap replaceMap;

        if ( dpb ) {
          DocPosition *newDp = new DocPosition( dpb->type() );
          newDp->setPositionNumber( QString::number( cnt++ ) );
          newDp->setAttributeMap( dpb->attributes() );
          newDp->setDbId( dpb->dbId().toInt() );
          newDp->setAssociatedWidget( widget );

          bool calculatable = true;

          if ( dpb->type() == DocPosition::ExtraDiscount ) {
            double discount = widget->mDiscountPercent->value();

            /* set Attributes with the discount percentage */
            Attribute a( DocPosition::Discount );
            a.setPersistant( true );
            a.setValue( discount );
            newDp->setAttribute( a );

            QString tagRequired = widget->extraDiscountTagRestriction();

            if ( !tagRequired.isEmpty() ) {
              Attribute tr(  DocPosition::ExtraDiscountTagRequired );
              tr.setValueRelation( "tagTemplates", "tagTmplID", "name" );
              tr.setPersistant( true );
              tr.setValue( QVariant( tagRequired ) );
              newDp->setAttribute( tr );
            }

            /* Calculate the actual sum over all widgets */
            PositionViewWidgetListIterator it( mPositionWidgetList );
            PositionViewWidget *w1;
            Geld sum;
            kdDebug() << "Starting to loop over the items " << endl;
            while (  calculatable && ( w1 = it.current() )!= 0 ) {
              ++it;
              if ( it != outerIt ) { // do not take the own value into account
                if ( tagRequired.isEmpty()  // means that all positions are to calculate
                     || w1->tagList().contains( tagRequired ) ) {
                  if ( w1->priceValid() ) {
                    sum += w1->currentPrice();
                    kdDebug() << "Summing up pos with text " << w1->ordNumber() << " and price "
                              << w1->currentPrice().toLong() << endl;
                  } else {
                    calculatable = false; // give up, we continue in outerIt
                    kdDebug() << "We skip pos " << w1->ordNumber() << endl;
                  }
                }
              } else {
                // we can not calculate ourselves.
                kdDebug() << "Skipping pos " << w1->ordNumber() << " in summing up, thats me!" << endl;
              }
            }
            kdDebug() << "Finished loop over items with calculatable=" << calculatable << endl;

            if ( calculatable ) {
              sum = sum.percent( discount );
              newDp->setUnitPrice( sum );
              newDp->setAmount( 1.0 );
              widget->setCurrentPrice( sum );
            }

            // replace some tags in the text

            replaceMap["%DISCOUNT"]     = getDocument()->locale()->formatNumber( discount );
            replaceMap["%ABS_DISCOUNT"] = getDocument()->locale()->formatNumber( QABS( discount ) );

          } else {
            /* Type is ordinary position */
            newDp->setUnitPrice( widget->unitPrice() );

            double v = widget->m_sbAmount->value();
            newDp->setAmount( v );
            widget->setCurrentPrice( newDp->overallPrice() );
          }

          if ( calculatable ) {
            // copy information from the widget
            newDp->setToDelete( widget->deleted() );

            QString t = widget->m_teFloskel->text();
            if ( !replaceMap.empty() ) {
              t = KraftDB::self()->replaceTagsInWord( t, replaceMap );
            }
            newDp->setText( t );

            QString h = widget->m_cbUnit->currentText();
            int eId   = UnitManager::getUnitIDSingular( h );
            Einheit e = UnitManager::getUnit( eId );
            newDp->setUnit( e );

            PositionViewWidget::Kind k = widget->kind();

            if ( k != PositionViewWidget::Normal ) {
              Attribute a( DocPosition::Kind );
              a.setPersistant( true );
              a.setValue( widget->kindString() );
              newDp->setAttribute( a );
            } else {
              newDp->removeAttribute( DocPosition::Kind );
            }

            /* set Attribute with the tags */
            QStringList tagStrings = widget->tagList();
            if ( !tagStrings.isEmpty() ) {
              Attribute tags( DocPosition::Tags );
              tags.setValueRelation( "tagTemplates", "tagTmplID", "name" );
              tags.setPersistant( true );
              tags.setListValue( true );
              tags.setValue( QVariant( tagStrings ) );
              newDp->setAttribute( tags );
              // kdDebug() << "============ " << tags.toString() << endl;
            } else {
              newDp->removeAttribute( DocPosition::Tags );
            }

            // tax settings
            newDp->setTaxType( currentTaxSetting() );
            list.append( newDp );
          }
        } else {
          kdError() << "Fatal: Widget without position found!" << endl;
        }
      }
      // kdDebug() << " Post List Count: " << list.count() << endl;

      if ( preListCnt == list.count() ) {
        kdError() << "No progress in widget list processing - abort!" << endl;
        progress = false;
      }
    }
    return list;
}

void KraftView::slotShowCatalog( bool on )
{
  if ( on ) {
    mAssistant->slotShowCatalog();
  } else {
    mAssistant->setFullPreview( true, KraftDoc::Positions );
  }
}

void KraftView::slotModifiedPositions()
{
  kdDebug() << "Position Modified" << endl;
  mModified = true;
}

void KraftView::slotModifiedHeader()
{
  kdDebug() << "Modified the header!" << endl;
  mModified = true;

  QTimer::singleShot( 0, this, SLOT( refreshPostCard() ) );
}

void KraftView::slotModifiedFooter()
{
  kdDebug() << "Modified the footer!" << endl;
  mModified = true;

  QTimer::singleShot( 0, this, SLOT( refreshPostCard() ) );
}

QStringList KraftView::generateLetterHead( Addressee adr )
{
    QStringList s;

    KraftDB::StringMap m;
    m[ "%NAME"]       = adr.familyName();
    m[ "%GIVEN_NAME"] = adr.givenName();

    return KraftDB::self()->wordList( "salut", m );
}

KraftDoc *KraftView::getDocument() const
{
  return m_doc;
}

void KraftView::done( int r )
{
  kdDebug() << "View closed with ret value " << r << endl;
  KraftDoc *doc = getDocument();
  if( doc )
    doc->removeView( this );
  KDialogBase::done( r );
}

void KraftView::slotOk()
{
    kdDebug() << "Accept Slot hit!" << endl;

    KraftDoc *doc = getDocument();

    if( !doc ) {
      kdDebug() << "ERR: No document available in view, return!" << endl;
      return;
    }
    // transfer all values to the document
    doc->setDate( m_headerEdit->m_dateEdit->date() );
    doc->setAddressUid( mContactUid );
    doc->setAddress(  m_headerEdit->m_postAddressEdit->text() );
    doc->setDocType(  m_headerEdit->m_cbType->currentText() );
    doc->setPreText(  m_headerEdit->m_teEntry->text() );
    doc->setWhiteboard( m_headerEdit->m_whiteboardEdit->text() );
    doc->setProjectLabel( m_headerEdit->mProjectLabelEdit->text() );
    doc->setSalut(    m_headerEdit->m_letterHead->currentText() );
    doc->setPostText( m_footerEdit->m_teSummary->text() );
    doc->setGoodbye(  m_footerEdit->m_cbGreeting->currentText() );

    DocPositionList list = currentPositionList();
    doc->setPositionList( list );

    doc->saveDocument( );

    if ( doc->isNew() ) {
      // For new documents the user had to select a greeting and we make this
      // default for the future
      KraftSettings::self()->setGreeting( m_footerEdit->m_cbGreeting->currentText() );
      KraftSettings::self()->setSalut( m_headerEdit->m_letterHead->currentItem() );
    }

    KraftSettings::self()->setDocViewSplitter( mCSplit->sizes() );
    KraftSettings::self()->setDocViewSize( size() );
    KraftSettings::self()->setDocViewPosition( pos() );
    KraftSettings::self()->writeConfig();

    // Save newly created templates
    if ( mNewTemplates.count() > 0 ) {
      for ( CatalogTemplate *ct = mNewTemplates.first(); ct; ct = mNewTemplates.next() ) {
        ct->save();
      }
      mNewTemplates.clear();
      // reload the entire katalog
      Katalog *defaultKat = KatalogMan::self()->defaultTemplateCatalog();
      KatalogMan::self()->notifyKatalogChange( defaultKat , dbID() );
    }

    emit viewClosed( true, m_doc );
    KDialogBase::slotOk(  );
}

void KraftView::slotFocusPosition( PositionViewWidget *posWidget, int pos )
{
  kdDebug() << "Focussing on widget " << posWidget << " on pos " << pos << endl;
  if( posWidget && pos > 0) {
    int w = posWidget->height();
    m_positionScroll->ensureVisible( 0, (pos)*w, 0, w );
  } else {
    m_positionScroll->ensureVisible( 0, 0 );
  }
  if( posWidget ) {
    if( posWidget->m_teFloskel->text().isEmpty() ) {
      posWidget->m_teFloskel->setFocus();
    } else {
      posWidget->m_sbAmount->setFocus();
    }
  }
}

void KraftView::slotCancel()
{
  // We need to reread the document
  if ( mModified ) {
    if ( KMessageBox::warningContinueCancel( this, i18n( "The document was modified. Do "
                                                "you really want to discard all changes?" ),
           i18n( "Document Modified" ), KGuiItem( i18n( "Discard" ) ) )
          == KMessageBox::Cancel  )
    {
      return;
    }
  }

  KraftDoc *doc = getDocument();
  if( doc && doc->isModified() ) {
    kdDebug() << "Document refetch from database" << endl;
    doc->reloadDocument();
  }

  mNewTemplates.clear();

  emit viewClosed( false, 0 );
  KDialogBase::slotCancel();
}

void KraftView::print(QPrinter * /* pPrinter */ )
{
    // create a archive document and start printing
    // ArchivedDoc *archDoc = doc->archive();

}

#include "kraftview.moc"
