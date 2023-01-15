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

#include <QSplitter>
#include <QTreeWidgetItem>
#include <QScopedPointer>
#include <QStackedWidget>

#include <kcontacts/addressee.h>

#include "kraftdoc.h"
#include "catalogtemplate.h"
#include "docpostcard.h"
#include "catalogselection.h"

class TextSelection;
class QWidget;
class QPushButton;
class Katalog;
class TemplateProvider;
class HeaderTemplateProvider;
class CatalogTemplateProvider;
class FooterTemplateProvider;
class AddressTemplateProvider;
class DocText;
class QSplitter;

using namespace KContacts;

class DocAssistant : public QSplitter
{
  Q_OBJECT

public:
  DocAssistant( QWidget* );

  DocPostCard *postCard();
  CatalogSelection *catalogSelection();

  void saveSplitterSizes();

public slots:
  void slotShowCatalog();
  void slotShowHeaderTemplates();
  void slotShowFooterTemplates();
  void setFullPreview( bool, KraftDoc::Part);
  void slotSelectDocPart(KraftDoc::Part);
  void slotToggleShowTemplates( bool );
  void slotAddToDocument();
  void slotInsertIntoDocument();
  void slotNewTemplate();
  void slotEditTemplate();
  void slotDeleteTemplate();

  void slotSetDocType( const QString& );

protected slots:
  void slotTemplateSelectionChanged();
  void slotFooterTextDeleted( const DocText& );
  void slotHeaderTextDeleted( const DocText& );
  void slotNewHeaderDocText( const DocText& );
  void slotUpdateHeaderDocText( const DocText& );
  void slotCatalogSelectionChanged( QTreeWidgetItem*,QTreeWidgetItem* );
  void slotNewFooterDocText( const DocText& );
  void slotUpdateFooterDocText( const DocText& );

signals:
  void selectPage( KraftDoc::Part );
  void templatesToDocument( Katalog*, CatalogTemplateList, const QString&);
  void toggleShowTemplates( bool );

  void addressTemplate( const Addressee& );
  void headerTextTemplate( const DocText&, bool replace );
  void footerTextTemplate( const DocText&, bool replace );

private:
  DocPostCard      *mPostCard;
  CatalogSelection *mCatalogSelection;
  QStackedWidget   *mWidgetStack;
  TextSelection    *mFooterSelection;
  TextSelection    *mHeaderSelector;

  bool mFullPreview;
  KraftDoc::Part  mActivePage;
  QPushButton    *mPbAdd;
  QPushButton    *mPbInsert;
  QPushButton    *mPbNew;
  QPushButton    *mPbEdit;
  QPushButton    *mPbDel;
  QWidget        *mTemplatePane;
  QString         mDocType;

  // QSplitter      *mMainSplit;

  TemplateProvider        *mCurrTemplateProvider;
  HeaderTemplateProvider  *mHeaderTemplateProvider;
  AddressTemplateProvider *mAddressTemplateProvider;
  CatalogTemplateProvider *mCatalogTemplateProvider;
  FooterTemplateProvider  *mFooterTemplateProvider;

};


#endif
