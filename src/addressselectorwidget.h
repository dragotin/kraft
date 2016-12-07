/***************************************************************************
    akonadiaddressselector - Address Selection Widget based on Akonadi
                             -------------------
    begin                : Jul 2011
    copyright            : (C) 2011 by Klaas Freitag
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

#ifndef ADDRESSSELECTORWIDGET_H
#define ADDRESSSELECTORWIDGET_H

#include <QWidget>

#include <kcontacts/addressee.h>
#include "ui_addressselectorwidget.h"

using namespace KContacts;

class QLabel;
class QPushButton;
class QSplitter;
class QTreeView;
class QuickSearchWidget;
class QItemSelectionModel;
class QuickSearchWidget;

class AddressSelectorWidget : public QWidget
{
    Q_OBJECT
public:
  explicit AddressSelectorWidget(QWidget *parent = 0, bool showText = true );

  ~AddressSelectorWidget();

signals:
  void addressSelected( const Addressee& );

public slots:
  void saveState();

protected slots:
  void slotCreateNewContact();
  void slotEditContact();
  void restoreState();
  void slotItemActivated( const QModelIndex& index );

private:
  void setupGui();

  QPushButton       *mButEditContact;

  Ui::AddressSelectorWidget *mAddressSelectorUi;
};

#endif // AKONADIADDRESSSELECTOR_H
