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
#include <qtextedit.h>

KraftDocFooterEdit::KraftDocFooterEdit( QWidget *parent )
  : KraftDocEdit( parent )
{
  QBoxLayout *topLayout = new QVBoxLayout( this );

  mDocFooterEdit = new DocFooterEdit( this );
  topLayout->addWidget( mDocFooterEdit );  

  connect( mDocFooterEdit->m_cbGreeting, SIGNAL( activated( int ) ),
    SLOT( slotModified() ) );
  connect( mDocFooterEdit->m_teSummary, SIGNAL( textChanged() ),
    SLOT( slotModified() ) );

  setTitle( i18n( "Document Footer" ) );
  setColor( "#f0ff9a" );
}
