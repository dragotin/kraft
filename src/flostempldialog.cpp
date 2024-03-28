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
#include <QButtonGroup>
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
#include <QDialogButtonBox>

// include files for KDE
#include <QDebug>
#include <qmessagebox.h>
#include <qcombobox.h>
#include <QVBoxLayout>

#include "floskeltemplate.h"
#include "catalogtemplate.h"
#include "flostempldialog.h"
#include "unitmanager.h"
#include "timecalcpart.h"
#include "fixcalcpart.h"
#include "portal.h"
#include "materialcalcpart.h"
#include "matcalcdialog.h"
#include "stockmaterial.h"
#include "timecalcdialog.h"
#include "fixcalcdialog.h"
#include "stdsatzman.h"
#include "katalogman.h"
#include "katalog.h"
#include "materialselectdialog.h"
#include "defaultprovider.h"

FlosTemplDialog::FlosTemplDialog( QWidget *parent, bool modal )
    : QDialog( parent ),
    m_template(0),
    m_katalog(0),
    m_fixCalcDia(0),
    m_timePartDialog(0),
    m_matPartDialog(0)
{
  QWidget *w = new QWidget( this );
  QVBoxLayout *mainLayout = new QVBoxLayout;
  setLayout(mainLayout);
  mainLayout->addWidget(w);

  setupUi( w );

  setWindowTitle( i18n("Create or Edit Template Items") );
  setModal( modal );
  _buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
  QPushButton *okButton = _buttonBox->button(QDialogButtonBox::Ok);
  okButton->setDefault(true);
  okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
  connect(_buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
  connect(_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
  mainLayout->addWidget(_buttonBox);
  okButton->setDefault(true);

  //Initialise the buttongroup to switch between manual and calculated price
  m_gbPriceSrc = new QButtonGroup(this);
  m_gbPriceSrc->addButton(m_rbManual, 0);
  m_gbPriceSrc->addButton(m_rbCalculation, 1);

  m_timeParts->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
  m_fixParts->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
  m_matParts->header()->setSectionResizeMode(QHeaderView::ResizeToContents);

  // disable for now, not used
  cbMwst->setVisible(false);
  m_mwstLabel->setVisible(false);
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
  m_butAddTime->setIcon(DefaultProvider::self()->icon("plus"));
  m_butEditTime->setIcon(DefaultProvider::self()->icon("edit"));
  m_butRemoveTime->setIcon(DefaultProvider::self()->icon("minus"));

  m_butAddFix->setIcon(DefaultProvider::self()->icon("plus"));
  m_butEditFix->setIcon(DefaultProvider::self()->icon("edit"));
  m_butRemoveFix->setIcon(DefaultProvider::self()->icon("minus"));

  m_butAddMat->setIcon(DefaultProvider::self()->icon("plus"));
  m_butEditMat->setIcon(DefaultProvider::self()->icon("edit"));
  m_butRemoveMat->setIcon(DefaultProvider::self()->icon("minus"));
}

void FlosTemplDialog::setTemplate( FloskelTemplate *t, const QString& katalogname, bool newTempl )
{
  if( ! t ) return;
  m_template = t;
  m_templateIsNew = newTempl;

  m_katalog = KatalogMan::self()->getKatalog(katalogname);

  if( m_katalog == 0 ) {
    // qDebug () << "ERR: Floskel Dialog called without valid Katalog!";
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

  m_manualPriceVal->setSuffix(m_katalog->locale()->currencySymbol());

  /* Text of the template */
  m_text->setText( t->getText());

  /* Unit */
  m_unit->clear();
  m_unit->insertItems(-1, UnitManager::self()->allUnits());
  m_unit->setCurrentIndex(m_unit->findText( m_template->unit().einheitSingular() ));

  m_manualPriceVal->setValue( t->unitPrice().toDouble());

  /* Kind of Calculation: Manual or calculated?  */
  _origCalcType = m_template->calcKind();
  if( t->calcKind() == CatalogTemplate::ManualPrice ) {
    slCalcOrFix(0);
    m_rbManual->setChecked(true);
    m_rbCalculation->setChecked(false);
  } else {
    slCalcOrFix(1);
    m_rbManual->setChecked(false);
    m_rbCalculation->setChecked(true);
  }

  /* set up the different calculation parts */
  setCalcparts();
  _calcPartsModified = false;

  /* set text */
  slSetNewText();

  m_addTime->setChecked( m_template->hasTimeslice() );
  m_text->setFocus();
  m_text->selectAll();

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
  t = i18n("Calculated price: ");

  if( m_template->calcKind() == CatalogTemplate::ManualPrice )
  {
    t = i18n("Manual price: ");
  }
  else if( m_template->calcKind() == CatalogTemplate::Calculation )
  {
    int benefit = spBenefit->value();
    QString benefitStr = i18n("(+%1%)", benefit);

    if( benefit < 0 )
    {
      benefitStr = QLatin1String("<font color=\"red\">")+i18n("%1%", benefit)+QLatin1String("</font>");
    }
    benefitStr += i18n(": ");
    t += benefitStr;
  }
  else
  {
    // qDebug () << "ERR: unknown calculation type!";
  }
  m_resPreisName->setText(t);
  m_resPreisName->setTextFormat(Qt::RichText);

  /* set Price */
  t = m_template->unitPrice().toLocaleString();
  m_resultPrice->setText( t );
  m_manualPriceVal->setValue( m_template->unitPrice().toDouble() );

  /* Price parts per calculation part */
  Geld g( m_template->costsByCalcPart( KALKPART_TIME ));
  m_textTimePart->setText( g.toLocaleString());

  g = m_template->costsByCalcPart( KALKPART_FIX );
  m_textFixPart->setText( g.toLocaleString());

  g = m_template->costsByCalcPart( KALKPART_MATERIAL );
  m_textMaterialPart->setText(g.toLocaleString());

  // Benefit
  double b = m_template->getBenefit();
  spBenefit->setValue( qRound(b));
}

FlosTemplDialog::~FlosTemplDialog( )
{
  delete m_fixCalcDia;
  delete m_timePartDialog;
  delete m_matPartDialog;
}

// Check if the template was modified in the dialog
bool FlosTemplDialog::templModified()
{
    bool modified = false;

    QString str = m_text->toPlainText();
    modified = str != m_template->getText();

    modified = modified || (m_unit->currentText() != m_template->unit().einheitSingular());

    modified = modified || (m_addTime->isChecked() != m_template->hasTimeslice());

    str = spBenefit->cleanText();
    bool b;
    int new_val = str.toInt(&b);
    modified = modified || ( b && new_val != qRound(m_template->getBenefit()));

    // calculation kind
    CatalogTemplate::CalculationType currCalcType = m_template->calcKind();
    modified = modified || (currCalcType != _origCalcType);

    modified = modified || _calcPartsModified;

    return modified;
}

void FlosTemplDialog::accept()
{
    if( m_template ) {
        // qDebug () << "Saving template ID " << m_template->getTemplID();

        QString h;
        h = m_text->toPlainText();

        if( h != m_template->getText() ) {
            // qDebug () << "Template Text dirty -> update";
            m_template->setText( h );
        }

        h = m_unit->currentText();
        if( h != m_template->unit().einheitSingular()) {
            // qDebug () << "Template Einheit dirty -> update to " << h;
            m_template->setUnitId( UnitManager::self()->getUnitIDSingular(h));
        }

        /* count time */
        bool c = m_addTime->isChecked();
        if( c != m_template->hasTimeslice() ) {
            m_template->setHasTimeslice(c);
        }

        /* benefit */
        h = spBenefit->cleanText();
        bool b;
        int new_val = h.toInt(&b);
        if( b && new_val != qRound(m_template->getBenefit())) {
            m_template->setBenefit(new_val);
            // qDebug () << "benefit dirty ->update to " << g;
        }

        // Calculationtype
        int selId = m_gbPriceSrc->checkedId();
        CatalogTemplate::CalculationType calcType = CatalogTemplate::Unknown;
        if( selId == 0 ) {
            calcType = CatalogTemplate::ManualPrice;
        } else if( selId == 1 ) {
            calcType = CatalogTemplate::Calculation;
        } else {
            // qDebug () << "ERROR: Calculation type not selected, id is " << selId;
        }
        m_template->setCalculationType( calcType );

        // reread the manual price
        double dd = m_manualPriceVal->value();
        m_template->setManualPrice( Geld( dd ) );

        h = cbChapter->currentText();
        // qDebug () << "catalog chapter is " << h;

        _calcPartsModified = false;

        if( m_template->save() ) {
            emit( editAccepted( m_template ) );
            KatalogMan::self()->notifyKatalogChange( m_katalog, m_template->getTemplID() );
        } else {
            QMessageBox::warning(0, i18n("Template Error"), i18n("Saving of this template failed, sorry"));

        }
    }
    // qDebug () << "*** Saving finished ";
    QDialog::accept();
}

void FlosTemplDialog::reject()
{
  if(confirmClose() == true) {
    // let QDialog clean away the dialog.
    QDialog::reject();
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
    if( templModified() ) {
        QMessageBox msgBox;
        msgBox.setText(i18n("The template has been modified."));
        msgBox.setInformativeText(i18n("Do you want to discard your changes?"));
        msgBox.setStandardButtons(QMessageBox::Discard | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Cancel);
        int ret = msgBox.exec();

        if( ret == QMessageBox::Cancel  ) {
            return false;
        }
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

    return true;
}

bool FlosTemplDialog::askChapterChange( FloskelTemplate*, int )
{
    QMessageBox msgBox;
    msgBox.setText(i18n( "The catalog chapter was changed for this template.\n"
                         "Do you really want to move the template to the new chapter?"));
    msgBox.setInformativeText(i18n("Chapter Change"));
    msgBox.setStandardButtons(QMessageBox::Yes| QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::Yes);
    int ret = msgBox.exec();

    return( ret == QMessageBox::Yes);
}

void FlosTemplDialog::slManualPriceChanged(double dd)
{
    qDebug () << "Changing manual price:" << dd;
  if( ! m_template ) return;
  // qDebug () << "Updating manual price!";
  m_template->setManualPrice( Geld( dd ));
  refreshPrices();
}

double FlosTemplDialog::benefitValue()
{
    double val = 0;
    CalcPartList cparts = m_template->getCalcPartsList();

    // Currently still all calcparts have the same value of benefit.
    // once this changes, this needs to be fixed accordingly.
    if( cparts.size() > 0 ) {
        val = cparts.at(0)->getProzentPlus();
    }

    return val;
}

void FlosTemplDialog::slBenefitChange( int newBen )
{
    int oldBen = qRound(m_template->getBenefit());
    if( oldBen != newBen ) {
        m_template->setBenefit(newBen);
        refreshPrices();
        _calcPartsModified = true;
    }
}

void FlosTemplDialog::slAddFixPart()
{
  if( ! m_template ) return;

  FixCalcDialog dia(this);

  if( dia.exec() == QDialog::Accepted )
  {
    FixCalcPart *cp = new FixCalcPart( dia.getName(), dia.getPreis());
    cp->setMenge( dia.getMenge());
    cp->setProzentPlus(benefitValue());

    cp->setDirty(true);

    QTreeWidgetItem *lvItem = new QTreeWidgetItem( m_fixParts);
    drawFixListEntry( lvItem, cp );
    mCalcPartDict.insert( lvItem, cp );
    m_template->addCalcPart( cp );
    refreshPrices();
    _calcPartsModified = true;
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
    _calcPartsModified = true;
  }
}

void FlosTemplDialog::slEditFixPart()
{
  if( ! m_template || !m_fixParts ) return;

  // qDebug () << "Edit fix part!";

  QTreeWidgetItem *item = m_fixParts->currentItem();

  if( item )
  {
    FixCalcPart *cp = static_cast<FixCalcPart*>(mCalcPartDict[item]);
    if( cp )
    {
      m_fixCalcDia = new FixCalcDialog(this);
      m_fixCalcDia->setCalcPart(cp);
      connect( m_fixCalcDia, SIGNAL(fixCalcPartChanged(FixCalcPart*)),
               this, SLOT(slFixCalcPartChanged(FixCalcPart*)));
      m_fixCalcDia->show();
    }
  }
}

void FlosTemplDialog::slFixCalcPartChanged(FixCalcPart *cp)
{
  refreshPrices();
  _calcPartsModified = true;
  drawFixListEntry(m_fixParts->currentItem(), cp);
}

void FlosTemplDialog::slTimeCalcPartChanged(TimeCalcPart *cp)
{
  refreshPrices();
  _calcPartsModified = true;
  drawTimeListEntry(m_timeParts->currentItem(), cp);
}

void FlosTemplDialog::slAddTimePart()
{
  if( ! m_template ) return;
  TimeCalcDialog dia(this);

  if( dia.exec() == QDialog::Accepted ) {
    TimeCalcPart *cp = new TimeCalcPart( dia.getName(), dia.getDauer(),
                                         TimeCalcPart::timeUnitFromString(dia.unitStr()),
                                         0 );
    cp->setGlobalStdSetAllowed( dia.allowGlobal());
    StdSatz std = StdSatzMan::self()->getStdSatz( dia.getStundensatzName());
    cp->setStundensatz( std );
    cp->setProzentPlus(benefitValue());
    QTreeWidgetItem *lvItem = new QTreeWidgetItem( m_timeParts);
    drawTimeListEntry( lvItem, cp );
    mCalcPartDict.insert( lvItem, cp );
    m_template->addCalcPart( cp );
    refreshPrices();
    _calcPartsModified = true;
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
  it->setText( 1, QString::number(cp->duration())
               + QLatin1Char(' ') + TimeCalcPart::timeUnitString(cp->timeUnit()));
  it->setText( 2, cp->getStundensatz().getName());
  it->setText( 3, cp->globalStdSetAllowed() ? i18n("Yes") : i18n("No"));
}

void FlosTemplDialog::drawFixListEntry( QTreeWidgetItem* it, FixCalcPart *cp )
{
  if( !( it && cp) )
    return;

  it->setText( 0, DefaultProvider::self()->locale()->toString(cp->getMenge()));
  it->setText( 1, cp->getName());
  it->setText( 2, cp->unitPreis().toLocaleString());
  it->setText( 3, cp->basisKosten().toLocaleString());
}

void FlosTemplDialog::drawMatListEntry( QTreeWidgetItem *it, MaterialCalcPart *mc )
{
  it->setText( 0, mc->getName());
  it->setText( 1, QString::number(mc->getCalcAmount(), 'f',2));
  it->setText( 2, mc->getMaterial()->unit().einheitSingular());
  it->setText( 3, mc->basisKosten().toLocaleString());
  it->setText( 4, QString::number(mc->getMaterial()->getAmountPerPack(), 'f',2));
  it->setText( 5, mc->getMaterial()->salesPrice().toLocaleString());
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
    _calcPartsModified = true;
  }
}

void FlosTemplDialog::slEditTimePart()
{
  if( ! m_template || !m_timeParts ) return;

  // qDebug () << "Edit time part!";

  QTreeWidgetItem *item = m_timeParts->currentItem();

  if( item ) {
    TimeCalcPart *cp = static_cast<TimeCalcPart*>(mCalcPartDict[item]);
    if( cp ) {
      m_timePartDialog = new TimeCalcDialog(this);
      m_timePartDialog->setTimeCalcPart(cp);
      connect( m_timePartDialog, SIGNAL(timeCalcPartChanged(TimeCalcPart*)),
               this, SLOT(slTimeCalcPartChanged(TimeCalcPart*)) );
      m_timePartDialog->show();
    } else {
      // qDebug () << "No time calc part found for this item";
    }
  } else {
    // qDebug () << "No current Item!";
  }
  refreshPrices();
  _calcPartsModified = true;
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
    // qDebug () << "Material ID: " << matID;

    // TODO: Checken, ob der richtige Tab aktiv ist.
    // TODO: Check if the material is already in the calcpart (is this really needed??)
    MaterialCalcPart *mc = new MaterialCalcPart(matID, 0, amount);
    if( mc && ! mc->getMaterial() ) {
        // the material is still unknown to the catalog because it was just entered
        // in the material catalog
        qDebug() << "ERR: MaterialCalcPart without Material!";
        return;
    }

    if( mc ) {
        mc->setProzentPlus(benefitValue());

        m_template->addCalcPart( mc );
        QTreeWidgetItem *lvItem = new QTreeWidgetItem(m_matParts);
        drawMatListEntry( lvItem, mc );
        mCalcPartDict.insert( lvItem, mc );
        refreshPrices();
        _calcPartsModified = true;
    }
}


void FlosTemplDialog::slEditMatPart()
{
  if( ! m_template || !m_matParts ) return;

  // qDebug () << "Edit Material part!";

  QTreeWidgetItem *item = m_matParts->currentItem();

  MaterialCalcPart *mc = static_cast<MaterialCalcPart*>( mCalcPartDict[item] );

  if( mc ) {
      m_matPartDialog = new MatCalcDialog( mc, this);

      connect( m_matPartDialog, SIGNAL(matCalcPartChanged(MaterialCalcPart*)),
               this, SLOT(slMatCalcPartChanged(MaterialCalcPart*)));
      m_matPartDialog->setModal(true);
      m_matPartDialog->show();
    } else {
      // qDebug () << "No such MaterialCalcPart!";
    }
}

void FlosTemplDialog::slMatCalcPartChanged(MaterialCalcPart *mc)
{
  drawMatListEntry(m_matParts->currentItem(), mc);
  refreshPrices();
  _calcPartsModified = true;
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
    _calcPartsModified = true;
  }
}

/*
 * Slot for managing the switch from manual to calculated price
 * and vice versa
 */
void FlosTemplDialog::slCalcOrFix(int button)
{
  bool ok = true;
  bool manualEnabled = true;

  if( button == 0 ) {
    /* switched to manual price */
    if( m_template ) {
        m_template->setCalculationType( CatalogTemplate::ManualPrice );
    }
  } else if( button == 1 ) {
    /* switched to calculated */
    if( m_template ) {
      m_template->setCalculationType( CatalogTemplate::Calculation );
    }
    manualEnabled = false;
  } else {
    /* unknown knob*/
    ok = false;
  }

  if( ok ) {
      m_manualPriceVal->setEnabled( manualEnabled );

      m_textTimePart->setEnabled(!manualEnabled);
      m_textFixPart->setEnabled(!manualEnabled);
      m_textMaterialPart->setEnabled(!manualEnabled);

      m_tLabelMat->setEnabled(!manualEnabled);
      m_tLabelFix->setEnabled(!manualEnabled);
      m_tLabelTime->setEnabled(!manualEnabled);
      m_tLabelProfit->setEnabled(!manualEnabled);
      spBenefit->setEnabled(!manualEnabled);

      refreshPrices();
  }
}


void FlosTemplDialog::slSetNewText()
{
    QPushButton *okButton = _buttonBox->button(QDialogButtonBox::Ok);

    if( ! m_text || m_text->toPlainText().isEmpty() ) {
        okButton->setEnabled(false);
    } else {
        okButton->setEnabled(true);
    }

    if( m_text ) {
        const QString t = Portal::textWrap( m_text->toPlainText(), 80, 5 );
        const QStringList li = t.split(QLatin1Char('\n'));
        QString longest;
        for( const QString& p : li ) {
            if( p.length() > longest.length() )
                longest = p;
        }
        QFontMetrics fm(m_textDispFix->font());
        int w = 10+fm.width(longest);

        if( m_textDispTime) {
            m_textDispTime->setText(t);
            m_textDispTime->setMinimumWidth(w);
        }
        if( m_textDispFix) {
            m_textDispFix->setText(t);
            m_textDispFix->setMinimumWidth(w);
        }
        if( m_textDispMat) {
            m_textDispMat->setText(t);
            m_textDispMat->setMinimumWidth(w);
        }
    }
}
/* END */
