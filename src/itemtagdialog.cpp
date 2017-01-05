/***************************************************************************
              postiontagdialog.h  - Edit tags of positions
                             -------------------
    begin                : Aug 2008
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

#include <QWidget>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QLabel>
#include <QItemDelegate>
#include <QStyleOptionViewItem>
#include <QModelIndex>
#include <qdrawutil.h>

#include <QDialog>
#include <QBoxLayout>
#include <QDebug>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QStringList>

#include <klocalizedstring.h>

#include "itemtagdialog.h"
#include "defaultprovider.h"
#include "tagman.h"


class TagDelegate : public QItemDelegate
{
  public:
    TagDelegate( QObject *parent = 0 );
    void paint( QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index ) const;
};


TagDelegate::TagDelegate( QObject *parent  )
  :QItemDelegate( parent )
{
}

void TagDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
  //If we're in the color column
  if(index.column() == 1)
  {
    QColor c( index.model()->data(index, Qt::DisplayRole).toString() );
    QBrush b( c );
    int x = option.rect.left();
    int y = option.rect.top();
    int height = option.rect.height();
    int width = option.rect.width();
    qDrawShadeRect( painter, x+5, y+4, width-10, height-8, c, false, 1, 0, &b );
  }
  else
  {
    QItemDelegate::paint(painter, option, index);
  }
}

// ################################################################################

ItemTagDialog::ItemTagDialog( QWidget *parent )
  : QDialog( parent )
{
  setObjectName( "ITEM_TAG_DIALOG" );
  setModal( true );
  setWindowTitle( i18n("Edit Item Tags" ) );
  QVBoxLayout *mainLayout = new QVBoxLayout;
  setLayout(mainLayout);

  setMinimumWidth ( 375 );

  mainLayout->addWidget( new QLabel( QString::fromLatin1( "<h2>" )
                                     + i18n( "Item Tags" ) + QString::fromLatin1( "</h2>" ), this ) );
  mainLayout->addWidget( new QLabel( i18n( "Select all tags for the item should be tagged with." ), this) );

  mListView = new QTreeWidget( this );
  mListView->setAlternatingRowColors( true );
  mListView->setItemDelegate(new TagDelegate());

  mListView->setContentsMargins ( 3, 3, 3, 3 );
  QPalette palette;
  palette.setColor(QPalette::AlternateBase, QColor( "#dffdd0" ));
  mListView->setPalette(palette);
  mListView->setHeaderHidden(true);
  mListView->setRootIsDecorated( false );

  mListView->setColumnCount( 3 );
  QStringList headers;
  headers << i18n( "Tag" );
  headers << i18n( "Color" );
  headers << i18n( "Description" );
  mListView->setHeaderLabels( headers );
  mListView->setSelectionMode( QAbstractItemView::NoSelection );
  mListView->setColumnWidth(1, 50);
  mainLayout->addWidget(mListView);

  QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
  QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
  okButton->setDefault(true);
  okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
  connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
  connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
  mainLayout->addWidget(buttonBox);

}

ItemTagDialog::~ItemTagDialog()
{

}

void ItemTagDialog::setPositionTags( const QStringList& checkedTags )
{
  QStringList allTags = TagTemplateMan::self()->allTagTemplates();

  foreach( QString string, allTags ) {
      TagTemplate templ = TagTemplateMan::self()->getTagTemplate( string );

      QStringList contents;
      contents << templ.name();
      contents << templ.color().name();
      contents << templ.description();

      QTreeWidgetItem *item = new QTreeWidgetItem( mListView, contents );
      if(checkedTags.contains(templ.name()))
          item->setCheckState( 0, Qt::Checked );
      else
          item->setCheckState( 0, Qt::Unchecked );
  }
}

QStringList ItemTagDialog::getSelectedTags()
{
  QStringList re;

  QTreeWidgetItem *item = mListView->topLevelItem( 0 );
  while ( item ) {
    if( item->checkState( 0 ) == Qt::Checked ) {
      re << item->text( 0 );
    }
    item = mListView->itemBelow( item );
  }
  return re;
}


