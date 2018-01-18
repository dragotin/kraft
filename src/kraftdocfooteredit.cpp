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
#include "kraftdb.h"

#include <QLocale>
#include <QDebug>
#include <QLayout>
#include <QComboBox>
#include <QVBoxLayout>

KraftDocFooterEdit::KraftDocFooterEdit( QWidget *parent )
  : KraftDocEdit( parent ),
  mDocFooterEdit( 0 ),
  mCustomGreetingIndex(-1)
{
  QVBoxLayout *topLayout = new QVBoxLayout;
  Q_ASSERT( parent );
  setLayout( topLayout );

  mDocFooterEdit = new Ui::DocFooterEdit;
  QWidget *w = new QWidget;
  mDocFooterEdit->setupUi(w);
  topLayout->addWidget(w);

  mDocFooterEdit->m_cbGreeting->insertItems(-1, KraftDB::self()->wordList( "greeting" ) );

  connect( mDocFooterEdit->m_cbGreeting, SIGNAL( activated( int ) ),
           SLOT( slotModified() ) );
  connect( mDocFooterEdit->m_cbGreeting, SIGNAL(currentIndexChanged(int)),
           this, SLOT(slotGreeterIndexChanged(int)));
  connect( mDocFooterEdit->m_cbGreeting, SIGNAL(editTextChanged(QString)),
           this, SLOT(slotGreeterEditTextChanged(QString)));
  connect( mDocFooterEdit->m_teSummary, SIGNAL( textChanged() ),
           SLOT( slotModified() ) );

  setTitle( i18n( "Document Footer" ) );
  setColor( "#f0ff9a" );
}

void KraftDocFooterEdit::slotSetGreeting( const QString& newText )
{
    slotGreeterEditTextChanged(newText);
}

QString KraftDocFooterEdit::greeting()
{
    return mGreeting;
}

void KraftDocFooterEdit::slotGreeterIndexChanged(int newIndex)
{
    mGreeting = mDocFooterEdit->m_cbGreeting->itemText(newIndex);
    slotModified();
}

void KraftDocFooterEdit::slotGreeterEditTextChanged(const QString& newText)
{
    QComboBox *greeterCombo = qobject_cast<QComboBox*>(mDocFooterEdit->m_cbGreeting);
    if( !greeterCombo ) return;
    // qDebug () << "II Combo box text changed to" << newText << "in" << greeterCombo;

    const QStringList texts = KraftDB::self()->wordList("greeting");
    int indx = greeterCombo->currentIndex();
    if( !texts.contains(newText)) {
        if( mCustomGreetingIndex == -1 ) {
            // no custom Entry yet
            greeterCombo->insertItem(0, newText);
            mCustomGreetingIndex = 0;
        }
        indx = mCustomGreetingIndex;
    }
    greeterCombo->setItemText(indx, newText);
    greeterCombo->setCurrentIndex(indx);

    mGreeting = newText;
    slotModified();
}
