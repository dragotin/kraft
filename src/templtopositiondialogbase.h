/***************************************************************************
         templtopositiondialogbase.h  - base dialog template to doc
                             -------------------
    begin                : Mar 2007
    copyright            : (C) 2007 Klaas Freitag
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

#ifndef TEMPLTOPOSITIONDIALOGBASE
#define TEMPLTOPOSITIONDIALOGBASE

#include <QDialog>

#include <catalogchapter.h>

class QWidget;
class DocPosition;
class DocPositionList;
class QComboBox;

class TemplToPositionDialogBase: public QDialog
{
  Q_OBJECT

public:
  TemplToPositionDialogBase( QWidget* );
  ~TemplToPositionDialogBase( );

  virtual void setDocPosition( DocPosition*, bool, bool ) = 0;
  virtual void setCatalogChapters( const QList<CatalogChapter>& ) = 0;
  virtual QString chapter() const = 0;

  void setPositionList( DocPositionList, int );
  int insertAfterPosition();
  virtual DocPosition docPosition() = 0;
protected:
  /**
   * Needs to be reimplemented to return a pointer to a
   * combobox which can be filled with the current list
   * of positions to let the user select where the new
   * pos should go to.
   */
  virtual QComboBox* getPositionCombo() = 0;
};

#endif
