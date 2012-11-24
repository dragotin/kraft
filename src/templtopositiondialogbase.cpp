/***************************************************************************
       templtopositiondialogbase.cpp  - base dialog template to doc
                             -------------------
    begin                : Mar 2007
    copyright            : (C) 2007 Klaas Freitag
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

#include <QComboBox>

#include <kdialog.h>
#include <kdebug.h>
#include <klocale.h>

#include "templtopositiondialogbase.h"
#include "docposition.h"

TemplToPositionDialogBase::TemplToPositionDialogBase( QWidget *w )
  : KDialog( w )
{
  setObjectName( "TEMPL_DIALOG" );
  setButtons( KDialog::Ok | KDialog::Cancel );
  setCaption( i18n("Create Position from Template" ) );
  setModal( true );

  showButtonSeparator( true );
}

TemplToPositionDialogBase::~TemplToPositionDialogBase()
{

}

void TemplToPositionDialogBase::setPositionList( DocPositionList list, int intendedPos )
{
  if ( ! getPositionCombo() ) {
    kError() << "Can not get a ptr to the position combo" << endl;
    return;
  }
  QStringList strList;
  strList << i18n( "the Header of the Document as first position" );
  DocPositionListIterator it( list );
  while( it.hasNext() ) {
    DocPosition *dp = static_cast<DocPosition*>( it.next() );
    QString h = QString( "%1. %2" ).arg( list.posNumber( dp ) ).arg( dp->text() );
    if ( h.length() > 50 ) {
      h = h.left( 50 );
      h += i18n( "..." );
    }
    strList.append( h );
  }

  getPositionCombo()->insertItems( -1, strList );
  getPositionCombo()->setCurrentIndex( intendedPos );
}

int TemplToPositionDialogBase::insertAfterPosition()
{
  int itemPos = getPositionCombo()->currentIndex();
  kDebug() << "Current item selected: " << itemPos << endl;

  return itemPos;
}

