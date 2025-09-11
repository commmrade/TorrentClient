#include "filestatusdelegate.h"
#include <QCheckBox>
#include <QApplication>

FileStatusDelegate::FileStatusDelegate(QObject *parent)
    : QStyledItemDelegate{parent}
{
    qDebug() << "FIle status created";
}

void FileStatusDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    bool checked = index.model()->data(index, Qt::DisplayRole).toBool();

    QStyleOptionButton checkBoxOption;
    checkBoxOption.state |= QStyle::State_Enabled;
    if (checked)
        checkBoxOption.state |= QStyle::State_On;
    else
        checkBoxOption.state |= QStyle::State_Off;
    checkBoxOption.rect = option.rect;

    QApplication::style()->drawControl(QStyle::CE_CheckBox, &checkBoxOption, painter);
}

bool FileStatusDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    if (event->type() == QEvent::MouseButtonRelease) {
        auto isEnabled = model->data(index).toBool();
        model->setData(index, !isEnabled);
    }
    return true;
}
