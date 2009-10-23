/***************************************************************************
   kraftdocfooteredit.cpp  - inherited class from designer generated class
                             -------------------
    begin                : Sept. 2006
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

#include "kraftdocfooteredit.h"

#include <kcombobox.h>
#include <klocale.h>

#include <qlayout.h>
#include <qcombobox.h>
#include <q3textedit.h>
//Added by qt3to4:

#include <QVBoxLayout>

KraftDocFooterEdit::KraftDocFooterEdit( QWidget *parent )
  : KraftDocEdit( parent ),
  mDocFooterEdit( 0 )
{
  QVBoxLayout *topLayout = new QVBoxLayout;
  Q_ASSERT( parent );
  setLayout( topLayout );

  mDocFooterEdit = new Ui::DocFooterEdit;
  QWidget *w = new QWidget;
  mDocFooterEdit->setupUi(w);
  topLayout->addWidget(w);

  connect( mDocFooterEdit->m_cbGreeting, SIGNAL( activated( int ) ),
    SLOT( slotModified() ) );
  connect( mDocFooterEdit->m_teSummary, SIGNAL( textChanged() ),
    SLOT( slotModified() ) );

  setTitle( i18n( "Document Footer" ) );
  setColor( "#f0ff9a" );
}
