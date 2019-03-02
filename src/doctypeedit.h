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

#include <QDialog>

#include <QMap>
#include <QLabel>

#include "doctype.h"
#include "ui_doctypeeditbase.h"

class QLineEdit;
class QLabel;
class QPushButton;
class QComboBox;
class QCheckBox;


/**
 *  @author Klaas Freitag
 */

// ################################################################################

class DocTypeEdit : public QWidget, protected Ui::DocTypeEditBase
{
  Q_OBJECT

public:
  DocTypeEdit( QWidget *parent  = 0 );
  void saveDocTypes();

public slots:
  void slotDocTypeSelected( const QString& = QString() );
  void slotAddDocType();
  void slotEditDocType();
  void slotRemoveDocType();

protected slots:
  void fillNumberCycleCombo();
  void slotNumberCycleChanged( const QString& );
  void slotEditNumberCycles();
  void slotWatermarkModeChanged( int );

  void slotWatermarkUrlChanged( const QString& );
  void slotTemplateUrlChanged( const QString& );

signals:
  /**
   * emitted for every doctype which is deleted 
   */
  void removedType( const QString& );

private:
  DocType currentDocType();

  DocType mOrigDocType;

  QStringList allNumberCycles();
  QStringList removedTypes() { return mRemovedTypes; }

  void removeTypeFromDb( const QString& );
  void renameTypeInDb( const QString&, const QString& );

  QMap<QString, QString> mTypeNameChanges;
  
  QMap<QString, DocType> mChangedDocTypes;
  QMap<QString, DocType> mOrigDocTypes;
  QStringList mAddedTypes;
  QStringList mRemovedTypes;

  QString mPreviousType;
  QString mExampleDocType;
  QString mExampleAddressUid;
};

#endif
