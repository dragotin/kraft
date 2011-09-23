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

#include <QMap>
#include <QWidget>

#include <kabc/addressee.h>

#include "kraftdoc.h"

class QTreeWidget;
class QTreeWidgetItem;
class QString;
class QPoint;
class QMenu;
class FilterHeader;
class KPushButton;
class DocText;
class KAction;
class KActionCollection;
class QListView;
class QStringListModel;
class QTextEdit;
class QLabel;
class QModelIndex;

class TextSelection : public QWidget
{
  Q_OBJECT
public:
  TextSelection( QWidget*, KraftDoc::Part );

  ~TextSelection();

  QString currentText() const;
  DocText currentDocText() const;

signals:
  void actionCurrentTextToDoc();
  void currentTextChanged( const QString& );

public slots:
  QTreeWidgetItem* addNewDocText( const DocText& );
  void deleteCurrentText();
  void updateDocText( const DocText& );
  void slotSelectDocType( const QString& );
  void slotRMB( QPoint );

protected:
  void initActions();
  void buildTextList( KraftDoc::Part );
  QTreeWidgetItem* addOneDocText( QTreeWidgetItem*, const DocText& );

protected slots:
  // void slotSelectionChanged( QTreeWidgetItem* );
  void slotTemplateNameSelected( const QModelIndex&, const QModelIndex& );
  void showHelp( const QString& help = QString() );

private:
  QListView                       *mTextNameView;
  QStringListModel                *mTemplNamesModel;
  QLabel                          *mTextDisplay;
  QLabel                          *mHelpDisplay;
  QLabel                          *mHeadLabel;
  KraftDoc::Part                   mPart;

  QString                          mDocType;
  QString                          mCurrTemplateName;

  FilterHeader                    *mListSearchLine;
  QMap<QTreeWidgetItem*, DocText> mTextMap;
  QMap<QString, QTreeWidgetItem*> mDocTypeItemMap;
  QMap<QString, QTreeWidgetItem*> mStandardItemMap;

  QMenu             *mMenu;
  KActionCollection *mActions;
  KAction           *mAcMoveToDoc;
};

#endif
