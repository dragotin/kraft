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

#include <kdialog.h>
#include <kdebug.h>
#include <klocale.h>
#include <kcombobox.h>
#include <ktextedit.h>
#include <klineedit.h>
#include <kvbox.h>

#include "positiontagdialog.h"
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
    QColor test(index.data(1).toString());
    qDrawShadeRect( painter, x+5, y+4, width-10, height-8, c, false, 1, 0, &b );
  }
  else
  {
    QItemDelegate::paint(painter, option, index);
  }
}

// ################################################################################

PositionTagDialog::PositionTagDialog( QWidget *parent )
  : KDialog( parent )

{
  setObjectName( "POSITION_TAG_DIALOG" );
  setModal( true );
  setCaption( i18n("Edit Item Tags" ) );
  setButtons( KDialog::Ok | KDialog::Cancel );
  setMinimumWidth ( 375 );

  KVBox *w = new KVBox( this );
  setMainWidget( w );


  ( void ) new QLabel( QString::fromLatin1( "<h2>" )
                       + i18n( "<h2>Item Tags</h2>" ) + QString::fromLatin1( "</h2>" ), w );
  ( void ) new QLabel( i18n( "Select all tags for the item should be tagged with." ), w );

  mListView = new QTreeWidget( w );
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
}

PositionTagDialog::~PositionTagDialog()
{

}

void PositionTagDialog::setPositionTags( const QStringList& checkedTags )
{
  QStringList allTags = TagTemplateMan::self()->allTagTemplates();

  for ( QStringList::ConstIterator it = allTags.begin(); it != allTags.end(); ++it ) {
      TagTemplate templ = TagTemplateMan::self()->getTagTemplate( *it );

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

QStringList PositionTagDialog::getSelectedTags()
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


#include "positiontagdialog.moc"
