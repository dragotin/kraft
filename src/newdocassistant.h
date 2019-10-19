/***************************************************************************
                 new doc assistant - widget to select Addresses
                             -------------------
    begin                : 2008-02-12
    copyright            : (C) 2008 by Klaas Freitag
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

#ifndef NEWDOCASSISTANT_H
#define NEWDOCASSISTANT_H

#include <QMap>
#include <QLabel>

#include <kcontacts/addressee.h>
#include <kassistantdialog.h>

#include "kraftdoc.h"
#include "docguardedptr.h"

class DocText;
class TextSelection;
class KraftWizard;
class AddressSelectorWidget;
class QDateEdit;
class QComboBox;
class QHBox;
class QTextEdit;
class QCheckBox;

using namespace KContacts;


// ---------------------------------------------------------------------------

class CustomerSelectPage: public QWidget
{
  Q_OBJECT

  friend class KraftWizard;

public:
  CustomerSelectPage( QWidget *parent = 0 );
  ~CustomerSelectPage();
  void setupAddresses();

public slots:
  void saveState();

signals:
  void addresseeSelected( const KContacts::Addressee& );

private:
  AddressSelectorWidget *mAddresses;
};

// ---------------------------------------------------------------------------

class DocDetailsPage : public QWidget
{
  Q_OBJECT

  friend class KraftWizard;

public:
  DocDetailsPage( QWidget *parent = 0 );
  ~DocDetailsPage();

private:
  QLabel      *mCustomerLabel;
  QDateEdit   *mDateEdit;
  QComboBox   *mTypeCombo;
  QTextEdit   *mWhiteboardEdit;
  QCheckBox   *mKeepItemsCB;
  QComboBox   *mSourceDocIdentsCombo;
};

// ---------------------------------------------------------------------------

class KraftWizard: public KAssistantDialog
{
  Q_OBJECT

public:
  KraftWizard(QWidget *parent = 0, const char* name = 0, bool modal = false );
  void init(bool haveAddressSelect, const QString& followUpDoc = QString());

  ~KraftWizard();

  QDate date() const ;
  QString addressUid() const;
  QString docType() const;
  QString whiteboard() const;
  void setCustomer( const QString& );
  void setDocToFollow( DocGuardedPtr sourceDoc);
  void setAvailDocTypes( const QStringList& );
  void done(int r);
  QString copyItemsFromPredecessor();

protected slots:
  void slotAddressee( const KContacts::Addressee& );

private:
  CustomerSelectPage *mCustomerPage;
  DocDetailsPage *mDetailsPage;
  QHBox *mCustomerBox;
  QWidget *mParent;

  KContacts::Addressee mAddressee;

  KPageWidgetItem *mCustomerPageItem;
  KPageWidgetItem *mDetailsPageItem;
};

#endif
