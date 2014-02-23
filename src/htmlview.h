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

#include <khtml_part.h>
class KAction;
class KUrl;

class HtmlView : public KHTMLPart
{
    Q_OBJECT
  public:
    HtmlView( QWidget *parent = 0);

    void clearView();

    void setupActions( KActionCollection * );

    QString title() const { return mTitle; }

    void setInternalUrl( const QString & );
    QString internalUrl() const;

    void showWelcomePage();
    void setBaseUrl( const QString& );

  public slots:
    void setTitle( const QString & );
    void setStylesheetFile( const QString & );
    void displayContent( const QString& );

    void zoomIn();
    void zoomOut();

  protected:
    virtual void writeTopFrame();
    virtual void writeBottomFrame();
    virtual void writeContent( const QString& );

    void updateZoomActions();
  private:

    QString locateCSSImages( const QByteArray& line );

    QString mTitle;
    QString mInternalUrl;
    QString mStyleSheetFile;

    KAction *mZoomInAction;
    KAction *mZoomOutAction;
    KUrl     mBaseUrl;

    int mZoomStep;
};

#endif
