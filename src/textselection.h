/***************************************************************************
  textselection  - widget to select header- and footer text data for the doc
                             -------------------
    begin                : 2007-06-01
    copyright            : (C) 2007 by Klaas Freitag
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

#ifndef FOOTERSELECTION_H
#define FOOTERSELECTION_H

#include <qtabwidget.h>
#include <qmap.h>
#include <qvbox.h>

#include <kabc/addressee.h>

#include "kraftdoc.h"

class QComboBox;
class QPopupMenu;
class FilterHeader;
class KListView;
class KListViewItem;
class QListViewItem;
class AddressSelection;
class KPushButton;
class DocText;
class KAction;
class KActionCollection;

class TextSelection : public QVBox
{
  Q_OBJECT
public:
  TextSelection( QWidget*, KraftDoc::Part );

  ~TextSelection();

  QString currentText() const;
  DocText currentDocText() const;

  KListView *textsListView() { return mTextsView; }

signals:
  void textSelectionChanged( QListViewItem* );
  void actionCurrentTextToDoc();

public slots:
  QListViewItem* addNewDocText( const DocText& );
  void deleteCurrentText();
  void updateDocText( const DocText& );
  void slotSelectDocType( const QString& );
  void slotRMB( KListView*, QListViewItem* ,const QPoint& );
protected:
  void initActions();
  void buildTextList( KraftDoc::Part );
  KListViewItem* addOneDocText( QListViewItem*, const DocText& );

private:
  FilterHeader   *mListSearchLine;
  KListView      *mTextsView;
  QMap<QListViewItem*, DocText> mTextMap;
  QMap<QString, QListViewItem*> mDocTypeItemMap;

  QPopupMenu        *mMenu;
  KActionCollection *mActions;
  KAction           *mAcMoveToDoc;
};

#endif
