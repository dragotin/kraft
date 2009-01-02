/***************************************************************************
                   doctypeedit.h  - the document type editor 
                             -------------------
    begin                : Fri Jan 2 2009
    copyright            : (C) 2009 by Klaas Freitag
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

#ifndef DOCTYPEEDIT_H
#define DOCTYPEEDIT_H

#include <kdialogbase.h>

#include <qmap.h>

#include "doctype.h"
#include "doctypeedit.h"
#include "doctypeeditbase.h"
#include "doctypedetailseditbase.h"

class QLineEdit;
class QLabel;
class QTextEdit;
class QPushButton;
class QComboBox;
class QCheckBox;


/**
 *  @author Klaas Freitag
 */
class DocTypeDetailsEdit : public KDialogBase
{
  Q_OBJECT

public:
  DocTypeDetailsEdit( QWidget *parent, const QString& docType );

protected slots:
  void slotUpdateExample();

private:
  DocTypeDetailsEditBase *mBaseWidget;
  DocType mDocType;
};

// ################################################################################

class DocTypeEdit : public DocTypeEditBase
{
  Q_OBJECT

public:
  DocTypeEdit( QWidget *parent );
  void saveDocTypes();

public slots:
  void slotDocTypeSelected( const QString& = QString() );
  void slotAddDocType();
  void slotEditDocType();
  void slotRemoveDocType();

protected slots:
  void slotEditDetails();

private:
  DocType mOrigDocType;

  QStringList allNumberCycles();
  void removeTypeFromDb( const QString& );
  void renameTypeInDb( const QString&, const QString& );

  QMap<QString, QString> mTypeNameChanges;
  QMap<QString, DocType> mOrigDocTypes;
  QStringList mAddedTypes;
  QStringList mRemovedTypes;
};

#endif
