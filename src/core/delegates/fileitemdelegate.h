#ifndef FILEITEMDELEGATE_H
#define FILEITEMDELEGATE_H

#include <QStyledItemDelegate>

class FileItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT
  public:
    explicit FileItemDelegate(QObject *parent = nullptr);
    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;
  signals:
};

#endif // FILEITEMDELEGATE_H
