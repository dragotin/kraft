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

#include <qcombobox.h>
#include <qwidget.h>
#include <qvbox.h>
#include <qlabel.h>
#include <qlistview.h>
#include <qdrawutil.h>
#include <qheader.h>
#include "tagman.h"
#include <qpushbutton.h>

#include <kdialogbase.h>
#include <kdebug.h>
#include <klocale.h>
#include <kcombobox.h>
#include <ktextedit.h>
#include <klineedit.h>

#include "tagtemplatesdialog.h"
#include "defaultprovider.h"
#include <kcolorbutton.h>
#include <kmessagebox.h>


TagTemplateEditor::TagTemplateEditor( QWidget *parent )
  : KDialogBase( parent, "TAG_TEMPLATES_EDITOR", true, i18n( "Edit Tag Template" ),
                 Ok | Cancel )
{
  enableButtonSeparator( true );
  QWidget *w = makeVBoxMainWidget();

  ( void ) new QLabel( QString::fromLatin1( "<h2>" )
                       + i18n( "<h2>Edit a Tag Template</h2>" ) + QString::fromLatin1( "</h2>" ), w );
  ( void ) new QLabel( i18n( "Adjust settings for name, color and description." ), w );

  QHBox *h1 = new QHBox( w );
  h1->setSpacing( KDialog::spacingHint() );
  ( void ) new QLabel( i18n( "Name:" ), h1 );
  mNameEdit = new KLineEdit( h1 );

  // QHBox *h2 = new QHBox( w );
  ( void ) new QLabel( i18n( "Description:" ), w );
  mDescriptionEdit = new KTextEdit( w );

  QHBox *h2 = new QHBox( w );
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
  tt.setDescription( mDescriptionEdit->text() );
  tt.setColor( mColorButton->color() );

  return tt;
}

// ################################################################################

TagTemplatesDialog::TagTemplatesDialog( QWidget *parent )
  : KDialogBase( parent, "TAG_TEMPLATES_DIALOG", true, i18n( "Edit Tag Templates" ),
                 Close )
{
  enableButtonSeparator( true );
  QWidget *w = makeVBoxMainWidget();
  ( void ) new QLabel( QString::fromLatin1( "<h2>" )
                       + i18n( "Edit Tag Templates" ) + QString::fromLatin1( "</h2>" ), w );
  ( void ) new QLabel( i18n( "Add, edit and remove tag templates for use in the documents." ), w );

  mListView = new KListView( w );
  mListView->setItemMargin( 3 );
  mListView->setAlternateBackground( QColor( "#dffdd0" ) );
  mListView->header()->hide();
  mListView->setRootIsDecorated( false );
  mListView->setSelectionMode( QListView::Single );
  mListView->addColumn( i18n( "Tag" ) );
  mListView->addColumn( i18n( "Color" ) );
  mListView->addColumn( i18n( "Description" ) );
  mListView->setAllColumnsShowFocus( true );
  mListView->setSelectionMode( QListView::Single );

  connect( mListView, SIGNAL( selectionChanged() ),
           this, SLOT( slotSelectionChanged() ) );

  setTags();

  QHBox *buttBox = new QHBox( w );
  buttBox->setSpacing( KDialog::spacingHint() );
  mAddButton = new QPushButton( i18n( "Add..." ), buttBox );
  mEditButton = new QPushButton( i18n( "Edit.." ), buttBox );
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
        i18n("Delete Tag Template"), KStdGuiItem::yes(), KStdGuiItem::no(),
        "deletetemplate" ) == KMessageBox::Yes )
    {
      TagTemplateMan::self()->deleteTemplate( currentTemplate().dbId() );
      setTags();
    }
}

void TagTemplatesDialog::slotSelectionChanged()
{
  bool state = false;
  if ( mListView->selectedItem() ) {
    state = true;
  }
  mEditButton->setEnabled( state );
  mDeleteButton->setEnabled( state );

}

TagTemplate TagTemplatesDialog::currentTemplate()
{
  QListViewItem *item = mListView->currentItem();

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
    KListViewItem *item = new KListViewItem( mListView, templ.name() );
    QPixmap pix( 16, 12 );
    pix.fill( templ.color() );
    item->setPixmap( 0, pix );

    // item->setColorGroup( templ.colorGroup() );
    item->setText( 2, templ.description() );

    mItemMap[item] = *it;
  }
}



#include "tagtemplatesdialog.moc"
