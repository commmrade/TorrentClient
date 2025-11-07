#include "core/filters/categorysortfilter.h"
#include "core/utils/category.h"

CategorySortFilter::CategorySortFilter(QObject *parent)
    : QSortFilterProxyModel(parent), m_category(Categories::ALL)
{
}

bool CategorySortFilter::filterAcceptsRow(int                                 sourceRow,
                                          [[maybe_unused]] const QModelIndex &sourceParent) const
{
    auto categoryIndex = sourceModel()->index(sourceRow, 1); // 1 is category, 0 is id
    auto categoryStr   = sourceModel()->data(categoryIndex, Qt::UserRole).toString();
    if (m_category == Categories::ALL || m_category == categoryStr)
    {
        return true;
    }
    return false;
}
