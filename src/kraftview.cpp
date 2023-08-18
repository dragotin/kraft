/***************************************************************************
                 kraftview.cpp  - Interactive document view
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
#include <QLayout>
#include <QLabel>
#include <QScrollArea>
#include <QStackedWidget>
#include <QSizePolicy>
#include <QTextEdit>
#include <QSignalMapper>
#include <QTabWidget>
#include <QColor>
#include <QSplitter>
#include <QToolTip>
#include <QFont>
#include <QResizeEvent>
#include <QPalette>
#include <QTimer>
#include <QScrollBar>
#include <QComboBox>
#include <QDateTimeEdit>
#include <QMessageBox>

#include <QDebug>
#include <QDialog>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QVBoxLayout>

// application specific includes
#include "kraftdb.h"
#include "kraftsettings.h"
#include "kraftview.h"
#include "kraftdoc.h"
#include "tagman.h"
#include "ui_docheader.h"
#include "unitmanager.h"
#include "docassistant.h"
#include "positionviewwidget.h"
#include "ui_docfooter.h"
#include "docposition.h"
#include "unitmanager.h"
#include "documentman.h"
#include "docpostcard.h"
#include "katalogman.h"
#include "catalogselection.h"
#include "kraftdocheaderedit.h"
#include "kraftdocpositionsedit.h"
#include "kraftdocfooteredit.h"
#include "inserttempldialog.h"
#include "defaultprovider.h"
#include "stockmaterial.h"
#include "templtopositiondialogbase.h"
#include "doctype.h"
#include "catalogtemplate.h"
#include "importitemdialog.h"
#include "addressprovider.h"
#include "addressselectordialog.h"
#include "format.h"
#include "stringutil.h"

#define NO_TAX   0
#define RED_TAX  1
#define FULL_TAX 2
#define INDI_TAX 3

KraftView::KraftView(QWidget *parent) :
  KraftViewBase( parent ),
  mHelpLabel(nullptr), mRememberAmount( -1 ), mModified( false ),
  mTaxBefore( -1 ), mDocPosEditorIndx( -1 )
{
  setWindowTitle( i18n("Document" ) );
  setModal( false );
  QVBoxLayout *mainLayout = new QVBoxLayout;
  setLayout(mainLayout);


  mDetailHeader = new QLabel;
  mDetailHeader->setSizePolicy( QSizePolicy( QSizePolicy::Minimum, QSizePolicy::Fixed ) );
  mDetailHeader->setFrameStyle( QFrame::Box + QFrame::Plain );
  mDetailHeader->setLineWidth( 1 );
  mDetailHeader->setAutoFillBackground(true);

  mAddressProvider = new AddressProvider( this );
  connect( mAddressProvider, SIGNAL(lookupResult(QString,KContacts::Addressee)),
           this, SLOT( slotAddresseeFound(QString,KContacts::Addressee)));

  QPalette palette;
  palette.setColor(mDetailHeader->backgroundRole(), QColor( "darkBlue" ));
  palette.setColor(mDetailHeader->foregroundRole(), QColor( "white "));
  mDetailHeader->setPalette( palette );
  mDetailHeader->setTextFormat( Qt::PlainText );
  mDetailHeader->setFixedHeight( 40 ); // FIXME
  QFont f = mDetailHeader->font();
  f.setPointSize( qRound( 1.4 * f.pointSize() ) );
  f.setBold( true );
  mDetailHeader->setFont( f );
  mainLayout->addWidget( mDetailHeader );

  mCSplit    = new QSplitter(this);
  mainLayout->addWidget( mCSplit );

  mViewStack = new QStackedWidget;
  mCSplit->addWidget( mViewStack );

  // qDebug () << "mViewSTack height is " << mViewStack->height();

  mAssistant = new DocAssistant( mCSplit );
  mCSplit->addWidget( mAssistant );

  /* catalog template selection signal */
  connect(mAssistant, &DocAssistant::templatesToDocument, this, &KraftView::slotAddItems);

  /* signal to toggle the visibility of the template section in the assistant */
  connect(mAssistant, &DocAssistant::toggleShowTemplates, this, &KraftView::slotShowTemplates);

  /* signal that brings a new address to the document */
  connect(mAssistant, &DocAssistant::headerTextTemplate, this, &KraftView::slotNewHeaderText);
  connect(mAssistant, &DocAssistant::footerTextTemplate, this, &KraftView::slotNewFooterText);

  connect(mAssistant, &DocAssistant::selectPage, this, &KraftView::slotSwitchToPage);

  mAssistant->slotSelectDocPart( KraftDoc::Part::Header );

  setupMappers();

  QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
  QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
  okButton->setDefault(true);
  okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
  connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
  connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
  mainLayout->addWidget(buttonBox);


}

KraftView::~KraftView()
{
    // qDebug () << "KRAFTVIEW going away.";
}

void KraftView::setupMappers()
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
  // block signals as long as the widget is built up.
  // unblocking happens in redrawDocument()
  mModifiedMapper->blockSignals(true);
}

void KraftView::setup( DocGuardedPtr doc )
{
  KraftViewBase::setup(doc);

  if ( KraftSettings::self()->docViewSplitter().count() == 2 ) {
    mCSplit->setSizes( KraftSettings::self()->docViewSplitter() );
  }

  setupDocHeaderView();
  setupItems();
  setupFooter();
  setWindowTitle( m_doc->docIdentifier() );
  slotSwitchToPage( KraftDoc::Part::Header );

  if( doc->isNew() ) {
      mModified = true;
  }
}


void KraftView::slotSwitchToPage(KraftDoc::Part p)
{
    // check if the wanted part is already visible
    int id{0}; // Header
    if (p == KraftDoc::Part::Positions) id = 1;
    if (p == KraftDoc::Part::Footer) id = 2;

    if ( mViewStack->currentWidget() == mViewStack->widget( id ) ) return;

    mViewStack->setCurrentIndex( id );

    KraftDocEdit *edit =
            static_cast<KraftDocEdit *>( mViewStack->currentWidget() );

    mDetailHeader->setText( edit->title() );

    QPalette palette;
    palette.setColor(mDetailHeader->backgroundRole(), edit->color());
    // FIXME: color
    palette.setColor(mDetailHeader->foregroundRole(), QColor( "#00008b" ));
    mDetailHeader->setPalette( palette );

    mAssistant->slotSelectDocPart(p);
}

void KraftView::slotShowTemplates( bool )
{
}

void KraftView::setupDocHeaderView()
{
    KraftDocHeaderEdit *edit = new KraftDocHeaderEdit(this);

    mHeaderId = mViewStack->addWidget( edit ); // , KraftDoc::Header );

    m_headerEdit = edit->docHeaderEdit();

    m_headerEdit->m_cbType->clear();
    // m_headerEdit->m_cbType->insertStringList( DefaultProvider::self()->docTypes() );
    m_headerEdit->m_cbType->insertItems(-1, DocType::allLocalised() );
    m_headerEdit->mButtLang->hide();

    const QString predecessorDbId = m_doc->predecessorDbId();
    bool predecIsVisible = false;
    if( !predecessorDbId.isEmpty() ) {
        DocGuardedPtr predecDoc = DocumentMan::self()->openDocumentByIdent(predecessorDbId);
        if( predecDoc ) {
            const QString id = predecDoc->docIdentifier();
            const QString link = QString("<a href=\"doc://show?id=%1\">%2</a>").arg(predecessorDbId).arg(id);
            m_headerEdit->_labFollowup->setText( i18n("Successor of %1", link));
            predecIsVisible = true;
            connect( m_headerEdit->_labFollowup, SIGNAL(linkActivated(QString)),
                     this, SLOT(slotLinkClicked(QString)));
            delete predecDoc;
        }
    }
    m_headerEdit->_labFollowup->setVisible(predecIsVisible);

    connect( m_headerEdit->m_cbType,  SIGNAL( activated( const QString& ) ),
             this, SLOT( slotDocTypeChanged( const QString& ) ) );

    connect( m_headerEdit->mButtLang, SIGNAL( clicked() ),
             this, SLOT( slotLanguageSettings() ) );
    connect( edit, SIGNAL( modified() ),
              this, SLOT( slotModifiedHeader() ) );
    connect( edit, SIGNAL(pickAddressee()), this, SLOT(slotPickAddressee()) );
}

void KraftView::slotLinkClicked(const QString& link)
{
    QUrl u(link);
    if( u.scheme() == "doc" && u.host() == "show" ) {
        QUrlQuery uq(link);
        const QString ident = uq.queryItemValue("id");
        qDebug() << "Link clicked to open document " << ident;
        emit openROView( ident );
    }
}

void KraftView::setupItems()
{
    KraftDocPositionsEdit *edit = new KraftDocPositionsEdit(this);
    mDocPosEditorIndx = mViewStack->addWidget( edit ); // , KraftDoc::Positions );

    m_positionScroll = edit->positionScroll();

    connect( edit, SIGNAL( addPositionClicked() ), SLOT( slotAddNewItem() ) );
    connect( edit, SIGNAL( addExtraClicked() ), SLOT( slotAddExtraPosition() ) );
    connect( edit, SIGNAL( importItemsClicked() ), SLOT( slotImportItems() ) );

}

void KraftView::redrawDocument( )
{
    KraftDoc *doc = getDocument();
    if( !doc ) {
      // qDebug () << "ERR: No document available in view, return!";
    } else {
      // qDebug () << "** Starting redraw of document " << doc;
    }

    /* header: date and document type */
    QDate date = doc->date();
    m_headerEdit->m_dateEdit->setDate( date );
    m_headerEdit->m_cbType->setCurrentIndex(m_headerEdit->m_cbType->findText( doc->docType() ));

    /* header: address */
    mContactUid  = doc->addressUid();
    QString address = doc->address();

    // qDebug () << "Loaded address uid from database " << mContactUid;
    if( ! mContactUid.isEmpty() ) {
      mAddressProvider->lookupAddressee( mContactUid );
    }

    if( !address.isEmpty() ) {
      // custom address stored in the document.
      // qDebug() << "custom address came in: " << address;
      m_headerEdit->m_postAddressEdit->setText( address );
    }

    if( !doc->salut().isEmpty() ) {
      m_headerEdit->m_letterHead->insertItem(-1, doc->salut() );
      m_headerEdit->m_letterHead->setCurrentIndex(m_headerEdit->m_letterHead->findText( doc->salut() ));
    }
    /* pre- and post text */
    m_headerEdit->m_teEntry->setPlainText( doc->preTextRaw() );
    m_headerEdit->m_whiteboardEdit->setPlainText( doc->whiteboard() );
    m_headerEdit->mProjectLabelEdit->setText( doc->projectLabel() );
    m_footerEdit->ui()->m_teSummary->setPlainText( doc->postTextRaw() );
    const QString goodbye = doc->goodbye();
    m_footerEdit->slotSetGreeting(goodbye);

    mAssistant->slotSetDocType( doc->docType() );

    redrawDocPositions( );
    slotDocTypeChanged( doc->docType() );
    refreshPostCard();
    mModifiedMapper->blockSignals(false);

   // mModified = false;
}

void KraftView::slotPickAddressee()
{
    AddressSelectorDialog dialog(this);

    if( dialog.exec() ) {
        const KContacts::Addressee contact = dialog.addressee();
        slotNewAddress( contact );
    }
}

void KraftView::slotAddresseeFound( const QString& uid, const KContacts::Addressee& contact )
{
    if( contact.isEmpty() ) {
        const QString err = mAddressProvider->errorMsg(uid);
        if( !err.isEmpty() ) {
            qDebug () << "Error while looking up address for uid" << uid << ":" << err;
        }
    } else {
        slotNewAddress( contact, false );
        qDebug () << "No contact found for uid " << uid;
    }
}

void KraftView::slotNewAddress( const KContacts::Addressee& contact, bool interactive )
{

    Addressee adr( contact );

    if( contact.isEmpty() ) {
        return;
    }
    QString newAddress = mAddressProvider->formattedAddress( contact );
    const QString currAddress = m_headerEdit->m_postAddressEdit->toPlainText();

    bool replace = true;
    m_headerEdit->m_labName->setText( contact.realName() );

    if( currAddress.isEmpty() ) {
        replace = true;
    } else if( currAddress != newAddress ) {
        // non empty and current different from new address
        // need to ask first if we overwrite
        if( interactive ) {
            QMessageBox msgBox;
            msgBox.setText(i18n( "The address label is not empty and different from the selected one.<br/>"
                                 "Do you really want to replace it with the text shown below?<pre>%1</pre>", newAddress));

            msgBox.setStandardButtons(QMessageBox::Yes| QMessageBox::No);
            msgBox.setDefaultButton(QMessageBox::Yes);
            int ret = msgBox.exec();

            if ( ret != QMessageBox::Yes) {
                replace = false;
            }
        } else {
            // this happens when the document is loaded and the address arrives from addressbook
            replace = false;
        }
    } else if( currAddress == newAddress ) {
        // both are equal, no action needed
        replace = false;
    }

    if( replace ) {
        mContactUid = contact.uid();

        m_headerEdit->m_postAddressEdit->setText( newAddress );

        // Generate the welcome
        m_headerEdit->m_letterHead->clear();
        QStringList li = generateLetterHead(adr.familyName(), adr.givenName());

        m_headerEdit->m_letterHead->insertItems(-1, li );
        m_headerEdit->m_letterHead->setCurrentIndex( KraftSettings::self()->salut() );
    }
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
      mHelpLabel = new QLabel(this);
      mHelpLabel->setTextFormat(Qt::RichText);
      // mHelpLabel->setMinimumHeight(400);
//TODO PORT QT5       mHelpLabel->setMargin( QDialog::marginHint() );
      mHelpLabel->setText( i18n( "<qt><h2>The Document Items List is still empty, but Items "
                                 "can be added now.</h2>"
                                 "To add items to the document either "
                                 "<ul>"
                                 "<li>Press the 'Add item' button above.</li>"
                                 "<li>Open the template catalog by clicking on the 'show Template' "
                                  "button on the right and pick one of the available templates.</li>"
                                   "</ul></qt>" ) );
      mHelpLabel->setWordWrap(true);
      mHelpLabel->setMinimumHeight(200);
      m_positionScroll->addChild( mHelpLabel, 0);
    }
    return;

  } else {
    if ( mHelpLabel ) {
      delete mHelpLabel;
      mHelpLabel = nullptr;
    }
  }

  // qDebug () << "starting to redraw " << list.count() << " positions!";
  int cnt = 0;
  DocPositionListIterator it( list );
  while( it.hasNext() ) {
    DocPositionBase *dp = it.next();
    PositionViewWidget *w = mPositionWidgetList.widgetFromPosition(dp);
    if( !w ) {
      w = createPositionViewWidget( dp, cnt);
      w->slotAllowIndividualTax( currentTaxSetting() == DocPositionBase::TaxIndividual );
    }
    cnt++;
    // qDebug () << "now position " << dp->positionNumber();
  }

  // now go through the positionWidgetMap and check if it contains elements
  // that are not any longer in the list of positions

  QMutableListIterator<PositionViewWidget*> it2( mPositionWidgetList );
  while( it2.hasNext() ) {
    PositionViewWidget *w = it2.next();

    if( w && (list.contains( w->position() ) == 0) ) {
      // qDebug () << "Removing this one: " << w;
      m_positionScroll->removeChild( w );
      it2.remove();
      // mPositionWidgetList.erase( it2 );
      break;
    }
  }
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
  
  int cw = m_positionScroll->width();
  if ( cw < 400 ) cw = 400;
  w->resize( cw, w->height() );

  connect( w, SIGNAL( deletePosition() ),  mDeleteMapper, SLOT( map() ) );
  connect( w, SIGNAL( moveUp() ),          mMoveUpMapper, SLOT( map() ) );
  connect( w, SIGNAL( moveDown() ),        mMoveDownMapper, SLOT( map() ) );
  connect( w, SIGNAL( lockPosition() ),    mLockPositionMapper, SLOT( map() ) );
  connect( w, SIGNAL( unlockPosition() ),  mUnlockPositionMapper, SLOT( map() ) );
  connect( w, SIGNAL( positionModified()), mModifiedMapper,  SLOT( map() ) );

  setMappingId( w, pos );

  QStringList units = UnitManager::self()->allUnits();
  units.sort();
  w->m_cbUnit->addItems(units);

  if( dp->dbId().toInt() < 0 ) {
    w->slotSetState( PositionViewWidget::New );
  }

  /* do resizing and add the widget to the scrollview and move it to the final place */

  if ( mHelpLabel ) {
    mHelpLabel->hide();
  }

  mPositionWidgetList.insert( pos,  w );
  m_positionScroll->addChild( w, pos );

  //Set the correct indexes when the widget is not appended
  if(pos < mPositionWidgetList.count())
  {
    for(int i = pos+1; i < mPositionWidgetList.count(); ++i)
    {
      mPositionWidgetList.at(i)->setOrdNumber(i+1);
      setMappingId(mPositionWidgetList.at(i), i);
    }
  }

  w->setDocPosition(dp);
  w->setOrdNumber( pos+1 );
  w->slotSetTax( dp->taxType() );
  return w;
}

DocPositionBase::TaxType KraftView::currentTaxSetting()
{
  // add 1 to the currentItem since that starts with zero.
  int taxKind = 1+( m_footerEdit->ui()->mTaxCombo->currentIndex() );
  DocPositionBase::TaxType tt = DocPositionBase::TaxInvalid;

  if ( taxKind == 1 ) { // No Tax at all
    tt = DocPositionBase::TaxNone;
  } else if ( taxKind == 2 ) { // Reduced tax for all items
    tt = DocPositionBase::TaxReduced;
  } else if ( taxKind == 3 ) { // Full tax for all items
    tt = DocPositionBase::TaxFull;
  } else { // individual level
    tt = DocPositionBase::TaxIndividual;
  }
  return tt;
}

void KraftView::refreshPostCard()
{
  DocPositionList positions = currentPositionList();

  KraftDoc *kraftDoc = getDocument();
  if( !kraftDoc) return;

  if ( mAssistant->postCard() ) {
    QDate d = m_headerEdit->m_dateEdit->date();
    const QString dStr = Format::toDateString( d, KraftSettings::self()->dateFormat() );

    double fullTax = DocumentMan::self()->tax(d);
    double redTax = DocumentMan::self()->reducedTax(d);
    const QString preText = kraftDoc->resolveMacros(m_headerEdit->m_teEntry->toPlainText(),
                                                    positions, d, fullTax, redTax);
    const QString postText = kraftDoc->resolveMacros(m_footerEdit->ui()->m_teSummary->toPlainText(),
                                                     positions, d, fullTax, redTax);
    mAssistant->postCard()->setHeaderData( m_headerEdit->m_cbType->currentText(),
                                           dStr, m_headerEdit->m_postAddressEdit->toPlainText(),
                                           getDocument()->ident(),
                                           preText );


    mAssistant->postCard()->setPositions( positions,  currentTaxSetting(),
                                          UnitManager::self()->tax( d ),
                                          UnitManager::self()->reducedTax( d ) );

    mAssistant->postCard()->setFooterData( postText,
                                           m_footerEdit->greeting() );

    KraftDoc::Part p {KraftDoc::Part::Header};
    if (mViewStack->currentIndex() == 1) p = KraftDoc::Part::Positions;
    if (mViewStack->currentIndex() == 2) p = KraftDoc::Part::Footer;

    mAssistant->postCard()->renderDoc(p);
  }


  DocPositionListIterator it( positions );
  DocPositionBase *dp;
  while( it.hasNext()) {
    dp = it.next();

    if (  dp->type() == DocPositionBase::ExtraDiscount ) {
      PositionViewWidget *w = ( static_cast<DocPosition*>( dp ) )->associatedWidget();
      if( w ) {
        w->slotSetOverallPrice( ( static_cast<DocPosition*>( dp ) )->overallPrice() );
      } else {
        // qDebug () << "Warning: Position object has no associated widget!";
      }
    }
  }
}

void KraftView::setupFooter()
{
  m_footerEdit = new KraftDocFooterEdit(this);

  mViewStack->addWidget( m_footerEdit ); //  KraftDoc::Footer );

  // ATTENTION: If you change the following inserts, make sure to check the code
  //            in method currentPositionList!
  m_footerEdit->ui()->mTaxCombo->insertItem( NO_TAX,   DefaultProvider::self()->icon("circle-0"), i18n( "Display no tax at all" ));
  m_footerEdit->ui()->mTaxCombo->insertItem( RED_TAX,  DefaultProvider::self()->icon("circle-half-2"), i18n( "Calculate reduced tax for all items" ));
  m_footerEdit->ui()->mTaxCombo->insertItem( FULL_TAX, DefaultProvider::self()->icon("coin"), i18n( "Calculate full tax for all items" ));
  m_footerEdit->ui()->mTaxCombo->insertItem( INDI_TAX, i18n( "Calculate individual tax for each item"));

  // set the tax type combo correctly: If all items have the same tax type, take it.
  // If items have different, its the individual thing.
  DocPositionList list = m_doc->positions();

  int tt = -1;
  DocPositionBase *dp = nullptr;

  DocPositionListIterator it( list );
  int taxIndex = 0;

  while( it.hasNext()) {
    dp = it.next();
    if ( tt == -1 )
      tt = dp->taxTypeNumeric(); // store the first entry.
    else {
      if ( tt != dp->taxTypeNumeric() ) {
        taxIndex = INDI_TAX;
      } else {
        // old and new taxtype are the same.
      }
    }
  }
  if ( tt == -1 ) {
    // means that there is no item yet, the default tax type needs to be used.
    int deflt = KraftSettings::self()->defaultTaxType();
    if ( deflt > 0 ) {
      deflt -= 1;
    }
    taxIndex = deflt;
  } else {
    if ( taxIndex == 0 ) {
      taxIndex = tt-1;
    }
  }

  connect( m_footerEdit->ui()->mTaxCombo,SIGNAL(currentIndexChanged(int)),this,SLOT(slotTaxComboChanged(int)));

  m_footerEdit->ui()->mTaxCombo->setCurrentIndex( taxIndex );
  slotTaxComboChanged( taxIndex );

  mTaxBefore = taxIndex;

  connect(m_footerEdit,SIGNAL(modified()),this,SLOT(slotModifiedFooter()));
}

void KraftView::slotTaxComboChanged(int newId)
{
  bool allowTaxSetting = false;

  if( mTaxBefore == newId ) return;

  if( mTaxBefore == INDI_TAX ) {
    // we're changing away from individual settings. WARNING needed.
    // qDebug () << "You switch away from item individual tax settings.";
    if( QMessageBox::question( this,
                               i18n("Tax Settings Overwrite"),
                               i18n("Really overwrite all individual tax settings of the items?")
                               ) == QMessageBox::No ) {
      m_footerEdit->ui()->mTaxCombo->setCurrentIndex( INDI_TAX );
      return;
    }
  }
  DocPositionBase::TaxType tax = DocPositionBase::TaxFull;

  if( newId == INDI_TAX ) {
    allowTaxSetting = true;
  } else if( newId == RED_TAX ) {
    tax = DocPositionBase::TaxReduced;
  } else if( newId == NO_TAX ) {
    tax = DocPositionBase::TaxNone;
  }

  PositionViewWidgetListIterator it( mPositionWidgetList );
  while( it.hasNext() ) {
    PositionViewWidget *w = it.next();
    w->slotAllowIndividualTax( allowTaxSetting );
    w->slotSetTax( tax );
  }

  mTaxBefore = newId;
  slotModifiedFooter();
}

/* This is the flow in the move up method:
         0    Bla1          0 Bla1                         0 Bla1
         1    Bla2 <- w2    1 Bla2   -> insert at pos-1 => 1 Bla3
 pos ->  2    Bla3 <- w1    2 Bla4                         2 Bla2
         3    Bla4                                         3 Bla4
 */
void KraftView::slotMovePositionUp( int pos )
{
  PositionViewWidget *w1 = nullptr;
  PositionViewWidget *w2 = nullptr;

  // qDebug () << "Moving position up: " << pos;
  if( pos < 1 || pos > mPositionWidgetList.count() ) {
    // qDebug () << "ERR: position out of range: " << pos;
    return;
  }

  mPositionWidgetList.swap( pos, pos-1 );
  w1 = mPositionWidgetList.at( pos-1 );
  w2 = mPositionWidgetList.at( pos ); // Porting ATTENTION: check assignment of w1, w1

  // qDebug () << "Found at pos " << pos << " the widgets " << w1 << " and " << w2;

  if( w1 && w2 ) {
    // qDebug () << "Setting ord number: " << pos;
    w1->setOrdNumber( pos );  // note: ordnumbers start with 1, thus add one
    w2->setOrdNumber( pos+1 );
    setMappingId( w1, pos-1 );
    setMappingId( w2, pos );

    m_positionScroll->moveChild( w2, m_positionScroll->indexOf(w1) );
    mModified = true;
    QTimer::singleShot( 0, this, SLOT(refreshPostCard()  ) );
  } else {
    // qDebug () << "ERR: Did not find the two corresponding widgets!";
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
  PositionViewWidget *w1 = nullptr;
  PositionViewWidget *w2 = nullptr;
  // qDebug () << "Moving position down: " << pos;

  if( pos < 0 || pos >= mPositionWidgetList.count() -1 ) {
    // qDebug () << "ERR: position out of range: " << pos;
    return;
  }

  mPositionWidgetList.swap( pos, pos+1);
  w1 = mPositionWidgetList.at( pos+1 );
  w2 = mPositionWidgetList.at( pos );  // Porting ATTENTION: check assignment of w1, w1

  if( w1 && w2 ) {
    w1->setOrdNumber( pos+2  );
    w2->setOrdNumber( pos+1 );

    setMappingId( w1, pos+1 );
    setMappingId( w2, pos );

    m_positionScroll->moveChild( w1, m_positionScroll->indexOf( w2 ) );

    mModified = true;
    QTimer::singleShot( 0, this, SLOT( refreshPostCard() ) );
  } else {
    // qDebug () << "ERR: Did not find the two corresponding widgets!";
  }
}

void KraftView::slotDeletePosition( int pos )
{
  PositionViewWidget *w1 = mPositionWidgetList.at( pos );
  if( w1 ) {
    w1->slotSetState( PositionViewWidget::Deleted );
    w1->slotModified();
  }
}

void KraftView::slotLockPosition( int pos )
{
  // qDebug () << "Locking Position " << pos;

  PositionViewWidget *w1 = mPositionWidgetList.at( pos );
  if( w1 ) {
    w1->slotSetState( PositionViewWidget::Locked );
    refreshPostCard();
  }
}

void KraftView::slotUnlockPosition( int pos )
{
  // qDebug () << "Unlocking Position " << pos;

  PositionViewWidget *w1 = mPositionWidgetList.at( pos );
  if( w1 ) {
    w1->slotSetState( PositionViewWidget::Active );
    refreshPostCard();
  }
}

void KraftView::slotPositionModified( int )
{
    // qDebug () << "Modified Position " << pos;
    mModified = true;

    QTimer::singleShot( 0, this, SLOT( refreshPostCard() ) );
}

void KraftView::slotAddressFound(const QString& uid, const KContacts::Addressee& contact)
{
    const QString currAddress = m_headerEdit->m_postAddressEdit->toPlainText();
    const QString newAddress = mAddressProvider->formattedAddress(contact);
    bool replace = true;

    if( currAddress.isEmpty() ) {
        replace = true;
    } else if(currAddress != newAddress) {
        // non empty and current different from new address
        // need to ask first if we overwrite
        if( QMessageBox::question( this, i18n("Address Overwrite"),
                                   i18n("The address label is not empty and different from the selected one.<br/>"
                                        "Do you really want to replace it with the text shown below?<pre>%1</pre>", 
					newAddress)
                                   ) == QMessageBox::No ) {
            replace = false;
        }
    } else if( currAddress == newAddress ) {
        // both are equal, no action needed
        return;
    }

    if( replace ) {
        mContactUid = uid;
        m_headerEdit->m_labName->setText( contact.realName() );

        m_headerEdit->m_postAddressEdit->setText( newAddress );

        // Generate the welcome
        m_headerEdit->m_letterHead->clear();
        QStringList li = generateLetterHead( contact.familyName(), contact.givenName() );

        m_headerEdit->m_letterHead->insertItems(-1, li );
        KraftDoc *doc = getDocument();
        if( doc->isNew() ) {
            m_headerEdit->m_letterHead->setCurrentIndex( KraftSettings::self()->salut() );
        } else {
            if(!doc->salut().isEmpty()) {
                m_headerEdit->m_letterHead->setCurrentText( doc->salut() );
            }
        }
    }
}

void KraftView::slotDocTypeChanged( const QString& newType )
{
  // qDebug () << "Doc Type changed to " << newType;
  mAssistant->slotSetDocType( newType );

  DocType docType( newType );

  PositionViewWidgetListIterator it( mPositionWidgetList );
  while( it.hasNext() ) {
    PositionViewWidget *w = it.next();
    w->slotEnableKindMenu( docType.allowAlternative() );
    w->slotShowPrice(docType.pricesVisible());
  }

  mAssistant->postCard()->slotShowPrices( docType.pricesVisible() );
  m_footerEdit->ui()->_taxGroup->setVisible( docType.pricesVisible() );
  if( mDocPosEditorIndx > -1 ) {
      KraftDocPositionsEdit *w = dynamic_cast<KraftDocPositionsEdit*>(mViewStack->widget(mDocPosEditorIndx));
      if(w) {
          w->setDiscountButtonVisible(docType.pricesVisible());
      }
  }
}

void KraftView::slotLanguageSettings()
{
  // qDebug () << "Language Settings";
  // FIXME 
}

void KraftView::slotNewHeaderText( const DocText& dt, bool replace )
{
    if (replace) {
        m_headerEdit->m_teEntry->setPlainText( dt.text());
    } else {
        m_headerEdit->m_teEntry->insertPlainText(dt.text());
    }
    slotModifiedHeader();
}

void KraftView::slotNewFooterText( const DocText& dt, bool replace )
{
  if (replace) {
      m_footerEdit->ui()->m_teSummary->setPlainText(dt.text());
  } else {
      m_footerEdit->ui()->m_teSummary->insertPlainText(dt.text());
  }
  slotModifiedFooter();
}

// Add a new item. A katalog is required if user wants to store it in a
// catalog immediately. FIXME - now the current active catalog in the
// catalog selection is used.
void KraftView::slotAddNewItem()
{
  Katalog* kat = mAssistant->catalogSelection()->currentSelectedKat();
  slotAddItem( kat, nullptr, QString() );
}

void KraftView::slotAddItems( Katalog *kat, CatalogTemplateList templates, const QString& selectedChapter)
{
    if(templates.count() == 0) {
        slotAddItem(kat, nullptr, selectedChapter);
    } else {
        for(CatalogTemplate *templ : templates ) {
            slotAddItem( kat, templ, selectedChapter );
        }
    }
}

void KraftView::slotAddItem( Katalog *kat, CatalogTemplate *tmpl, const QString& selectedChapter )
{
    // newpos is a list position, starts counting at zero!
    int newpos = mPositionWidgetList.count();
    // qDebug () << "Adding Position at list position " << newpos;

    QScopedPointer<TemplToPositionDialogBase> dia;

    DocPosition *dp = new DocPosition();
    dp->setPositionNumber( newpos +1 );

    bool newTemplate = false;
    if ( !tmpl ) {
        newTemplate = true;
    }

    dia.reset(new InsertTemplDialog( this ));
    dia->setCatalogChapters( kat->getKatalogChapters(), selectedChapter);

    int tmplId = 0;

    if ( !newTemplate ) {
        //  it's not a new template
        dp->setText(tmpl->getText());
        dp->setUnit(tmpl->unit());
        dp->setUnitPrice(tmpl->unitPrice());
    }

    if ( mRememberAmount > 0 ) {
        dp->setAmount( mRememberAmount );
    }

    KraftDoc *doc = getDocument();
    if(doc) {
        DocType docType = doc->docType();
        dia->setDocPosition( dp, newTemplate, docType.pricesVisible() );
    }
    DocPositionList list = currentPositionList();
    dia->setPositionList( list, newpos );

    QSize s = KraftSettings::self()->templateToPosDialogSize();
    dia->resize( s );

    int execResult = dia->exec();

    if ( execResult == QDialog::Accepted ) {
        DocPosition diaPos = dia->docPosition();
        *dp = diaPos;

        // set the tax settings
        if( currentTaxSetting() == DocPositionBase::TaxIndividual ) {
            // FIXME: In case a new item is added, add the default tax type.
            // otherwise add the tax of the template
            dp->setTaxType( DocPositionBase::TaxFull );
        } else {
            dp->setTaxType( currentTaxSetting() );
        }

        // store the initial size of the template-to-doc-pos dialogs
        s = dia->size();
        KraftSettings::self()->setTemplateToPosDialogSize( s );

        if ( kat->type() == TemplateCatalog ) {

            // save the template if it is has a valid chapter.
            // the method chapter() considers the checkbox. If it is not checked to keep the template,
            // an empty chapter is returned.

            const QString chapter = dia->chapter();
            if (!chapter.isEmpty()) {
                int chapterId = KatalogMan::self()->defaultTemplateCatalog()->chapterID(chapter).toInt();

                FloskelTemplate *flos = new FloskelTemplate( -1, dp->text(),
                                                             dp->unit().id(),
                                                             chapterId,
                                                             1 /* CalcKind = Manual */ );
                flos->setManualPrice( dp->unitPrice() );
                flos->save();
                tmplId = flos->getTemplID();

                // reload the entire katalog
                Katalog *defaultKat = KatalogMan::self()->defaultTemplateCatalog();
                if( defaultKat ) {
                    defaultKat->load();
                    KatalogMan::self()->notifyKatalogChange( defaultKat , dbID() );
                }
            } else if (!newTemplate && tmpl){
                tmplId = static_cast<FloskelTemplate*>(tmpl)->getTemplID();
            }
        } else if ( kat->type() == MaterialCatalog ) {
            if ( !newTemplate ) {
                tmplId = static_cast<StockMaterial*>(tmpl)->getID();
            }
        }
    }

    if (execResult == QDialog::Accepted) {
        newpos = dia->insertAfterPosition();

        mRememberAmount = dp->amount();

        if (tmplId > 0 && tmpl) {
            QPair<int, QDateTime> newUsage = kat->recordUsage(tmplId);
            tmpl->setUseCounter(newUsage.first);
            tmpl->setLastUsedDate(newUsage.second);
        }

        PositionViewWidget *widget = createPositionViewWidget( dp, newpos );
        widget->slotModified();
        widget->slotAllowIndividualTax( currentTaxSetting() == DocPositionBase::TaxIndividual );

        // Check if the new widget is supposed to display prices, based on the doc type
        // FIXME: Shouldn't this be done by the positionViewWidget rather than here?
        const QString dt = getDocument()->docType();
        if( !dt.isEmpty() ) {
            DocType docType(dt);
            widget->slotShowPrice(docType.pricesVisible());
        }
        slotFocusItem( widget, newpos );
        refreshPostCard();
    }
}

void KraftView::slotImportItems()
{
  ImportItemDialog dia( this );
  DocPositionList list = currentPositionList();
  int newpos = list.count();
  dia.setPositionList( list, newpos );

  if ( dia.exec() ) {
    DocPositionList list = dia.positionList();
    if ( list.count() > 0 ) {
      // qDebug () << "Importlist amount of entries: " << list.count();
      int cnt = 0;
      int newpos = dia.getPositionCombo()->currentIndex();
      // qDebug () << "Newpos is " << newpos;

      DocPositionListIterator posIt( list );
      while( posIt.hasNext() ) {
        DocPosition *dp_old = static_cast<DocPosition*>(posIt.next());

        DocPosition *dp = new DocPosition( *(dp_old) );
        dp->setTaxType( currentTaxSetting() );
        PositionViewWidget *widget = createPositionViewWidget( dp, newpos + cnt++ );
        widget->slotSetTax( DocPositionBase::TaxFull ); // FIXME: Value from Import?
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
  // qDebug () << "Adding EXTRA Position at position " << newpos;

  DocPosition *dp = new DocPosition( DocPosition::ExtraDiscount );
  dp->setPositionNumber( newpos+1 );
  dp->setText( i18n( "Discount" ) );
  Einheit e = UnitManager::self()->getPauschUnit();
  dp->setUnit(e);
  if( currentTaxSetting() == DocPositionBase::TaxIndividual ) {
    dp->setTaxType( DocPositionBase::TaxFull );
  } else {
    dp->setTaxType( currentTaxSetting() );
  }

  // qDebug () << "New Extra position is " << dp;

  PositionViewWidget *widget = createPositionViewWidget( dp, newpos );
  // qDebug () << "PositionViewWiget doc position is: " << widget->position();
  widget->slotModified();
  slotFocusItem( widget, newpos );
  refreshPostCard();

}

DocPositionList KraftView::currentPositionList()
{
    DocPositionList list;
    PositionViewWidget *widget = nullptr;
    int cnt = 1;

    PositionViewWidgetListIterator outerIt( mPositionWidgetList );

    bool progress = true;

    while ( progress && ( list.count() != mPositionWidgetList.count() ) ) {
        // the loop runs until all positions have a valid price.

        int preListCnt = list.count();
        // qDebug() << "# Pre List Count: " << preListCnt;

        while ( outerIt.hasNext() ) {
            widget = outerIt.next();
            DocPositionBase *dpb = widget->position();

            QMap<QString, QString> replaceMap;

            if ( dpb ) {
                // FIXME what about discount?
                // FIXME get rid of PostionViewWidget::Kind
                const PositionViewWidget::Kind k = widget->kind();
                DocPositionBase::PositionType t {DocPositionBase::PositionType::Position};
                if (k == PositionViewWidget::Kind::Demand) {
                    t = DocPositionBase::PositionType::Demand;
                } else if (k == PositionViewWidget::Kind::Alternative) {
                    t = DocPositionBase::PositionType::Alternative;
                } else if (k == PositionViewWidget::Kind::Demand) {
                    t = DocPositionBase::PositionType::Demand;
                }
                DocPosition *newDp = new DocPosition(t);

                newDp->setPositionNumber( cnt++ );
                for (const auto& a: dpb->attributes()) {
                    newDp->setAttribute(a);
                }
                newDp->setDbId( dpb->dbId().toInt() );
                newDp->setAssociatedWidget( widget );

                bool calculatable = true;

                if ( dpb->type() == DocPosition::ExtraDiscount ) {
                    double discount = widget->mDiscountPercent->value();

                    /* set Attributes with the discount percentage */
                    if (discount > 0.0) {
                        KraftAttrib a( DocPosition::Discount, discount, KraftAttrib::Type::Float);
                        newDp->setAttribute(a);
                    }

                    // get the required tag as String.
                    const QString tagRequiredStr = widget->extraDiscountTagRestriction();

                    if ( !tagRequiredStr.isEmpty() ) {
                        // save the required tag as KraftAttrib
                        const TagTemplate ttRequired = TagTemplateMan::self()->getTagTemplate(tagRequiredStr);
                        int trId = TagTemplateMan::self()->getTagTemplate(tagRequiredStr).dbId().toInt();
                        const KraftAttrib a(DocPosition::ExtraDiscountTagRequired, trId, KraftAttrib::Type::Integer);
                        newDp->setAttribute(a);
                    }

                    /* Calculate the current sum over all widgets */
                    PositionViewWidgetListIterator it( mPositionWidgetList );
                    PositionViewWidget *w1;
                    Geld sum;
                    // qDebug () << "Starting to loop over the items ";
                    while (  calculatable && it.hasNext() ) {
                        w1 = it.next();

                        if ( widget != w1 ) { // ATTENTION Porting: do not take the own value into account
                            if ( tagRequiredStr.isEmpty()  // means that all positions are to calculate
                                 || w1->tagList().contains( tagRequiredStr ) ) { // tagList() returns strings, not Ids.
                                if ( w1->priceValid() ) {
                                    sum += w1->currentPrice();
                                    // qDebug () << "Summing up pos with text " << w1->ordNumber() << " and price "
                                    // << w1->currentPrice().toLong();
                                } else {
                                    calculatable = false; // give up, we continue in outerIt
                                    // qDebug () << "We skip pos " << w1->ordNumber();
                                }
                            }
                        } else {
                            // we can not calculate ourselves.
                            // qDebug () << "Skipping pos " << w1->ordNumber() << " in summing up, thats me!";
                        }
                    }
                    // qDebug () << "Finished loop over items with calculatable=" << calculatable;

                    if ( calculatable ) {
                        sum = sum.percent( discount );
                        newDp->setUnitPrice( sum );
                        newDp->setAmount( 1.0 );
                        widget->setCurrentPrice( sum );
                    }

                    // replace some tags in the text

                    replaceMap["%DISCOUNT"]     = Format::localeDoubleToString(discount);
                    replaceMap["%ABS_DISCOUNT"] = Format::localeDoubleToString(qAbs(discount));

                } else {
                    /* Type is ordinary position */
                    newDp->setUnitPrice( widget->unitPrice() );

                    double v = widget->m_sbAmount->value();
                    newDp->setAmount( v );
                    widget->setCurrentPrice( newDp->overallPrice() );
                }

                if ( calculatable ) {
                    // copy information from the widget
                    bool deleted = widget->deleted();
                    newDp->setToDelete(deleted);

                    QString t = widget->m_teFloskel->toPlainText();
                    if ( !replaceMap.empty() ) {
                        t = StringUtil::replaceTagsInString(t, replaceMap);
                    }
                    newDp->setText( t );

                    QString h = widget->m_cbUnit->currentText();
                    int eId   = UnitManager::self()->getUnitIDSingular( h );
                    Einheit e = UnitManager::self()->getUnit( eId );
                    newDp->setUnit( e );

                    /* set the tags */
                    const QStringList tagStrings = widget->tagList();
                    newDp->setTags(tagStrings);
                    // qDebug() << "============ " << tags.toString() << endl;

                    // tax settings
                    if( currentTaxSetting() == DocPositionBase::TaxIndividual ) {
                        newDp->setTaxType( widget->taxType() );
                    } else {
                        newDp->setTaxType( currentTaxSetting() );
                    }
                    list.append( newDp );
                }
            } else {
                qCritical() << "Fatal: Widget without position found!";
            }
        }
        // qDebug() << " Post List Count: " << list.count();

        if ( preListCnt == list.count() ) {
            qCritical() << "No progress in widget list processing - abort!";
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
    mAssistant->setFullPreview( true, KraftDoc::Part::Positions );
  }
}

void KraftView::slotModifiedPositions()
{
    // As long as the modified mapper is blocked, ignore this.
    if( mModifiedMapper->signalsBlocked() ) {
        return;
    }

    qDebug () << "Position Modified";
    mModified = true;
}

void KraftView::slotModifiedHeader()
{
    // qDebug () << "Modified the header!";
    // As long as the modified mapper is blocked, ignore this.
    if( mModifiedMapper->signalsBlocked() ) {
        return;
    }

    mModified = true;

    QTimer::singleShot( 0, this, SLOT( refreshPostCard() ) );
}

void KraftView::slotModifiedFooter()
{
    // qDebug () << "Modified the footer!";
    // As long as the modified mapper is blocked, ignore this.
    if( mModifiedMapper->signalsBlocked() ) {
        return;
    }

    mModified = true;

    QTimer::singleShot( 0, this, SLOT( refreshPostCard() ) );
}

QStringList KraftView::generateLetterHead( const QString& familyName, const QString& givenName )
{
    QStringList s;

    QMap<QString, QString> m;
    m[ "%NAME"]       = familyName;
    m[ "%GIVEN_NAME"] = givenName;

    return KraftDB::self()->wordList( "salut", m );
}

void KraftView::done( int r )
{
    //Closed using the cancel button .. Check if we can close
    bool doSave = false;
    if (mModified) {
        doSave = true;
        if(r == 0) {
            QMessageBox msgBox;
            msgBox.setText(i18n("The document has been modified."));
            msgBox.setInformativeText(i18n("Do you want to save your changes?"));
            msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
            msgBox.setDefaultButton(QMessageBox::Save);
            msgBox.exec();
            auto res = msgBox.result();

            if (res == QMessageBox::Cancel)
                return;
            if (res == QMessageBox::Discard)
                doSave = false;
        }
        //Closed using the OK button .. it can be closed, but data needs saved
        if( doSave)
            saveChanges();
        emit viewClosed( r == 1, m_doc );
    }
    // remember the sizes of the docassistant splitter if visible.
    mAssistant->saveSplitterSizes();
    KraftSettings::self()->setDocViewSplitter(mCSplit->sizes());
    // the size of the doc edit is saved in Portal::slotViewClosed

    QDialog::done( r );
}

void KraftView::saveChanges()
{
    // qDebug () << "Saving changes!";

    KraftDoc *doc = getDocument();

    if( !doc ) {
      // qDebug () << "ERR: No document available in view, return!";
      return;
    }
    // transfer all values to the document
    doc->setDate( m_headerEdit->m_dateEdit->date() );
    doc->setAddressUid( mContactUid );
    doc->setAddress(  m_headerEdit->m_postAddressEdit->toPlainText() );
    doc->setDocType(  m_headerEdit->m_cbType->currentText() );
    doc->setPreTextRaw(  m_headerEdit->m_teEntry->toPlainText() );
    doc->setWhiteboard( m_headerEdit->m_whiteboardEdit->toPlainText() );
    doc->setProjectLabel( m_headerEdit->mProjectLabelEdit->text() );
    doc->setSalut(    m_headerEdit->m_letterHead->currentText() );
    doc->setPostTextRaw( m_footerEdit->ui()->m_teSummary->toPlainText() );
    doc->setGoodbye(  m_footerEdit->greeting() );

    DocPositionList list = currentPositionList();
    doc->setPositionList( list );

    bool ok = DocumentMan::self()->saveDocument(doc);
    if (!ok) {
        qDebug() << "document saving failed";
    }

    if ( doc->isNew() ) {
      // For new documents the user had to select a greeting and we make this
      // default for the future
      KraftSettings::self()->setGreeting( m_footerEdit->greeting() );
      KraftSettings::self()->setSalut( m_headerEdit->m_letterHead->currentIndex() );
    }
}

void KraftView::slotFocusItem( PositionViewWidget *posWidget, int pos )
{
  if( posWidget && pos > 0) {
    int y = (1+pos)*posWidget->height();
    m_positionScroll->ensureVisible(0, y);
  } else {
    m_positionScroll->ensureVisible( 0, 0 );
  }
  // setting Focus within the item.
  if( posWidget ) {
    if( posWidget->m_teFloskel->toPlainText().isEmpty() ) {
      posWidget->m_teFloskel->setFocus();
    } else {
      posWidget->m_sbAmount->setFocus();
    }
  }
}

void KraftView::discardChanges()
{
  // We need to reread the document
  KraftDoc *doc = getDocument();
  if( doc && doc->modified() ) {
      bool ok = DocumentMan::self()->reloadDocument(doc);
      if (!ok) {
          qDebug() << "Failed to reload doc" << doc->docIdentifier();
      }
    // qDebug () << "Document refetch from database" << endl;
  }
}

