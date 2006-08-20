/*
    This file is part of Fate.

    Copyright (c) 2005 SUSE LINUX Products GmbH

    Author: Cornelius Schumacher <cschum@suse.de>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/
#ifndef HTMLVIEW_H
#define HTMLVIEW_H

#include <khtml_part.h>

class MetaFile;

class HtmlView : public KHTMLPart
{
    Q_OBJECT
  public:
    HtmlView( QWidget *parent );

    void clearView();

    void setupActions( KActionCollection * );

    QString title() const { return mTitle; }

    void setInternalUrl( const QString & );
    QString internalUrl() const;

    void showWelcomePage();

    void setStyleSheet();

  public slots:
    void setTitle( const QString & );
    void displayContent( const QString& );

    void zoomIn();
    void zoomOut();

  protected:
    virtual void writeTopFrame();
    virtual void writeBottomFrame();
    virtual void writeContent( const QString& );

    void updateZoomActions();
    virtual bool loadCss();
  private:
    MetaFile *mCssFile;
    QString mCss;

    QString mTitle;
    QString mInternalUrl;

    KAction *mZoomInAction;
    KAction *mZoomOutAction;

    int mZoomStep;
};

#endif
