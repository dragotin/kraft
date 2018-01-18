/***************************************************************************
      addresstemplateprovider - template provider class for addresses
                             -------------------
    begin                : Jun 2007
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

#include <QDebug>
#include <klocale.h>
#include <krun.h>

#include "addresstemplateprovider.h"
#include "texteditdialog.h"
#include "doctext.h"
#include "defaultprovider.h"

AddressTemplateProvider::AddressTemplateProvider( QWidget *parent )
  :TemplateProvider( parent ),
  mParent( parent )
{

}

void AddressTemplateProvider::slotNewTemplate()
{
  // qDebug () << "SlotNewTemplate for addresses called!" << endl;

  KRun::runCommand( QString::fromLatin1( "kaddressbook --new-contact" ),
                    QString::fromLatin1("kaddressbook" ), "address", mParent, "" );
}

void AddressTemplateProvider::slotEditTemplate()
{
  // qDebug () << "SlotEditTemplate called!" << endl;

  KRun::runCommand( QString::fromLatin1( "kaddressbook --uid %1" ).arg(
                    mCurrentAddress.uid() ),
                    QString::fromLatin1("kaddressbook" ), "address", mParent, "" );
  
}

void AddressTemplateProvider::slotDeleteTemplate()
{

}

void AddressTemplateProvider::slotSetCurrentAddress( const Addressee& adr )
{
  // qDebug () << "Current Address was set to " << adr.realName();
  mCurrentAddress = adr;
}

void AddressTemplateProvider::slotTemplateToDocument()
{
  if( mCurrentAddress.isEmpty() ) {
    // qDebug () << "Current address is empty, that should not happen";
    return;
  }

  // qDebug () << "Moving address of " << mCurrentAddress.realName() << " to document" << endl;
  emit addressToDocument( mCurrentAddress );
}


