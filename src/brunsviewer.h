/***************************************************************************
                          kange.h  -
                             -------------------
    begin                : Mit Dez 31 19:24:05 CET 2003
    copyright            : (C) 2003 by Klaas Freitag
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

#ifndef BRUNSVIEWER_H
#define BRUNSVIEWER_H


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// include files for Qt

// include files for KDE
#include <kapp.h>
#include <kmainwindow.h>
#include <kaccel.h>
#include <kaction.h>
#include <kurl.h>


class Brunsviewer : public KMainWindow
{
  Q_OBJECT

  public:
    /** construtor of Brunsviewer, calls all init functions to create the application.
     */
    Brunsviewer(QWidget* parent=0, const char* name=0);
    ~Brunsviewer();
    /** opens a file specified by commandline option
     */
    static QString textWrap( const QString& t, unsigned int width=40);

  protected:
    // void initActions();
    /** sets up the statusbar for the main window by initialzing a statuslabel.
     */
    void initStatusBar();
    /** initializes the document object of the main window that is connected to the view in initView().
     * @see initView();
     */
    void initView();
    /** queryClose is called by KTMainWindow on each closeEvent of a window. Against the
     * default implementation (only returns true), this calles saveModified() on the document object to ask if the document shall
     * be saved if Modified; on cancel the closeEvent is rejected.
     * @see KTMainWindow#queryClose
     * @see KTMainWindow#closeEvent
     */
  protected slots:
    void slotStartupChecks();
      
  public slots:
      void slotViewToolBar() {};
    /** toggles the statusbar
     */
    void slotViewStatusBar() {};
    /** changes the statusbar contents for the standard label permanently, used to indicate current actions.
     * @param text the text that is displayed in the statusbar
     */
    void slotStatusMsg(const QString &text);
    // void slotKatalogToXML(const QString&);
  private:
};

#endif // KANGE_H

