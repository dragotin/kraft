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
#include "kraftview_ro.h"
#include "kraftdoc.h"
#include "portal.h"
#include "docheader.h"
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

#include <qtimer.h>
#include "doclocaledialog.h"
#include <kstandarddirs.h>
#include "texttemplate.h"
#include "documentman.h"

// #########################################################

KraftViewRO::KraftViewRO(QWidget *parent, const char *name) :
  KDialogBase( parent, name, false /* modal */, i18n("Document"),
	      Close, Ok, true /* separator */ ),
  m_doc( 0 )
{
  mGlobalVBox = makeVBoxMainWidget();
  mGlobalVBox->setMargin( 3 );

  mHtmlView = new HtmlView( mGlobalVBox );
  mHtmlView->setStylesheetFile( "docoverview.css" );
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
  if ( !locale ) locale = KGlobal().locale();

  // do stuff like open a template and render values into it.
  KStandardDirs stdDirs;
  // QString templFileName = QString( "kraftdoc_%1_ro.trml" ).arg( doc->docType() );
  QString templFileName = QString( "kraftdoc_ro.trml" );
  QString findFile = "kraft/reports/" + templFileName;

  QString tmplFile = stdDirs.findResource( "data", findFile );


  if ( ! tmplFile ) {
    kdDebug() << "Could not find template to render ro view of document." << endl;
    return;
  }

  TextTemplate tmpl( tmplFile );

  tmpl.setValue( DOC_RO_TAG( "DATE" ), locale->formatDate( doc->date(), true ) );
  tmpl.setValue( DOC_RO_TAG( "DOC_TYPE" ),  doc->docType() );
  tmpl.setValue( DOC_RO_TAG( "ADDRESS" ), doc->address() );
  tmpl.setValue( DOC_RO_TAG( "DOCNO" ), doc->ident() );
  tmpl.setValue( DOC_RO_TAG( "PRETEXT" ), doc->preText() );
  tmpl.setValue( DOC_RO_TAG( "POSTTEXT" ), doc->postText() );
  tmpl.setValue( DOC_RO_TAG( "SALUT" ), doc->salut() );
  tmpl.setValue( DOC_RO_TAG( "GOODBYE" ), doc->goodbye() );


  DocPositionList positions = doc->positions();
  DocPosition *dp;
  DocPositionBase *dpb;

  DocPositionListIterator it( positions );
  int pos = 1;

  while ( ( dpb = it.current() ) != 0 ) {
    ++it;
    dp = static_cast<DocPosition*>( dpb );
    tmpl.createDictionary( "POSITIONS" );

    tmpl.setValue( "POSITIONS", "NUMBER", QString::number( pos++ ) );
    tmpl.setValue( "POSITIONS", "TEXT", dp->text() );
    tmpl.setValue( "POSITIONS", "AMOUNT", locale->formatNumber( dp->amount() ) );
    tmpl.setValue( "POSITIONS", "UNIT", dp->unit().einheit( dp->amount() ) );
    tmpl.setValue( "POSITIONS", "SINGLE_PRICE", locale->formatMoney( dp->unitPrice().toDouble() ) );
    tmpl.setValue( "POSITIONS", "PRICE", locale->formatMoney( dp->overallPrice().toDouble() ) );
  }

  tmpl.setValue( DOC_RO_TAG( "NETTOSUM" ), locale->formatMoney( doc->nettoSum().toDouble() ) );
  tmpl.setValue( DOC_RO_TAG( "BRUTTOSUM" ), locale->formatMoney( doc->nettoSum().toDouble() ) );
  tmpl.setValue( DOC_RO_TAG( "VATSUM" ), locale->formatMoney( doc->vatSum().toDouble() ) );
  tmpl.setValue( DOC_RO_TAG( "VAT" ), locale->formatNumber( DocumentMan::self()->tax( doc->date() ) ) );
  tmpl.setValue( DOC_RO_TAG( "TAX" ), locale->formatNumber( DocumentMan::self()->tax( doc->date() ) ) );
  tmpl.setValue( DOC_RO_TAG( "REDUCED_TAX" ),
                 locale->formatNumber( DocumentMan::self()->reducedTax( doc->date() ) ) );
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
  kdDebug() << "View closed with ret value " << r << endl;
  KraftDoc *doc = getDocument();
  if( doc )
    // doc->removeView( this );
  KDialogBase::done( r );
}

void KraftViewRO::slotClose()
{
    kdDebug() << "Close Slot hit!" << endl;

    KraftDoc *doc = getDocument();

    if( !doc ) {
      kdDebug() << "ERR: No document available in view, return!" << endl;
      return;
    }
    KraftSettings::self()->setRODocViewSize( size() );
    KraftSettings::self()->writeConfig();

    emit viewClosed( true, m_doc );
    KDialogBase::slotClose(  );
}

#include "kraftview_ro.moc"
