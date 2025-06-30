#ifndef CONTACTSDIRMODEL_H
#define CONTACTSDIRMODEL_H

#include <QObject>
#include <QAbstractItemModel>
#include <QDirListing>
#include <KContacts/Addressee>

/**
 * @brief The CTMItem class - ContactsDirModel Item
 */
class CTMItem
{
public:
    explicit CTMItem(KContacts::Addressee& addressee, CTMItem* parentItem = nullptr);

    CTMItem *child(int row);
    int childCount() const;
    int columnCount() const;
    QVariant data(int column) const;
    int row() const;
    CTMItem *parentItem();
    void appendChild(std::unique_ptr<CTMItem> &child);
    KContacts::Addressee getAddressee() const;

    static QVariantList _headers;
private:
    std::vector<std::unique_ptr<CTMItem>> _childItems;
    CTMItem* _parentItem;
    KContacts::Addressee _addressee;
};

// =============================================================================

class ContactsDirModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    Q_DISABLE_COPY_MOVE(ContactsDirModel)

    explicit ContactsDirModel(const QString& baseDir, QObject *parent = nullptr);

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

    const static int ColumnCount{3};
private:
    static void setupModelData(const QList<QStringView> &lines, CTMItem *parent);

    std::unique_ptr<CTMItem> _rootItem;
};

#endif // CONTACTSDIRMODEL_H
