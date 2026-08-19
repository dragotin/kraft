/***************************************************************************
              htmlwindow.cpp  - non modal window showing a html page
                             -------------------
    begin                : Jul 2026
    copyright            : (C) 2026 Klaas Freitag <kraft@freisturz.de>
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "htmlwindow.h"
#include "htmlview.h"
#include "kraftsettings.h"

#include <QDesktopServices>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QTextEdit>
#include <QUrl>
#include <QVBoxLayout>

#include <klocalizedstring.h>

HtmlWindow::HtmlWindow(const QString& title, const QString &name, QWidget *parent)
    : QWidget(parent, Qt::Window)
{
    setObjectName(name.isEmpty() ? QStringLiteral("HTML_WINDOW") : name);
    // Non modal, self destructing top level window.
    setWindowModality(Qt::NonModal);
    setAttribute(Qt::WA_DeleteOnClose);

    QVBoxLayout *vbox = new QVBoxLayout(this);
    _htmlView = new HtmlView(this);
    vbox->addWidget(_htmlView, 1);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Close, this);
    QPushButton *closeButton = buttonBox->button(QDialogButtonBox::Close);
    closeButton->setDefault(true);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &HtmlWindow::close);
    vbox->addWidget(buttonBox);

    const QByteArray geo = QByteArray::fromBase64(KraftSettings::self()->templVarWinGeometry().toLatin1());
    restoreGeometry(geo);

    setWindowTitle(title);
    connect(_htmlView, &HtmlView::openUrl, this, [](const QUrl &url) {
        QDesktopServices::openUrl(url);
    });
}

void HtmlWindow::closeEvent(QCloseEvent *ev)
{
    Q_EMIT closing();
    QWidget::closeEvent(ev);
}

void HtmlWindow::setHtml(const QString &content)
{
    _htmlView->displayContent(content);
}

void HtmlWindow::setStylesheetFile(const QString &styleFile)
{
    _htmlView->setStylesheetFile(styleFile);
}
