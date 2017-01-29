/***************************************************************************
                 tagtemplatesdialog.h  - Edit tag templates
                             -------------------
    begin                : Sep 2008
    copyright            : (C) 2008 by Klaas Freitag
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
#include <QPixmap>
#include <QTreeWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QTextEdit>
#include <QMessageBox>
#include <QPainter>

#include <QDialog>
#include <QDebug>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QColorDialog>

#include <klocalizedstring.h>

#include "tagtemplatesdialog.h"
#include "defaultprovider.h"
#include "tagman.h"

TagTemplateEditor::TagTemplateEditor( QWidget *parent )
  : QDialog( parent )
{
  setObjectName("TAG_TEMPLATES_EDITOR");
  setModal( true );
  setWindowTitle( i18n("Edit Tag Template" ));
  QVBoxLayout *mainLayout = new QVBoxLayout;
  setLayout(mainLayout);

  QVBoxLayout *w = new QVBoxLayout( this );

  w->addWidget(new QLabel( QString::fromLatin1( "<h2>" )
                           + i18n( "Edit a Tag Template" ) + QString::fromLatin1( "</h2>" )));
  w->addWidget( new QLabel( i18n( "Adjust settings for name, color and description." )));

  QHBoxLayout *h1 = new QHBoxLayout;
  h1->addWidget( new QLabel( i18n( "Name:" ) ));
  mNameEdit = new QLineEdit;
  h1->addWidget(mNameEdit);
  w->addLayout(h1);
  // QHBox *h2 = new QHBox( w );
  w->addWidget(new QLabel( i18n( "Description:" )));
  mDescriptionEdit = new QTextEdit;
  w->addWidget(mDescriptionEdit);

  QHBoxLayout *h2 = new QHBoxLayout;
  h2->addStretch(1);
  h2->addWidget(new QLabel( i18n( "Associated Color:" )));
  mColorButton = new QPushButton;
  h2->addWidget(mColorButton);
  connect( mColorButton, SIGNAL(clicked(bool)), SLOT(slotColorSelect(bool)));

  w->addLayout(h2);
  mainLayout->addLayout(w);

  QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
  mOkButton = buttonBox->button(QDialogButtonBox::Ok);
  mOkButton->setDefault(true);
  mOkButton->setShortcut(Qt::CTRL | Qt::Key_Return);
  connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
  connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

  mainLayout->addWidget(buttonBox);
}

TagTemplateEditor::~TagTemplateEditor()
{

}

void TagTemplateEditor::slotColorSelect(bool)
{
    mColor = QColorDialog::getColor(mOrigTemplate.color(), this);
    setColorButton();
    mOkButton->setFocus();
}

void TagTemplateEditor::setColorButton()
{
    QPixmap pix(32, 32);

    QPainter painter(&pix);
    painter.setBrush(QBrush(mColor));
    painter.drawRect( QRect(0 ,0 , 32, 32));
    mColorButton->setIcon(QIcon(pix));
}

void TagTemplateEditor::setTemplate( const TagTemplate& tt )
{
  mOrigTemplate = tt;

  mNameEdit->setText( tt.name() );
  mDescriptionEdit->setText( tt.description() );
  mColor = tt.color();
  setColorButton();
}

TagTemplate TagTemplateEditor::currentTemplate()
{
  TagTemplate tt = mOrigTemplate;
  tt.setName( mNameEdit->text() );
  tt.setDescription( mDescriptionEdit->toPlainText() );
  tt.setColor(mColor);
  return tt;
}

// ################################################################################

TagTemplatesDialog::TagTemplatesDialog( QWidget *parent )
  : QDialog( parent )
{
  setObjectName( "TAG_TEMPLATES_DIALOG" );
  setModal( true );
  setWindowTitle( i18n("Edit Tag Templates" ) );

  QVBoxLayout *mainLayout = new QVBoxLayout;

  mainLayout->addWidget(new QLabel( QString::fromLatin1( "<h2>" )
                       + i18n( "Edit Tag Templates" ) + QString::fromLatin1( "</h2>" )));
  mainLayout->addWidget(new QLabel( i18n( "Add, edit and remove tag templates for use in the documents." )));

  mListView = new QTreeWidget;
  // mListView->setItemMargin( 3 );
  // mListView->setAlternateBackground( QColor( "#dffdd0" ) );
  // mListView->headerItem()->hide();
  mListView->setRootIsDecorated( false );
  mListView->setSelectionMode( QAbstractItemView::SingleSelection );
  QStringList headers;
  headers << i18n( "Tag" );
  headers << i18n( "Color" );
  headers << i18n( "Description" );
  mListView->setHeaderLabels( headers );

  mListView->setAllColumnsShowFocus( true );
  mListView->setSelectionMode( QAbstractItemView::SingleSelection );

  connect( mListView, SIGNAL( itemSelectionChanged() ),
           this, SLOT( slotSelectionChanged() ) );

  mainLayout->addWidget(mListView);
  setTags();

  QHBoxLayout *buttBox = new QHBoxLayout;
  mAddButton = new QPushButton( i18n( "Add..." ));
  buttBox->addWidget(mAddButton);
  mEditButton = new QPushButton( i18n( "Edit..." ));
  buttBox->addWidget(mEditButton);
  mEditButton->setEnabled( false );
  mDeleteButton = new QPushButton( i18n( "Delete..." ));
  buttBox->addWidget(mDeleteButton);
  mDeleteButton->setEnabled( false );
  mainLayout->addLayout(buttBox);

  connect( mAddButton, SIGNAL( clicked() ), SLOT( slotAddTemplate() ) );
  connect( mEditButton, SIGNAL( clicked() ), SLOT( slotEditTemplate() ) );
  connect( mDeleteButton, SIGNAL( clicked() ), SLOT( slotDeleteTemplate() ) );

  QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
  QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
  okButton->setDefault(true);
  okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
  connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
  connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
  mainLayout->addWidget(buttonBox);

  setLayout(mainLayout);
  slotSelectionChanged();
}

TagTemplatesDialog::~TagTemplatesDialog()
{

}

void TagTemplatesDialog::slotAddTemplate()
{
  TagTemplateEditor dia( this );
  if ( dia.exec() ) {
    TagTemplateMan::self()->writeTemplate( dia.currentTemplate() );
    setTags();
  }
}

void TagTemplatesDialog::slotEditTemplate()
{
  TagTemplateEditor dia( this );
  TagTemplate curr = currentTemplate();

  dia.setTemplate( curr );
  if ( dia.exec() ) {
    TagTemplate tt = dia.currentTemplate();
    if ( tt != curr ) {
      TagTemplateMan::self()->writeTemplate( tt );
      setTags();
    }
  }
}

void TagTemplatesDialog::slotDeleteTemplate()
{
    QMessageBox msgBox;
    msgBox.setText(i18n( "Do you really want to delete the template?"));

    msgBox.setStandardButtons(QMessageBox::Yes| QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::Yes);
    int ret = msgBox.exec();

    if ( ret == QMessageBox::Yes) {
      TagTemplateMan::self()->deleteTemplate( currentTemplate().dbId() );
      setTags();
    }
}

void TagTemplatesDialog::slotSelectionChanged()
{
  bool state = false;
  if ( mListView->selectedItems().size() ) {
    state = true;
  }
  mEditButton->setEnabled( state );
  mDeleteButton->setEnabled( state );
}

TagTemplate TagTemplatesDialog::currentTemplate()
{
  QTreeWidgetItem *item = mListView->currentItem();

  if ( item ) {
    QString templName = mItemMap[item];
    return TagTemplateMan::self()->getTagTemplate( templName );
  }
  return TagTemplate();
}

void TagTemplatesDialog::setTags()
{
  mListView->clear();
  QStringList tags = TagTemplateMan::self()->allTagTemplates();

  foreach( const QString t, tags ) {
    TagTemplate templ = TagTemplateMan::self()->getTagTemplate( t );

    // TagItem *item = new QListViewItem( mListView, templ.name(), QCheckListItem::CheckBox );
    QTreeWidgetItem *item = new QTreeWidgetItem( mListView );
    item->setText( 1, templ.name() );
    QPixmap pix( 16, 12 );
    pix.fill( templ.color() );
    item->setIcon( 0, pix );

    // item->setColorGroup( templ.colorGroup() );
    item->setText( 2, templ.description() );

    mItemMap[item] = t;
  }
}


