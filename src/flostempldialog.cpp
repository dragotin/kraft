/***************************************************************************
             flostempldialog - dialog to edit templates
                             -------------------
    begin                : 2004-15-08
    copyright            : (C) 2004 by Klaas Freitag
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
#include <QRadioButton>
#include <QLabel>
#include <QString>
#include <QComboBox>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QCheckBox>
#include <QPushButton>
#include <QCloseEvent>
#include <QHeaderView>

// include files for KDE
#include <klocale.h>
#include <kdebug.h>
#include <kglobal.h>
#include <knuminput.h>
#include <kmessagebox.h>
#include <kcombobox.h>
#include <kpushbutton.h>

#include "floskeltemplate.h"
#include "catalogtemplate.h"
#include "flostempldialog.h"
#include "unitmanager.h"
#include "timecalcpart.h"
#include "fixcalcpart.h"
#include "materialcalcpart.h"
#include "matcalcdialog.h"
#include "stockmaterial.h"
#include "stockmaterialman.h"
#include "timecalcdialog.h"
#include "fixcalcdialog.h"
#include "stdsatzman.h"
#include "katalogman.h"
#include "katalog.h"
#include "materialselectdialog.h"
#include "defaultprovider.h"

FlosTemplDialog::FlosTemplDialog( QWidget *parent, bool modal )
    : KDialog( parent ),
    m_template(0),
    m_katalog(0),
    m_fixCalcDia(0),
    m_timePartDialog(0),
    m_matPartDialog(0)
{
  QWidget *w = new QWidget( this );
  setMainWidget(w);

  setupUi( w );

  setCaption( i18n("Create or Edit Template Items") );
  setModal( modal );
  setButtons( Ok | Cancel );
  setDefaultButton( Ok );
  showButtonSeparator( true);

  //Initialise the buttongroup to switch between manual and calculated price
  m_gbPriceSrc = new QButtonGroup(this);
  m_gbPriceSrc->addButton(m_rbManual, 0);
  m_gbPriceSrc->addButton(m_rbCalculation, 1);

  m_timeParts->header()->setResizeMode(QHeaderView::ResizeToContents);
  m_fixParts->header()->setResizeMode(QHeaderView::ResizeToContents);
  m_matParts->header()->setResizeMode(QHeaderView::ResizeToContents);

  setupConnections();
  setButtonIcons();
}

void FlosTemplDialog::setupConnections()
{
  connect(m_gbPriceSrc, SIGNAL(buttonClicked(int)), this, SLOT(slCalcOrFix(int)));

  /* connect a value Changed signal of the manual price field */
  connect( m_manualPriceVal, SIGNAL( valueChanged(double)),
           this, SLOT( slManualPriceChanged(double)));

  connect( m_text, SIGNAL(textChanged()),this, SLOT(slSetNewText()));

  connect( spBenefit, SIGNAL(valueChanged(int)), this, SLOT(slBenefitChange(int)));


  //Time calculation
  connect(m_butAddTime, SIGNAL(clicked()), this, SLOT(slAddTimePart()));
  connect(m_butEditTime, SIGNAL(clicked()), this, SLOT(slEditTimePart()));
  connect(m_butRemoveTime, SIGNAL(clicked()), this, SLOT(slRemoveTimePart()));

  //Fix costs
  connect(m_butAddFix, SIGNAL(clicked()), this, SLOT(slAddFixPart()));
  connect(m_butEditFix, SIGNAL(clicked()), this, SLOT(slEditFixPart()));
  connect(m_butRemoveFix, SIGNAL(clicked()), this, SLOT(slRemoveFixPart()));

  //Material
  connect(m_butAddMat, SIGNAL(clicked()), this, SLOT(slAddMatPart()));
  connect(m_butEditMat, SIGNAL(clicked()), this, SLOT(slEditMatPart()));
  connect(m_butRemoveMat, SIGNAL(clicked()), this, SLOT(slRemoveMatPart()));
}

void FlosTemplDialog::setButtonIcons()
{
  m_butAddTime->setIcon(KIcon("list-add"));
  m_butEditTime->setIcon(KIcon("document-edit"));
  m_butRemoveTime->setIcon(KIcon("list-remove"));

  m_butAddFix->setIcon(KIcon("list-add"));
  m_butEditFix->setIcon(KIcon("document-edit"));
  m_butRemoveFix->setIcon(KIcon("list-remove"));

  m_butAddMat->setIcon(KIcon("list-add"));
  m_butEditMat->setIcon(KIcon("document-edit"));
  m_butRemoveMat->setIcon(KIcon("list-remove"));
}

void FlosTemplDialog::setTemplate( FloskelTemplate *t, const QString& katalogname, bool newTempl )
{
  if( ! t ) return;
  m_template = t;
  m_templateIsNew = newTempl;

  m_katalog = KatalogMan::self()->getKatalog(katalogname);

  if( m_katalog == 0 ) {
    kDebug() << "ERR: Floskel Dialog called without valid Katalog!" << endl;
    return;
  }

  QList<CatalogChapter> chapters = m_katalog->getKatalogChapters( );
  QStringList chapNames;
  foreach( CatalogChapter chap, chapters ) {
    chapNames.append( chap.name() );
  }
  cbChapter->insertItems(-1, chapNames );
  int chapID = t->chapterId().toInt();
  QString chap = m_katalog->chapterName(dbID(chapID));
  cbChapter->setCurrentIndex(cbChapter->findText( chap ));

  /* Text of the template */
  m_text->setText( t->getText());

  /* Unit */
  m_unit->clear();
  m_unit->insertItems(-1, UnitManager::self()->allUnits());
  m_unit->setCurrentIndex(m_unit->findText( m_template->unit().einheitSingular() ));

  m_manualPriceVal->setValue( t->unitPrice().toDouble());

  /* Kind of Calculation: Manual or calculated?  */

  if( t->calcKind() == CatalogTemplate::ManualPrice )
  {
    slCalcOrFix(0);
    m_rbManual->setChecked(true);
    m_rbCalculation->setChecked(false);

  }
  else /* if( t->calcKind() == Calculation ) */
  {
    slCalcOrFix(1);
    m_rbManual->setChecked(false);
    m_rbCalculation->setChecked(true);
  }

  /* set up the different calculation parts */
  setCalcparts();

  /* set text */
  slSetNewText();

  m_text->setFocus();
  m_text->selectAll();

  pbRundPreis->setEnabled(false);

  //Fixme: Only set this to true if something really changed
  modified = true;
}

void FlosTemplDialog::setCalcparts( )
{
  /* time calculation in widget m_timeParts */
  CalcPartList tpList = m_template->getCalcPartsList( KALKPART_TIME );
  m_timeParts->clear();

  QListIterator<CalcPart*> it( tpList );
  while( it.hasNext() ) {

    TimeCalcPart *cp = static_cast<TimeCalcPart*>(it.next());

    QString stdStd = i18n("No");
    if( cp->globalStdSetAllowed() ) stdStd = i18n("Yes");

    QTreeWidgetItem *lvItem = new QTreeWidgetItem( m_timeParts );
    drawTimeListEntry( lvItem, cp );
    mCalcPartDict.insert(lvItem, cp );
  }

  /* Fix calculation parts */
  m_fixParts->clear();
  tpList = m_template->getCalcPartsList( KALKPART_FIX );
  QListIterator<CalcPart*> fixIt( tpList );
  while( fixIt.hasNext() ) {
    FixCalcPart *fc = static_cast<FixCalcPart*>(fixIt.next());
    QTreeWidgetItem *lvItem = new QTreeWidgetItem( m_fixParts );
    drawFixListEntry( lvItem, fc );
    mCalcPartDict.insert( lvItem, fc );
  }

  /* Material calculation */
  m_matParts->clear();
  tpList = m_template->getCalcPartsList( KALKPART_MATERIAL );
  QListIterator<CalcPart*> matIt( tpList );
  while( matIt.hasNext() ) {
    MaterialCalcPart *mc = static_cast<MaterialCalcPart*>(matIt.next());
    QTreeWidgetItem *lvItem = new QTreeWidgetItem( m_matParts );
    mCalcPartDict.insert( lvItem, mc );
    drawMatListEntry( lvItem, mc );
  }
}


void FlosTemplDialog::refreshPrices()
{
  if( ! m_template ) return;

  /* assemble the pricing label */
  QString t;
  t = i18n("Calculated Price: ");
  int kType = m_template->calcKind();
  kDebug() << "CalcType in integer is " << kType << endl;
  if( m_template->calcKind() == CatalogTemplate::ManualPrice )
  {
    t = i18n("Manual Price: ");
  }
  else if( m_template->calcKind() == CatalogTemplate::Calculation )
  {
    int benefit = spBenefit->value();
    QString benefitStr = i18n("(+%1%)").arg(benefit);

    if( benefit < 0 )
    {
      benefitStr = "<font color=\"red\">"+i18n("%1%").arg(benefit)+"</font>";
    }
    benefitStr += i18n(": ");
    t += benefitStr;
  }
  else
  {
    kDebug() << "ERR: unknown calculation type!" << endl;
  }
  m_resPreisName->setText(t);

  /* set Price */
  t = m_template->unitPrice().toString( m_katalog->locale() );
  m_resultPrice->setText( t );
  m_manualPriceVal->setValue( m_template->unitPrice().toDouble() );

  /* Price parts per calculation part */
  Geld g( m_template->costsByCalcPart( KALKPART_TIME ));
  m_textTimePart->setText( g.toString( m_katalog->locale() ));

  g = m_template->costsByCalcPart( KALKPART_FIX );
  m_textFixPart->setText( g.toString( m_katalog->locale() ));

  g = m_template->costsByCalcPart( KALKPART_MATERIAL );
  m_textMaterialPart->setText(g.toString( m_katalog->locale() ));

}

FlosTemplDialog::~FlosTemplDialog( )
{
  delete m_fixCalcDia;
  delete m_timePartDialog;
  delete m_matPartDialog;
}

void FlosTemplDialog::accept()
{
  if( m_template ) {
    kDebug() << "Saving template ID " << m_template->getTemplID() << endl;

    QString h;
    h = m_text->toPlainText();

    if( h != m_template->getText() ) {
      kDebug() << "Template Text dirty -> update" << endl;
      m_template->setText( h );
    }

    h = m_unit->currentText();
    if( h != m_template->unit().einheitSingular()) {
      kDebug() << "Template Einheit dirty -> update to " << h << endl;
      m_template->setUnitId( UnitManager::self()->getUnitIDSingular(h));
    }

#if 0
    // chapter ID is not touched anymore
    /* compare catalog chapter */
    int chapterId = 0; // m_katalog->chapterID(cbChapter->currentText()).toInt();
    // FIXME: need new way of picking hte chapterId bcause of hirarchical.

    if( chapterId != m_template->getChapterID() ) {
      kDebug() << "Chapter ID dirty ->update" << endl;
      if( askChapterChange( m_template, chapterId )) {
        m_template->setChapterID( chapterId );
        emit( chapterChanged( chapterId ));
      }
    }
#endif
    /* count time */
    bool c = m_addTime->isChecked();
    if( c != m_template->hasTimeslice() ) {
      m_template->setHasTimeslice(c);
    }

    /* benefit */
    h = spBenefit->cleanText();
    bool b;
    double g = h.toDouble( &b );
    if( b  && g != m_template->getBenefit() ) {
      m_template->setBenefit(g);
      kDebug() << "benefit dirty ->update to " << g << endl;
    }

    h = cbMwst->currentText();
    // TODO!

    // Calculationtype
    int selId = m_gbPriceSrc->checkedId();
    CatalogTemplate::CalculationType calcType = CatalogTemplate::Unknown;
    if( selId == 0 ) {
      calcType = CatalogTemplate::ManualPrice;
    } else if( selId == 1 ) {
      calcType = CatalogTemplate::Calculation;
    } else {
      kDebug() << "ERROR: Calculation type not selected, id is " << selId << endl;
    }
    m_template->setCalculationType( calcType );

    // reread the manual price
    double dd = m_manualPriceVal->value();
    m_template->setManualPrice( Geld( dd ) );

    h = cbChapter->currentText();
    kDebug() << "catalog chapter is " << h << endl;

    if( m_template->save() ) {
      emit( editAccepted( m_template ) );
      KatalogMan::self()->notifyKatalogChange( m_katalog, m_template->getTemplID() );
    } else {
      KMessageBox::error( this, i18n("Saving of this template failed, sorry"),
                          i18n( "Template Save Error" ) );
    }
  }
  kDebug() << "*** Saving finished " << endl;

  modified = false;
  KDialog::accept();
}

void FlosTemplDialog::reject()
{
  if(confirmClose() == true) {
    // let KDialog clean away the dialog.
    KDialog::reject();
  }
}

void FlosTemplDialog::closeEvent ( QCloseEvent * event )
{
  if(confirmClose() == false)
    event->ignore();
  else
    event->accept();
}

bool FlosTemplDialog::confirmClose()
{
  if(modified == true) {
    if ( KMessageBox::warningContinueCancel( this, i18n( "The template was modified. Do "
                                                         "you really want to discard all changes?" ),
                                             i18n( "Template Modified" ), KGuiItem( i18n( "Discard" ), KIcon("edit-clear") ) )
      == KMessageBox::Cancel  ) {
      return false;
    }

    mCalcPartDict.clear();

    m_timeParts->clear ();
    m_fixParts->clear ();
    m_matParts->clear ();

    m_katalog->reload(m_template->getTemplID());

    if ( m_templateIsNew ) {
      // remove the listview item if it was created newly
      emit editRejected();
    }

  }
  return true;
}

bool FlosTemplDialog::askChapterChange( FloskelTemplate*, int )
{
  if( KMessageBox::questionYesNo( this,
                                  i18n( "The catalog chapter was changed for this template.\nDo you really want to move the template to the new chapter?"),
                                  i18n("Changed Chapter"), KStandardGuiItem::yes(), KStandardGuiItem::no(),
                                  "chapterchange" ) == KMessageBox::Yes )
  {
    return true;
  } else {
    return false;
  }
}

void FlosTemplDialog::slManualPriceChanged(double dd)
{
  kDebug() << "Changing manual price!" << endl;
  if( ! m_template ) return;
  kDebug() << "Updating manual price!" << endl;
  m_template->setManualPrice( Geld( dd ));
  refreshPrices();
}


void FlosTemplDialog::slBenefitChange( int neuPreis )
{
  CalcPartList tpList = m_template->getCalcPartsList( );
  QListIterator<CalcPart*> it( tpList );
  while( it.hasNext() ) {
    CalcPart *cp = it.next();
    cp->setProzentPlus( neuPreis );
  }
  refreshPrices();
}

void FlosTemplDialog::slAddFixPart()
{
  if( ! m_template ) return;

  FixCalcDialog dia(this);

  if( dia.exec() == QDialog::Accepted )
  {
    FixCalcPart *cp = new FixCalcPart( dia.getName(), dia.getPreis());
    cp->setMenge( dia.getMenge());
    cp->setDirty(true);

    QTreeWidgetItem *lvItem = new QTreeWidgetItem( m_fixParts);
    drawFixListEntry( lvItem, cp );
    mCalcPartDict.insert( lvItem, cp );
    m_template->addCalcPart( cp );
    refreshPrices();
  }
}

void FlosTemplDialog::slRemoveFixPart()
{
  if( ! m_template || ! m_fixParts ) return;

  QTreeWidgetItem *item = m_fixParts->currentItem();

  if( item )
  {
    CalcPart *cp = mCalcPartDict[item];
    if( cp )
    {
      m_template->removeCalcPart(cp);
    }
    delete item;

    refreshPrices();
  }
}

void FlosTemplDialog::slEditFixPart()
{
  if( ! m_template || !m_fixParts ) return;

  kDebug() << "Edit fix part!" << endl;

  QTreeWidgetItem *item = m_fixParts->currentItem();

  if( item )
  {
    FixCalcPart *cp = static_cast<FixCalcPart*>(mCalcPartDict[item]);
    if( cp )
    {
      // FixCalcDialog dia(cp, this);
      m_fixCalcDia = new FixCalcDialog(cp, this);
      m_fixCalcDia->setModal(true);
      connect( m_fixCalcDia, SIGNAL(fixCalcPartChanged(FixCalcPart*)),
               this, SLOT(slFixCalcPartChanged(FixCalcPart*)));
      m_fixCalcDia->show();
    }
  }
}

void FlosTemplDialog::slFixCalcPartChanged(FixCalcPart *cp)
{
  refreshPrices();
  drawFixListEntry(m_fixParts->currentItem(), cp);
}

void FlosTemplDialog::slTimeCalcPartChanged(TimeCalcPart *cp)
{
  refreshPrices();
  drawTimeListEntry(m_timeParts->currentItem(), cp);
}

void FlosTemplDialog::slAddTimePart()
{
  if( ! m_template ) return;
  TimeCalcDialog dia(this);

  if( dia.exec() == QDialog::Accepted )
  {
    TimeCalcPart *cp = new TimeCalcPart( dia.getName(), dia.getDauer(), 0 );
    cp->setGlobalStdSetAllowed( dia.allowGlobal());
    StdSatz std = StdSatzMan::self()->getStdSatz( dia.getStundensatzName());
    cp->setStundensatz( std );
    QTreeWidgetItem *lvItem = new QTreeWidgetItem( m_timeParts);
    drawTimeListEntry( lvItem, cp );
    mCalcPartDict.insert( lvItem, cp );
    m_template->addCalcPart( cp );
    refreshPrices();
  }
}

/*
 * stellt einen TimeCalcPart als ListViewItem dar. Wird gebraucht, wenn das
 * item neu ist, aber auch beim Editieren
 */
void FlosTemplDialog::drawTimeListEntry( QTreeWidgetItem *it, TimeCalcPart *cp )
{

  if( !( it && cp) )
    return;

  it->setText( 0, cp->getName());
  it->setText( 1, i18n("%1 Min.").arg(cp->getMinuten()));
  it->setText( 2, cp->getStundensatz().getName());
  it->setText( 3, cp->globalStdSetAllowed() ? i18n("Yes") : i18n("No"));
}

void FlosTemplDialog::drawFixListEntry( QTreeWidgetItem* it, FixCalcPart *cp )
{
  if( !( it && cp) )
    return;

  it->setText( 0, DefaultProvider::self()->locale()->formatNumber(cp->getMenge()));
  it->setText( 1, cp->getName());
  it->setText( 2, cp->unitPreis().toString( m_katalog->locale() ));
  it->setText( 3, cp->basisKosten().toString( m_katalog->locale() ));
}

void FlosTemplDialog::drawMatListEntry( QTreeWidgetItem *it, MaterialCalcPart *mc )
{
  it->setText( 0, mc->getName());
  it->setText( 1, QString::number(mc->getCalcAmount(), 'f',2));
  it->setText( 2, mc->getMaterial()->getUnit().einheitSingular());
  it->setText( 3, mc->basisKosten().toString( m_katalog->locale() ));
  it->setText( 4, QString::number(mc->getMaterial()->getAmountPerPack(), 'f',2));
  it->setText( 5, mc->getMaterial()->salesPrice().toString( m_katalog->locale() ));
}


void FlosTemplDialog::slRemoveTimePart()
{
  if( ! m_template || !m_timeParts ) return;

  QTreeWidgetItem *item = m_timeParts->currentItem();

  if( item )
  {
    CalcPart *cp = mCalcPartDict[item];
    if( cp )
    {
      m_template->removeCalcPart(cp);
    }
    delete item;

    refreshPrices();
  }
}

void FlosTemplDialog::slEditTimePart()
{
  if( ! m_template || !m_timeParts ) return;

  kDebug() << "Edit time part!" << endl;

  QTreeWidgetItem *item = m_timeParts->currentItem();

  if( item ) {
    TimeCalcPart *cp = static_cast<TimeCalcPart*>(mCalcPartDict[item]);
    if( cp ) {
      m_timePartDialog = new TimeCalcDialog(cp, this);
      m_timePartDialog->setModal(true);
      connect( m_timePartDialog, SIGNAL(timeCalcPartChanged(TimeCalcPart*)),
               this, SLOT(slTimeCalcPartChanged(TimeCalcPart*)) );
      m_timePartDialog->show();
    } else {
      kDebug() << "No time calc part found for this item" << endl;
    }
  } else {
    kDebug() << "No current Item!";
  }
  refreshPrices();
}

/*
 * Achtung: slAddMatPart fgt keinen neuen kompletten Materialpart
 * hinzu, sondern nur ein neues Material zu dem einen, bestehenden
 * Materialpart.
 */
void FlosTemplDialog::slAddMatPart()
{
  if( ! m_template ) return;

  MaterialSelectDialog dia( this );
  connect( &dia, SIGNAL( materialSelected( int, double ) ),
           this, SLOT( slNewMaterial( int, double ) ) );
  dia.exec();
}

/*
 * Slot, der aufgerufen wird, wenn im Materialeditor ein Material zur Kalkulation
 * geschickt wird.
 */
void FlosTemplDialog::slNewMaterial( int matID, double amount )
{
  kDebug() << "Material ID: " << matID << endl;

  // TODO: Checken, ob der richtige Tab aktiv ist.
  // TODO: Check if the material is already in the calcpart (is this really needed??)
  MaterialCalcPart *mc;

  mc = new MaterialCalcPart(matID, 0, amount);
  m_template->addCalcPart( mc );
  QTreeWidgetItem *lvItem = new QTreeWidgetItem(m_matParts);
  drawMatListEntry( lvItem, mc );
  mCalcPartDict.insert( lvItem, mc );
  refreshPrices();
}


void FlosTemplDialog::slEditMatPart()
{
  if( ! m_template || !m_matParts ) return;

  kDebug() << "Edit Material part!" << endl;

  QTreeWidgetItem *item = m_matParts->currentItem();

  MaterialCalcPart *mc = static_cast<MaterialCalcPart*>( mCalcPartDict[item] );

  if( mc ) {
      m_matPartDialog = new MatCalcDialog( mc, this);

      connect( m_matPartDialog, SIGNAL(matCalcPartChanged(MaterialCalcPart*)),
               this, SLOT(slMatCalcPartChanged(MaterialCalcPart*)));
      m_matPartDialog->setModal(true);
      m_matPartDialog->show();
    } else {
      kDebug() << "No such MaterialCalcPart!";
    }
}

void FlosTemplDialog::slMatCalcPartChanged(MaterialCalcPart *mc)
{
  drawMatListEntry(m_matParts->currentItem(), mc);
  refreshPrices();
}

void FlosTemplDialog::slRemoveMatPart()
{
  if( ! m_template || ! m_matParts ) return;

  QTreeWidgetItem *item = m_matParts->currentItem();

  if( item )
  {
    CalcPart *cp = mCalcPartDict[item];
    if( cp )
    {
      m_template->removeCalcPart(cp);
    }
    delete item;

    refreshPrices();
  }
}

/*
 * dieser Slot wird betreten, wenn der Radiobutton Kalkulierter
 * Preis/Manueller Preis umgeschaltet wird.
 */
void FlosTemplDialog::slCalcOrFix(int button)
{
  bool ok = true;

  if( button == 0 )
  {
    /* auf manuell geschaltet */
    if( m_template )
      m_template->setCalculationType( CatalogTemplate::ManualPrice );

    m_manualPriceVal->setEnabled( true );

    m_textTimePart->setEnabled(false);
    m_textFixPart->setEnabled(false);
    m_textMaterialPart->setEnabled(false);

    m_tLabelMat->setEnabled(false);
    m_tLabelFix->setEnabled(false);
    m_tLabelTime->setEnabled(false);
    spBenefit->setEnabled(false);
  }
  else if( button == 1 )
  {
    /* auf kalkuliert geschaltet */
    if( m_template )
      m_template->setCalculationType( CatalogTemplate::Calculation );

    m_manualPriceVal->setEnabled( false );

    m_textTimePart->setEnabled(true);
    m_textFixPart->setEnabled(true);
    m_textMaterialPart->setEnabled(true);

    m_tLabelMat->setEnabled(true);
    m_tLabelFix->setEnabled(true);
    m_tLabelTime->setEnabled(true);
    spBenefit->setEnabled(true);
  }
  else
  {
    /* unbekannter knopf -> fehler */
    kDebug() << "--- Error: Falsche Button ID " << button <<  endl;
    ok = false;
  }

  if( ok )
  {
    refreshPrices();
  }
}


void FlosTemplDialog::slSetNewText( )
{
  if( ! m_text || m_text->toPlainText().isEmpty() ) {
    this->button(KDialog::Ok)->setEnabled(false);
  } else {
    this->button(KDialog::Ok)->setEnabled(true);
  }

  if( m_text ) {
    QString t = m_text->toPlainText();

    if( m_textDispTime)
      m_textDispTime->setText(t);
    if( m_textDispFix)
      m_textDispFix->setText(t);
    if( m_textDispMat)
      m_textDispMat->setText(t);
  }

}
/* END */


// #include "flostempldialog.moc"
