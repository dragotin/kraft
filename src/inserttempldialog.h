/***************************************************************************
 inserttemplatedialog.h  - small dialog to insert templates into documents
                             -------------------
    begin                : Sep 2006
    copyright            : (C) 2006 Klaas Freitag
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
#ifndef INSERTTEMPLDIALOG_H
#define INSERTTEMPLDIALOG_H

#include <QDialog>
#include <qmap.h>

#include "docposition.h"
#include "templtopositiondialogbase.h"
#include "ui_inserttmplbase.h"

class QCheckBox;

class InsertTemplDialog: public TemplToPositionDialogBase
{
  Q_OBJECT

public:
  InsertTemplDialog( QWidget* );
  ~InsertTemplDialog();

  void setDocPosition( DocPosition*, bool, bool );
  DocPosition docPosition();

  void setCatalogChapters( const QList<CatalogChapter>& );
  QString chapter() const;
protected:
  QComboBox *getPositionCombo();

private:
  QString prepareText( const QString& input );

  Ui::insertTmplBase *mBaseWidget;
  DocPosition mParkPosition;
  QMap<QCheckBox*, QString> mTagMap;
};

#endif
