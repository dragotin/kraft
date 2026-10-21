/*
 * Copyright (C) 2026 by Klaas Freitag <kraft@volle-kraft-voraus.de>
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

// Generates Kraft demo documents for a small PV installation company,
// in German or English.
//
// Usage: kraftdemodata [--lang de|en] <outdir>
// Creates a Kraft v2 base dir below <outdir> and saves the documents into it
// with DocumentSaverXML, including the document index.

#include <QCoreApplication>
#include <QDate>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QStringList>
#include <QUuid>

#include "defaultprovider.h"
#include "documentsaverxml.h"
#include "kraftattrib.h"
#include "kraftdoc.h"
#include "xmlindexdb.h"

#include <cstdio>

using namespace Qt::StringLiterals;

namespace
{

bool En = false;

QString T(const char *de, const char *en)
{
    return QString::fromUtf8(En ? en : de);
}

// Money and amounts are integers in hundredths to compute derived prices exactly.
// Rounds half away from zero, like Decimal ROUND_HALF_UP.
qint64 divRound(qint64 n, qint64 d)
{
    return (n + (n < 0 ? -d / 2 : d / 2)) / d;
}

QString fmt(qint64 h)
{
    return QString::asprintf("%s%lld.%02lld", h < 0 ? "-" : "", qAbs(h) / 100, qAbs(h) % 100);
}

QString uid(const QString &name)
{
    // stable uuids, so that rerunning the tool yields the same files
    static const QUuid ns(QStringLiteral("6f1c2a4e-3b7d-4c55-9a51-2f0a6e1d8b11"));
    return QUuid::createUuidV5(ns, En ? u"en-"_s + name : name).toString(QUuid::WithoutBraces);
}

struct Item {
    QString type, text;
    qint64 amount; // hundredths
    QString unit;
    qint64 price; // cents
    QString tax;
    QStringList tags;
    QList<KraftAttrib> attribs;
};

using Items = QList<Item>;
using Group = std::pair<QString, Items>;

qint64 itemPrice(const Item &i)
{
    return divRound(i.amount * i.price, 100);
}

Item item(const QString &text,
          double amount,
          const QString &unit,
          double price,
          const QString &tax = u"None"_s,
          const QString &typ = u"Normal"_s,
          const QStringList &tags = {})
{
    return Item{typ, text, qRound64(amount * 100), unit, qRound64(price * 100), tax, tags, {}};
}

Item textItem(const QString &text)
{
    return item(text, 0, QString(), 0, u"None"_s, u"Text"_s);
}

Item discount(const QString &text, int percent, const Items &base)
{
    qint64 sum = 0;
    for (const auto &i : base)
        if (i.type == u"Normal")
            sum += itemPrice(i);
    Item it = item(text, 1, T("pausch.", "flat"), 0, u"None"_s, u"ExtraDiscount"_s);
    it.price = divRound(sum * percent, 100);
    it.attribs.append(KraftAttrib(DocPosition::Discount, double(percent), KraftAttrib::Type::Float));
    return it;
}

QString identFor(const QDate &date, int counter)
{
    int y;
    const int w = date.weekNumber(&y);
    return QString::asprintf("%d%02d-%d", y, w, counter);
}

QString money(qint64 cents)
{
    QString s = fmt(cents);
    return En ? s : s.replace(u'.', u',');
}

struct Doc {
    QString uuid, docType, desc;
    QDate date;
    int counter = 0; // 0: draft without ident
    QString client, clientKey, predecessor, project;
    std::pair<QString, QString> supply;
    QString salut, pre;
    QList<KraftAttrib> attribs;
    QList<Group> groups;
    bool pricesZero = false;
    QString post, goodbye;
};

// ---------------------------------------------------------------- items

QString nullsteuer()
{
    return T(
        "Die Lieferung und Installation der Photovoltaikanlage samt Speicher erfolgt gemäß "
        "§ 12 Abs. 3 UStG zum Nullsteuersatz (Anlage auf bzw. in der Nähe eines Wohngebäudes, "
        "Bruttoleistung nicht mehr als 30 kWp).",
        "Supply and installation of the photovoltaic system including storage are subject to the "
        "zero tax rate according to § 12 (3) UStG (system on or near a residential building, "
        "gross output not more than 30 kWp).");
}

QString pcs()
{
    return T("Stk.", "pcs.");
}
QString flat()
{
    return T("pausch.", "flat");
}
QString hrs()
{
    return T("Std.", "hrs");
}
QStringList mat()
{
    return {u"Material"_s};
}
QStringList work()
{
    return {T("Arbeit", "Work")};
}

Items beckerGenerator()
{
    return {
        item(T("Solarmodul Glas-Glas, 430 Wp, Full Black, monokristallin, 30 Jahre Produktgarantie",
               "Solar module glass-glass, 430 Wp, full black, monocrystalline, 30 years product warranty"),
             22,
             pcs(),
             165.00,
             u"None"_s,
             u"Normal"_s,
             mat()),
        item(T("Unterkonstruktion Schrägdach (Ziegel): Dachhaken Edelstahl, Montageschienen Alu, Modulklemmen, je Modul",
               "Mounting system pitched roof (tiles): stainless steel roof hooks, aluminium rails, module clamps, per module"),
             22,
             pcs(),
             48.00,
             u"None"_s,
             u"Normal"_s,
             mat()),
        item(T("Solarkabel 6 mm², doppelt isoliert, inkl. MC4-Steckverbinder", "Solar cable 6 mm², double insulated, incl. MC4 connectors"),
             60,
             u"m"_s,
             2.80,
             u"None"_s,
             u"Normal"_s,
             mat()),
    };
}

Items beckerStorage()
{
    return {
        item(T("Hybrid-Wechselrichter 10 kW, 3-phasig, 2 MPP-Tracker, notstromfähig", "Hybrid inverter 10 kW, 3-phase, 2 MPP trackers, backup power capable"),
             1,
             pcs(),
             1890.00,
             u"None"_s,
             u"Normal"_s,
             mat()),
        item(T("Batteriespeicher LFP 10,2 kWh nutzbar, modular erweiterbar, 10 Jahre Garantie",
               "Battery storage LFP 10.2 kWh usable, modular expandable, 10 years warranty"),
             1,
             pcs(),
             4650.00,
             u"None"_s,
             u"Normal"_s,
             mat()),
        item(T("Smart Meter / Energiezähler für Eigenverbrauchsoptimierung", "Smart meter / energy meter for self-consumption optimisation"),
             1,
             pcs(),
             240.00,
             u"None"_s,
             u"Normal"_s,
             mat()),
    };
}

Items beckerWork()
{
    return {
        item(T("Gerüststellung inkl. Absturzsicherung, Auf- und Abbau, bis 2 Wochen Standzeit",
               "Scaffolding incl. fall protection, erection and dismantling, up to 2 weeks"),
             1,
             flat(),
             890.00),
        item(T("Montage Unterkonstruktion und Module", "Installation of mounting system and modules"), 16, hrs(), 68.00, u"None"_s, u"Normal"_s, work()),
        item(T("Elektroinstallation DC/AC, Überspannungsschutz Typ 1+2, Anpassung Zählerschrank",
               "Electrical installation DC/AC, surge protection type 1+2, meter cabinet adjustment"),
             1,
             flat(),
             1450.00,
             u"None"_s,
             u"Normal"_s,
             work()),
        item(T("Inbetriebnahme, Funktionsprüfung, Einweisung in App und Notstromfunktion",
               "Commissioning, functional test, introduction to the app and backup power function"),
             1,
             flat(),
             290.00,
             u"None"_s,
             u"Normal"_s,
             work()),
        item(T("Anmeldung beim Netzbetreiber und Eintragung im Marktstammdatenregister",
               "Registration with the grid operator and the market master data register"),
             1,
             flat(),
             180.00),
    };
}

Items beckerWallbox()
{
    return {
        item(T("Wallbox 11 kW, PV-Überschussladen, inkl. Installation und Anmeldung beim Netzbetreiber "
               "(Regelsteuersatz, da nicht Bestandteil der PV-Anlage)",
               "Wallbox 11 kW, PV surplus charging, incl. installation and registration with the grid operator "
               "(standard tax rate, as not part of the PV system)"),
             1,
             pcs(),
             1290.00,
             u"Full"_s),
    };
}

// ---------------------------------------------------------------- documents

QList<Doc> docs()
{
    QList<Doc> out;

    const QString famBecker =
        T("Familie\nJulia und Markus Becker\nAm Oberberg 12\n91126 Schwabach", "Becker family\nJulia and Markus Becker\nAm Oberberg 12\n91126 Schwabach");
    const QString yilmaz = T("Herrn\nDeniz Yilmaz\nWildrosenstraße 4\n90547 Stein", "Mr\nDeniz Yilmaz\nWildrosenstraße 4\n90547 Stein");
    const QString schaefer = T("Frau\nSabine Schäfer\nLindenallee 23\n90513 Zirndorf", "Ms\nSabine Schäfer\nLindenallee 23\n90513 Zirndorf");
    const QString hoffmann = T("Herrn\nPeter Hoffmann\nKirchgasse 7\n91207 Lauf an der Pegnitz", "Mr\nPeter Hoffmann\nKirchgasse 7\n91207 Lauf an der Pegnitz");
    const QString beckerSalut = T("Sehr geehrte Frau Becker, sehr geehrter Herr Becker,", "Dear Mrs Becker, dear Mr Becker,");
    const QString beckerProject = T("PV-Anlage Becker, Schwabach", "PV system Becker, Schwabach");

    // --- Becker: offer -> order confirmation -> progress invoice -> delivery receipt -> final invoice
    const QString aUuid = uid(u"becker-angebot"_s);
    {
        Doc d;
        d.uuid = aUuid;
        d.docType = T("Angebot", "Offer");
        d.desc = T("Becker 9,5 kWp + Speicher, Vor-Ort-Termin 10.06.", "Becker 9.5 kWp + storage, site visit June 10");
        d.date = QDate(2026, 6, 15);
        d.counter = 9001;
        d.client = famBecker;
        d.clientKey = u"becker"_s;
        d.project = beckerProject;
        d.salut = beckerSalut;
        d.pre =
            T("vielen Dank für das freundliche Gespräch bei Ihnen vor Ort am 10.06.2026. Wie besprochen "
              "bieten wir Ihnen eine Photovoltaikanlage mit 9,46 kWp auf der Süd-Dachfläche Ihres "
              "Wohnhauses samt Batteriespeicher an. Nach unserer Simulation erzeugt die Anlage rund "
              "9.300 kWh im Jahr, mit dem Speicher erreichen Sie einen Eigenverbrauchsanteil von etwa 65 %.",
              "thank you for the pleasant meeting at your home on June 10, 2026. As discussed, we are pleased "
              "to offer you a 9.46 kWp photovoltaic system on the south-facing roof of your house including a "
              "battery storage. According to our simulation the system generates about 9,300 kWh per year; "
              "with the storage you reach a self-consumption rate of about 65 %.");
        d.attribs = {KraftAttrib(u"validUntil"_s, QDate(2026, 7, 15), KraftAttrib::Type::Date)};
        Items storage = beckerStorage();
        storage.append(item(
            T("Alternativ: Batteriespeicher LFP 15,3 kWh nutzbar statt 10,2 kWh", "Alternatively: battery storage LFP 15.3 kWh usable instead of 10.2 kWh"),
            1,
            pcs(),
            6590.00,
            u"None"_s,
            u"Alternative"_s,
            mat()));
        d.groups = {
            {T("PV-Generator", "PV generator"), beckerGenerator()},
            {T("Wechselrichter und Speicher", "Inverter and storage"), storage},
            {T("Montage und Inbetriebnahme", "Installation and commissioning"), beckerWork()},
            {T("Optionen", "Options"),
             {item(T("Auf Wunsch: ", "On request: ") + beckerWallbox().first().text, 1, pcs(), 1290.00, u"Full"_s, u"Demand"_s), textItem(nullsteuer())}},
        };
        d.post =
            T("Das Angebot ist gültig bis zum 15.07.2026. Die Montage ist nach Auftragserteilung "
              "innerhalb von ca. sechs Wochen möglich. Zahlungsbedingungen: 30 % Abschlag bei "
              "Auftragserteilung, Rest nach Inbetriebnahme.",
              "This offer is valid until July 15, 2026. Installation is possible within about six weeks "
              "after the order. Terms of payment: 30 % progress payment with the order, the rest after "
              "commissioning.");
        out.append(d);
    }

    const QString abUuid = uid(u"becker-ab"_s);
    const QDate abDate(2026, 6, 29);
    const QList<Group> abGroups = {
        {T("PV-Generator", "PV generator"), beckerGenerator()},
        {T("Wechselrichter und Speicher", "Inverter and storage"), beckerStorage()},
        {T("Montage und Inbetriebnahme", "Installation and commissioning"), beckerWork()},
        {u"Wallbox"_s, beckerWallbox()},
        {T("Hinweis", "Note"), {textItem(nullsteuer())}},
    };
    {
        Doc d;
        d.uuid = abUuid;
        d.docType = T("Auftragsbestätigung", "Acceptance of Order");
        d.desc = T("Wallbox mitbestellt, Montage KW 34", "Wallbox ordered too, installation week 34");
        d.date = abDate;
        d.counter = 9004;
        d.client = famBecker;
        d.clientKey = u"becker"_s;
        d.predecessor = aUuid;
        d.project = beckerProject;
        d.salut = beckerSalut;
        d.pre =
            T("vielen Dank für Ihren Auftrag vom 26.06.2026, den wir hiermit gerne bestätigen. "
              "Zusätzlich zum Angebot haben Sie die Wallbox mitbestellt. Die Montage ist für die "
              "Kalenderwoche 34 (17.–19.08.2026) eingeplant.",
              "thank you for your order of June 26, 2026, which we are pleased to confirm. "
              "In addition to the offer you have ordered the wallbox. Installation is scheduled for "
              "calendar week 34 (August 17–19, 2026).");
        d.attribs = {KraftAttrib(u"orderNumber"_s, T("Auftrag vom 26.06.2026", "Order of June 26, 2026"), KraftAttrib::Type::String)};
        d.groups = abGroups;
        d.post =
            T("Gemäß Angebot stellen wir Ihnen 30 % der Auftragssumme als Abschlag in Rechnung. "
              "Die Schlussrechnung erhalten Sie nach der Inbetriebnahme.",
              "As agreed in the offer, we will invoice 30 % of the order total as a progress payment. "
              "You will receive the final invoice after commissioning.");
        out.append(d);
    }

    qint64 netNone = 0, netFull = 0;
    for (const auto &g : abGroups)
        for (const auto &i : g.second)
            if (i.type == u"Normal")
                (i.tax == u"Full" ? netFull : netNone) += itemPrice(i);
    const qint64 absNone = divRound(netNone * 30, 100);
    const qint64 absFull = divRound(netFull * 30, 100);

    const QDate absDate(2026, 6, 30);
    const QString absIdent = identFor(absDate, 9005);
    {
        Doc d;
        d.uuid = uid(u"becker-abschlag"_s);
        d.docType = T("Abschlagsrechnung", "Progress Payment Invoice");
        d.desc = T("Abschlag 30 %, bezahlt 07.07.", "Progress payment 30 %, paid July 7");
        d.date = absDate;
        d.counter = 9005;
        d.client = famBecker;
        d.clientKey = u"becker"_s;
        d.predecessor = abUuid;
        d.project = beckerProject;
        d.salut = beckerSalut;
        d.pre = T("gemäß unserer Auftragsbestätigung %1 vom 29.06.2026 "
                  "berechnen wir Ihnen den vereinbarten Abschlag in Höhe von 30 % der Auftragssumme.",
                  "according to our order confirmation %1 of June 29, 2026 "
                  "we invoice the agreed progress payment of 30 % of the order total.")
                    .arg(identFor(abDate, 9004));
        d.attribs = {KraftAttrib(u"SentDate"_s, QDate(2026, 6, 30), KraftAttrib::Type::Date)};
        d.groups = {{T("Abschlag", "Progress payment"),
                     {item(T("Abschlag 30 % auf PV-Anlage mit Speicher, Montage und Inbetriebnahme "
                             "(Auftragssumme %1 €, Nullsteuersatz)",
                             "Progress payment 30 % on PV system with storage, installation and commissioning "
                             "(order total € %1, zero tax rate)")
                               .arg(money(netNone)),
                           1,
                           flat(),
                           absNone / 100.0),
                      item(T("Abschlag 30 % auf Wallbox inkl. Installation (Auftragssumme netto %1 €)",
                             "Progress payment 30 % on wallbox incl. installation (order total net € %1)")
                               .arg(money(netFull)),
                           1,
                           flat(),
                           absFull / 100.0,
                           u"Full"_s),
                      textItem(nullsteuer())}}};
        d.post =
            T("Bitte überweisen Sie den Betrag innerhalb von 14 Tagen ohne Abzug auf unser unten "
              "genanntes Konto.",
              "Please transfer the amount within 14 days without deduction to our account stated below.");
        out.append(d);
    }

    {
        Doc d;
        d.uuid = uid(u"becker-lieferschein"_s);
        d.docType = T("Lieferschein", "Delivery Receipt");
        d.desc = T("Material angeliefert, Gerüst steht", "Material delivered, scaffolding is up");
        d.date = QDate(2026, 8, 17);
        d.counter = 9011;
        d.client = famBecker;
        d.clientKey = u"becker"_s;
        d.predecessor = abUuid;
        d.project = beckerProject;
        d.salut = beckerSalut;
        d.pre = T("folgendes Material haben wir heute für Ihre Photovoltaikanlage angeliefert:",
                  "today we have delivered the following material for your photovoltaic system:");
        Items material = beckerGenerator() + beckerStorage();
        material.append(item(T("Wallbox 11 kW, PV-Überschussladen", "Wallbox 11 kW, PV surplus charging"), 1, pcs(), 0, u"Full"_s));
        d.groups = {{u"Material"_s, material}};
        d.pricesZero = true;
        d.post = T("Ware vollständig und unbeschädigt erhalten:\n\n\n______________________________\nDatum, Unterschrift",
                   "Goods received complete and undamaged:\n\n\n______________________________\nDate, signature");
        d.goodbye = T("Mit freundlichen Grüßen\n\nIhr Team von Kraft Photovoltaik", "Kind regards\n\nYour team at Kraft Photovoltaik");
        out.append(d);
    }

    {
        Doc d;
        d.uuid = uid(u"becker-schlussrechnung"_s);
        d.docType = T("Schlussrechnung", "Final Invoice");
        d.desc = T("Noch Abnahmeprotokoll beilegen!", "Attach the acceptance report!");
        d.date = QDate(2026, 8, 21);
        d.client = famBecker;
        d.clientKey = u"becker"_s;
        d.predecessor = abUuid;
        d.project = beckerProject;
        d.supply = {u"2026-08-17T00:00:00"_s, u"2026-08-19T23:59:59"_s};
        d.salut = beckerSalut;
        d.pre =
            T("Ihre Photovoltaikanlage wurde am 19.08.2026 in Betrieb genommen. Wir bedanken uns für "
              "die angenehme Zusammenarbeit und berechnen Ihnen unsere Leistungen wie folgt:",
              "your photovoltaic system was commissioned on August 19, 2026. Thank you for the pleasant "
              "cooperation. We invoice our services as follows:");
        d.groups = abGroups.mid(0, abGroups.size() - 1); // without the note group
        d.groups.append(
            {T("Abzüglich Abschlag", "Less progress payment"),
             {item(T("Abzüglich Abschlagsrechnung %1 vom 30.06.2026 (Nullsteuersatz)", "Less progress payment invoice %1 of June 30, 2026 (zero tax rate)")
                       .arg(absIdent),
                   1,
                   QString(),
                   -absNone / 100.0),
              item(T("Abzüglich Abschlagsrechnung %1 vom 30.06.2026 (Wallbox)", "Less progress payment invoice %1 of June 30, 2026 (wallbox)").arg(absIdent),
                   1,
                   QString(),
                   -absFull / 100.0,
                   u"Full"_s),
              textItem(nullsteuer())}});
        d.post =
            T("Bitte überweisen Sie den Restbetrag innerhalb von 14 Tagen ohne Abzug. Das Abnahme- "
              "und Inbetriebnahmeprotokoll sowie die Herstellerunterlagen liegen dieser Rechnung bei.",
              "Please transfer the remaining amount within 14 days without deduction. The acceptance and "
              "commissioning report as well as the manufacturer documents are enclosed with this invoice.");
        out.append(d);
    }

    // --- Yilmaz: small offer, draft
    {
        Doc d;
        d.uuid = uid(u"yilmaz-angebot"_s);
        d.docType = T("Angebot", "Offer");
        d.desc = T("Biberschwanz! Statik prüfen lassen", "Plain tiles! Have the statics checked");
        d.date = QDate(2026, 9, 28);
        d.client = yilmaz;
        d.clientKey = u"yilmaz"_s;
        d.project = T("PV-Anlage Yilmaz, Stein", "PV system Yilmaz, Stein");
        d.salut = T("Sehr geehrter Herr Yilmaz,", "Dear Mr Yilmaz,");
        d.pre =
            T("vielen Dank für Ihre Anfrage. Für Ihr Einfamilienhaus mit Ost-West-Dach bieten wir Ihnen "
              "eine Photovoltaikanlage mit 6,45 kWp an. Der Wechselrichter ist bereits für die spätere "
              "Nachrüstung eines Speichers vorbereitet.",
              "thank you for your inquiry. For your detached house with east-west roof we offer you "
              "a 6.45 kWp photovoltaic system. The inverter is already prepared for retrofitting "
              "a storage later.");
        d.attribs = {KraftAttrib(u"validUntil"_s, QDate(2026, 10, 31), KraftAttrib::Type::Date)};
        const Items gen = {
            item(T("Solarmodul Glas-Glas, 430 Wp, Full Black, monokristallin", "Solar module glass-glass, 430 Wp, full black, monocrystalline"),
                 15,
                 pcs(),
                 165.00,
                 u"None"_s,
                 u"Normal"_s,
                 mat()),
            item(T("Unterkonstruktion Schrägdach (Biberschwanz), je Modul", "Mounting system pitched roof (plain tiles), per module"),
                 15,
                 pcs(),
                 56.00,
                 u"None"_s,
                 u"Normal"_s,
                 mat()),
            item(T("Wechselrichter 6 kW, 3-phasig, speicherfähig", "Inverter 6 kW, 3-phase, storage ready"), 1, pcs(), 1240.00, u"None"_s, u"Normal"_s, mat()),
            item(T("Solarkabel 6 mm² inkl. MC4-Steckverbinder", "Solar cable 6 mm² incl. MC4 connectors"), 40, u"m"_s, 2.80, u"None"_s, u"Normal"_s, mat()),
        };
        const Items wrk = {
            item(T("Gerüststellung inkl. Absturzsicherung", "Scaffolding incl. fall protection"), 1, flat(), 790.00),
            item(T("Montage Unterkonstruktion und Module", "Installation of mounting system and modules"), 12, hrs(), 68.00, u"None"_s, u"Normal"_s, work()),
            item(T("Elektroinstallation DC/AC inkl. Überspannungsschutz", "Electrical installation DC/AC incl. surge protection"),
                 1,
                 flat(),
                 1150.00,
                 u"None"_s,
                 u"Normal"_s,
                 work()),
            item(T("Inbetriebnahme, Netzanmeldung, Marktstammdatenregister", "Commissioning, grid registration, market master data register"),
                 1,
                 flat(),
                 390.00,
                 u"None"_s,
                 u"Normal"_s,
                 work()),
        };
        d.groups = {
            {T("PV-Anlage 6,45 kWp", "PV system 6.45 kWp"), gen},
            {T("Montage und Inbetriebnahme", "Installation and commissioning"), wrk},
            {T("Optionen", "Options"),
             {item(T("Alternativ: Batteriespeicher LFP 5,1 kWh, gleich mitinstalliert", "Alternatively: battery storage LFP 5.1 kWh, installed right away"),
                   1,
                   pcs(),
                   2890.00,
                   u"None"_s,
                   u"Alternative"_s),
              item(T("Auf Wunsch: Vogelschutzgitter rund um das Modulfeld", "On request: bird protection mesh around the module field"),
                   24,
                   u"m"_s,
                   18.50,
                   u"None"_s,
                   u"Demand"_s),
              textItem(nullsteuer())}},
        };
        d.post =
            T("Gerne erläutern wir Ihnen das Angebot in einem persönlichen Gespräch. Das Angebot ist "
              "gültig bis zum 31.10.2026.",
              "We are happy to explain the offer in a personal meeting. The offer is valid until October 31, 2026.");
        out.append(d);
    }

    // --- Schaefer: storage retrofit with autumn discount
    {
        Doc d;
        d.uuid = uid(u"schaefer-angebot"_s);
        d.docType = T("Angebot", "Offer");
        d.desc = T("Bestandsanlage 7,2 kWp von 2019", "Existing system 7.2 kWp from 2019");
        d.date = QDate(2026, 10, 6);
        d.counter = 9017;
        d.client = schaefer;
        d.clientKey = u"schaefer"_s;
        d.project = T("Speichernachrüstung Schäfer, Zirndorf", "Storage retrofit Schäfer, Zirndorf");
        d.salut = T("Sehr geehrte Frau Schäfer,", "Dear Ms Schäfer,");
        d.pre =
            T("wie telefonisch besprochen erhalten Sie unser Angebot für die Nachrüstung eines "
              "Batteriespeichers an Ihrer bestehenden Photovoltaikanlage (7,2 kWp, Baujahr 2019). "
              "Damit steigern Sie Ihren Eigenverbrauch von heute ca. 30 % auf rund 60 %.",
              "as discussed on the phone, please find our offer for retrofitting a battery storage to "
              "your existing photovoltaic system (7.2 kWp, built 2019). This increases your "
              "self-consumption from about 30 % today to around 60 %.");
        d.attribs = {KraftAttrib(u"validUntil"_s, QDate(2026, 11, 6), KraftAttrib::Type::Date)};
        Items sp = {
            item(T("Batteriespeicher LFP 7,7 kWh nutzbar, AC-gekoppelt, inkl. Batteriewechselrichter",
                   "Battery storage LFP 7.7 kWh usable, AC coupled, incl. battery inverter"),
                 1,
                 pcs(),
                 4380.00,
                 u"None"_s,
                 u"Normal"_s,
                 mat()),
            item(T("Energiezähler und Kommunikationsmodul", "Energy meter and communication module"), 1, pcs(), 260.00, u"None"_s, u"Normal"_s, mat()),
            item(T("Montage Speicher im Hauswirtschaftsraum, Wandhalterung, Brandschutzunterlage",
                   "Installation of the storage in the utility room, wall bracket, fire protection base"),
                 4,
                 hrs(),
                 68.00,
                 u"None"_s,
                 u"Normal"_s,
                 work()),
            item(T("Elektroanschluss, Konfiguration, Einbindung in die bestehende Anlage",
                   "Electrical connection, configuration, integration into the existing system"),
                 1,
                 flat(),
                 490.00,
                 u"None"_s,
                 u"Normal"_s,
                 work()),
            item(T("Änderungsmeldung Netzbetreiber und Marktstammdatenregister", "Change notification to grid operator and market master data register"),
                 1,
                 flat(),
                 95.00),
        };
        const Item disc = discount(T("Herbstaktion: 5 % Nachlass auf die Speichernachrüstung", "Autumn special: 5 % discount on the storage retrofit"), -5, sp);
        sp.append(disc);
        sp.append(textItem(T("Die Lieferung und Installation des Speichers erfolgt gemäß § 12 Abs. 3 UStG zum Nullsteuersatz.",
                             "Supply and installation of the storage are subject to the zero tax rate according to § 12 (3) UStG.")));
        d.groups = {{T("Speichernachrüstung", "Storage retrofit"), sp}};
        d.post = T("Bei Auftragserteilung bis zum 06.11.2026 erfolgt die Installation noch im November.",
                   "If you order by November 6, 2026, installation will take place in November.");
        out.append(d);
    }

    // --- Hoffmann: maintenance, standard tax rate
    {
        Doc d;
        d.uuid = uid(u"hoffmann-rechnung"_s);
        d.docType = T("Rechnung", "Invoice");
        d.desc = T("Wartungsvertrag, nächster Termin 09/2027", "Maintenance contract, next date 09/2027");
        d.date = QDate(2026, 9, 10);
        d.counter = 9012;
        d.client = hoffmann;
        d.clientKey = u"hoffmann"_s;
        d.project = T("Wartung PV-Anlage Hoffmann", "Maintenance PV system Hoffmann");
        d.supply = {u"2026-09-08T00:00:00"_s, u"2026-09-08T23:59:59"_s};
        d.salut = T("Sehr geehrter Herr Hoffmann,", "Dear Mr Hoffmann,");
        d.pre =
            T("für die jährliche Wartung Ihrer Photovoltaikanlage (11,2 kWp) am 08.09.2026 berechnen "
              "wir Ihnen gemäß Wartungsvertrag:",
              "for the annual maintenance of your photovoltaic system (11.2 kWp) on September 8, 2026 we "
              "invoice according to the maintenance contract:");
        d.attribs = {KraftAttrib(u"SentDate"_s, QDate(2026, 9, 10), KraftAttrib::Type::Date)};
        d.groups = {{T("Wartung", "Maintenance"),
                     {item(T("Jahreswartung PV-Anlage: Sichtprüfung Module und Unterkonstruktion, Prüfung "
                             "Wechselrichter und Speicher, Auslesen Fehlerspeicher",
                             "Annual maintenance PV system: visual inspection of modules and mounting system, check of "
                             "inverter and storage, reading the error memory"),
                           1,
                           flat(),
                           189.00,
                           u"Full"_s,
                           u"Normal"_s,
                           work()),
                      item(T("Elektrische Prüfung nach DIN VDE 0126-23 inkl. Prüfprotokoll", "Electrical test according to DIN VDE 0126-23 incl. test report"),
                           1,
                           flat(),
                           145.00,
                           u"Full"_s,
                           u"Normal"_s,
                           work()),
                      item(T("Thermografie des Modulfelds per Drohne", "Thermography of the module field by drone"), 1, flat(), 120.00, u"Full"_s),
                      item(T("Modulreinigung mit entmineralisiertem Wasser", "Module cleaning with demineralised water"),
                           26,
                           pcs(),
                           6.50,
                           u"Full"_s,
                           u"Normal"_s,
                           work()),
                      item(T("Anfahrt", "Travel"), 1, flat(), 35.00, u"Full"_s),
                      textItem(T("Alle Module und Anschlüsse in einwandfreiem Zustand. Ein Stringstecker wurde "
                                 "vorsorglich getauscht (Garantie).",
                                 "All modules and connections in perfect condition. One string connector was "
                                 "replaced as a precaution (warranty)."))}}};
        d.post =
            T("Bitte überweisen Sie den Rechnungsbetrag innerhalb von 14 Tagen. Die nächste Wartung "
              "planen wir für September 2027.",
              "Please transfer the invoice amount within 14 days. We plan the next maintenance for September 2027.");
        out.append(d);
    }

    return out;
}

// ---------------------------------------------------------------- saving

// Note: DocumentSaverXML writes all items into one item group, the group names get lost.
void fillKraftDoc(const Doc &d, KraftDoc *doc)
{
    doc->setUuid(d.uuid);
    doc->setDocType(d.docType);
    doc->setWhiteboard(d.desc);
    doc->setDate(d.date);
    if (d.counter) {
        doc->setIdent(identFor(d.date, d.counter));
        doc->state().setState(KraftDocState::State::Final);
    } // else the doc stays New and DocumentSaverXML saves it as Draft
    doc->setTaxValues(19.0, 7.0);
    doc->setPredecessor(d.predecessor);
    for (const auto &a : d.attribs)
        doc->setAttribute(a);
    doc->setAddress(d.client);
    doc->setAddressUid(uid(u"client-"_s + d.clientKey));
    doc->setProjectLabel(d.project);
    if (!d.supply.first.isEmpty())
        doc->setTimeOfSupply(QDateTime::fromString(d.supply.first, Qt::ISODate), QDateTime::fromString(d.supply.second, Qt::ISODate));
    doc->setSalut(d.salut);
    doc->setPreTextRaw(d.pre);
    doc->setPostTextRaw(d.post);
    doc->setGoodbye(d.goodbye.isEmpty() ? T("Mit sonnigen Grüßen\n\nIhr Team von Kraft Photovoltaik", "Sunny regards\n\nYour team at Kraft Photovoltaik")
                                        : d.goodbye);

    for (const auto &[gname, items] : d.groups) {
        for (const Item &i : items) {
            DocPosition *pos = doc->createPosition(DocPosition::typeStrToType(i.type));
            pos->setText(i.text);
            pos->setAmount(i.amount / 100.0);
            pos->setUnit(Einheit(i.unit));
            pos->setUnitPrice(Geld(d.pricesZero ? 0.0 : i.price / 100.0));
            pos->setTaxType(i.tax);
            for (const auto &t : i.tags)
                pos->addTag(t);
            for (const auto &a : i.attribs)
                pos->setAttribute(a);
        }
    }
    // last, as the setters above update it
    doc->setLastModified(QDateTime(d.date, QTime(16, 12)));
}

} // namespace

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QStringList args = app.arguments().mid(1);
    if (args.size() == 3 && args.at(0) == u"--lang" && (args.at(1) == u"de" || args.at(1) == u"en")) {
        En = args.at(1) == u"en";
        args = args.mid(2);
    }
    if (args.size() != 1) {
        fprintf(stderr, "Usage: kraftdemodata [--lang de|en] <outdir>\n");
        return 1;
    }

    // UnitManager reads the tax rates from the default database connection, a temporary one is enough.
    QSqlDatabase db = QSqlDatabase::addDatabase(u"QSQLITE"_s);
    db.setDatabaseName(u":memory:"_s);
    QSqlQuery q(db);
    if (!db.open() || !q.exec(u"CREATE TABLE taxes (fullTax DECIMAL(5,2), reducedTax DECIMAL(5,2), startDate DATE)"_s)
        || !q.exec(u"INSERT INTO taxes VALUES (19, 7, '2000-01-01')"_s)) {
        fprintf(stderr, "Can not create the tax database\n");
        return 1;
    }

    // the saver takes currency, country, language and owner from the environment
    *DefaultProvider::self()->locale() = QLocale(En ? QLocale::English : QLocale::German, QLocale::Germany);
    qputenv("USER", "demo");

    // creates a new base dir with a random name below outdir, it does not touch the Kraft settings
    const QString baseDir = DefaultProvider::self()->createV2BaseDir(QDir(args.first()).absolutePath());
    if (baseDir.isEmpty()) {
        fprintf(stderr, "Can not create a Kraft base dir in %s\n", qPrintable(args.first()));
        return 1;
    }

    XmlIndexDb indx; // opens the index database the saver writes to

    DocumentSaverXML saver;
    saver.setArchiveMode(true); // keeps the lastModified date
    const auto dds = docs();
    for (const auto &d : dds) {
        KraftDoc doc;
        fillKraftDoc(d, &doc);
        if (!saver.saveDocument(&doc)) {
            fprintf(stderr, "Can not save %s\n", qPrintable(saver.lastSavedFileName()));
            return 1;
        }
        printf("%s  %-24s %-12s %-6s netto %10s  brutto %10s  %s\n",
               qPrintable(d.date.toString(Qt::ISODate)),
               qPrintable(d.docType),
               qPrintable(doc.ident()),
               qPrintable(d.counter ? u"Final"_s : u"Draft"_s),
               qPrintable(fmt(doc.nettoSum().toLong())),
               qPrintable(fmt(doc.bruttoSum().toLong())),
               qPrintable(saver.lastSavedFileName()));
    }
    printf("Kraft base dir: %s\n", qPrintable(baseDir));
    return 0;
}
