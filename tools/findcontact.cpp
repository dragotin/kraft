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
#include <QStringList>
#include <QUrl>
#include <QFile>
#include <QObject>
#include <QDebug>
#include <QCoreApplication>

#include "addressprovider.h"

#include <kcontacts/vcardconverter.h>


class FindContact : public QObject
{
    Q_OBJECT

signals:
    void quitLoop();

public slots:
    void slotAddresseeFound( const QString&, const KContacts::Addressee& contact )
    {
        dumpContact(contact, _options._outputType);
        emit quitLoop();
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
                _addressProvider.reset( new AddressProvider(this) );

        connect( _addressProvider.data(),
                 SIGNAL(lookupResult(QString,KContacts::Addressee)),
                 this,
                 SLOT(slotAddresseeFound(QString, KContacts::Addressee)));
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
    void search( )
    {
        const QString uid = _options.uid;

        if( uid.isEmpty() ) return;

        AddressProvider::LookupState state = _addressProvider->lookupAddressee(uid);
        if( state == AddressProvider::LookupFromCache ) {
            const KContacts::Addressee addressee = _addressProvider->getAddresseeFromCache(uid);
            // this cant actually happen because the cache can not be prefilled.
            slotAddresseeFound( QString(), addressee );
        } else if( state == AddressProvider::LookupOngoing ) {
        } else if( state == AddressProvider::LookupStarted ) {
            // thats the supposed return type.
        } else if( state == AddressProvider::LookupNotFound ||
                   state == AddressProvider::BackendError   ||
                   state == AddressProvider::ItemError ) {
            // errors
            exit(1);
        }
    }


#define NL (QLatin1Char('\n'));
    // print the output
    void dumpContact( KContacts::Addressee contact, OutputType dt) {
        QString out;

        if( contact.isEmpty() ) {
            return;
        }

        if( dt == VCard ) {
            KContacts::VCardConverter convert;
            QByteArray arr = convert.exportVCard(contact, KContacts::VCardConverter::v3_0);
            out = QString::fromUtf8(arr);
        } else if( dt == Pretty ) {
            out += contact.realName() + NL;
            KContacts::Address address = contact.address(KContacts::Address::Pref);
            if( address.isEmpty() )
                address = contact.address(KContacts::Address::Work );
            if( address.isEmpty() )
                address = contact.address(KContacts::Address::Home );
            if( address.isEmpty() )
                address = contact.address(KContacts::Address::Postal );

            if(address.isEmpty()) {
                // std::cout << "Warn: No address found!";
            } else {
                out += address.street() + NL;
                out += address.locality() + NL;
            }
            out += QLatin1Char('\n');

            foreach( KContacts::PhoneNumber pnum, contact.phoneNumbers() ) {
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
    QScopedPointer<AddressProvider> _addressProvider;
    CmdOptions _options;
};

// main function, not part of the object, program start.
int main(int argc, char **argv) {
    QCoreApplication app(argc, argv);


    FindContact fc;
    fc.parseOptions( app.arguments());
    fc.search();

    QEventLoop loop;
    QObject::connect(&fc, SIGNAL(quitLoop()), &loop, SLOT(quit()), Qt::QueuedConnection);
    loop.exec();

    app.exec();
    return 0;
}

// Needed to pull in the generated moc file for QObject (signals, slots...)
#include "findcontact.moc"
