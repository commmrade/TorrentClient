#ifndef FILESTATUSDELEGATE_H
#define FILESTATUSDELEGATE_H

#include <QObject>
#include <QStyledItemDelegate>

class FileStatusDelegate : public QStyledItemDelegate
{
    Q_OBJECT
  public:
    explicit FileStatusDelegate(QObject *parent = nullptr);
    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;
    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option,
                     const QModelIndex &index) override;
};

#endif // FILESTATUSDELEGATE_H
