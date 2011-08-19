/***************************************************************************
      footertemplateprovider - template provider classes for footer data
                               like addresses or texts
                             -------------------
    begin                : 2007-05-02
    copyright            : (C) 2007 by Klaas Freitag
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

#include <kdebug.h>
#include <klocale.h>

#include "footertemplateprovider.h"
#include "texteditdialog.h"
#include "doctext.h"
#include "defaultprovider.h"

FooterTemplateProvider::FooterTemplateProvider( QWidget *parent )
  :TemplateProvider( parent )
{

}

void FooterTemplateProvider::slotNewTemplate()
{
  kDebug() << "SlotNewTemplate called!" << endl;

  TextEditDialog dia( mParent, KraftDoc::Footer );

  DocText dt;
  dt.setTextType( KraftDoc::Footer );
  dt.setDocType( mDocType );

  dia.setDocText( dt );

  if ( dia.exec() ) {
    kDebug() << "Successfully edited new text" << endl;
    DocText dt = dia.docText();
    /* save to database */
    dbID newId = DefaultProvider::self()->saveDocumentText( dt );
    dt.setDbId( newId );

    emit newFooterText( dt );
  }
}

void FooterTemplateProvider::slotEditTemplate()
{
  kDebug() << "SlotEditTemplate called!" << endl;

  TextEditDialog dia( mParent, KraftDoc::Footer );

  /* mCurrentText is set through the slot slotSetCurrentDocText */
  DocText dt = currentText();
  if ( dt.type() == KraftDoc::Unknown ) {
    dt.setTextType( KraftDoc::Footer );
    dt.setDocType( mDocType );
  }

  dia.setDocText( dt );

  if ( dia.exec() ) {
    kDebug() << "Successfully edited texts" << endl;
    DocText dt = dia.docText();

    /* write back the listview item stored in the input text */
    dt.setListViewItem( currentText().listViewItem() );
    /* save to database */
    DefaultProvider::self()->saveDocumentText( dt );
    // this ends up in the footerselection, slot updateDocText
    emit updateFooterText( dt );
  }

}

void FooterTemplateProvider::slotDeleteTemplate()
{
  DocText dt = currentText();
  emit deleteFooterText( dt );
  DefaultProvider::self()->deleteDocumentText( dt );
}

void FooterTemplateProvider::slotTemplateToDocument()
{
  kDebug() << "Moving template to document" << endl;
  emit footerTextToDocument( currentText() );
}

