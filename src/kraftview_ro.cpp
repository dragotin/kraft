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
#include <qsizepolicy.h>
#include <qsignalmapper.h>
#include <qtabwidget.h>
#include <qcolor.h>
#include <qsplitter.h>
#include <qtooltip.h>
#include <qfont.h>

#include <kdebug.h>
#include <kdialog.h>
#include <kpushbutton.h>
#include <kcombobox.h>
#include <kdatewidget.h>
#include <knuminput.h>
#include <kactioncollection.h>
#include <kmessagebox.h>
#include <khtmlview.h>
#include <kiconloader.h>
#include <kvbox.h>

// application specific includes
#include "kraftdb.h"
#include "kraftsettings.h"
#include "kraftview_ro.h"
#include "kraftdoc.h"
#include "portal.h"
#include "ui_docheader.h"
#include "docassistant.h"
#include "positionviewwidget.h"
#include "ui_docfooter.h"
#include "docposition.h"
#include "unitmanager.h"
#include "docpostcard.h"
#include "kataloglistview.h"
#include "katalogman.h"
#include "templkatalog.h"
#include "templkataloglistview.h"
#include "catalogselection.h"
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

#include <qtimer.h>
#include "doclocaledialog.h"
#include <kstandarddirs.h>
#include "texttemplate.h"
#include "documentman.h"

// #########################################################

KraftViewRO::KraftViewRO(QWidget *parent, const char *name) :
    KraftViewBase( parent )
{
  setObjectName( name );
  setModal( false );
  setCaption( i18n("Document" ) );
  setButtons( Close );
  m_type = ReadOnly;

  KVBox *w = new KVBox( parent );
  mGlobalVBox = w;
  setMainWidget( w );
  mGlobalVBox->setMargin( 3 );

  mHtmlView = new HtmlView( mGlobalVBox );
  mHtmlView->setStylesheetFile( "docoverview_ro.css" );
}

KraftViewRO::~KraftViewRO()
{

}

#define DOC_RO_TAG

void KraftViewRO::setup( DocGuardedPtr doc )
{
    KraftViewBase::setup( doc );

    if ( !doc ) return;

    KLocale *locale = doc->locale();
    if ( !locale ) locale = KGlobal::locale();

    // do stuff like open a template and render values into it.
    KStandardDirs stdDirs;
    QString templFileName = QString( "kraftdoc_ro.trml" );
    QString findFile = "kraft/reports/" + templFileName;

    QString tmplFile = stdDirs.findResource( "data", findFile );


    QByteArray kraftHome = qgetenv("KRAFT_HOME");

    if( !kraftHome.isEmpty() ) {
        QString file = QString( "%1/reports/kraftdoc_ro.trml").arg(QString::fromLocal8Bit(kraftHome));
        QFileInfo fi(file);
        if( fi.exists() && fi.isReadable() ) {
            tmplFile = file;
        }
    }
    if( tmplFile.isEmpty() ) {
        kDebug() << "Could not find template to render ro view of document.";
        return;
    }


    TextTemplate tmpl( tmplFile );
    if( !tmpl.open() ) {
        return;
    }
    tmpl.setValue( DOC_RO_TAG( "HEADLINE" ), doc->docType() + " " + doc->ident() );
    tmpl.setValue( DOC_RO_TAG( "DATE" ), locale->formatDate( doc->date(), KLocale::ShortDate ) );
    tmpl.setValue( DOC_RO_TAG( "DOC_TYPE" ),  doc->docType() );
    QString address = doc->address();
    address.replace( '\n', "<br/>" );
    tmpl.setValue( DOC_RO_TAG( "ADDRESS" ), address );
    tmpl.setValue( DOC_RO_TAG( "DOCNO" ), doc->ident() );
    tmpl.setValue( DOC_RO_TAG( "PRETEXT" ), doc->preText() );
    tmpl.setValue( DOC_RO_TAG( "POSTTEXT" ), doc->postText() );
    tmpl.setValue( DOC_RO_TAG( "SALUT" ), doc->salut() );
    tmpl.setValue( DOC_RO_TAG( "GOODBYE" ), doc->goodbye() );


    DocPositionList positions = doc->positions();

    // check the tax settings: If all items have the same settings, its not individual.
    bool individualTax = false;
    int ttype = -1;
    foreach( DocPositionBase *dp, positions  ) {
        if( ttype == -1 ) {
            ttype = dp->taxType();
        } else {
            if( ttype != dp->taxType() ) { // different from previous one?
                individualTax = true;
                break;
            }
        }
    }

    int pos = 1;
    int taxFreeCnt = 0;
    int reducedTaxCnt = 0;
    int fullTaxCnt = 0;

    QString docType = doc->docType();
    DocType dt(docType);

    foreach( DocPositionBase *dpb, positions ) {
        DocPosition *dp = static_cast<DocPosition*>(dpb);
        tmpl.createDictionary( "ITEMS" );

        tmpl.setValue( "ITEMS", "NUMBER", QString::number( pos++ ) );
        tmpl.setValue( "ITEMS", "TEXT", dp->text() );
        tmpl.setValue( "ITEMS", "AMOUNT", locale->formatNumber( dp->amount() ) );
        tmpl.setValue( "ITEMS", "UNIT", dp->unit().einheit( dp->amount() ) );
        double singlePrice = dp->unitPrice().toDouble();

        if( dt.pricesVisible() ) {
            tmpl.createSubDictionary("ITEMS", "PRICE_DISPLAY");
            tmpl.setValue( "PRICE_DISPLAY", "SINGLE_PRICE", locale->formatMoney( singlePrice ) );
            QString style( "positive" );
            if ( singlePrice < 0 ) {
                style = "negative";
            }

            tmpl.setValue( "PRICE_DISPLAY", "PRICE_STYLE", style );

            tmpl.setValue( "PRICE_DISPLAY", "PRICE", locale->formatMoney( dp->overallPrice().toDouble() ) );
        }
#if 0
        QString taxType;
        if( individualTax ) {
            if( dp->taxType() == 1 ) {
                taxFreeCnt++;
                taxType = "TAX_FREE";
            } else if( dp->taxType() == 2 ) {
                taxType = "REDUCED_TAX";
                reducedTaxCnt++;
            } else {
                // ATTENTION: Default for all non known tax types is full tax.
                fullTaxCnt++;
                taxType = "FULL_TAX";
            }
        }
#endif
    }

    if( dt.pricesVisible()) {
        tmpl.createDictionary("DISPLAY_SUM_BLOCK");

        tmpl.setValue( "DISPLAY_SUM_BLOCK", DOC_RO_TAG( "TAXLABEL" ), i18n( "VAT" ) );
        tmpl.setValue( "DISPLAY_SUM_BLOCK", DOC_RO_TAG( "REDUCED_TAXLABEL" ), i18n( "Reduced TAX" ) );
        tmpl.setValue( "DISPLAY_SUM_BLOCK", DOC_RO_TAG( "NETTOSUM" ), locale->formatMoney( doc->nettoSum().toDouble() ) );
        tmpl.setValue( "DISPLAY_SUM_BLOCK", DOC_RO_TAG( "BRUTTOSUM" ), locale->formatMoney( doc->bruttoSum().toDouble() ) );

        if( individualTax ) {
            tmpl.createSubDictionary( "DISPLAY_SUM_BLOCK", "TAX_FREE_ITEMS" );
            tmpl.setValue( "TAX_FREE_ITEMS", "COUNT", QString::number( taxFreeCnt ));

            tmpl.createSubDictionary( "DISPLAY_SUM_BLOCK", "REDUCED_TAX_ITEMS" );
            tmpl.setValue( "REDUCED_TAX_ITEMS", "COUNT", QString::number( reducedTaxCnt ));
            tmpl.setValue( "REDUCED_TAX_ITEMS", "TAX",
                           locale->formatNumber( DocumentMan::self()->reducedTax( doc->date() )));
            tmpl.createSubDictionary( "DISPLAY_SUM_BLOCK", "FULL_TAX_ITEMS" );
            tmpl.setValue( "FULL_TAX_ITEMS", "COUNT", QString::number( fullTaxCnt ));
            tmpl.setValue( "FULL_TAX_ITEMS", "TAX",
                           locale->formatNumber( DocumentMan::self()->tax( doc->date() )) );
        }

        double redTax = DocumentMan::self()->reducedTax( doc->date() );
        double fullTax = DocumentMan::self()->tax( doc->date() );
        QString h;
        if ( positions.reducedTaxSum( redTax ).toLong() > 0 ) {
            tmpl.createSubDictionary( "DISPLAY_SUM_BLOCK", "SECTION_REDUCED_TAX"  );
            tmpl.setValue( "SECTION_REDUCED_TAX", DOC_RO_TAG( "REDUCED_TAX_SUM" ),
                           positions.reducedTaxSum( redTax ).toString( positions.locale() ) );
            h.setNum( redTax, 'f', 1 );
            tmpl.setValue( "SECTION_REDUCED_TAX", DOC_RO_TAG( "REDUCED_TAX" ), h );
            tmpl.setValue( "SECTION_REDUCED_TAX", DOC_RO_TAG( "REDUCED_TAX_LABEL" ), i18n( "reduced VAT" ) );

        }
        if ( positions.fullTaxSum( fullTax ).toLong() > 0 ) {
            tmpl.createSubDictionary( "DISPLAY_SUM_BLOCK", "SECTION_FULL_TAX" );
            tmpl.setValue( "SECTION_FULL_TAX", DOC_RO_TAG( "FULL_TAX_SUM" ),
                           positions.fullTaxSum( fullTax ).toString( positions.locale() ) );
            h.setNum( fullTax, 'f', 1 );
            tmpl.setValue( "SECTION_FULL_TAX", DOC_RO_TAG( "FULL_TAX" ), h );
            tmpl.setValue( "SECTION_FULL_TAX", DOC_RO_TAG( "FULL_TAX_LABEL" ), i18n( "VAT" ) );
        }

        tmpl.setValue( "DISPLAY_SUM_BLOCK", DOC_RO_TAG( "TAXSUM" ), locale->formatMoney( doc->vatSum().toDouble() ) );
    } // Visible sum block

    setCaption( m_doc->docIdentifier() );

    mHtmlView->setTitle( doc->docIdentifier() );
    mHtmlView->displayContent( tmpl.expand() );
}

void KraftViewRO::done( int r )
{
  kDebug() << "View closed with ret value " << r;

  KraftDoc *doc = getDocument();

  if( !doc ) {
    kDebug() << "ERR: No document available in view, return!";
    return;
  }
  KraftSettings::self()->setRODocViewSize( size() );
  KraftSettings::self()->writeConfig();
  KraftSettings::self()->readConfig();

  emit viewClosed( true, m_doc );

  KraftViewBase::done(r);
}

