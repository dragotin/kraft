/***************************************************************************
      headertemplateprovider - template provider classes for header data
                               like addresses or texts
                             -------------------
    begin                : 2007-05-02
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
#ifndef HEADERTEMPLATEPROVIDER_H
#define HEADERTEMPLATEPROVIDER_H

#include "templateprovider.h"
#include "doctext.h"

class QWidget;

class HeaderTemplateProvider : public TemplateProvider
{
  Q_OBJECT
public:
  HeaderTemplateProvider( QWidget* );

public slots:
  void slotNewTemplate() override;
  void slotEditTemplate() override;
  void slotDeleteTemplate() override;

  void slotTemplateToDocument() override;
  void slotInsertTemplateToDocument() override;

signals:
  void newHeaderText( const DocText& );
  void updateHeaderText( const DocText& );
  void headerTextToDocument(const DocText&, bool replace);
  void deleteHeaderText( const DocText& );

private:

};


#endif

