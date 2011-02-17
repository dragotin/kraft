/***************************************************************************
             docdigestdetailview.cpp  - Details of a doc digest
                             -------------------
    begin                : 2011
    copyright            : (C) 2011 by Klaas Freitag
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

#include <QtGui>

#include <khtmlview.h>
#include <kstandarddirs.h>

#include "docdigest.h"
#include "docdigestdetailview.h"

#include "htmlview.h"
#include "texttemplate.h"


DocDigestDetailView::DocDigestDetailView(QWidget *parent) :
    QWidget(parent)
{
  QHBoxLayout *hbox = new QHBoxLayout;
  hbox->setMargin(0);
  setLayout( hbox );
  mHtmlCanvas = new HtmlView( this );
  mHtmlCanvas->setStylesheetFile( "docdigestview.css");
  hbox->addWidget( mHtmlCanvas->view() );
}

#define DOCDIGEST_TAG

void DocDigestDetailView::slotShowDocDetails( DocDigest digest )
{
  kDebug() << "Showing details about this doc: " << digest.id();

  if( mTemplFile.isEmpty() ) {
    KStandardDirs stdDirs;
    // QString templFileName = QString( "kraftdoc_%1_ro.trml" ).arg( doc->docType() );
    QString templFileName = QString( "docdigest.trml" );
    QString findFile = "kraft/reports/" + templFileName;

    QString tmplFile = stdDirs.findResource( "data", findFile );

    if ( tmplFile.isEmpty() ) {
      kDebug() << "Could not find template to render document digest.";
      return;
    }
    mTemplFile = tmplFile;
  }

  TextTemplate tmpl( mTemplFile ); // template file with name docdigest.trml
  tmpl.setValue( DOCDIGEST_TAG( "HEADLINE" ), digest.type() + " " + digest.ident() );
  tmpl.setValue( DOCDIGEST_TAG( "DATE" ), digest.date() );

  // Information about archived documents.
  ArchDocDigestList archDocs = digest.archDocDigestList();
  if( archDocs.isEmpty() ) {
    kDebug() << "No archived docs for this document!";
    tmpl.setValue( DOCDIGEST_TAG("ARCHDOCS_TAG"), i18n("This document was never printed."));
  } else {
    ArchDocDigest digest = archDocs[0];
    kDebug() << "Last printed at " << digest.printDate().toString() << " and " << archDocs.count() -1 << " other prints.";
    tmpl.setValue( DOCDIGEST_TAG("ARCHDOCS_TAG"), i18n("Last Printed %1, %2 older prints.").arg(digest.printDate().toString()).arg(archDocs.count()-1));
  }

  mHtmlCanvas->displayContent( tmpl.expand() );


}
