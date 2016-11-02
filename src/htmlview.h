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

#include <QWebView>

class QAction;
class QUrl;

class HtmlView : public QWebView
{
    Q_OBJECT
  public:
    HtmlView( QWidget *parent = 0);

    void clearView();

    QString title() const { return mTitle; }

    void setInternalUrl( const QString & );
    QString internalUrl() const;

    void setBaseUrl( const QString& );

  public slots:
    void setTitle( const QString & );
    void setStylesheetFile( const QString & );
    void displayContent( const QString& );

    void zoomIn();
    void zoomOut();

  protected:
    virtual QString topFrame();
    virtual QString bottomFrame();

    void updateZoomActions();
  private:

    QString locateCSSImages( const QByteArray& line );

    QString mTitle;
    QString mInternalUrl;
    QString mStyleSheetFile;

    QAction *mZoomInAction;
    QAction *mZoomOutAction;
    QUrl     mBaseUrl;

    int mZoomStep;
};

#endif
