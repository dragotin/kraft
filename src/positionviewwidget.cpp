/***************************************************************************
        positionviewwidget - inherited class for doc position views.
                             -------------------
    begin                : 2006-02-20
    copyright            : (C) 2006 by Klaas Freitag
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

#include <QWidget>
#include <QStackedWidget>
#include <QPainter>
#include <QTextEdit>
#include <QMenu>
#include <QIcon>
#include <QActionGroup>
#include <QDebug>
#include <qdrawutil.h>

#include "positionviewwidget.h"
#include "geld.h"
#include "kraftsettings.h"
#include "defaultprovider.h"
#include "itemtagdialog.h"
#include "tagman.h"


PositionViewWidget::PositionViewWidget()
    :QWidget(), Ui_positionWidget(),
   mModified( false ),
   mToDelete(false),
   mOrdNumber(0),
   mPositionPtr( 0 ),
   mExecPopup( new QMenu( this ) ) ,
   mStateSubmenu( 0 ),
   mState( Active ),
   mKind(DocPosition::Type::Position),
   mPositionPriceValid( false ),
   mLocale( 0 )
{
  setupUi( this );
  m_sbUnitPrice->setMinimum( -999999.99 );
  m_sbUnitPrice->setMaximum( 999999.99 );
  m_sbUnitPrice->setDecimals( 2 );
  const QString currSymbol = DefaultProvider::self()->locale()->currencySymbol();
  m_sbUnitPrice->setSuffix(" " + currSymbol);


  m_sbAmount->setMinimum( -999999.99 );
  m_sbAmount->setMaximum( 999999.99 );
  m_sbAmount->setDecimals( 2 );

  mDiscountPercent->setMinimum( -100.0 );
  mDiscountPercent->setMaximum( 9999.99 );
  mDiscountPercent->setDecimals( 2 );

  pbExec->setCheckable( false );
  pbExec->setIcon( DefaultProvider::self()->icon( "tool") );
  pbTagging->setCheckable( false );
  pbTagging->setIcon( DefaultProvider::self()->icon( "flag" ) );

  connect( m_sbAmount, &QDoubleSpinBox::valueChanged, this, &PositionViewWidget::slotRefreshPrice);
  connect( m_sbUnitPrice, &QDoubleSpinBox::valueChanged, this, &PositionViewWidget::slotRefreshPrice);
  connect( mDiscountPercent, &QDoubleSpinBox::valueChanged, this, &PositionViewWidget::slotRefreshPrice);

  connect( pbExec, &QPushButton::pressed, this, &PositionViewWidget::slotExecButtonPressed);
  connect( pbTagging, &QPushButton::pressed, this, &PositionViewWidget::slotTaggingButtonPressed);


  /* modified signals */
  connect( m_cbUnit, &QComboBox::activated, this, &PositionViewWidget::slotModified);
  connect( m_teFloskel, &QTextEdit::textChanged, this, &PositionViewWidget::slotModified);

  connect( m_sbAmount, &QDoubleSpinBox::valueChanged, this, &PositionViewWidget::slotModified);
  connect( m_sbUnitPrice, &QDoubleSpinBox::valueChanged, this, &PositionViewWidget::slotModified);
  connect( mDiscountPercent, &QDoubleSpinBox::valueChanged, this, &PositionViewWidget::slotModified);
  connect( mDiscountTag, &QComboBox::activated, this, &PositionViewWidget::slotModified);

  mExecPopup->setTitle(i18n("Item Actions") );

  // state submenu:
  mStateSubmenu = mExecPopup->addMenu(i18n( "Item Kind" ));
  mStateSubmenu->addAction( i18n( "Normal" ), this, SIGNAL( positionStateNormal() ) );
  mStateSubmenu->addAction( DefaultProvider::self()->icon( "arrow-ramp-right" ),
                            i18n( "Alternative" ), this, SIGNAL( positionStateAlternative() ) );
  mStateSubmenu->addAction( DefaultProvider::self()->icon( "arrow-move-right" ),
                            i18n( "On Demand" ), this, SIGNAL( positionStateDemand() ) );

  // mExecPopup->addSeparator();

  // mTaxSubMenu
  mTaxSubmenu = mExecPopup->addMenu(i18n( "Tax" ));
  QActionGroup *agroup = new QActionGroup( this );
  agroup->setExclusive ( true );
  mNilTaxAction = new QAction( DefaultProvider::self()->icon("circle-0"),  i18n("Taxfree Item"), this );
  connect( mNilTaxAction, SIGNAL(triggered()), this, SLOT(slotSetNilTax()) );
  mNilTaxAction->setCheckable( true );
  agroup->addAction( mNilTaxAction );
  mTaxSubmenu->addAction( mNilTaxAction );

  mRedTaxAction = new QAction( DefaultProvider::self()->icon("circle-half-2"), i18n("Reduced Tax"),  this );
  connect( mRedTaxAction, SIGNAL(triggered()), this, SLOT(slotSetReducedTax()));
  mRedTaxAction->setCheckable( true );
  agroup->addAction( mRedTaxAction );
  mTaxSubmenu->addAction( mRedTaxAction );

  mFullTaxAction = new QAction( DefaultProvider::self()->icon("coin"), i18n("Full Tax"),  this );
  connect( mFullTaxAction, SIGNAL(triggered()), this, SLOT(slotSetFullTax()));
  mFullTaxAction->setCheckable( true );
  agroup->addAction( mFullTaxAction );
  mTaxSubmenu->addAction( mFullTaxAction );

  mExecPopup->addSeparator();

  mExecPopup->addAction(  DefaultProvider::self()->icon("arrow-up"),
                           i18n("Move Up"),         this, SIGNAL( moveUp() ) );
  mExecPopup->addAction(  DefaultProvider::self()->icon("arrow-down"),
                           i18n("Move Down"),       this, SIGNAL( moveDown() ) );
  mLockId = mExecPopup->addAction(  DefaultProvider::self()->icon("lock"),
                           i18n("Lock Item"),   this, SIGNAL( lockPosition() ) );
  mUnlockId = mExecPopup->addAction(  DefaultProvider::self()->icon("lock-open"),
                           i18n("Unlock Item"), this, SIGNAL( unlockPosition() ) );
  mDeleteId = mExecPopup->addAction(  DefaultProvider::self()->icon("minus"),
                           i18n("Delete Item"), this, SIGNAL( deletePosition() ) );

  connect(this, &PositionViewWidget::positionStateNormal, this, [this]() {
      slotSetPositionKind(DocPosition::Type::Position, true);
      slotRefreshPrice();
      Q_EMIT positionModified();
  } );
  connect(this, &PositionViewWidget::positionStateAlternative, this, [this]() {
      slotSetPositionKind(DocPosition::Type::Alternative, true);
      slotRefreshPrice();
      Q_EMIT positionModified();
  } );
  connect(this, &PositionViewWidget::positionStateDemand, this, [this]() {
      slotSetPositionKind(DocPosition::Type::Demand, true);
      slotRefreshPrice();
      Q_EMIT positionModified();
  } );

  connect(this, &PositionViewWidget::lockPosition, this, &PositionViewWidget::slotLockPosition);
  connect(this, &PositionViewWidget::unlockPosition, this, &PositionViewWidget::slotUnlockPosition);

  connect( mExecPopup, &QMenu::aboutToShow, this, &PositionViewWidget::slotMenuAboutToShow);
  connect( mExecPopup, &QMenu::aboutToHide, this, &PositionViewWidget::slotMenuAboutToHide);

  mUnlockId->setEnabled(false);
  lStatus->setPixmap( QPixmap() );
  lKind->setPixmap( QPixmap() );

  this->setAutoFillBackground(true);
  this->setBaseSize(this->width(), 100);
  this->layout()->setContentsMargins(6, 6, 6, 6);
}

void PositionViewWidget::setDocPosition( DocPosition *pos)
{
  if( ! pos ) {
    qCritical() << "setDocPosition got empty position!";
    return;
  }

  // FIXME: Do not keep this pointer ...
  mPositionPtr = pos;

  this->blockSignals(true);

  m_teFloskel->setPlainText( pos->text() );
  lStatus->hide();
  lKind->hide();

  QString unit = pos->unit().einheitSingular();
  m_cbUnit->setCurrentIndex(m_cbUnit->findText( unit ));

  if( pos->type() == DocPosition::Type::Position ||
          pos->type() == DocPosition::Type::Demand ||
          pos->type() == DocPosition::Type::Alternative) {

    positionDetailStack->setCurrentWidget( positionPage );

    m_sbAmount->setValue( pos->amount() );

    m_sbUnitPrice->setValue( pos->unitPrice().toDouble() );

    const QString kindStr = pos->typeStr();
    DocPosition::Type kind = techStringToKind(kindStr);
    slotSetPositionKind(kind, false);

    // qDebug () << "Setting position ptr. in viewwidget: " << pos;
  } else if ( pos->type() == DocPosition::Type::ExtraDiscount ) {
    positionDetailStack->setCurrentWidget( discountPage );
    // qDebug() << " " << pos->type();
    KraftAttrib discount = pos->attribute(DocPosition::Discount);
    mDiscountPercent->setValue( discount.value().toDouble() );

    QString selTag;
    if ( pos->hasAttribute( DocPosition::ExtraDiscountTagRequired ) ) {
        const QString tagName = pos->attribute(DocPosition::ExtraDiscountTagRequired).value().toString();
        const TagTemplate tt = TagTemplateMan::self()->getTagTemplateFromId(tagName);
        selTag = tt.name();
    }

    /* Fill and set the extra discount selection combo */
    const QString allPos = i18n( "All items" );
    mDiscountTag->addItem( allPos ); // , i18n( "Overall Position Discount" ) );
    QStringList taglist = TagTemplateMan::self()->allTagTemplates();
    QString currentEntry = allPos;

    for ( QStringList::Iterator tagIt = taglist.begin(); tagIt != taglist.end(); ++tagIt ) {
      QString tagger;
      TagTemplate tmpl = TagTemplateMan::self()->getTagTemplate( *tagIt );
      QPixmap pix( 16, 12 );
      pix.fill( tmpl.color() );
      tagger = i18n( "%1-tagged items", *tagIt );
      mDiscountTag->addItem(pix, tagger);
      if ( selTag == *tagIt ) {
        currentEntry = tagger;
      }
    }
    mDiscountTag->setCurrentIndex(mDiscountTag->findText( currentEntry ));
  } else {
      qDebug () << "unknown doc position type " << pos->typeStr();
  }
  slotSetOverallPrice( currentPrice() );

  // set tags marked
  slotUpdateTagToolTip();
  slotSetTax( pos->taxType() );

  this->blockSignals(false);
}

void PositionViewWidget::slotShowPrice( bool show )
{
    m_sumLabel->setVisible(show);
    m_sbUnitPrice->setVisible(show);
}

void PositionViewWidget::slotUpdateTagToolTip()
{
  QString tip;
  bool first = true;

  const QStringList& tags = position()->allTags();

  if ( tags.count() == 1 ) {
    tip = i18n( "Tag: %1", tags.first() );
  } else if ( tags.count() > 1 ) {
    tip = i18n( "Tags:<ul>" );
    for (const auto& t:tags) {
      if ( first ) {
        tip += QString( "<li>%1</li>" ).arg(t);
        first = false;
      } else {
        tip += QString( "<li>%1</li>" ).arg(t);
      }
    }
    tip += "</ul>";
  } else {
    tip = i18n( "No tags assigned yet." );
  }

  pbTagging->setToolTip( tip );
}

QString PositionViewWidget::extraDiscountTagRestriction()
{
  QStringList taglist = TagTemplateMan::self()->allTagTemplates();

  int currentItem = mDiscountTag->currentIndex();
  if ( currentItem > 0 && currentItem <= taglist.count() ) {
    // subtract one for the "all items" entry in the combo box at first position
    currentItem -= 1;
    return taglist[currentItem];
  } else {
    // qDebug () << "taglist index possibly out of range!";
  }
  return QString();
}

void PositionViewWidget::slotTaggingButtonPressed()
{
  // qDebug () << "opening tagging dialog";

  ItemTagDialog dia( 0 );

  const QStringList& tags = position()->allTags();
  dia.setPositionTags(tags);
  if ( dia.exec() ) {
    QStringList newTags = dia.getSelectedTags();
    if (newTags != tags) {
        position()->setTags(newTags);
        slotUpdateTagToolTip();
        slotModified();
        update();
    }
    qDebug () << "Selected tags: " << newTags.join( ", " );
  }
}

void PositionViewWidget::slotSetNilTax()
{
  slotSetTax( DocPosition::Tax::None );
}

void PositionViewWidget::slotSetReducedTax()
{
  slotSetTax( DocPosition::Tax::Reduced );
}

void PositionViewWidget::slotSetFullTax()
{
  slotSetTax( DocPosition::Tax::Full );
}

void PositionViewWidget::slotSetTax(DocPosition::Tax tt )
{
  mTax = tt;

  QString icon;
  if( tt == DocPosition::Tax::Full ) {
    icon = QString::fromLatin1("coin");
    mFullTaxAction->setChecked( true );
  } else if( tt == DocPosition::Tax::Reduced ) {
    icon = QString::fromLatin1("circle-half-2");
    mRedTaxAction->setChecked( true );
  } else if( tt == DocPosition::Tax::None ) {
    icon = QString::fromLatin1("circle-0");
    mNilTaxAction->setChecked( true );
  }

  mTaxSubmenu->setIcon( DefaultProvider::self()->icon( icon ));
  Q_EMIT positionModified();
}

void PositionViewWidget::slotAllowIndividualTax( bool allow )
{
  mFullTaxAction->setEnabled(allow);
  mRedTaxAction->setEnabled(allow);
  mNilTaxAction->setEnabled(allow);
  mTaxSubmenu->setEnabled( allow );
}

DocPosition::Tax PositionViewWidget::taxType() const
{
  return mTax;
}

void PositionViewWidget::slotExecButtonPressed()
{
  // qDebug () << "Opening Context Menu over exec button";

  // set bg-color
  mExecPopup->popup( QWidget::mapToGlobal( pbExec->pos() ) );

}

void PositionViewWidget::slotMenuAboutToShow()
{
  QPalette palette;
  palette.setColor(this->backgroundRole(), QColor(0x757476));
  this->setPalette(palette);
}

void PositionViewWidget::slotMenuAboutToHide()
{
  // qDebug () << "Set normal again";
  QPalette palette;
  setPalette( palette );
  pbExec->setChecked(false);
}

void PositionViewWidget::slotLockPosition( )
{
  slotSetState( Locked );
}

void PositionViewWidget::slotUnlockPosition( )
{
  slotSetState( Active );
}

void PositionViewWidget::slotEnableKindMenu( bool s )
{
  mStateSubmenu->setEnabled( s );
}

QString PositionViewWidget::stateString( const State& state ) const
{
  QString str;

  if( state == Active ) {
    str = i18n( "Active" );
  } else if( state == New ) {
    str = i18n( "New" );
  } else if( state == Deleted ) {
    str = i18n( "Deleted" );
  } else if( state == Locked ) {
    str = i18n( "Locked" );
  } else {
    str = i18n( "Unknown" );
  }
  return str;
}

void PositionViewWidget::slotSetState( State state )
{
  mState = state;
  // qDebug () << "Setting new widget state " << stateString( state );
  if( state == Active ) {
    mLockId->setEnabled( true );
    mUnlockId->setEnabled( false );

    lStatus->hide();
    lStatus->setPixmap( QPixmap() );
    mToDelete = false;
    slotSetEnabled( true );
  } else if( state == New ) {
    lStatus->setPixmap( DefaultProvider::self()->icon("file-plus").pixmap(QSize(20,20)));
    lStatus->show();
  } else if( state == Deleted ) {
    lStatus->setPixmap( DefaultProvider::self()->icon( "minus" ).pixmap(QSize(20,20)) );
    lStatus->show();
    mToDelete = true;
    slotSetEnabled( false );
  } else if( state == Locked ) {
    mLockId->setEnabled( false );
    mUnlockId->setEnabled( true );
    slotSetEnabled( false );
    lStatus->setPixmap( DefaultProvider::self()->icon( "lock" ).pixmap(QSize(20,20)));
    lStatus->show();
  }
}

void PositionViewWidget::setOrdNumber(int o)
{
  mOrdNumber = o;
  if( mModified ) {
      QColor c( "darkred" );
      QPalette palette = m_labelPosition->palette();
      palette.setColor(m_labelPosition->foregroundRole(), c);
      m_labelPosition->setPalette(palette);
  }
  m_labelPosition->setText( QString("%1.").arg( mOrdNumber ) );
}

void PositionViewWidget::slotSetEnabled( bool doit )
{
  if( !doit ) {
    m_sbAmount->setEnabled( false );
    m_sbUnitPrice->setEnabled( false );
    m_labelPosition->setEnabled( false );
    m_teFloskel->setEnabled( false );
    m_sumLabel->setEnabled( false );
    m_cbUnit->setEnabled( false );
  } else {
    m_sbAmount->setEnabled( true );
    m_sbUnitPrice->setEnabled( true );
    m_labelPosition->setEnabled( true );
    m_teFloskel->setEnabled( true );
    m_sumLabel->setEnabled( true );
    m_cbUnit->setEnabled( true );
  }
}

bool PositionViewWidget::priceValid()
{
  bool isValid = true;

  if ( position()->type() == DocPosition::Type::ExtraDiscount ) {
    isValid = mPositionPriceValid;
  }

  return isValid;
}

void PositionViewWidget::setCurrentPrice( Geld g )
{
  // do nothing for normal positions
  if ( position()->type() == DocPosition::Type::ExtraDiscount ) {
    mPositionPrice = g;
    mPositionPriceValid = true;
  }
}

Geld PositionViewWidget::currentPrice()
{
  Geld sum;
  if ( mKind == DocPosition::Type::Position ) {
    if ( position()->type() == DocPosition::Type::ExtraDiscount ) {
      sum = mPositionPrice;
      if ( ! mPositionPriceValid ) {
        qWarning() << "Asking for price of Discount item, but invalid!";
      }
    } else {
      double amount = m_sbAmount->value();
      Geld g( m_sbUnitPrice->value() );
      sum = g * amount;
    }
  }
  return sum;
}

Geld PositionViewWidget::unitPrice()
{
  Geld p(  m_sbUnitPrice->value() );
  return p;
}

void PositionViewWidget::slotRefreshPrice()
{
  const Geld sum = currentPrice();
  slotSetOverallPrice( sum );
  Q_EMIT priceChanged( sum );
}

void PositionViewWidget::slotSetOverallPrice( Geld g )
{
  // if ( mPositionPtr->type() == DocPosition::Type::ExtraDiscount ) {
  //   m_sumLabel->setText( "--" );
  // } else {
    m_sumLabel->setText( g.toLocaleString() );
    // }
}

void PositionViewWidget::slotModified()
{
    if(this->signalsBlocked()) return;
    // qDebug () << "Modified Position!";

    mModified = true;

    m_labelPosition->setStyleSheet("font-weight: bold; color: red");

    Q_EMIT positionModified();
}

PositionViewWidget::~PositionViewWidget()
{
}

PositionViewWidgetList::PositionViewWidgetList()
  : QList<PositionViewWidget*>()
{
  // setAutoDelete( true );
}

PositionViewWidget* PositionViewWidgetList::widgetFromPosition( DocPositionGuardedPtr ptr)
{
  PositionViewWidgetListIterator it( *this );
  while( it.hasNext() ) {
    PositionViewWidget *pvw = it.next();
    if( pvw ->position() == ptr ) {
      return pvw;
    }
  }

  return 0;
}

Geld PositionViewWidgetList::nettoPrice()
{
  Geld res;

  PositionViewWidgetListIterator it( *this );
  while( it.hasNext() ) {
    PositionViewWidget *pvw = it.next();
    res += pvw->currentPrice();
  }
  return res;
}

QString PositionViewWidget::cleanKindString(const QString& src)
{
    QString current {src};
    if ( current.startsWith( kindLabel(DocPosition::Type::Alternative) ) ) {
        current.remove( 0, kindLabel(DocPosition::Type::Alternative).length() );
    } else if ( current.startsWith( kindLabel(DocPosition::Type::Demand) ) ) {
        current.remove( 0, kindLabel(DocPosition::Type::Demand).length());
    }
    return current;
}

void PositionViewWidget::slotSetPositionKind(DocPosition::Type kind, bool alterText)
{
    QString tt;
    QIcon icon;
    bool showLabel {false};
    auto oldKind = mKind;

    mKind = kind;
    if (kind == DocPosition::Type::Position) {

    } else if (kind == DocPosition::Type::Demand) {
        tt = i18n( "This item is either completely optional or its "
                   "amount varies depending on the needs.<br/><br/>"
                   "Use the item toolbox to change the item type." );
        showLabel = true;
        icon = DefaultProvider::self()->icon("arrow-move-right");
    } else if (kind == DocPosition::Type::Alternative) {
        tt = i18n( "This is an alternative item.<br/><br/>"
                   "Use the item toolbox to change the item type." );
        showLabel = true;
        icon = DefaultProvider::self()->icon("arrow-ramp-right");
    }
    showLabel ? lKind->show() : lKind->hide();
    lKind->setToolTip(tt);
    lKind->setPixmap(icon.pixmap(QSize(20,20)));

    if (alterText) {
        QString text =  m_teFloskel->toPlainText();
        if (oldKind == DocPosition::Type::Position) {
            QString pre;
            if (kind == DocPosition::Type::Alternative) {
                pre = kindLabel(DocPosition::Type::Alternative);
            } else if (kind == DocPosition::Type::Demand) {
                pre = kindLabel(DocPosition::Type::Demand);
            }
            if (!pre.isEmpty()) {
                m_teFloskel->setPlainText(pre + text);
            }
        } else {
              // from demand to normal or alternative for example
            if (kind == DocPosition::Type::Position) {
                text = cleanKindString(text);
            } else {
                text = kindLabel(kind) + cleanKindString(text);
            }
            m_teFloskel->setPlainText(text);
        }
    }
}

// The technical label
// Do not
QString PositionViewWidget::techKindString(DocPosition::Type kind)
{
    switch(kind) {
    case DocPosition::Type::Position:
        return QStringLiteral("Normal");
        break;
    case DocPosition::Type::Demand:
        return QStringLiteral("Demand");
        break;
    case DocPosition::Type::Alternative:
        return QStringLiteral("Alternative");
        break;
    default:
        qDebug() << "Invalid Kind set";
        return QStringLiteral("Invalid");
    }
}

DocPosition::Type PositionViewWidget::techStringToKind( const QString& kindStr )
{
    if (kindStr == techKindString(DocPosition::Type::Position)) {
        return DocPosition::Type::Position;
    } else if (kindStr == techKindString(DocPosition::Type::Demand)) {
        return DocPosition::Type::Demand;
    } else if (kindStr == techKindString(DocPosition::Type::Alternative)) {
        return DocPosition::Type::Alternative;
    }
    return DocPosition::Type::Position;
}

// The label that is prepended to a positions text
QString PositionViewWidget::kindLabel( DocPosition::Type k )
{
  auto kind = k;

  QString re;
  if ( kind == DocPosition::Type::Position) {
    re = KraftSettings::self()->normalLabel();
    if ( re.isEmpty() ) re = i18n( "Normal" );
  }
  if ( kind == DocPosition::Type::Demand ) {
    re = KraftSettings::self()->demandLabel();
    if ( re.isEmpty() ) re = i18n( "Demand" );
  }
  if ( kind == DocPosition::Type::Alternative ) {
    re = KraftSettings::self()->alternativeLabel();
    if ( re.isEmpty() ) re = i18n( "Alternative" );
  }

  if ( ! re.endsWith( ": " ) ) {
    re += QStringLiteral( ": " );
  }
  return re;
}

void PositionViewWidget::paintEvent ( QPaintEvent*)
{
  QScopedPointer<QPainter> painter(new QPainter( this ));

  if (position() == nullptr) return;

  // visualize the tags
  const QStringList taglist = position()->allTags();
  if ( taglist.count() ) {
      int share = ( height() - 24 ) / taglist.count();
      int cnt = 0;

    for ( QStringList::ConstIterator it = taglist.begin(); it != taglist.end(); ++it ) {
      const QString tag(*it);
      TagTemplate tagTemplate = TagTemplateMan::self()->getTagTemplate( tag );

      const QColor c = tagTemplate.color();
      // qDebug() << "color: " << c.red() << ", " << c.green() << ", " << c.blue();
      painter->setBrush( c );

      int starty = 6+cnt*share;
      qDrawShadeLine( painter.data(), QPoint(3, starty), QPoint(3, starty+share-1), tagTemplate.palette(), false, 1, 4 );
      cnt++;
    }
  }
}

