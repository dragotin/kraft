/***************************************************************************
   kraftdocheaderview.cpp  - inherited class from designer generated class
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

#include "kraftdocheaderedit.h"

#include <kdatewidget.h>
#include <kcombobox.h>
#include <klocale.h>
#include <kdebug.h>

#include <QLayout>
#include <QComboBox>

KraftDocHeaderEdit::KraftDocHeaderEdit( QWidget *parent )
  : KraftDocEdit( parent )
{
  QVBoxLayout *topLayout = new QVBoxLayout;
  setLayout( topLayout );
  mDocHeaderEdit = new Ui::DocHeaderEdit;
  QWidget *w = new QWidget;
  mDocHeaderEdit->setupUi( w );
  topLayout->addWidget( w );

  mDocHeaderEdit->mButtLang->setIcon(KIcon("preferences-desktop-locale"));

  connect( mDocHeaderEdit->m_cbType, SIGNAL( activated( int ) ),
    SLOT( slotModified() ) );
  connect( mDocHeaderEdit->m_cbType, SIGNAL( textChanged( const QString & ) ),
    SLOT( slotModified() ) );
  connect( mDocHeaderEdit->m_dateEdit, SIGNAL( changed( QDate ) ),
    SLOT( slotModified() ) );
  connect( mDocHeaderEdit->m_postAddressEdit, SIGNAL( textChanged() ),
    SLOT( slotModified() ) );
  connect( mDocHeaderEdit->m_letterHead, SIGNAL( activated( int ) ),
    SLOT( slotModified() ) );
  connect( mDocHeaderEdit->m_letterHead, SIGNAL( textChanged( const QString & ) ),
    SLOT( slotModified() ) );
  connect( mDocHeaderEdit->m_teEntry, SIGNAL( textChanged() ),
    SLOT( slotModified() ) );
  connect( mDocHeaderEdit->m_whiteboardEdit, SIGNAL( textChanged() ),
    SLOT( slotModified() ) );

  connect( mDocHeaderEdit->pb_pickAddressee, SIGNAL(clicked()), SIGNAL(pickAddressee()));

  setTitle( i18n( "Document Header" ) );
  setColor( "#9af0ff" );
}
