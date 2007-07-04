/***************************************************************************
                     portal.h  - Portal view for Kraft
                             -------------------
    begin                : Mar 2006
    copyright            : (C) 2006 by Klaas Freitag
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

#ifndef PORTAL_H
#define PORTAL_H


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// include files for Qt
#include <qmap.h>

// include files for KDE
#include <kapp.h>
#include <kmainwindow.h>
#include <kaccel.h>
#include <kaction.h>
#include <kurl.h>

#include "kraftdoc.h"
#include "katalogview.h"

class KraftView;
class PortalView;
class ReportGenerator;
class KCmdLineArgs;
/**
  */
class Portal : public KMainWindow
{
  Q_OBJECT

  friend class KraftView;

  public:
    /** construtor of Portal, calls all init functions to create the application.
     */
    Portal( QWidget* parent = 0, KCmdLineArgs *args = 0, const char* name = 0);
    ~Portal();

    static QString textWrap( const QString& t, unsigned int width=40);

    QWidget* mainWidget();
  protected:
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


  protected slots:
    void slotStartupChecks();
    void slotOpenArchivedDoc( const dbID& );
    void slotMailDocument();
    void slotMailDocument( const QString& );

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
    void slotKatalogToXML(const QString&);
    void preferences();
    void slotNewDocument();
    void slotOpenDocument( const QString& );
    void slotOpenDocument();
    void slotDocumentSelected( const QString& );
    void slotArchivedDocExecuted();
    void slotArchivedDocSelected( const dbID& );
    void slotPrintDocument();
    void slotPrintDocument( const QString&, const dbID& );
    void slotViewClosed( bool );

    void busyCursor( bool );

    void slotOpenPdf( const QString& );

  private:
    void createView( DocGuardedPtr );

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

    KAction* actNewDocument;
    KAction* actOpenDocument;
    KAction* actPrintDocument;
    KAction* actMailDocument;

    KAction* actOpenArchivedDocument;

    KToggleAction* viewFlosTemplates;
    KToggleAction* viewStatusBar;
    KCmdLineArgs *mCmdLineArgs;

  QMap<QString, KatalogView*> mKatalogViews;
  QString mMailReceiver;
};

#endif

