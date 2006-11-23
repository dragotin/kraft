/***************************************************************************
                   prefsdialog.h  - the preferences Dialog
                             -------------------
    begin                : Sun Jul 3 2004
    copyright            : (C) 2004 by Klaas Freitag
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

#ifndef PREFSDIALOG_H
#define PREFSDIALOG_H

#include <kdialogbase.h>

class QLineEdit;
class QLabel;
class QTextEdit;
class QPushButton;
class QComboBox;

/**
 *  @author Klaas Freitag
 */
class PrefsDialog : public KDialogBase
{
  Q_OBJECT

public:
  PrefsDialog(QWidget *parent);

  ~PrefsDialog();


protected:
  void readConfig();
  void writeConfig();

protected slots:
  void slotOk();
  void slotTextChanged( const QString& );
  void slotCheckConnect();

  void footerTextTab();
  void entryTextTab();
  void slotFooterDocTypeChanged( const QString& );
  void slotHeaderDocTypeChanged( const QString& );
private:
  void databaseTab();

  QLineEdit *m_leHost;
  QLineEdit *m_leUser;
  QLineEdit *m_leName;
  QLineEdit *m_lePasswd;
  QLabel    *m_statusLabel;

  QTextEdit *mHeaderEdit;
  QTextEdit *mFooterEdit;
  QPushButton *m_pbCheck;
  QComboBox   *mTextTypeEntry;
  QComboBox   *mTextTypeFooter;
  QString     mFooterDocType;
  QString     mHeaderDocType;
};

#endif
