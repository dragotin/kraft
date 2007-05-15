/***************************************************************************
                 addressselection  - widget to select Addresses
                             -------------------
    begin                : 2006-09-03
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

#ifndef HEADERSELECTION_H
#define HEADERSELECTION_H

#include <qtabwidget.h>
#include <qmap.h>

#include <kabc/addressee.h>

class QComboBox;
class FilterHeader;
class KListView;
class KListViewItem;
class QListViewItem;
class AddressSelection;
class KPushButton;
class DocText;

class HeaderSelection : public QTabWidget
{
  Q_OBJECT
public:
  HeaderSelection( QWidget* );

  ~HeaderSelection();

  bool textPageActive();
  bool addressPageActive();

  KABC::Addressee currentAddressee();
  QString currentText() const;
  DocText currentDocText() const;

  KListView *addressListView() { return mAddressView; }
  KListView *textsListView() { return mTextsView; }

signals:
  void addressSelectionChanged();
  void textSelectionChanged();

public slots:
  QListViewItem* addNewDocText( const DocText& );
  void deleteCurrentText();
  void updateDocText( const DocText& );
  void slotSelectDocType( const QString& );
protected:
  void initActions();
  void buildHeaderTextList();
  KListViewItem* addOneDocText( QListViewItem*, const DocText& );

protected slots:
  void slotAddressNew();
  void slotCurrentTabChanged( QWidget * );

private:
  FilterHeader   *mListSearchLine;
  KListView      *mTextsView;
  KListView      *mAddressView;
  QMap<QListViewItem*, DocText> mTextMap;
  QMap<QString, QListViewItem*> mDocTypeItemMap;

  AddressSelection *mAddressSelection;
  int mAddressTabId;
  int mTextsTabId;
};

#endif
