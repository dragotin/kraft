/***************************************************************************
                 tagtemplatedit.h  - Edit tag templates 
                             -------------------
    begin                : Sep 2008
    copyright            : (C) 2008 by Klaas Freitag
    email                : freitag@kde.org
 ***************************************************************************/

/***************************************************************************
 *
 *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef TAGTEMPLATESDIALOG_H
#define TAGTEMPLATESDIALOG_H

#include <qmap.h>

#include <QtCore>

#include "tagman.h"


class TagTemplateEditor: public QDialog
{
  Q_OBJECT

  public:
  TagTemplateEditor( QWidget* );
  ~TagTemplateEditor();

  void setTemplate( const TagTemplate& );
  TagTemplate currentTemplate();
   
private Q_SLOTS:
  void slotColorSelect(bool);
  void setColorButton();

private:
  TagTemplate mOrigTemplate;
  QLineEdit *mNameEdit;
  QTextEdit *mDescriptionEdit;
  QPushButton *mColorButton;
  QPushButton *mOkButton;
  QColor mColor;
    
};

class TagTemplatesDialog: public QDialog
{
  Q_OBJECT

public:
  TagTemplatesDialog( QWidget* );
  ~TagTemplatesDialog( );

  TagTemplate currentTemplate();

protected Q_SLOTS:
  void slotSelectionChanged();
  void slotAddTemplate();
  void slotEditTemplate();
  void slotDeleteTemplate();

protected:
  void setTags( );

private:
  QTreeWidget *mListView;
  QMap<QTreeWidgetItem*, QString> mItemMap;

  QPushButton *mAddButton;
  QPushButton *mEditButton;
  QPushButton *mDeleteButton;
};

#endif
