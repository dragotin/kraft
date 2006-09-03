/***************************************************************************
        addressselection  - widget to select address entries
                             -------------------
    begin                : 2006-09-03
    copyright            : (C) 2006 by Klaas Freitag
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
#include "addressselection.h"

#include <klocale.h>
#include <kdebug.h>
#include <kdialog.h>

#include <qsizepolicy.h>
#include <qcombobox.h>
#include <qwidgetstack.h>
#include <qlabel.h>
#include <qvbox.h>

AddressSelection::AddressSelection( QWidget *parent )
  :QVBox( parent )
{
  setMargin( KDialog::marginHint() );
  setSpacing( KDialog::spacingHint() );

  setupAddressList();
}

void AddressSelection::setupAddressList()
{
}


#include "addressselection.moc"
