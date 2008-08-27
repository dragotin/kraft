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
#ifndef TEMPLATEPROVIDER_H
#define TEMPLATEPROVIDER_H

#include <qobject.h>

class QWidget;
class TextSelection;
class DocText;

class TemplateProvider : public QObject
{
  Q_OBJECT
public:
  TemplateProvider( QWidget* );
  ~TemplateProvider();
  virtual void setSelection( TextSelection* );
  virtual DocText currentText();

public slots:
  virtual void slotNewTemplate() = 0;
  virtual void slotEditTemplate() = 0;
  virtual void slotDeleteTemplate() = 0;

  virtual void slotTemplateToDocument() = 0;

  void slotSetDocType( const QString& );
protected:
  QWidget *mParent;
  QString  mDocType;
  TextSelection *mTextSelection;
};


#endif

