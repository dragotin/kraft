/***************************************************************************
      templateprovider - base class for the template provider classes.
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

#include "templateprovider.h"

TemplateProvider::TemplateProvider( QWidget *parent )
  : QObject(), mParent( parent )
{

}

TemplateProvider::~TemplateProvider()
{

}

void TemplateProvider::slotSetDocType( const QString& str )
{
  mDocType = str;
}

#include "templateprovider.moc"
