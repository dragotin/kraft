/***************************************************************************
                          main.cpp  -
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
#include <QPixmap>
#include <QBitmap>
#include <QImage>
#include <QPalette>
#include <QMetaType>
#include <QDebug>
#include <QApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QStandardPaths>
#include <QSplashScreen>
#include <QScopedPointer>

#include <KLocalizedString>

#include "version.h"
#include "portal.h"
#include "archdocposition.h"

int main(int argc, char *argv[])
{
    Q_INIT_RESOURCE(kraft);
    const QByteArray domain {"kraft"};

    qRegisterMetaType<ArchDocPositionList>("ArchDocPositionList");

    QApplication app(argc, argv);
    app.setWindowIcon(QIcon(":/kraft/custom-icons/kraft-simple.svg"));
    app.setApplicationName("kraft");
    app.setApplicationDisplayName("Kraft");
    app.setApplicationVersion(QString("version %1").arg(Kraft::Version::number()));

    const QString path = QCoreApplication::applicationDirPath()+ QStringLiteral("/../share/locale");
    qDebug() << "Setting additional Locale path:" << path;
    KLocalizedString::setApplicationDomain(domain.data());
    KLocalizedString::addDomainLocaleDir(domain, path);

    QCommandLineParser parser;
    parser.addVersionOption();
    parser.addHelpOption();
    parser.addOption(QCommandLineOption(QStringList() << QLatin1String("d"), i18n("Open document with arch doc number <number>"), QLatin1String("number")));
    parser.addOption(QCommandLineOption(QStringList() << QLatin1String("r"), i18n("Open Kraft in read only mode - document changes prohibited") ));

    parser.process(app);

    // Register the supported options
    QScopedPointer<Portal> kraftPortal;
    kraftPortal.reset( new Portal( nullptr, &parser, "kraft main window" ));
    kraftPortal->show();

    return app.exec();
}
