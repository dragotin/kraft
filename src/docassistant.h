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

#include <kabc/addressee.h>

#include "kraftdoc.h"

class DocPostCard;
class CatalogSelection;
class HeaderSelection;
class QWidgetStack;
class QWidget;
class QListViewItem;
class KPushButton;
class Katalog;
class TemplateProvider;
class HeaderTemplateProvider;
class CatalogTemplateProvider;
class DocText;

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
  void slotShowHeaderTemplates();
  void setFullPreview( bool, int );
  void slotRenderCompleted();
  void slotSelectDocPart( int );
  void slotToggleShowTemplates( bool );
  void slotAddToDocument();
  void slotNewTemplate();
  void slotEditTemplate();
  void slotDeleteTemplate();

  void slotSetDocType( const QString& );

protected slots:
  void slotAddressSelectionChanged();
  void slotTextsSelectionChanged();
  void slotHeaderTextToDocument( const DocText& );
  void slotTextDeleted( const DocText& dt );
  void slotNewHeaderDocText( const DocText& );
  void slotUpdateHeaderDocText( const DocText& );
  void slotCatalogSelectionChanged( QListViewItem* );

signals:
  void selectPage( int );
  void positionSelected( Katalog*, void* );
  void toggleShowTemplates( bool );

  void addressTemplate( const KABC::Addressee& );
  void headerTextTemplate( const QString& );

private:
  DocPostCard *mPostCard;
  CatalogSelection *mCatalogSelection;
  QWidgetStack *mWidgetStack;
  HeaderSelection *mHeaderSelection;
  /* FooterSelection */ QWidget *mFooterSelection;
  bool mFullPreview;
  int            mActivePage;
  KPushButton    *mPbAdd;
  KPushButton    *mPbNew;
  KPushButton    *mPbEdit;
  KPushButton    *mPbDel;
  QWidget        *mTemplatePane;
  QString         mDocType;

  TemplateProvider* mCurrTemplateProvider;
  HeaderTemplateProvider *mHeaderTemplateProvider;
  CatalogTemplateProvider *mCatalogTemplateProvider;
};


#endif
