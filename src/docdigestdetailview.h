/***************************************************************************
             docdigestdetailview.cpp  - Details of a doc digest
                             -------------------
    begin                : februry 2011
    copyright            : (C) 2011 by Klaas Freitag
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
#ifndef DOCDIGESTDETAILVIEW_H
#define DOCDIGESTDETAILVIEW_H

#include <QWidget>
#include <QLabel>

#include "docdigest.h"
#include "htmlview.h"

class dbID;
class TextTemplate;
class GrantleeFileTemplate;

class DocDigestHtmlView : public HtmlView
{
    Q_OBJECT

public:
    DocDigestHtmlView( QWidget *parent );

signals:
    void showLastPrint( const dbID& );

protected slots:
    void slotLinkClicked(const QUrl& url);

};

// #########################################################################################################

class GrantleeDocWrapper : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString type READ type)
    Q_PROPERTY(QString sum  READ sum)
    Q_PROPERTY(QString amount READ amount)

public:
    QString type() const { return _type; }
    QString sum() const { return _sum; }
    QString amount() const {return QString::number(_amount); }

    int _amount;
    QString _type;
    QString _sum;
};

class GrantleeDocListWrapper : public QObject{
    Q_OBJECT
    Q_PROPERTY(QList<GrantleeDocWrapper*> docs READ docs)

public:
    QList<GrantleeDocWrapper*> docs() { return _list; }

    QList<GrantleeDocWrapper*> _list;

};

// #########################################################################################################

class DocDigestDetailView : public QFrame
{
  Q_OBJECT
public:
  explicit DocDigestDetailView(QWidget *parent = 0);

signals:
  void showLastPrint( const dbID& );

public slots:
  void slotShowDocDetails( DocDigest );
  void slotClearView();

  void slotShowMonthDetails( int year, int month );
  void slotShowYearDetails( int year);

private:
  void showAddress( const KContacts::Addressee& addressee, const QString& manAddress );
    void documentListing(GrantleeFileTemplate *tmpl, int year, int month );

  enum Location { Left, Middle, Right };
  enum Detail { Month, Year, Document };

  QString widgetStylesheet( Location loc, Detail det );

  DocDigestHtmlView *mHtmlCanvas;
  QLabel    *_leftDetails;
  QLabel    *_rightDetails;
  QString   _docTemplFileName;
  QString   _monthTemplFileName;
  QString   _yearTemplFileName;
};

#endif // DOCDIGESTDETAILVIEW_H
