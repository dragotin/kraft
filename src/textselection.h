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
#include <q3vbox.h>
//Added by qt3to4:
#include <Q3PopupMenu>

#include <QTreeWidget>

#include <kabc/addressee.h>

#include "kraftdoc.h"

class QComboBox;
class Q3PopupMenu;
class FilterHeader;
class AddressSelection;
class KPushButton;
class DocText;
class KAction;
class KActionCollection;

class TextSelection : public Q3VBox
{
  Q_OBJECT
public:
  TextSelection( QWidget*, KraftDoc::Part );

  ~TextSelection();

  QString currentText() const;
  DocText currentDocText() const;

  QTreeWidget *textsListView() { return mTextsView; }

signals:
  void textSelectionChanged( QTreeWidgetItem* );
  void actionCurrentTextToDoc();

public slots:
  QTreeWidgetItem* addNewDocText( const DocText& );
  void deleteCurrentText();
  void updateDocText( const DocText& );
  void slotSelectDocType( const QString& );
  void slotRMB( QTreeWidget*, QTreeWidgetItem* ,const QPoint& );

protected:
  void initActions();
  void buildTextList( KraftDoc::Part );
  QTreeWidgetItem* addOneDocText( QTreeWidgetItem*, const DocText& );

protected slots:
  void slotSelectionChanged( QTreeWidgetItem* );

private:
  FilterHeader   *mListSearchLine;
  QTreeWidget      *mTextsView;
  QMap<QTreeWidgetItem*, DocText> mTextMap;
  QMap<QString, QTreeWidgetItem*> mDocTypeItemMap;
  QMap<QString, QTreeWidgetItem*> mStandardItemMap;

  Q3PopupMenu        *mMenu;
  KActionCollection *mActions;
  KAction           *mAcMoveToDoc;
};

#endif
