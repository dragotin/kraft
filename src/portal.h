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

// include files for Qt
#include <qmap.h>

// include files for KDE
#include <ktoggleaction.h>

#include <kapplication.h>
#include <kxmlguiwindow.h>
#include <kabc/addressee.h>

#include <kaction.h>
#include <kurl.h>

#include "docguardedptr.h"
#include "katalogview.h"
#include "dbids.h"

class KraftViewBase;
class PortalView;
class ReportGenerator;
class KCmdLineArgs;
class ArchDocDigest;
class AddressProvider;
class PrefsDialog;

/**
  */
class Portal : public KXmlGuiWindow
{
  Q_OBJECT

  friend class KraftView;

  public:
    /** construtor of Portal, calls all init functions to create the application.
     */
    Portal( QWidget* parent = 0, KCmdLineArgs *args = 0, const char* name = 0);

    static QString textWrap( const QString& t, int width=40);

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
    void slotOpenArchivedDoc( const ArchDocDigest& );
    void slotMailDocument();
    void slotPrefsDialogFinished( int );

  public slots:
    /** closes all open windows by calling close() on each memberList item until the list is empty, then quits the application.
     * If queryClose() returns false because the user canceled the saveModified() dialog, the closing breaks.
     */
    void slotFileQuit();
    void closeEvent( QCloseEvent * event );
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
    void slotCopyDocument();
    void slotCopyDocument( const QString& );
    void slotOpenDocument( const QString& );
    void slotOpenDocument();

    void slotViewDocument();
    void slotViewDocument( const QString& );

    void slotFollowUpDocument();
    void slotDocumentSelected( const QString& );
    void slotArchivedDocExecuted();
    void slotArchivedDocSelected( const ArchDocDigest& );
    void slotPrintDocument();
    void slotPrintDocument( const QString&, const dbID& );
    void slotViewClosed( bool, DocGuardedPtr );
    void slotEditTagTemplates();
    void slotReconfigureDatabase();

    void busyCursor( bool );

    void slotOpenPdf( const QString& );

    void slotReceivedMyAddress( const QString&, const KABC::Addressee& );

    void slotMailPdfAvailable( const QString& fileName );
    void slotMailAddresseeFound( const QString&, const KABC::Addressee& );

  private:
    void createView( DocGuardedPtr );
    void createROView( DocGuardedPtr );

    PortalView *m_portalView;

    // KAction pointers to enable/disable actions
    KAction* fileQuit;
    KAction* editCut;
    KAction* editCopy;
    KAction* editPaste;

    KAction* actNewDocument;
    KAction* actCopyDocument;
    KAction* actOpenDocument;
    KAction* actViewDocument;
    KAction* actFollowDocument;
    KAction* actPrintDocument;
    KAction* actMailDocument;
    KAction* actEditTemplates;

    KAction* actOpenArchivedDocument;

    KToggleAction* viewFlosTemplates;
    KToggleAction* viewStatusBar;
    KCmdLineArgs *mCmdLineArgs;

    QMap<QString, KatalogView*> mKatalogViews;
    QMap<KraftDoc*, KraftViewBase*> mViewMap;

    QString _clientId;
    QString _pdfFileName;
    AddressProvider *mAddressProvider;
    KABC::Addressee myContact;
    PrefsDialog *_prefsDialog;
    DocGuardedPtr _currentDoc;
};

#endif

