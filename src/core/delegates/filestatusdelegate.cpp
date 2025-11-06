#include "filestatusdelegate.h"
#include <QCheckBox>
#include <QApplication>
#include "core/models/filetreemodel.h"

FileStatusDelegate::FileStatusDelegate(QObject *parent) : QStyledItemDelegate{parent} {}

void FileStatusDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                               const QModelIndex &index) const
{
    BaseItem *obj = static_cast<BaseItem *>(index.internalPointer());
    if (obj->isDir())
    {
        QStyledItemDelegate::paint(painter, option, index);
        return;
    }

    bool checked = index.model()->data(index, Qt::DisplayRole).toBool();

    QStyleOptionButton checkBoxOption;
    checkBoxOption.state |= QStyle::State_Enabled;
    checkBoxOption.state |= checked ? QStyle::State_On : QStyle::State_Off;
    QSize checkBoxSize = QApplication::style()->sizeFromContents(QStyle::CT_CheckBox,
                                                                 &checkBoxOption, QSize(), nullptr);
    // center the checkbox rect in the item rect
    checkBoxOption.rect =
        QStyle::alignedRect(option.direction, Qt::AlignCenter, checkBoxSize, option.rect);
    QApplication::style()->drawControl(QStyle::CE_CheckBox, &checkBoxOption, painter);
}

bool FileStatusDelegate::editorEvent(QEvent *event, QAbstractItemModel *model,
                                     [[maybe_unused]] const QStyleOptionViewItem &option,
                                     const QModelIndex                           &index)
{
    if (event->type() == QEvent::MouseButtonRelease)
    {
        auto isEnabled = model->data(index).toBool();
        model->setData(index, !isEnabled);
    }
    return true;
}
