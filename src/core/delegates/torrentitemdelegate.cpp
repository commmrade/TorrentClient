#include "core/delegates/torrentitemdelegate.h"
#include <QApplication>

TorrentItemDelegate::TorrentItemDelegate(QObject *parent) : QStyledItemDelegate(parent) {}

void TorrentItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                                const QModelIndex &index) const
{
    double value = index.data().toDouble();
    if (std::isnan(value))
    {
        value = 0.0;
    }
    QStyleOptionProgressBar pbOption;
    pbOption.progress      = value;
    pbOption.minimum       = 0;
    pbOption.maximum       = 100;
    pbOption.rect          = option.rect;
    pbOption.text          = QString::number(value) + "%";
    pbOption.textVisible   = true;
    pbOption.textAlignment = Qt::AlignCenter;
    QApplication::style()->drawControl(QStyle::CE_ProgressBar, &pbOption, painter);
}
