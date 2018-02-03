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

#include <kcontacts/addressee.h>

#include "kraftdoc.h"

class QTreeWidget;
class QTreeWidgetItem;
class QString;
class QPoint;
class QMenu;
class DocText;
class QAction;
class QListView;
class QStringListModel;
class QTextEdit;
class QLabel;
class QModelIndex;
class QGroupBox;

class TextSelection : public QWidget
{
  Q_OBJECT
public:
  TextSelection( QWidget*, KraftDoc::Part );

  ~TextSelection();

  QString currentText() const;
  DocText currentDocText() const;
  bool    validSelection() const;

signals:
  void actionCurrentTextToDoc();
  void currentTextChanged( const QString& );
  void validTemplateSelected();
  void editCurrentTemplate();

public slots:
  void addNewDocText( const DocText& );
  void deleteCurrentText();
  void updateDocText( const DocText& );
  void slotSelectDocType( const QString& );
  void slotRMB( QPoint );

protected:
  void initActions();
  void buildTextList( KraftDoc::Part );
  void showDocText( DocText );

protected slots:
  // void slotSelectionChanged( QTreeWidgetItem* );
  void slotTemplateNameSelected( const QModelIndex&, const QModelIndex& );
  void showHelp( const QString& help = QString() );

private:
  QListView                       *mTextNameView;
  QStringListModel                *mTemplNamesModel;
  QTextEdit                       *mTextDisplay;
  QLabel                          *mHelpDisplay;
  KraftDoc::Part                   mPart;

  QString                          mDocType;
  QString                          mCurrTemplateName;

  QMap<QTreeWidgetItem*, DocText> mTextMap;
  QMap<QString, QTreeWidgetItem*> mDocTypeItemMap;
  QMap<QString, QTreeWidgetItem*> mStandardItemMap;

  QMenu             *mMenu;
  QGroupBox         *mGroupBox;
  QAction           *mAcMoveToDoc;
};

#endif
