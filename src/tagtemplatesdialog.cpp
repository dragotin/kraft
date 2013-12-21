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

#include <kdialog.h>
#include <kdebug.h>
#include <klocale.h>
#include <kcombobox.h>
#include <ktextedit.h>
#include <klineedit.h>
#include <khbox.h>
#include <kvbox.h>
#include <kcolorbutton.h>
#include <kmessagebox.h>

#include "tagtemplatesdialog.h"
#include "defaultprovider.h"
#include "tagman.h"

TagTemplateEditor::TagTemplateEditor( QWidget *parent )
  : KDialog( parent )
{
  setObjectName("TAG_TEMPLATES_EDITOR");
  setModal( true );
  setCaption( i18n("Edit Tag Template" ));
  setButtons( Ok | Cancel );

  showButtonSeparator( true );
  KVBox *w = new KVBox( this );
  setMainWidget( w );

  ( void ) new QLabel( QString::fromLatin1( "<h2>" )
                       + i18n( "<h2>Edit a Tag Template</h2>" ) + QString::fromLatin1( "</h2>" ), w );
  ( void ) new QLabel( i18n( "Adjust settings for name, color and description." ), w );

  KHBox *h1 = new KHBox( w );
  h1->setSpacing( KDialog::spacingHint() );
  ( void ) new QLabel( i18n( "Name:" ), h1 );
  mNameEdit = new KLineEdit( h1 );

  // QHBox *h2 = new QHBox( w );
  ( void ) new QLabel( i18n( "Description:" ), w );
  mDescriptionEdit = new KTextEdit( w );

  KHBox *h2 = new KHBox( w );
  h2->setSpacing( KDialog::spacingHint() );
  QWidget *spaceEater = new QWidget( h2 );
  spaceEater->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
  ( void ) new QLabel( i18n( "Associated Color:" ), h2 );
  mColorButton = new KColorButton( h2 );

}

TagTemplateEditor::~TagTemplateEditor()
{

}

void TagTemplateEditor::setTemplate( const TagTemplate& tt )
{
  mOrigTemplate = tt;

  mNameEdit->setText( tt.name() );
  mDescriptionEdit->setText( tt.description() );
  mColorButton->setColor( tt.color() );
}

TagTemplate TagTemplateEditor::currentTemplate()
{
  TagTemplate tt = mOrigTemplate;
  tt.setName( mNameEdit->text() );
  tt.setDescription( mDescriptionEdit->toPlainText() );
  tt.setColor( mColorButton->color() );

  return tt;
}

// ################################################################################

TagTemplatesDialog::TagTemplatesDialog( QWidget *parent )
  : KDialog( parent )
{
  setObjectName( "TAG_TEMPLATES_DIALOG" );
  setModal( true );
  setCaption( i18n("Edit Tag Templates" ) );
  setButtons( Close );

  showButtonSeparator( true );
  KVBox *w= new KVBox( this );
  setMainWidget( w );

  ( void ) new QLabel( QString::fromLatin1( "<h2>" )
                       + i18n( "Edit Tag Templates" ) + QString::fromLatin1( "</h2>" ), w );
  ( void ) new QLabel( i18n( "Add, edit and remove tag templates for use in the documents." ), w );

  mListView = new QTreeWidget( w );
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

  setTags();

  KHBox *buttBox = new KHBox( w );
  buttBox->setSpacing( KDialog::spacingHint() );
  mAddButton = new QPushButton( i18n( "Add..." ), buttBox );
  mEditButton = new QPushButton( i18n( "Edit..." ), buttBox );
  mEditButton->setEnabled( false );
  mDeleteButton = new QPushButton( i18n( "Delete..." ), buttBox );
  mDeleteButton->setEnabled( false );

  connect( mAddButton, SIGNAL( clicked() ), SLOT( slotAddTemplate() ) );
  connect( mEditButton, SIGNAL( clicked() ), SLOT( slotEditTemplate() ) );
  connect( mDeleteButton, SIGNAL( clicked() ), SLOT( slotDeleteTemplate() ) );

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
  if( KMessageBox::questionYesNo( this,
        i18n( "Do you really want to delete the template?"),
        i18n("Delete Tag Template"), KStandardGuiItem::yes(), KStandardGuiItem::no(),
        "deletetemplate" ) == KMessageBox::Yes )
    {
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

  for ( QStringList::ConstIterator it = tags.begin(); it != tags.end(); ++it ) {
    TagTemplate templ = TagTemplateMan::self()->getTagTemplate( *it );

    // TagItem *item = new QListViewItem( mListView, templ.name(), QCheckListItem::CheckBox );
    QTreeWidgetItem *item = new QTreeWidgetItem( mListView );
    item->setText( 1, templ.name() );
    QPixmap pix( 16, 12 );
    pix.fill( templ.color() );
    item->setIcon( 0, pix );

    // item->setColorGroup( templ.colorGroup() );
    item->setText( 2, templ.description() );

    mItemMap[item] = *it;
  }
}



#include "tagtemplatesdialog.moc"
