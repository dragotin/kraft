 /***************************************************************************
                        htmlview.h  - show a html page
                             -------------------
    begin                : Aug 2006
    copyright            : (C) 2006 Klaas Freitag <freitag@kde.org>
                           (C) 2006 Cornelius Schumacher <schumacher@kde.org>
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef HTMLVIEW_H
#define HTMLVIEW_H

#include <QTextBrowser>
#include <QDesktopServices>

class QUrl;

class HtmlView : public QTextBrowser
{
    Q_OBJECT
  public:
    HtmlView( QWidget *parent = 0);

    QString title() const { return mTitle; }

  public slots:
    void setTitle( const QString & );
    void setStylesheetFile( const QString & );
    void displayContent( const QString& );

    void zoomIn();
    void zoomOut();

  protected:

    void updateZoomActions();

  signals:
    void openUrl( const QUrl& );

  private:
    QString topFrame() const;
    QString bottomFrame() const;
    QString readStyles(const QString &styleFile) const;

    QString mTitle;
    QString mInternalUrl;
    QString mStyles;

    QAction *mZoomInAction;
    QAction *mZoomOutAction;

    int mZoomStep;
};

#endif
