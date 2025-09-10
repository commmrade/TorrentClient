#ifndef FILELISTWIDGET_H
#define FILELISTWIDGET_H

#include <QWidget>
#include "filestablemodel.h"
#include "fileitemdelegate.h"

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
    FileItemDelegate m_itemDelegate;
};

#endif // FILELISTWIDGET_H
