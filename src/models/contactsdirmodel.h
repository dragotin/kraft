#ifndef CONTACTSDIRMODEL_H
#define CONTACTSDIRMODEL_H

#include <QObject>
#include <QAbstractItemModel>
#include <QDirListing>
#include <KContacts/Addressee>

class TreeItem
{
public:
    explicit TreeItem(KContacts::Addressee& addressee, TreeItem* parentItem = nullptr);

    TreeItem *child(int row);
    int childCount() const;
    int columnCount() const;
    QVariant data(int column) const;
    int row() const;
    TreeItem *parentItem();
    void appendChild(std::unique_ptr<TreeItem> &child);

private:
    std::vector<std::unique_ptr<TreeItem>> _childItems;
    static QVariantList _headers;
    QVariantList _itemData;
    TreeItem* _parentItem;
    KContacts::Addressee _addressee;
    QString _name; // for the root item
};

class ContactsDirModel : public QAbstractItemModel
{
public:
    ContactsDirModel(QObject *parent, const QString& baseDir);

public:
    Q_DISABLE_COPY_MOVE(ContactsDirModel)

    explicit ContactsDirModel(const QString &data, QObject *parent = nullptr);
    ~ContactsDirModel() override;

    QVariant data(const QModelIndex &index, int role) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;
    QModelIndex index(int row, int column,
                      const QModelIndex &parent = {}) const override;
    QModelIndex parent(const QModelIndex &index) const override;
    int rowCount(const QModelIndex &parent = {}) const override;
    int columnCount(const QModelIndex &parent = {}) const override;

private:
    static void setupModelData(const QList<QStringView> &lines, TreeItem *parent);

    std::unique_ptr<TreeItem> _rootItem;
};

#endif // CONTACTSDIRMODEL_H
