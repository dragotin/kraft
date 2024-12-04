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

  void setDocPosition( DocPositionBase*, bool, bool ) override;
  DocPositionBase docPosition() override;

  void setCatalogChapters( const QList<CatalogChapter>&, const QString& selectedChap) override;
  QString chapter() const override;
protected:
  QComboBox *getPositionCombo() override;

private:
  QString prepareText( const QString& input );

  Ui::insertTmplBase *mBaseWidget;
  DocPositionBase mParkPosition;
  QMap<QCheckBox*, QString> mTagMap;
};

#endif
