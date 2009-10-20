/***************************************************************************
                     extendedcombo - a better combo box
                             -------------------
    begin                : April 2008
    copyright            : (C) 2008 by Klaas Freitag
                           (C) 2007, 2008 Andre Duffeck
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
#ifndef EXTENDEDCOMBO_H
#define EXTENDEDCOMBO_H

#include <QComboBox>
#include "kraftcat_export.h"

class QString;

#if 0
class KRAFTCAT_EXPORT ExtendedComboItem : public Q3ListBoxItem
{
  public:
    ExtendedComboItem( const QString &name, const QString &description, ExtendedCombo *combo );

    virtual int width(const Q3ListBox *) const;
    virtual int height(const Q3ListBox *) const;
    virtual void setPixmap( const QPixmap& );
  protected:
    virtual void paint(QPainter *p);
    virtual QFont descriptionFont() const;

  private:
    QString mDescription;
    QPixmap mPixmap;
};
#endif

class KRAFTCAT_EXPORT ExtendedCombo : public QComboBox
{
  public:
    ExtendedCombo(QWidget *parent, const char *name = 0);
    ~ExtendedCombo();
    void insertEntry( const QString &name, const QString &description );

};

#endif


