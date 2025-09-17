#ifndef FILELISTWIDGET_H
#define FILELISTWIDGET_H

#include <QWidget>
#include "filestablemodel.h"
#include "fileitemdelegate.h"
#include "filestatusdelegate.h"
#include "fileprioritydelegate.h"

namespace Ui {
class FileListWidget;
}

class FileListWidget : public QWidget
{
    Q_OBJECT

public:
    explicit FileListWidget(QWidget *parent = nullptr);
    ~FileListWidget();

    void setFiles(const QList<File>& files);
    void clearFiles();
private:
    void contextMenuRequested(const QPoint &pos);

    Ui::FileListWidget *ui;

    FileTableModel m_fileModel;
    void setupTableView();

    FileStatusDelegate m_statusDelegate;
    FileItemDelegate m_itemDelegate;
    FilePriorityDelegate m_priorityDelegate;
};

#endif // FILELISTWIDGET_H
