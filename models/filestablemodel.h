#ifndef FILESTABLEMODEL_H
#define FILESTABLEMODEL_H

#include <QWidget>
#include <QAbstractTableModel>
#include "file.h"

namespace Ui {
class FilesTableModel;
}

class FileTableModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit FileTableModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex& index = QModelIndex{}) const override {
        return m_files.size();
    }
    int columnCount(const QModelIndex& index = QModelIndex{}) const override {
        return FILE_FIELD_COUNT;
    }

    QVariant data(const QModelIndex& index = QModelIndex{}, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex& index = QModelIndex{}, const QVariant& value = QVariant{}, int role = Qt::EditRole) override; // Probably will be needed for editing (enabled, disabled, priority)

    QVariant headerData(int section, Qt::Orientation, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;


    int getFileId(int index) const { return m_files[index].id; }
    bool getIsEnabled(int index) const { return m_files[index].isEnabled; }

    void setFiles(const QList<File>& files);
    void clearFiles();
signals:
    void statusChanged(int index, bool value);
private:
    QList<File> m_files;
};


#endif // FILESTABLEMODEL_H
