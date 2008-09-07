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

#include <kdialogbase.h>
#include <klistview.h>

#include "tagman.h"

class QWidget;
class QStringList;
class QListViewItem;
class KListView;
class FilterHeader;
class QPushButton;
class KColorButton;

class TagTemplateEditor: public KDialogBase
{
  Q_OBJECT

  public:
  TagTemplateEditor( QWidget* );
  ~TagTemplateEditor();

  void setTemplate( const TagTemplate& );
  TagTemplate currentTemplate();
   
private:
  TagTemplate mOrigTemplate;
  KLineEdit *mNameEdit;
  KTextEdit *mDescriptionEdit;
  KColorButton *mColorButton;
    
};

class TagTemplatesDialog: public KDialogBase
{
  Q_OBJECT

public:
  TagTemplatesDialog( QWidget* );
  ~TagTemplatesDialog( );

  TagTemplate currentTemplate();

protected slots:
  void slotSelectionChanged();
  void slotAddTemplate();
  void slotEditTemplate();
  void slotDeleteTemplate();

protected:
  void setTags( );

private:
  KListView *mListView;
  QMap<QListViewItem*, QString> mItemMap;

  QPushButton *mAddButton;
  QPushButton *mEditButton;
  QPushButton *mDeleteButton;
};

#endif
