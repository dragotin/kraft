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
#include "ui_documentactions.h"

class DocDigestHtmlView : public HtmlView
{
    Q_OBJECT

public:
    DocDigestHtmlView( QWidget *parent );

protected Q_SLOTS:
    void slotLinkClicked(const QUrl& url);
};

class DocDigestDetailView : public QFrame
{
    Q_OBJECT
public:
    explicit DocDigestDetailView(QWidget *parent = 0);
    void initViewActions(const std::array<QAction *, 4> actions);

public Q_SLOTS:
    void slotShowStart();
    void slotShowDocDetails(const DocDigest &, const QString &errHeader = QString(), const QString &errDetails = QString());
    void slotClearView();

    void slotShowMonthDetails( int year, int month );
    void slotShowYearDetails( int year);
    void setErrorStrings(const QString& = QString(), const QString& = QString());

private:
    void showAddress( const KContacts::Addressee& addressee, const QString& manAddress );
    QList<QObject*> documentListing(int year, int month );

    enum Location { Left, Middle, Right };
    enum Detail { Start, Month, Year, Document };

    QString widgetStylesheet( Location loc, Detail det );

    DocDigestHtmlView *mHtmlCanvas;
    QLabel    *_leftDetails;
    QLabel    *_rightDetails;
    QString   _docTemplFileName;
    QString   _monthTemplFileName;
    QString   _yearTemplFileName;
    DocDigest _currentDigest;
    Ui::docActionsWidget *_docActionsWidget;
};

#endif // DOCDIGESTDETAILVIEW_H
