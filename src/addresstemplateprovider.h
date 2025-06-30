/***************************************************************************
      addresstemplateprovider - template provider class for address data
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
#ifndef ADDRESSTEMPLATEPROVIDER_H
#define ADDRESSTEMPLATEPROVIDER_H

#include <QWidget>

#include "templateprovider.h"
#include "doctext.h"
#include <kcontacts/addressee.h>

using namespace KContacts;

class AddressTemplateProvider : public TemplateProvider
{
  Q_OBJECT
public:
  AddressTemplateProvider( QWidget* );

Q_SIGNALS:
  void newAddress( Addressee );
  void addressToDocument( const Addressee& );

public Q_SLOTS:
  void slotNewTemplate() override;
  void slotEditTemplate() override;
  void slotDeleteTemplate() override;

  void slotTemplateToDocument() override;
  void slotInsertTemplateToDocument() override;

  void slotSetCurrentAddress( const Addressee& );

private:
  Addressee mCurrentAddress;
  QWidget *mParent;
};


#endif

