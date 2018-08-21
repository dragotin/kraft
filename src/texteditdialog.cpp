/***************************************************************************
              texteditdialog.cpp  - Edit document text templates
                             -------------------
    begin                : Apr 2007
    copyright            : (C) 2007 by Klaas Freitag
    email                : freitag@kde.org
 ***************************************************************************/

/***************************************************************************
 *
 *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QComboBox>
#include <QWidget>
#include <QLabel>
#include <QLocale>
#include <QDialog>
#include <QDebug>
#include <QTextEdit>
#include <QLineEdit>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QVBoxLayout>

#include "templtopositiondialogbase.h"
#include "texteditdialog.h"
#include "doctext.h"
#include "defaultprovider.h"


TextEditDialog::TextEditDialog( QWidget *parent, KraftDoc::Part docPart )
  : QDialog( parent )
{
  setObjectName( "TEMPL_DIALOG" );
  setModal( true );
  setWindowTitle(  i18n("Edit Text Templates" ));
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

  //PORTING: Verify that widget was added to mainLayout: //PORTING: Verify that widget was added to mainLayout:   setMainWidget( mainWidget );
  // Add mainLayout->addWidget(mainWidget); if necessary
  // Add mainLayout->addWidget(mainWidget); if necessary

  mBaseWidget = new Ui::TextEditBase;
  mBaseWidget->setupUi( mainWidget );
  mBaseWidget->mDocTypeLabel->setText( DocText::textTypeToString( docPart ) );

  QString h = i18n( "Edit %1 Template", DocText::textTypeToString( docPart  ) );

  mBaseWidget->dmHeaderText->setText( h );

  mainLayout->addWidget(buttonBox);
}

TextEditDialog::~TextEditDialog()
{

}

void TextEditDialog::setDocText( DocText dt )
{
  QString name = i18n( "Template" );
  if ( ! dt.name().isEmpty() ) {
    name = dt.name();
  }
  mBaseWidget->mEditName->setText( name );

  // mBaseWidget->mEditDescription->setText( dt.description() );
  mBaseWidget->mEditText->setText( dt.text() );

  mBaseWidget->mDocPartLabel->setText( dt.textTypeString() );
  mBaseWidget->mDocTypeLabel->setText( dt.docType() );

  mOriginalText = dt;
}

DocText TextEditDialog::docText()
{
  DocText dt;
  dt = mOriginalText;

  dt.setName( mBaseWidget->mEditName->text() );
  dt.setDescription( QString() ); // mBaseWidget->mEditDescription->text() );
  dt.setText( mBaseWidget->mEditText->toPlainText() );
  // dt.setDocType( mBaseWidget->mCbDocType->currentText() );
  // dt.setTextType( DocText::stringToTextType( mBaseWidget->mCbTextType->currentText() ) );

  return dt;
}

