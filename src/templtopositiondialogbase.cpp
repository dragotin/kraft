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

#include <QDialog>
#include <QDebug>
#include <klocale.h>
#include <KConfigGroup>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QVBoxLayout>

#include "templtopositiondialogbase.h"
#include "docposition.h"

TemplToPositionDialogBase::TemplToPositionDialogBase( QWidget *w )
  : QDialog( w )
{
  setObjectName( "TEMPL_DIALOG" );
  QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
  QWidget *mainWidget = new QWidget(this);
  QVBoxLayout *mainLayout = new QVBoxLayout;
  setLayout(mainLayout);
  mainLayout->addWidget(mainWidget);
  QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
  okButton->setDefault(true);
  okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
  connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
  connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
  //PORTING SCRIPT: WARNING mainLayout->addWidget(buttonBox) must be last item in layout. Please move it.
  mainLayout->addWidget(buttonBox);
  setWindowTitle( i18n("Create Position from Template" ) );
  setModal( true );

}

TemplToPositionDialogBase::~TemplToPositionDialogBase()
{

}

void TemplToPositionDialogBase::setPositionList( DocPositionList list, int intendedPos )
{
  if ( ! getPositionCombo() ) {
    qCritical() << "Can not get a ptr to the position combo" << endl;
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
  // qDebug () << "Current item selected: " << itemPos << endl;

  return itemPos;
}

