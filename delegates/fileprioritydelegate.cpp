#include "fileprioritydelegate.h"
#include <QComboBox>
#include <libtorrent/download_priority.hpp> // TODO: Avoid using this

FilePriorityDelegate::FilePriorityDelegate(QObject *parent)
    : QStyledItemDelegate{parent}
{}

QWidget *FilePriorityDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    qDebug() << "Create editor priority";
    QComboBox* comboBox = new QComboBox(parent);
    comboBox->addItem("Don't download");
    comboBox->addItem("Default");
    comboBox->addItem("Low");
    comboBox->addItem("High");
    // TODO: Remove these magic values

    auto priorityStr = index.data().toString();
    comboBox->setCurrentText(priorityStr);
    return comboBox;
}

void FilePriorityDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    qDebug() << "set editor data";
}

void FilePriorityDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    editor->setGeometry(option.rect);
}

void FilePriorityDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{

    QComboBox* comboBox = static_cast<QComboBox*>(editor);
    auto priorityStr = comboBox->currentText();
    int priority = 0;
    // TODO: Avoid these magic values
    if (priorityStr == "Don't download") {
        priority = lt::dont_download;
    } else if (priorityStr == "Default") {
        priority = lt::default_priority;
    } else if (priorityStr == "Low") {
        priority = lt::low_priority;
    } else if (priorityStr == "High") {
        priority = lt::top_priority;
    }
    model->setData(index, priority);
}
