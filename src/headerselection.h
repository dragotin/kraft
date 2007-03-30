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

#include <qsplitter.h>

class QComboBox;
class FilterHeader;
class KListView;
class KListViewItem;
class AddressSelection;

class HeaderSelection : public QSplitter
{
  Q_OBJECT
public:
  HeaderSelection( QWidget* );

  ~HeaderSelection();
protected:
  void initActions();
  void getHeaderTextList();
protected slots:

private:
  FilterHeader   *mListSearchLine;
  KListView      *mTextsView;
  KListView      *mAddressView;

  AddressSelection *mAddressSelection;
};

#endif
