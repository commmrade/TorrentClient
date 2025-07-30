#ifndef TORRENTITEMDELEGATE_H
#define TORRENTITEMDELEGATE_H
#include <QStyledItemDelegate>

class TorrentItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit TorrentItemDelegate(QObject* parent);

    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
};

#endif // TORRENTITEMDELEGATE_H
