/***************************************************************************
        katalogselection  - widget to select catalog entries from
                             -------------------
    begin                : 2006-08-30
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

#ifndef CATALOGSELECTION_H
#define CATALOGSELECTION_H

#include <qvbox.h>
#include <qasciidict.h>

class QComboBox;
class QWidgetStack;
class KActionCollection;
class KAction;
class QListViewItem;

class DocPosition;
class FilterHeader;
class KatalogListView;
class CalcPartList;
class Katalog;

class CatalogSelection : public QVBox
{
  Q_OBJECT
public:
  CatalogSelection( QWidget* );

  ~CatalogSelection() { };

  Katalog* currentSelectedKat();

protected:
  void setupCatalogList();
  void initActions();

signals:
  /*
   * a template was selected to be inserted into the document. This
   * transports a ptr to the katalog and the item in it. Since the
   * template type is dependent on the katalog type it is not known
   * what type of template is coming. It is up to the receiver to
   * decide (and cast) to the correct template on the katalog type.
   *
   * FIXME: Better approach: all catalog items inherit from a base
   *        type.
   */
  void positionSelected( Katalog*, void* );
  void selectionChanged( QListViewItem* );

protected slots:
  void slotSelectCatalog( const QString& );
  void slotAppendToDoc( QListViewItem *item = 0 );
  void slCatalogDoubleClicked( QListViewItem*,  const QPoint&,  int );

private:
  QComboBox *mCatalogSelector;
  QWidgetStack *mWidgets;
  QAsciiDict<KatalogListView> mWidgetDict;
  KActionCollection *mActions;
  KAction *mAcAddToDoc;
  FilterHeader *mListSearchLine;
};

#endif
