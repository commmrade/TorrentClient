#ifndef CATEGORYSORTFILTER_H
#define CATEGORYSORTFILTER_H

#include <QObject>
#include <QSortFilterProxyModel>

class CategorySortFIlter : public QSortFilterProxyModel
{
    Q_OBJECT
    QString m_category{};
public:
    CategorySortFIlter(QObject* parent);

    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;
    void setCategory(const QString& category) {
        m_category = category;
    }
    QString getCategory() const {
        return m_category;
    }
};

#endif // CATEGORYSORTFILTER_H
