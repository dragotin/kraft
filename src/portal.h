/***************************************************************************
                     portal.h  - Portal view for Kraft
                             -------------------
    begin                : Mar 2006
    copyright            : (C) 2006 by Klaas Freitag
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

#ifndef PORTAL_H
#define PORTAL_H


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

#include "kraftdoc.h"

class KraftView;
class PortalView;
class ReportGenerator;

/**
  */
class Portal : public KMainWindow
{
  Q_OBJECT

  friend class KraftView;

  public:
    /** construtor of Portal, calls all init functions to create the application.
     */
    Portal( QWidget* parent=0, const char* name=0);
    ~Portal();

    static QString textWrap( const QString& t, unsigned int width=40);

    QWidget* mainWidget();
  protected:
    /** save general Options like all bar positions and status as well as the geometry and the recent file list to the configuration
     * file
     */
    void saveOptions();
    /** read general Options again and initialize all variables like the recent file list
     */
    void readOptions();
    /** initializes the KActions of the application */
    void initActions();
    /** sets up the statusbar for the main window by initialzing a statuslabel.
     */
    void initStatusBar();
    /** creates the centerwidget of the KTMainWindow instance and sets it as the view
     */
    void initView();
    /** queryClose is called by KTMainWindow on each closeEvent of a window. Against the
     * default implementation (only returns true), this calles saveModified() on the document object to ask if the document shall
     * be saved if Modified; on cancel the closeEvent is rejected.
     * @see KTMainWindow#queryClose
     * @see KTMainWindow#closeEvent
     */
    virtual bool queryClose();
    /** queryExit is called by KTMainWindow when the last window of the application is going to be closed during the closeEvent().
     * Against the default implementation that just returns true, this calls saveOptions() to save the settings of the last window's
     * properties.
     * @see KTMainWindow#queryExit
     * @see KTMainWindow#closeEvent
     */
    virtual bool queryExit();
    /** saves the window properties for each open window during session end to the session config file, including saving the currently
     * opened file by a temporary filename provided by KApplication.
     * @see KTMainWindow#saveProperties
     */
    virtual void saveProperties(KConfig *_cfg);
    /** reads the session config file and restores the application's state including the last opened files and documents by reading the
     * temporary files saved by saveProperties()
     * @see KTMainWindow#readProperties
     */
    virtual void readProperties(KConfig *_cfg);


  protected slots:
    void slotStartupChecks();
    void slotOfferNewPosition( const DocPosition& pos );

  public slots:
    /** closes all open windows by calling close() on each memberList item until the list is empty, then quits the application.
     * If queryClose() returns false because the user canceled the saveModified() dialog, the closing breaks.
     */
    void slotFileQuit();
    /** put the marked text/object into the clipboard and remove
     *	it from the document
     */
    void slotEditCut();
    /** put the marked text/object into the clipboard
     */
    void slotEditCopy();
    /** paste the clipboard into the document
     */
    void slotEditPaste();
    /** toggles the statusbar
     */
    void slotViewStatusBar();
    /** changes the statusbar contents for the standard label permanently, used to indicate current actions.
     * @param text the text that is displayed in the statusbar
     */
    void slotStatusMsg(const QString &text);
  /** Show the  window with floskeltemplates */
  void slotShowTemplates();

    void slotOpenKatalog(const QString& );
    void slotOpenKatalog();
    void slotOpenMaterialKatalog();
    void slotKatalogToXML(const QString&);
    void preferences();
    void slotNewDocument();
    void slotOpenDocument( const QString& );
    void slotOpenDocument();
    void slotDocumentSelected( const QString& );
    void slotPrintDocument();

  private:
    void createView( DocGuardedPtr );

    /** the configuration object of the application */
    KConfig *config;

    PortalView *m_portalView;

    /** view is the main widget which represents your working area. The View
     * class should handle all events of the view widget.  It is kept empty so
     * you can create your view according to your application's needs by
     * changing the view class.
     */
    KraftView *view;
    /** doc represents your actual document and is created only once. It keeps
     * information such as filename and does the serialization of your files.
     */
    KraftDoc *doc;
    ReportGenerator *mReportGenerator;

    // KAction pointers to enable/disable actions
    KAction* fileQuit;
    KAction* editCut;
    KAction* editCopy;
    KAction* editPaste;

    KAction* actOpenKatalog;
    KAction* actOpenMatKatalog;
    KAction* actNewDocument;
    KAction* actOpenDocument;
    KAction* actPrintDocument;

    KToggleAction* viewFlosTemplates;
    KToggleAction* viewStatusBar;

};

#endif

