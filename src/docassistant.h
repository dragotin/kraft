/***************************************************************************
                          docassistant.h  - Assistant widget
                             -------------------
    begin                : April 2007
    copyright            : (C) 2007 by Klaas Freitag
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

#ifndef DOCASSISTANT_H
#define DOCASSISTANT_H

#include <qsplitter.h>

class DocPostCard;
class CatalogSelection;
class HeaderSelection;
class QWidgetStack;
class QWidget;
class KPushButton;
class Katalog;

class DocAssistant : public QSplitter
{
  Q_OBJECT

public:
  DocAssistant( QWidget* );

  DocPostCard *postCard();
  CatalogSelection *catalogSelection();

  bool isFullPreview() {
    return mFullPreview;
  }
public slots:
  void slotShowCatalog();
  void slotShowAddresses();
  void setFullPreview( bool, int );
  void slotRenderCompleted();
  void slotSelectPage( int );
  void slotToggleShowTemplates( bool );
signals:
  void selectPage( int );
  void positionSelected( Katalog*, void* );
  void toggleShowTemplates( bool );
private:
  DocPostCard *mPostCard;
  CatalogSelection *mCatalogSelection;
  QWidgetStack *mWidgetStack;
  HeaderSelection *mHeaderSelection;
  bool mFullPreview;
  int  mActivePage;
  KPushButton    *mPbAdd;
  KPushButton    *mPbNew;
  QWidget        *mTemplatePane;
};


#endif
