/***************************************************************************
      headertemplateprovider - template provider classes for header data
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

#include "headertemplateprovider.h"
#include "texteditdialog.h"
#include "doctext.h"
#include "defaultprovider.h"

HeaderTemplateProvider::HeaderTemplateProvider( QWidget *parent )
  :TemplateProvider( parent )
{

}

void HeaderTemplateProvider::slotNewTemplate()
{
  kdDebug() << "SlotNewTemplate called!" << endl;

  TextEditDialog dia( mParent, KraftDoc::Header );

  DocText dt;
  dt.setTextType( KraftDoc::Header );
  dt.setDocType( mDocType );

  dia.setDocText( dt );

  if ( dia.exec() ) {
    kdDebug() << "Successfully edited texts" << endl;
    DocText dt = dia.docText();
    /* save to database */
    dbID newId = DefaultProvider::self()->saveDocumentText( dt );
    dt.setDbId( newId );

    mCurrentText = dt;
    emit newHeaderText( dt );
  }
}

void HeaderTemplateProvider::slotEditTemplate()
{
  kdDebug() << "SlotEditTemplate called!" << endl;

  TextEditDialog dia( mParent, KraftDoc::Header );

  /* mCurrentText is set through the slot slotSetCurrentDocText */
  DocText dt = mCurrentText;
  if ( dt.type() == KraftDoc::Unknown ) {
    dt.setTextType( KraftDoc::Header );
    dt.setDocType( mDocType );
  }

  dia.setDocText( dt );

  if ( dia.exec() ) {
    kdDebug() << "Successfully edited texts" << endl;
    DocText dt = dia.docText();

    /* write back the listview item stored in the input text */
    dt.setListViewItem( mCurrentText.listViewItem() );
    /* save to database */
    DefaultProvider::self()->saveDocumentText( dt );
    slotSetCurrentDocText( dt );

    emit updateHeaderText( dt );
  }

}

void HeaderTemplateProvider::slotSetCurrentDocText( const DocText& dt )
{
  mCurrentText = dt;
}

void HeaderTemplateProvider::slotDeleteTemplate()
{
  emit deleteHeaderText( mCurrentText );
  DefaultProvider::self()->deleteDocumentText( mCurrentText );
}

void HeaderTemplateProvider::slotTemplateToDocument()
{
  kdDebug() << "Moving template to document" << endl;

  if ( mCurrentTab == HeaderSelection::TextTab ) {
    emit headerTextToDocument( mCurrentText );
  } else if ( mCurrentTab == HeaderSelection::AddressTab ) {
    kdDebug() << "Addresstab is selected!" << endl;
  } else {
    kdDebug() << "Currently not a valid Tab selected" << endl;
  }
}

void HeaderTemplateProvider::slotSetCurrentTab( HeaderSelection::HeaderTabType t )
{
  mCurrentTab = t;
}

#include "headertemplateprovider.moc"

