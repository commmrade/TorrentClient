#include "fileitemdelegate.h"
#include <QApplication>
#include "filetreemodel.h"

FileItemDelegate::FileItemDelegate(QObject *parent)
    : QStyledItemDelegate{parent}
{}

void FileItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    BaseItem* obj = static_cast<BaseItem*>(index.internalPointer());
    if (obj->isDir()) {
        QStyledItemDelegate::paint(painter, option, index);
        return;
    }

    double value = index.data().toDouble();
    if (std::isnan(value)) {
        value = 0.0;
    }
    QStyleOptionProgressBar pbOption;
    pbOption.progress = value;
    pbOption.minimum = 0;
    pbOption.maximum = 100;
    pbOption.rect = option.rect;
    pbOption.text = QString::number(value) + "%";
    pbOption.textVisible = true;
    pbOption.textAlignment = Qt::AlignCenter;
    QApplication::style()->drawControl(QStyle::CE_ProgressBar, &pbOption, painter);
}
