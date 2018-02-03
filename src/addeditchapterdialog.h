/***************************************************************************
               addeditchapter - add and edit catalog chapters
                             -------------------
    begin                : Sat Nov 6 2010
    copyright            : (C) 2010 by Klaas Freitag
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

#ifndef ADDEDITCHAPTERDIALOG_H
#define ADDEDITCHAPTERDIALOG_H

#include <QLabel>
#include <QLineEdit>

#include <QDialog>

#include "catalogchapter.h"


class AddEditChapterDialog : public QDialog
{
  Q_OBJECT

public:
    AddEditChapterDialog(QWidget *parent = 0);

    void setEditChapter( const CatalogChapter& );
    void setParentChapter( const CatalogChapter& );

    QString name() const;
    QString description() const;

private:
    CatalogChapter mChapter;
    CatalogChapter mParentChapter;
    QLineEdit *mNameEdit;
    QLineEdit *mDescEdit;
    QLabel    *mTopLabel;
};

#endif // ADDEDITCHAPTERDIALOG_H
