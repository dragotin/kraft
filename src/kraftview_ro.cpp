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

#include <kabc/addressbook.h>
#include <kabc/stdaddressbook.h>
#include <kabc/addresseedialog.h>
#include <kabc/addressee.h>


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
  KDialog( parent ),

  // name, false /* modal */, i18n("Document"),
  // 	      Close, Ok, true /* separator */ ),
  m_doc( 0 )
{
  setObjectName( name );
  setModal( false );
  setCaption( i18n("Document" ) );
  setButtons( Ok | Close );

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
  m_doc = doc;

  if ( !doc ) return;

  KLocale *locale = doc->locale();
  if ( !locale ) locale = KGlobal::locale();

  // do stuff like open a template and render values into it.
  KStandardDirs stdDirs;
  // QString templFileName = QString( "kraftdoc_%1_ro.trml" ).arg( doc->docType() );
  QString templFileName = QString( "kraftdoc_ro.trml" );
  QString findFile = "kraft/reports/" + templFileName;

  QString tmplFile = stdDirs.findResource( "data", findFile );


  if ( tmplFile.isEmpty() ) {
    kDebug() << "Could not find template to render ro view of document.";
    return;
  }

  TextTemplate tmpl( tmplFile );
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
  DocPosition *dp;


  DocPositionListIterator it( positions );
  int pos = 1;

  while ( it.hasNext() ) {
    dp = static_cast<DocPosition*>( it.next() );
    tmpl.createDictionary( "POSITIONS" );

    tmpl.setValue( "POSITIONS", "NUMBER", QString::number( pos++ ) );
    tmpl.setValue( "POSITIONS", "TEXT", dp->text() );
    tmpl.setValue( "POSITIONS", "AMOUNT", locale->formatNumber( dp->amount() ) );
    tmpl.setValue( "POSITIONS", "UNIT", dp->unit().einheit( dp->amount() ) );
    double singlePrice = dp->unitPrice().toDouble();

    tmpl.setValue( "POSITIONS", "SINGLE_PRICE", locale->formatMoney( singlePrice ) );
    QString style( "positive" );
    if ( singlePrice < 0 ) {
      style = "negative";
    }
    tmpl.setValue( "POSITIONS", "PRICE_STYLE", style );

    tmpl.setValue( "POSITIONS", "PRICE", locale->formatMoney( dp->overallPrice().toDouble() ) );
  }

  tmpl.setValue( DOC_RO_TAG( "TAXLABEL" ), i18n( "VAT" ) );
  tmpl.setValue( DOC_RO_TAG( "REDUCED_TAXLABEL" ), i18n( "Reduced TAX" ) );
  tmpl.setValue( DOC_RO_TAG( "NETTOSUM" ), locale->formatMoney( doc->nettoSum().toDouble() ) );
  tmpl.setValue( DOC_RO_TAG( "BRUTTOSUM" ), locale->formatMoney( doc->bruttoSum().toDouble() ) );

  double redTax = DocumentMan::self()->reducedTax( doc->date() );
  double fullTax = DocumentMan::self()->tax( doc->date() );
  QString h;
  if ( positions.reducedTaxSum( redTax ).toLong() > 0 ) {
    tmpl.createDictionary( "SECTION_REDUCED_TAX"  );
    tmpl.setValue( "SECTION_REDUCED_TAX", DOC_RO_TAG( "REDUCED_TAX_SUM" ),
                   positions.reducedTaxSum( redTax ).toString( positions.locale() ) );
    h.setNum( redTax, 'f', 1 );
    tmpl.setValue( "SECTION_REDUCED_TAX", DOC_RO_TAG( "REDUCED_TAX" ), h );
    tmpl.setValue( "SECTION_REDUCED_TAX", DOC_RO_TAG( "REDUCED_TAX_LABEL" ), i18n( "reduced VAT" ) );

  }
  if ( positions.fullTaxSum( fullTax ).toLong() > 0 ) {
    tmpl.createDictionary( "SECTION_FULL_TAX" );
    tmpl.setValue( "SECTION_FULL_TAX", DOC_RO_TAG( "FULL_TAX_SUM" ),
                   positions.fullTaxSum( fullTax ).toString( positions.locale() ) );
    h.setNum( fullTax, 'f', 1 );
    tmpl.setValue( "SECTION_FULL_TAX", DOC_RO_TAG( "FULL_TAX" ), h );
    tmpl.setValue( "SECTION_FULL_TAX", DOC_RO_TAG( "FULL_TAX_LABEL" ), i18n( "VAT" ) );
  }

  tmpl.setValue( DOC_RO_TAG( "TAXSUM" ), locale->formatMoney( doc->vatSum().toDouble() ) );
  setCaption( m_doc->docIdentifier() );

  mHtmlView->setTitle( doc->docIdentifier() );
  mHtmlView->displayContent( tmpl.expand() );
}

KraftDoc *KraftViewRO::getDocument() const
{
  return m_doc;
}

void KraftViewRO::done( int r )
{
  kDebug() << "View closed with ret value " << r;
  //KraftDoc *doc = getDocument();
  //if( doc )
  //doc->removeView( this );
  this->close();
}

void KraftViewRO::slotClose()
{
    kDebug() << "Close Slot hit!";

    KraftDoc *doc = getDocument();

    if( !doc ) {
      kDebug() << "ERR: No document available in view, return!";
      return;
    }
    KraftSettings::self()->self()->setRODocViewSize( size() );
    KraftSettings::self()->self()->writeConfig();

    emit viewClosed( true, m_doc );
    KDialog::slotButtonClicked( Ok );
}

#include "kraftview_ro.moc"
