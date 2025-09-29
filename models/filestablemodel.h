#ifndef FILESTABLEMODEL_H
#define FILESTABLEMODEL_H

#include <QWidget>
#include <QAbstractTableModel>
#include "file.h"
#include <libtorrent/download_priority.hpp>
#include "priority.h"
#include "utils.h"

namespace Ui {
class FilesTableModel;
}

class FileItem {
    enum FileType {
        TYPE_DIRECTORY,
        TYPE_FILE
    };
public:
    FileItem(int type, FileItem* parent = nullptr) : m_fileType((FileType)type), m_parent(parent) {}
    ~FileItem() {
        qDeleteAll(m_children);
    }

    void addFile(FileItem* file) {
        m_children.append(file);
    }

    int childCount() const {
        return m_children.count();
    }

    QList<FileItem*> children() const {
        return m_children;
    }

    FileItem* parent() const {
        return m_parent;
    }
    void setParent(FileItem* newParent) {
        m_parent = newParent;
    }

    QVariant getValue(int col) const {
        switch (static_cast<FileFields>(col)) {
            case FileFields::STATUS: {
                return {file.isEnabled};
            }
            case FileFields::FILENAME: {
                return {file.filename};
            }
            case FileFields::PROGRESS: {
                auto ratio = static_cast<double>(file.downloaded) / file.filesize;
                ratio = std::round(ratio * 100.0) / 100.0;
                return {ratio};
            }
            case FileFields::DOWNLOADED: {
                return {utils::bytesToHigher(file.downloaded)};
            }
            case FileFields::FILESIZE: {
                return {utils::bytesToHigher(file.filesize)};
            }
            case FileFields::PRIORITY: {
                auto priorityToString = [](int const priority) -> QString {
                    switch (priority) {
                        case lt::dont_download: {
                            return {Priorities::DONT_DOWNLOAD};
                        }
                        case lt::default_priority: {
                            return {Priorities::DEFAULT};
                        }
                        case lt::low_priority: {
                            return {Priorities::LOW};
                        }
                        case lt::top_priority: {
                            return {Priorities::HIGH};
                        }
                        default: {
                            throw std::runtime_error("Cant be here");
                        }
                    }
                };
                return {priorityToString(file.priority)};
            }
            default: {
                throw std::runtime_error("TF u doing here?");
            }
        }
    }
private:
    FileType m_fileType;
    FileItem* m_parent{nullptr};
    QList<FileItem*> m_children;
    File file; // Only if is a file
};

class FileTreeModel : public QAbstractItemModel {
    Q_OBJECT

    FileItem* objByIndex(const QModelIndex& index) const {
        if (!index.isValid()) return m_rootItem;
        return static_cast<FileItem*>(index.internalPointer());
    }
public:
    explicit FileTreeModel(QObject* parent = nullptr) {
        m_rootItem = new FileItem{0, nullptr};
    }

    QModelIndex index(int row, int column, const QModelIndex &parent) const override {
        if (!hasIndex(row, column, parent)) return QModelIndex{};


        auto* obj = objByIndex(parent);
        return createIndex(row, column, obj->children().at(row));
    }
    QModelIndex parent(const QModelIndex &child) const override {
        auto* childObj = objByIndex(child);
        auto* parentObj = childObj->parent();

        if (parentObj == m_rootItem) return QModelIndex{};

        auto* grandparentObj = parentObj->parent();
        int row = grandparentObj->children().indexOf(parentObj);
        return createIndex(row, 0, parentObj);
    }

    int rowCount(const QModelIndex &parent) const override {
        return objByIndex(parent)->childCount();
    }
    int columnCount(const QModelIndex &parent) const override {
        return FILE_FIELD_COUNT;
    }
    QVariant data(const QModelIndex &index, int role) const override {
        if (!index.isValid()) return QVariant{};

        if (role == Qt::DisplayRole) {
            FileItem* obj = objByIndex(index);
            return obj->getValue(index.column());
        }
        return QVariant{};
    }

    void addItem(FileItem* item, const QModelIndex& parentIndex) {
        beginInsertRows(parentIndex, rowCount(parentIndex), rowCount(parentIndex));
        item->setParent(objByIndex(parentIndex));
        endInsertRows();
    }

private:
    FileItem* m_rootItem;
};

// class FileTableModel : public QAbstractTableModel
// {
//     Q_OBJECT
// public:
//     explicit FileTableModel(QObject *parent = nullptr);

//     int rowCount(const QModelIndex& index = QModelIndex{}) const override {
//         return m_files.size();
//     }
//     int columnCount(const QModelIndex& index = QModelIndex{}) const override {
//         return FILE_FIELD_COUNT;
//     }

//     Qt::ItemFlags flags(const QModelIndex &index) const override;

//     QVariant data(const QModelIndex& index = QModelIndex{}, int role = Qt::DisplayRole) const override;
//     bool setData(const QModelIndex& index = QModelIndex{}, const QVariant& value = QVariant{}, int role = Qt::EditRole) override; // Probably will be needed for editing (enabled, disabled, priority)

//     QVariant headerData(int section, Qt::Orientation, int role = Qt::DisplayRole) const override;
//     QHash<int, QByteArray> roleNames() const override;


//     int getFileId(int index) const { return m_files[index].id; }
//     bool getIsEnabled(int index) const { return m_files[index].isEnabled; }

//     void setFiles(const QList<File>& files);
//     void clearFiles();
// signals:
//     void statusChanged(int index, bool value);
//     void priorityChanged(int index, int priority);
// private:
//     QList<File> m_files;
// };


#endif // FILESTABLEMODEL_H
