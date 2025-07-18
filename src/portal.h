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
#include <QMap>
#include <QCommandLineParser>
#include <QMainWindow>
#include <QUrl>

#include "docguardedptr.h"
#include "katalogview.h"
#include "portalview.h"
#include "reportgenerator.h"
#include "myidentity.h"

class KraftViewBase;
class AddressProvider;
class PrefsDialog;

/**
  */
class Portal : public QMainWindow
{
  Q_OBJECT

  friend class KraftView;

  public:
    /** construtor of Portal, calls all init functions to create the application.
     */
    Portal( QWidget* parent = 0, QCommandLineParser *commandLineParser = 0, const char* name = 0);

    static QString textWrap(const QString& t, int width=40, int maxLines = -1 );

    QWidget* mainWidget();

    void slotStartupChecks();

  protected:
    /** initializes the QActions of the application */
    void initActions();
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

  protected Q_SLOTS:
    void slotXRechnungCurrentDocument();

    void slotDocConverted(ReportFormat format, const QString& file,
                          const KContacts::Addressee& customerContact);
    void slotDocConvertionFail(const QString &uuid, const QString& failString, const QString &details);
    void openInMailer(const QString& fileName, const KContacts::Addressee& contact);

    QString slotConvertToXML();

  public Q_SLOTS:

    void show();

    /** closes all open windows, then quits the application.
     */
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
    /** changes the statusbar contents for the standard label permanently, used to indicate current actions.
     * @param text the text that is displayed in the statusbar
     */
    void slotStatusMsg(const QString &text = QString());
    /** Show the  window with floskeltemplates */
    void slotShowTemplates();

    void slotOpenKatalog(const QString& );
    void slotKatalogToXML(const QString&);
    void preferences();
    void slotNewDocument();
    void slotCopyCurrentDocument();
    void slotCopyDocument(const QString& uuid);
    void slotOpenCurrentDocument();
    void slotOpenDocument(const QString& ident);
    void slotDoubleClicked();

    void slotViewCurrentDocument();
    void slotViewDocument( const QString& );
    void slotChangeDocStatus();
    void slotFinalizeDoc();
    void slotOpenCurrentPDF();
    void slotGenerateCurrentPDF();

    void slotFollowUpDocument();
    void slotDocumentSelected( const QString& );
    void slotPrintCurrentPDF();
    void slotPrintPDF(const QString &uuid);
    void slotGeneratePDF(const QString& uuid);

    void slotViewClosed(bool, DocGuardedPtr , bool modified);
    void slotEditTagTemplates();
    void slotReconfigureDatabase();
    void slotAboutQt();
    void slotAboutKraft();
    void slotHandbook();

    void busyCursor( bool );
    void slotMailDocument();
    void slotOpenPDF(const QString& uuid);

    void slotReceivedMyAddress( const QString&, const KContacts::Addressee& );

  private:
    void createView( DocGuardedPtr );
    void createROView( DocGuardedPtr );

    QScopedPointer<PortalView> m_portalView;

    QString _currentSelectedUuid, _toMailFile;

    // QAction pointers to enable/disable actions
    QAction* _actFileQuit;
    QAction* _actEditCut;
    QAction* _actEditCopy;
    QAction* _actEditPaste;
    QAction* _actAboutQt;
    QAction* _actAboutKraft;
    QAction* _actHandbook;
    QAction* _actPreferences;
    QAction* _actReconfDb;
    QAction* _actXmlConvert;

    QAction* _actNewDocument;
    QAction* _actCopyDocument;
    QAction* _actEditDocument;
    QAction* _actViewDocument;
    QAction* _actFollowDocument;
    QAction* _actXRechnung;
    QAction* _actEditTemplates;

    QAction* _actFinalizeDocument;
    QAction* _actChangeDocStatus;
    QAction* _actOpenDocumentPDF;
    QAction* _actGeneratePDF;
    QAction* _actPrintPDF;
    QAction* _actMailPDF;

    QCommandLineParser *mCmdLineArgs;

    QMap<QString, KatalogView*> mKatalogViews;
    QMap<KraftDoc*, KraftViewBase*> mViewMap;

    AddressProvider *mAddressProvider;
    PrefsDialog *_prefsDialog;

    ReportGenerator _reportGenerator;
    MyIdentity _myIdentity;
    bool _readOnlyMode;
};

#endif

