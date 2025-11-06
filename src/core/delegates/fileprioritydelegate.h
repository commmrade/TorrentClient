#ifndef FILEPRIORITYDELEGATE_H
#define FILEPRIORITYDELEGATE_H

#include <QObject>
#include <QStyledItemDelegate>

class FilePriorityDelegate : public QStyledItemDelegate
{
    Q_OBJECT
  public:
    explicit FilePriorityDelegate(QObject *parent = nullptr);

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const override;
    void     updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option,
                                  const QModelIndex &index) const override;
    void     setModelData(QWidget *editor, QAbstractItemModel *model,
                          const QModelIndex &index) const override;
};

#endif // FILEPRIORITYDELEGATE_H
