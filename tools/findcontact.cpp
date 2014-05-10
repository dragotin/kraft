/*
 * Copyright (C) 2014 by Klaas Freitag <kraft@volle-kraft-voraus.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 */

#include <iostream>
#include <qcoreapplication.h>
#include <QStringList>
#include <QUrl>
#include <QFile>
#include <QObject>

#include <kdebug.h>
#include <kdeversion.h>

#include <akonadi/contact/contactsearchjob.h>
#include <akonadi/session.h>
#include <Akonadi/Item>
#include <Akonadi/ItemFetchJob>
#include <Akonadi/ItemFetchScope>

#include <kabc/vcardconverter.h>
#include <kabc/vcard.h>


class FindContact : public QObject
{
    Q_OBJECT

public slots:
    void searchResult(KJob* job ) {
        if( !job ) {
            return;
        }

        Akonadi::ContactSearchJob *searchJob = qobject_cast<Akonadi::ContactSearchJob*>( job );

        if( searchJob->error() ) {
          kDebug() << "Address search job failed: " << job->errorString();
        }

        const KABC::Addressee::List contacts = searchJob->contacts();
        // we set a limit to 1, so there is only one result.
        if( contacts.size() > 0 ) {
            KABC::Addressee contact;
            contact = contacts.at(0);
            dumpContact(contact, _options._outputType);
        }

        job->deleteLater();
        exit(1);
    }

    void gidJobFinished( KJob *job ) {
        if (job->error()) {
            qDebug() << "gid job error: " << job->errorString();
            exit(1);
        }

        Akonadi::ItemFetchJob *fetchJob = qobject_cast<Akonadi::ItemFetchJob*>(job);

        const Akonadi::Item::List items = fetchJob->items();
        foreach( Akonadi::Item item, items ) {
            if( item.hasPayload<KABC::Addressee>() ) {
                dumpContact( item.payload<KABC::Addressee>(), _options._outputType );
            }
        }
        exit(0);
    }

public:
    typedef enum {
        VCard,
        Pretty,
        Template
    } OutputType;

    struct CmdOptions {
        QString uid;
        QString outputfile;
        OutputType _outputType;
        QString outTemplate;
    };

    // Constructor, called to initialize object
    FindContact() : QObject() {
        using namespace Akonadi;
    }

    void help()
    {
        std::cout << std::endl;
        std::cout << " findcontact - search for contact data." << std::endl;
        std::cout << " Usage: findcontact [-o filename] uid" << std::endl;
        std::cout << std::endl;
        std::cout << "  -o <filename>: dump output to filename" << std::endl;
        std::cout << "  -c: Output format VCard." << std::endl;
        std::cout << "  -t <template>: Output format defined by template" << std::endl;
        std::cout << "                 Not implemented yet." << std::endl;
        std::cout << std::endl;
        exit(1);

    }

    // method to parse the options coming from command line
    void parseOptions( const QStringList& app_args )
    {
        QStringList args(app_args);

        if( args.count() < 2 ) {
            help();
        }

        // fetch the last command line option, it's the UID to query for.
        // but only if it does not start with a "-"
        if( !args.last().startsWith("-")) {
            _options.uid = args.takeLast();
        }
        _options._outputType = Pretty;

        QStringListIterator it(args);
        // skip file name;
        if (it.hasNext()) it.next();

        while(it.hasNext()) {
            const QString option = it.next();

            if( option == "-o" && !it.peekNext().startsWith("-") ) {
                _options.outputfile = it.next();
            } else if( option == "-c" ) {
                _options._outputType = VCard;
            } else if( option == "-t" && !it.peekNext().startsWith("-") ) {
                _options.outTemplate = it.next();
                std::cout << "Not yet implemented!" << std::endl;
            } else {
                help();
            }
        }
    }

    // method to start the search job. It is asynchronous and ends up in the
    // slot searchResult()
    void akonadiSearch( )
    {
#if KDE_IS_VERSION(4,12,0)
        akonadiSearchGID();
#else
        QString uid = _options.uid;

        if( uid.isEmpty() ) return;

        _job = new Akonadi::ContactSearchJob( );
        _job->setLimit( 1 );
        _job->setQuery( Akonadi::ContactSearchJob::ContactUid , uid);

        connect( _job, SIGNAL( result( KJob* ) ), this, SLOT( searchResult( KJob* ) ) );

        _job->start();
#endif
    }

#if KDE_IS_VERSION(4,12,0)
    void akonadiSearchGID() {
        QString gid = _options.uid;

        if( gid.isEmpty() ) return;

        Akonadi::Item item;
        item.setGid( gid );

        Akonadi::ItemFetchJob *fetchJob = new Akonadi::ItemFetchJob(item, this);

        connect( fetchJob, SIGNAL(result(KJob*)), SLOT(gidJobFinished(KJob*)) );
        fetchJob->fetchScope().fetchFullPayload();
        fetchJob->start();

    }
#endif


#define NL (QLatin1Char('\n'));
    // print the output
    void dumpContact( KABC::Addressee contact, OutputType dt) {
        QString out;

        if( dt == VCard ) {
            KABC::VCardConverter convert;
            QByteArray arr = convert.exportVCard(contact, KABC::VCardConverter::v3_0);
            out = QString::fromUtf8(arr);
        } else if( dt == Pretty ) {
            out += contact.realName() + NL;
            KABC::Address address = contact.address(KABC::Address::Pref);
            if( address.isEmpty() )
                address = contact.address(KABC::Address::Work );
            if( address.isEmpty() )
                address = contact.address(KABC::Address::Home );
            if( address.isEmpty() )
                address = contact.address(KABC::Address::Postal );

            if(address.isEmpty()) {
                // std::cout << "Warn: No address found!";
            } else {
                out += address.street() + NL;
                out += address.locality() + NL;
            }
            out += QLatin1Char('\n');

            foreach( KABC::PhoneNumber pnum, contact.phoneNumbers() ) {
                out += QString( "Phone %1: %2").arg(pnum.typeLabel()).arg(pnum.number()) + NL;
            }

            foreach( QString mail, contact.emails() ) {
                out += QString( "Mail: %1" ).arg(mail) + NL;
            }

            out += QString("UID: %1").arg(contact.uid()) +NL;
            out += NL;
        }

        if( !out.isEmpty() ) {
            if( _options.outputfile.isEmpty() ) {
                std::cout << out.toUtf8().data();
            } else {
                QFile file(_options.outputfile);
                if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                    qDebug() << "Failed to open " << _options.outputfile;
                    return;
                }

                QTextStream outFile(&file);
                outFile << out;
            }
        }

    }

private:
    Akonadi::ContactSearchJob *_job;
    CmdOptions _options;
};

// main function, not part of the object, program start.
int main(int argc, char **argv) {
    QCoreApplication app(argc, argv);


    FindContact fc;
    fc.parseOptions( app.arguments());
    fc.akonadiSearch();
    app.exec();
    return 0;
}

// Needed to pull in the generated moc file for QObject (signals, slots...)
#include "findcontact.moc"
