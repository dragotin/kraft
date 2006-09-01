/***************************************************************************
             flostempldialog  -
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
#include <qtextedit.h>
#include <qradiobutton.h>
#include <qlabel.h>
#include <qstring.h>
#include <qcombobox.h>
#include <qlistview.h>
#include <qcheckbox.h>
#include <qbuttongroup.h>
#include <qpushbutton.h>

// include files for KDE
#include <klocale.h>
#include <kdebug.h>
#include <kglobal.h>
#include <knuminput.h>
#include <kmessagebox.h>
#include <kcombobox.h>
#include <kpushbutton.h>

#include "floskeltemplate.h"
#include "flostempldialog.h"
#include "unitmanager.h"
#include "zeitcalcpart.h"
#include "fixcalcpart.h"
#include "materialcalcpart.h"
#include "matcalcdialog.h"
#include "stockmaterial.h"
#include "stockmaterialman.h"
#include "zeitcalcdialog.h"
#include "fixcalcdialog.h"
#include "stdsatzman.h"
#include "mateditor.h"
#include "katalogman.h"
#include "katalog.h"

FlosTemplDialog::FlosTemplDialog( QWidget *parent, const char* name, bool modal, WFlags fl)
    : d_calcTempl(parent, name, modal, fl),
      m_template(0),
      m_saveTempl(0),
      m_katalog(0),
      m_matEdit(0)
{
    /* connect a value Changed signal of the manual price field */
    connect( m_manualPriceVal, SIGNAL( valueChanged(double)),
             this, SLOT( slManualPriceChanged(double)));

    connect( m_text, SIGNAL(textChanged()),this, SLOT(slSetNewText()));
}

void FlosTemplDialog::setVorlage( FloskelTemplate *t, const QString& katalogname )
{
    if( ! t ) return;
    m_template = t;
    m_saveTempl = new FloskelTemplate(*t);

    m_katalog = KatalogMan::self()->getKatalog(katalogname);

    if( m_katalog == 0 ) {
        kdDebug() << "ERR: Floskel Dialog called without valid Katalog!" << endl;
        return;
    }

    cbChapter->insertStringList( m_katalog->getKatalogChapters() );
    int chapID = t->getChapterID();
    QString chap = m_katalog->chapterName(dbID(chapID));
    cbChapter->setCurrentText(chap);

    /** der text der Vorlage **/
    m_text->setText( t->getText());

    /* Einheit */
    m_unit->clear();
    m_unit->insertStringList( UnitManager::allUnits());
    m_unit->setCurrentText( m_template->einheit().einheitSingular() );

    m_manualPriceVal->setValue( t->einheitsPreis().toDouble());

    /* Kalkulationsart: Manuell oder Kalkuliert? */

    if( t->calcKind() == ManualPrice )
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

    /* anzeige der verschiedenen Kalkulationsteile */
    setCalcparts();

    /* text setzen */
    slSetNewText();

    pbRundPreis->setEnabled(false);
}

void FlosTemplDialog::setCalcparts( )
{
    /* Zeitkalkulation in widget m_timeParts */
    CalcPartList tpList = m_template->getCalcPartsList( KALKPART_TIME );

    ZeitCalcPart *cp;
    cp = static_cast<ZeitCalcPart*>(tpList.first());

    m_timeParts->clear();

    for( ; cp; cp = static_cast<ZeitCalcPart*>(tpList.next()) )
    {
        QString stdStd = i18n("No");
        if( cp->globalStdSetAllowed() ) stdStd = i18n("Yes");

        QListViewItem *lvItem = new QListViewItem( m_timeParts );
        drawTimeListEntry( lvItem, cp );
	m_cpDict.insert(lvItem, cp );
    }

    /* Fixpart Kalkulationsanteile */
    m_fixParts->clear();
    tpList = m_template->getCalcPartsList( KALKPART_FIX );
    FixCalcPart *fc;
    fc = static_cast<FixCalcPart*>(tpList.first());
    for( ; fc; fc = static_cast<FixCalcPart*>(tpList.next()) )
    {
        QListViewItem *lvItem = new QListViewItem( m_fixParts );
        drawFixListEntry( lvItem, fc );
        m_cpDict.insert( lvItem, fc );
    }

    /* Materialpart Kalkulationsanteile */
    m_matParts->clear();
    m_matParts->setRootIsDecorated(true);
    tpList = m_template->getCalcPartsList( KALKPART_MATERIAL );
    MaterialCalcPart *mc;
    mc = static_cast<MaterialCalcPart*>(tpList.first());
    for( ; mc; mc = static_cast<MaterialCalcPart*>(tpList.next()) )
    {
        // QListViewItem *matPart = new QListViewItem( m_matParts,
        //                                            mc->getName() );
        // m_cpDict.insert( matPart, mc );

        // matPart->setOpen(true);
        StockMaterialList matList = mc->getCalcMaterialList();

        StockMaterialListIterator it( matList );

        StockMaterial *mat;
        while ( (mat = it.current()) != 0 )
        {
            ++it;
            QListViewItem *lvItem = new QListViewItem( m_matParts );
            m_matDict.insert( lvItem, mat );
            drawMatListEntry( lvItem, mc, mat );

	    // This is a bit tricky: We more material entries in the listview
 	    // for the same materila calcpart. Thus we insert an entry for every
  	    // listview item pointing to the same material calc part
	    m_cpDict.insert(lvItem, mc);
        }
    }
}


void FlosTemplDialog::refreshPrices()
{
    if( ! m_template ) return;

    /* Preisbezeichnung setzen */
    QString t;
    t = i18n("Calculated Price: ");
    int kType = m_template->calcKind();
    kdDebug() << "CalcType in integer is " << kType << endl;
    if( m_template->calcKind() == ManualPrice )
    {
        t = i18n("Manual Price: ");
    }
    else if( m_template->calcKind() == Calculation )
    {
        int gewinn = spGewinn->value();
        QString gewinnStr = i18n("(+%1%)").arg(gewinn);

        if( gewinn < 0 )
        {
            gewinnStr = "<font color=\"red\">"+i18n("%1%").arg(gewinn)+"</font>";
        }
        gewinnStr += i18n(": ");
        t += gewinnStr;
    }
    else
    {
        kdDebug() << "ERR: Unbekannter Kalkulationstyp!" << endl;
    }
    m_resPreisName->setText(t);

    /* Preis setzen */
    t = m_template->einheitsPreis().toString();
    m_resultPrice->setText("<font size=\"+1\"><b>"+t+"</b></font>");

    /* Preisteile nach Zeit-, Fix- und Materialkalkulation */
    Geld g( m_template->kostenPerKalcPart( KALKPART_TIME ));
    m_textTimePart->setText( g.toString());

    g = m_template->kostenPerKalcPart( KALKPART_FIX );
    m_textFixPart->setText( g.toString());

    g = m_template->kostenPerKalcPart( KALKPART_MATERIAL );
    m_textMaterialPart->setText(g.toString());

}

FlosTemplDialog::~FlosTemplDialog( )
{
    if( m_saveTempl ) delete m_saveTempl;
}

void FlosTemplDialog::accept()
{
    if( m_template )
    {
        kdDebug() << "Saving template ID " << m_template->getTemplID() << endl;

        QString h;
        h = m_text->text();

        if( h != m_template->getText() ) {
            kdDebug() << "Template Text dirty -> update" << endl;
            m_template->setText( h );
        }

        h = m_unit->currentText();
        if( h != m_template->einheit().einheitSingular()) {
            kdDebug() << "Template Einheit dirty -> update to " << h << endl;
            m_template->setEinheitId( UnitManager::getUnitIDSingular(h));
        }

        /* katalog chapter vergleichen */
        int chapID = m_katalog->chapterID(cbChapter->currentText());
        if( chapID != m_template->getChapterID() ) {
            kdDebug() << "Chapter ID dirty ->update" << endl;
	       if( askChapterChange( m_template, chapID )) {
             	m_template->setChapterID(chapID);
              emit( chapterChanged(chapID));
	       }
        }

        /* Zeit zählen */
        bool c = m_addTime->isChecked();
        if( c != m_template->hasTimeslice() ) {
            m_template->setHasTimeslice(c);
        }

        /* Gewinn */
        h = spGewinn->cleanText();
        bool b;
        double g = h.toDouble( &b );
        if( b  && g != m_template->getGewinn() ) {
            m_template->setGewinn(g);
            kdDebug() << "Gewinn dirty ->update to " << g << endl;
        }

        h = cbMwst->currentText();
        // TODO!

        // Calculationtype
        int selId = m_gbPriceSrc->selectedId();
        CalculationType calcType = Unknown;
        if( selId == 0 ) {
            calcType = ManualPrice;
        } else if( selId == 1 ) {
            calcType = Calculation;
        } else {
            kdDebug() << "ERROR: Calculation type not selected, id is " << selId << endl;
        }
        m_template->setCalculationType( calcType );

        h = cbChapter->currentText();
        kdDebug() << "Katalogkapitel ist " << h << endl;

        if( m_template->save() ) {
            emit( editAccepted( m_template ) );
            d_calcTempl::accept();
        } else {
            KMessageBox::error( this, i18n("Saving of this template failed, sorry"),
                i18n( "Template Save Error" ) );
        }
    }
    kdDebug() << "*** Saving finished " << endl;
}

bool FlosTemplDialog::askChapterChange( FloskelTemplate*, int )
{
    if( KMessageBox::questionYesNo( this,
        i18n( "The catalog chapter was changed for this template.\nDo you really want to move the template to the new chapter?"),
        i18n("Changed Chapter"), KStdGuiItem::yes(), KStdGuiItem::no(),
        "chapterchange" ) == KMessageBox::Yes )
    {
        return true;

    } else {
        return false;
    }
}

void FlosTemplDialog::reject()
{
    if( m_saveTempl )
    {
        kdDebug() << "Reverting to saved Template" << endl;
        *m_template = *m_saveTempl;
    }
    d_calcTempl::reject();
}

void FlosTemplDialog::slManualPriceChanged(double dd)
{
    if( ! m_template ) return;
    m_template->setManualPrice( Geld( dd ));
    refreshPrices();
}


void FlosTemplDialog::slGewinnChange( int neuPreis )
{
    CalcPart *cp;
    CalcPartList tpList = m_template->getCalcPartsList( );
    cp = static_cast<CalcPart*>(tpList.first());
    for( ; cp; cp = static_cast<CalcPart*>(tpList.next()) )
    {
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

        QListViewItem *lvItem = new QListViewItem( m_fixParts);
        drawFixListEntry( lvItem, cp );
        m_cpDict.insert( lvItem, cp );
        m_template->addCalcPart( cp );
        refreshPrices();
    }
}

void FlosTemplDialog::slRemoveFixPart()
{
    if( ! m_template || ! m_fixParts ) return;

    QListViewItem *item = m_fixParts->currentItem();

    if( item )
    {
        CalcPart *cp = m_cpDict[item];
        if( cp )
        {
            m_template->removeCalcPart(cp);
        }
        m_fixParts->takeItem(item);

        refreshPrices();
    }
}

void FlosTemplDialog::slEditFixPart()
{
    if( ! m_template || !m_fixParts ) return;

    kdDebug() << "Edit fix part!" << endl;

    QListViewItem *item = m_fixParts->currentItem();

    if( item )
    {
        FixCalcPart *cp = static_cast<FixCalcPart*>(m_cpDict[item]);
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

void FlosTemplDialog::slTimeCalcPartChanged(ZeitCalcPart *cp)
{
    refreshPrices();
    drawTimeListEntry(m_timeParts->currentItem(), cp);
}

void FlosTemplDialog::slAddTimePart()
{
    if( ! m_template ) return;
    ZeitCalcDialog dia(this);

    if( dia.exec() == QDialog::Accepted )
    {
        ZeitCalcPart *cp = new ZeitCalcPart( dia.getName(), dia.getDauer(), 0 );
        cp->setGlobalStdSetAllowed( dia.allowGlobal());
        StdSatz std = StdSatzMan::getStdSatz( dia.getStundensatzName());
        cp->setStundensatz( std );
        QListViewItem *lvItem = new QListViewItem( m_timeParts);
        drawTimeListEntry( lvItem, cp );
        m_cpDict.insert( lvItem, cp );
        m_template->addCalcPart( cp );
        refreshPrices();
    }
}

/*
 * stellt einen ZeitCalcPart als ListViewItem dar. Wird gebraucht, wenn das
 * item neu ist, aber auch beim Editieren
 */
void FlosTemplDialog::drawTimeListEntry( QListViewItem *it, ZeitCalcPart *cp )
{

    if( !( it && cp) )
        return;

    it->setText( 0, cp->getName());
    it->setText( 1, i18n("%1 Min.").arg(cp->getMinuten()));
    it->setText( 2, cp->getStundensatz().getName());
    it->setText( 3, cp->globalStdSetAllowed() ? i18n("Yes") : i18n("No"));
    it->repaint();
}

void FlosTemplDialog::drawFixListEntry( QListViewItem* it, FixCalcPart *cp )
{
    if( !( it && cp) )
        return;

    it->setText( 0, KGlobal().locale()->formatNumber(cp->getMenge()));
    it->setText( 1, cp->getName());
    it->setText( 2, cp->unitPreis().toString());
    it->setText( 3, cp->basisKosten().toString());
    it->repaint();
}

void FlosTemplDialog::drawMatListEntry( QListViewItem *it, MaterialCalcPart *mc, StockMaterial *mat )
{
    it->setText( 0, mat->getName());
    it->setText( 1, QString::number(mc->getCalcAmount( mat ), 'f',2));
    it->setText( 2, mat->getUnit().einheitSingular());
    it->setText( 3, mc->getPriceForMaterial(mat).toString());
    it->setText( 4, QString::number(mat->getAmountPerPack(), 'f',2));
    it->setText( 5, mat->getVPreis().toString());
    it->repaint();
}


void FlosTemplDialog::slRemoveTimePart()
{
    if( ! m_template || !m_timeParts ) return;

    QListViewItem *item = m_timeParts->currentItem();

    if( item )
    {
        CalcPart *cp = m_cpDict[item];
        if( cp )
        {
            m_template->removeCalcPart(cp);
        }
        m_timeParts->takeItem(item);

        refreshPrices();
    }
}

void FlosTemplDialog::slEditTimePart()
{
    if( ! m_template || !m_timeParts ) return;

    kdDebug() << "Edit time part!" << endl;

    QListViewItem *item = m_timeParts->currentItem();

    if( item )
    {
        ZeitCalcPart *cp = static_cast<ZeitCalcPart*>(m_cpDict[item]);
        if( cp )
        {
          m_timePartDialog = new ZeitCalcDialog(cp, this);
          m_timePartDialog->setModal(true);
          connect(m_timePartDialog, SIGNAL(timeCalcPartChanged(ZeitCalcPart*)),
                  this, SLOT(slTimeCalcPartChanged(ZeitCalcPart*)));
	  m_timePartDialog->show();
        } else {
	  kdDebug() << "No time calc part found for this item" << endl;
	}
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

    /* Zuerst Materialeditor zur Auswahl des Materials �fnen */
    if( ! m_matEdit)
    {
        m_matEdit = new MatEditor( i18n("Material Catalog"), true, this,
                                   "KalcMaterialEdit", false ); // , Qt::WDestructiveClose);

    connect( m_matEdit, SIGNAL(materialSelected(int, double)),
             this,      SLOT(slNewMaterial(int, double)));
    connect( this,      SIGNAL(takeMaterialAnswer(const QString&)),
             m_matEdit, SLOT(slGotAnswer( const QString& )));
    }

    m_matEdit->show();

}

/*
 * Slot, der aufgerufen wird, wenn im Materialeditor ein Material zur Kalkulation
 * geschickt wird.
 */
void FlosTemplDialog::slNewMaterial( int matID, double amount )
{
    kdDebug() << "Material ID: " << matID << endl;

    // TODO: Checken, ob der richtige Tab aktiv ist.
    StockMaterial *mat = StockMaterialMan::getMaterial(matID);

    if( mat )
    {
        CalcPartList tpList = m_template->getCalcPartsList( KALKPART_MATERIAL );
        MaterialCalcPart *mc;
        mc = static_cast<MaterialCalcPart*>(tpList.first());

        bool addMat = true;
        QListViewItem *lvItem = 0;

        if( ! mc )
        {
            /* Es gibt noch garkeinen Material-Kalkulationsanteil. Es wird
               ein neuer angelegt
             */
            mc = new MaterialCalcPart( stdMaterialKalcPartName(), 0);
            m_template->addCalcPart( mc );
        }
        else
        {
            /* Der Material-Kalkulationsanteil existiert bereits. Hier checken, ob es
               das vermeintlich neue material bereits in der Liste gibt */
            if( mc->containsMaterial( matID ) )
            {
                double oldAmount = mc->getCalcAmount(mat);

                addMat = false;

                if(oldAmount != amount )
                {
                    QString quest = i18n("This material kind is alreadz in the list of calculated materials.\n");
                    quest += i18n("Do you want to adjust the amount of it in the calculation?");

                    int answer = KMessageBox::questionYesNo( this,
                                                             quest,
                                                             i18n("Material duplicate"));

                    if( answer == KMessageBox::Yes )
                    {
                        mc->setCalcAmount( mat, amount );

                        QPtrDictIterator<StockMaterial> it(m_matDict);

                        /* Das entsprechende Listitem raussuchen */
                        for( ; !lvItem && it.current(); ++it ) {
                            StockMaterial *listmat = static_cast<StockMaterial*>(it.current());
                            if(  listmat == mat )
                                lvItem = static_cast<QListViewItem*>(it.currentKey());
                        }
                        if( lvItem )
                        {
                            drawMatListEntry( lvItem, mc, mat );
                            refreshPrices();
                            emit( takeMaterialAnswer(i18n("Amount changed")));
                        }
                    }
                    else
                    {
                        /* Menge sollte nciht angepasst werden */
                    }
                }
                else
                {
                    /* Die Anzahl fr das MAterial unterscheidet sich nicht => nix tun. */
                    emit( takeMaterialAnswer( i18n("Material amount already in calculation.")));
                }
            }
        }

        if( addMat )
        {
            if( mc )
            {
                mc->addMaterial(amount, matID);
                mc->setDirty(true);
                lvItem = new QListViewItem( m_matParts );
                m_matDict.insert( lvItem, mat);
                drawMatListEntry( lvItem, mc, mat );
                refreshPrices();
                emit( takeMaterialAnswer(i18n("Material inserted to calculation")));
            }
            else
            {
                /* Es gibt garkein Material-Kalcpart */
                kdDebug() << "WRN: No material calculation part found!" << endl;
            }
        }
    }
}


void FlosTemplDialog::slEditMatPart()
{
    if( ! m_template || !m_matParts ) return;

    kdDebug() << "Edit Material part!" << endl;

    QListViewItem *item = m_matParts->currentItem();

    /*
     * es gibt vorerst nur einen Material Calcpart pro template, der einen
     * Standard-Namen hat. Deshalb wird hier khn der erste eintrag in der
     * List verwendet.
     */
    CalcPartList cParts = m_template->getCalcPartsList( KALKPART_MATERIAL );
    MaterialCalcPart *mc;
    mc = static_cast<MaterialCalcPart*>(cParts.first());

    if( item )
    {
        StockMaterial *mat = static_cast<StockMaterial*>(m_matDict[item]);

        if( mat )
        {
          double amount = mc->getCalcAmount(mat);
          m_matPartDialog = new MatCalcDialog( amount, mat, this);

          connect( m_matPartDialog, SIGNAL(matCalcPartChanged(StockMaterial*, double)),
                   this, SLOT(slMatCalcPartChanged(StockMaterial*, double)));
          m_matPartDialog->setModal(true);
          m_matPartDialog->show();

        }
    }
}

void FlosTemplDialog::slMatCalcPartChanged(StockMaterial *mat, double amount)
{
    CalcPartList cParts = m_template->getCalcPartsList( KALKPART_MATERIAL );
    MaterialCalcPart *mc = static_cast<MaterialCalcPart*>(cParts.first());

    if(mc && mat) {
      mc->setCalcAmount(mat, amount);
      drawMatListEntry(m_matParts->currentItem(), mc, mat);
      refreshPrices();
    }
}

void FlosTemplDialog::slRemoveMatPart()
{
    if( ! m_template || ! m_matParts ) return;

    QListViewItem *item = m_matParts->currentItem();

    if( item )
    {
        CalcPartList cParts = m_template->getCalcPartsList( KALKPART_MATERIAL );
        MaterialCalcPart *mc;
        mc = static_cast<MaterialCalcPart*>(cParts.first());

        StockMaterial *mat = static_cast<StockMaterial*>(m_matDict[item]);

        mc->removeMaterial( mat );
        m_matParts->takeItem(item);
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
            m_template->setCalculationType(ManualPrice);

        m_manualPriceVal->setEnabled( true );

        m_textTimePart->setEnabled(false);
        m_textFixPart->setEnabled(false);
        m_textMaterialPart->setEnabled(false);

        m_tLabelMat->setEnabled(false);
        m_tLabelFix->setEnabled(false);
        m_tLabelTime->setEnabled(false);
        spGewinn->setEnabled(false);
    }
    else if( button == 1 )
    {
        /* auf kalkuliert geschaltet */
        if( m_template )
            m_template->setCalculationType(Calculation);

        m_manualPriceVal->setEnabled( false );

        m_textTimePart->setEnabled(true);
        m_textFixPart->setEnabled(true);
        m_textMaterialPart->setEnabled(true);

        m_tLabelMat->setEnabled(true);
        m_tLabelFix->setEnabled(true);
        m_tLabelTime->setEnabled(true);
        spGewinn->setEnabled(true);
    }
    else
    {
        /* unbekannter knopf -> fehler */
        kdDebug() << "--- Error: Falsche Button ID" << endl;
        ok = false;
    }

    if( ok )
    {
        refreshPrices();
    }
}


void FlosTemplDialog::slSetNewText( )
{
    if( ! m_text || m_text->text().isEmpty() ) {
        buttonOk->setEnabled(false);
    } else {
        buttonOk->setEnabled(true);
    }

    if( m_text ) {
        QString t = m_text->text();

        if( m_textDispTime)
            m_textDispTime->setText(t);
        if( m_textDispFix)
            m_textDispFix->setText(t);
        if( m_textDispMat)
            m_textDispMat->setText(t);
    }

}
/* END */


#include "flostempldialog.moc"
