#include <QTest>
#include <QTemporaryDir>
#include <QDomDocument>

#include "defaultprovider.h"
#include "xmldirlister.h"

using namespace Qt::StringLiterals;

// Minimal type satisfying the XmlDirLister<T> contract.
class Item {
public:
    Item() : _modified(true) {}

    QString name() const { return _name; }
    void setName(const QString& n) { _name = n; _modified = true; }

    QString value() const { return _value; }
    void setValue(const QString& v) { _value = v; _modified = true; }

    bool modified() const { return _modified; }

    void parseXml(QDomDocument& doc) {
        const QDomElement root = doc.firstChildElement("item");
        _name  = root.firstChildElement("name").text();
        _value = root.firstChildElement("value").text();
        _modified = false;
    }

    const QString toXml() const {
        QDomDocument doc("item");
        QDomProcessingInstruction instr =
            doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"utf-8\"");
        doc.appendChild(instr);

        QDomElement root = doc.createElement("item");
        doc.appendChild(root);

        auto textElem = [&](const QString& tag, const QString& text) {
            QDomElement e = doc.createElement(tag);
            e.appendChild(doc.createTextNode(text));
            return e;
        };
        root.appendChild(textElem("name", _name));
        root.appendChild(textElem("value", _value));
        return doc.toString();
    }

    bool isEmpty() {
        return _name.isEmpty();
    }

private:
    QString _name;
    QString _value;
    bool    _modified;
};

class Items : public XmlDirLister<Item> {};

template<>
DefaultProvider::KraftV2Dir XmlDirLister<Item>::v2SubDir()
{
    return DefaultProvider::KraftV2Dir::Tests;
}


class T_XmlDirLister : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void initTestCase() {
        _baseDir = DefaultProvider::self()->createV2BaseDir(_dir.path());
        QVERIFY(!_baseDir.isEmpty());

        // createV2BaseDir does not create the tests/ subdir — do it here
        const QString testsSubdir = DefaultProvider::self()->kraftV2Dir(DefaultProvider::KraftV2Dir::Tests);
        QVERIFY(QDir().mkpath(testsSubdir));
    }

    // get() on an empty lister returns a default-constructed T
    void getMiss() {
        const Item i = _items.get("nonexistent");
        QVERIFY(i.name().isEmpty());
    }

    // save() writes to disk and makes the item visible in the same instance
    void saveAndGet() {
        Item a;
        a.setName("Alpha");
        a.setValue("alpha-value");
        QCOMPARE(_items.save(a), Items::SaveResult::SaveOk);

        const Item found = _items.get("Alpha");
        QCOMPARE(found.name(), u"Alpha"_s);
        QCOMPARE(found.value(), u"alpha-value"_s);
    }

    // A fresh instance loads from disk what the previous instance saved
    void freshInstanceSeesFile() {
        Items fresh;
        const Item found = fresh.get("Alpha");
        QCOMPARE(found.name(), u"Alpha"_s);
        QCOMPARE(found.value(), u"alpha-value"_s);
    }

    // map() returns all items known to this instance
    void mapContainsAll() {
        const auto m = _items.map();
        QVERIFY(m.contains("Alpha"));
        QCOMPARE(m.size(), 1);
    }

    // save() with an empty name returns NameFail and does not write a file
    void saveEmptyNameFails() {
        Item unnamed;
        QCOMPARE(_items.save(unnamed), Items::SaveResult::NameFail);
        QCOMPARE(_items.map().size(), 1);
    }

    // insert() updates this instance's in-memory map without touching disk
    void insertInMemoryOnly() {
        Item b;
        b.setName("Beta");
        b.setValue("beta-value");
        _items.insert("Beta", b);

        QCOMPARE(_items.get("Beta").name(), u"Beta"_s);
        QCOMPARE(_items.map().size(), 2);

        // A fresh instance loaded from disk must not see Beta (never saved)
        Items fresh;
        QVERIFY(fresh.get("Beta").name().isEmpty());
        QCOMPARE(fresh.map().size(), 1);
    }

    void isEmpty() {
        Item b;
        b.setName("Beta");
        b.setValue("beta-value");
        QVERIFY(!b.isEmpty());

        Item c;
        QVERIFY(c.isEmpty());
    }

    // remove() hides an item from this instance without deleting the disk file
    void removeHidesFromInstance() {
        // Save Beta so it exists on disk, then remove it from _items
        Item b;
        b.setName("Beta");
        b.setValue("beta-value");
        _items.save(b);
        QCOMPARE(_items.map().size(), 2);

        _items.remove("Beta");
        QVERIFY(_items.get("Beta").name().isEmpty());
        QCOMPARE(_items.map().size(), 1);

        // The file is still on disk — a fresh instance will find it
        Items fresh;
        QCOMPARE(fresh.get("Beta").name(), u"Beta"_s);
    }

    // saveAll() writes modified items, skips unmodified ones, and
    // replaces this instance's in-memory map with the given map
    void saveAll() {
        // Start from what's on disk (Alpha + Beta)
        Items items;
        XmlDirLister<Item>::Map m = items.map();
        QCOMPARE(m.size(), 2);

        Item g;
        g.setName("Gamma");
        g.setValue("gamma-value");
        m["Gamma"] = g; // modified() == true

        QCOMPARE(items.saveAll(m), Items::SaveResult::SaveOk);

        // This instance's map reflects the new state
        const auto result = items.map();
        QCOMPARE(result.size(), 3);
        QVERIFY(result.contains("Gamma"));

        // A fresh instance loads from disk and sees all three
        Items fresh;
        QCOMPARE(fresh.map().size(), 3);
        QCOMPARE(fresh.get("Gamma").value(), u"gamma-value"_s);
    }

    // saveAll() with no modified items does not change the disk but still
    // updates this instance's in-memory map to the given map
    void saveAllSkipsUnmodified() {
        // Load from disk — parseXml sets modified() = false for all items
        Items items;
        const XmlDirLister<Item>::Map m = items.map();
        QCOMPARE(m.size(), 3);

        // None of the loaded items are modified; saveAll should be a no-op on disk
        QCOMPARE(items.saveAll(m), Items::SaveResult::SaveOk);

        // A fresh instance must still see exactly the same count
        Items fresh;
        QCOMPARE(fresh.map().size(), 3);
    }

    // saveAll() with a subset deletes files for removed items — a fresh
    // instance must not see them either
    void saveAllSubsetDeletesFiles() {
        Items items;
        XmlDirLister<Item>::Map m = items.map();
        QCOMPARE(m.size(), 3);

        m.remove("Gamma");
        QCOMPARE(items.saveAll(m), Items::SaveResult::SaveOk);

        // This instance now reports only 2
        QCOMPARE(items.map().size(), 2);
        QVERIFY(!items.map().contains("Gamma"));

        // Fresh instance loads from disk and must not find the deleted file
        Items fresh;
        QCOMPARE(fresh.map().size(), 2);
        QVERIFY(!fresh.map().contains("Gamma"));
    }

private:
    QTemporaryDir _dir;
    QString _baseDir;
    Items _items;
};

QTEST_MAIN(T_XmlDirLister)

#include "t_xmldirlister.moc"
