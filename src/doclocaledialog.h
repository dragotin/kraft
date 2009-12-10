/***************************************************************************
              doclocaledialog.h  - Edit document locale
                             -------------------
    begin                : Jan 2008
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


#ifndef DOCLOCALE_H
#define DOCLOCALE_H

#include <kdialog.h>
//Added by qt3to4:
#include <QLabel>

class QWidget;
class QComboBox;
class DocText;
class TextEditBase;
class KLanguageButton;
class QLabel;
class KLocale;

class DocLocaleDialog : public KDialog
{
  Q_OBJECT

public:
  DocLocaleDialog( QWidget* );
  ~DocLocaleDialog( );

  void setLocale( const QString&, const QString& );
  KLocale locale() const;

  void readLocale(const QString &, QString &,
		  const QString &) const;

public slots:
  void changedCountry(const QString & );
  void slotUpdateSample();

private:
  void loadLanguageList();
  void loadCountryList();

  QComboBox *mCountryButton;
  KLanguageButton *mLanguageButton;
  QStringList languageList() const;

  KLocale *mLocale;
  QLabel *mLabSample;
};

#endif
