#include "categorysortfilter.h"
#include "category.h"

CategorySortFilter::CategorySortFilter(QObject *parent) : QSortFilterProxyModel(parent), m_category(Categories::ALL)
{

}

bool CategorySortFilter::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    auto categoryIndex = sourceModel()->index(sourceRow, 1); // 1 is category, 0 is id
    auto categoryStr = sourceModel()->data(categoryIndex, Qt::UserRole).toString();
    // qDebug() << "Filter category:" << m_category << "Row category:" << categoryStr;
    if (m_category == Categories::ALL || m_category == categoryStr) {
        return true;
    }
    return false;
}
