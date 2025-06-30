/***************************************************************************
      footertemplateprovider - template provider classes for footer data
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
#ifndef FOOTERTEMPLATEPROVIDER_H
#define FOOTERTEMPLATEPROVIDER_H

#include "templateprovider.h"

class QWidget;
class DocText;

class FooterTemplateProvider : public TemplateProvider
{
    Q_OBJECT

public:
    FooterTemplateProvider( QWidget* );

public Q_SLOTS:
    void slotNewTemplate() override;
    void slotEditTemplate() override;
    void slotDeleteTemplate() override;

    void slotTemplateToDocument() override;
    void slotInsertTemplateToDocument() override;

Q_SIGNALS:
    void newFooterText( const DocText& );
    void updateFooterText( const DocText& );
    void footerTextToDocument(const DocText&, bool replace);
    void deleteFooterText( const DocText& );
};


#endif

