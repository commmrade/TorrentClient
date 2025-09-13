#include "categorysortfilter.h"


CategorySortFIlter::CategorySortFIlter(QObject *parent) : QSortFilterProxyModel(parent)
{

}

bool CategorySortFIlter::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    auto categoryIndex = sourceModel()->index(sourceRow, 1); // 1 is category, 0 is id
    auto categoryStr = sourceModel()->data(categoryIndex, Qt::UserRole).toString();
    // qDebug() << "Filter category:" << m_category << "Row category:" << categoryStr;
    if (m_category == "All" || m_category == categoryStr) {
        return true;
    }
    return false;
}
