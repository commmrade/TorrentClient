#include "filestablemodel.h"
#include "utils.h"
#include <libtorrent/download_priority.hpp>
#include "priority.h"

// FileTableModel::FileTableModel(QObject *parent) : QAbstractTableModel(parent)
// {
// }

// Qt::ItemFlags FileTableModel::flags(const QModelIndex &index) const
// {
//     if (index.column() == static_cast<int>(FileFields::PRIORITY)) {
//         return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
//     }
//     return QAbstractItemModel::flags(index);
// }

// QVariant FileTableModel::data(const QModelIndex &index, int role) const
// {
//     if (role < Qt::UserRole) {
//         if (role == Qt::DisplayRole) {
//             auto& file =  m_files[index.row()];
//             switch (static_cast<FileFields>(index.column())) {
//                 case FileFields::STATUS: {
//                     return {file.isEnabled};
//                 }
//                 case FileFields::FILENAME: {
//                     return {file.filename};
//                 }
//                 case FileFields::PROGRESS: {
//                     auto ratio = static_cast<double>(file.downloaded) / file.filesize;
//                     ratio = std::round(ratio * 100.0) / 100.0;
//                     return {ratio};
//                 }
//                 case FileFields::DOWNLOADED: {
//                     return {utils::bytesToHigher(file.downloaded)};
//                 }
//                 case FileFields::FILESIZE: {
//                     return {utils::bytesToHigher(file.filesize)};
//                 }
//                 case FileFields::PRIORITY: {
//                     auto priorityToString = [](int const priority) -> QString {
//                         switch (priority) {
//                             case lt::dont_download: {
//                                 return {Priorities::DONT_DOWNLOAD};
//                             }
//                             case lt::default_priority: {
//                                 return {Priorities::DEFAULT};
//                             }
//                             case lt::low_priority: {
//                                 return {Priorities::LOW};
//                             }
//                             case lt::top_priority: {
//                                 return {Priorities::HIGH};
//                             }
//                             default: {
//                                 throw std::runtime_error("Cant be here");
//                             }
//                         }
//                     };
//                     return {priorityToString(file.priority)};
//                 }
//                 default: {
//                     throw std::runtime_error("TF u doing here?");
//                 }
//             }
//         }
//     }
//     return {};
// }

// bool FileTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
// {
//     if (role == Qt::EditRole) {
//         auto& file = m_files[index.row()];
//         switch (static_cast<FileFields>(index.column())) {
//         case FileFields::STATUS: {
//             file.isEnabled = value.toBool();
//             emit statusChanged(file.id, file.isEnabled);
//             break;
//         }
//         case FileFields::PRIORITY: {
//             qDebug() << "Priority set data";
//             file.priority = value.toInt();
//             emit priorityChanged(file.id, file.priority);
//             break;
//         }
//         // ...
//         default: {
//             throw std::runtime_error("Something is wrong");
//             break;
//         }
//         }
//         return true;
//     }
//     return false;
// }

// QVariant FileTableModel::headerData(int section, Qt::Orientation orientation, int role) const
// {
//     if (role != Qt::DisplayRole) {
//         return {};
//     }

//     if (orientation == Qt::Horizontal) {
//         switch (static_cast<FileFields>(section)) {
//             case FileFields::STATUS: {
//                 return {tr("Status")};
//             }
//             case FileFields::FILENAME: {
//                 return {tr("Name")};
//             }
//             case FileFields::PROGRESS: {
//                 return {tr("Progress")};
//             }
//             case FileFields::DOWNLOADED: {
//                 return {tr("Downloaded")};
//             }
//             case FileFields::FILESIZE: {
//                 return {tr("Size")};
//             }
//             case FileFields::PRIORITY: {
//                 return {tr("Priority")};
//             }
//             default: {
//                 throw std::runtime_error("TF u doing here?");
//             }
//         }
//     } else {
//         return {section + 1};
//     }
// }

// QHash<int, QByteArray> FileTableModel::roleNames() const
// {
//     return {};
// }

// void FileTableModel::setFiles(const QList<File> &files)
// {
//     qDebug() << "Setting " << files.size();
//     // TODO: Optimize it, since it is suitable for torrents list, but i guess i can do it easier for (almost) static files
//     QHash<int, int> newTrackersMap; // key - index in trackers
//     QHash<int, int> oldTrackersMap; // key - index in m_trackers

//     // Cache trackers
//     for (int i = 0; i < files.size(); ++i) {
//         newTrackersMap.insert(files[i].id, i);
//     }
//     for (int i = 0; i < m_files.size(); ++i) {
//         oldTrackersMap.insert(m_files[i].id, i);
//     }

//     for (int i = 0; i < files.size(); ++i) {
//         auto const newKey = files[i].id;
//         if (oldTrackersMap.contains(newKey)) {
//             // Tracker exists, check for updates
//             int row = oldTrackersMap[newKey];
//             auto& oldFile = m_files[row];

//             oldFile.filename = files[i].filename; // Since files can be renamed
//             oldFile.downloaded = files[i].downloaded;
//             oldFile.priority = files[i].priority;
//         } else {
//             int row = m_files.size();
//             beginInsertRows(QModelIndex(), row, row);
//             m_files.append(files[i]);
//             endInsertRows();
//         }
//     }

//     // Handle deletions
//     for (int i = m_files.size() - 1; i >= 0; --i) {
//         auto const key = m_files[i].id;
//         if (!newTrackersMap.contains(key)) {
//             beginRemoveRows(QModelIndex(), i, i);
//             m_files.removeAt(i);
//             endRemoveRows();
//         }
//     }

//     if (!m_files.isEmpty()) {
//         emit dataChanged(index(0, 1), index(m_files.size() - 1, columnCount() - 1));
//     }
// }

// void FileTableModel::clearFiles()
// {
//     beginResetModel();
//     m_files.clear();
//     endResetModel();
// }

void BaseItem::addChild(BaseItem *file) {
    m_children.append(file);
    file->setParent(this);
}

void BaseItem::setParent(BaseItem *newParent) {
    if (m_parent && m_parent != newParent) {
        m_parent->removeChild(this);
    }
    m_parent = newParent;
}

QVariant FileItem::getValue(int column) const {
    switch (static_cast<FileFields>(column)) {
    case FileFields::STATUS: {
        return {m_fileData.isEnabled};
    }
    case FileFields::FILENAME: {
        return {m_fileData.filename};
    }
    case FileFields::PROGRESS: {
        auto ratio = static_cast<double>(m_fileData.downloaded) / m_fileData.filesize;
        ratio = std::round(ratio * 100.0) / 100.0;
        return {ratio};
    }
    case FileFields::DOWNLOADED: {
        return {utils::bytesToHigher(m_fileData.downloaded)};
    }
    case FileFields::FILESIZE: {
        return {utils::bytesToHigher(m_fileData.filesize)};
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
        return {priorityToString(m_fileData.priority)};
    }
    default: {
        throw std::runtime_error("TF u doing here?");
    }
    }
}

bool FileItem::setValue(int column, const QVariant &value) {
    switch (static_cast<FileFields>(column)) {
    case FileFields::STATUS: {
        m_fileData.isEnabled = value.toBool();
        return true;
    }
    case FileFields::PRIORITY: {
        m_fileData.priority = value.toInt();
        return true;
    }
    // ...
    default: {
        throw std::runtime_error("Something is wrong");
        return false;
    }
    }
}

QVariant DirItem::getValue(int column) const {
    switch (static_cast<FileFields>(column)) {
    case FileFields::FILENAME: {
        return m_path;
    }
    default: {
        return QVariant{};
    }
    }
}

BaseItem *FileTreeModel::objByIndex(const QModelIndex &index) const {
    if (!index.isValid()) return m_rootItem;
    return static_cast<FileItem*>(index.internalPointer());
}

void FileTreeModel::resetRoot() {
    delete m_rootItem;

    m_rootItem = nullptr;
    m_rootItem = new DirItem();
    m_rootItem->setPath("Root");
    // m_rootItem->setDirName("Root");
}

FileTreeModel::FileTreeModel(QObject *parent) : QAbstractItemModel(parent) {
    resetRoot();
}

QModelIndex FileTreeModel::index(int row, int column, const QModelIndex &parent) const {
    if (!hasIndex(row, column, parent)) return QModelIndex();

    auto* obj = objByIndex(parent);
    if (row < 0 || row >= obj->childCount()) return QModelIndex();  // Проверка bounds
    return createIndex(row, column, obj->children().at(row));
}

QModelIndex FileTreeModel::parent(const QModelIndex &child) const {
    if (!child.isValid()) return QModelIndex();

    auto* childObj = static_cast<FileItem*>(child.internalPointer());
    auto* parentObj = childObj->parent();

    if (parentObj == m_rootItem || parentObj == nullptr) return QModelIndex();

    auto* grandparentObj = parentObj->parent();
    int row = grandparentObj->children().indexOf(parentObj);
    if (row == -1) return QModelIndex();  // Если не найден
    return createIndex(row, 0, parentObj);
}

QVariant FileTreeModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || role != Qt::DisplayRole) return QVariant{};
    BaseItem* obj = static_cast<BaseItem*>(index.internalPointer());
    return obj->getValue(index.column());
}

bool FileTreeModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    if (!index.isValid() || role != Qt::EditRole) return false;
    if (role == Qt::EditRole) {

        BaseItem* obj = static_cast<BaseItem*>(index.internalPointer());
        FileItem* fileObj = static_cast<FileItem*>(obj);
        auto data = fileObj->getData();


        switch (static_cast<FileFields>(index.column())) {
        case FileFields::STATUS: {
            emit statusChanged(data.id, value.toBool());
            break;
        }
        case FileFields::PRIORITY: {
            emit priorityChanged(data.id, value.toInt());
            break;
        }
        // ...
        default: {
            throw std::runtime_error("Something is wrong");
            break;
        }
        }
        return obj->setValue(index.column(), value);
    }
    return false;
}

QVariant FileTreeModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole) return QVariant{};

    switch (static_cast<FileFields>(section)) {
    case FileFields::STATUS: return "Status";
    case FileFields::FILENAME: return "Filename";
    case FileFields::PROGRESS: return "Progress";
    case FileFields::DOWNLOADED: return "Downloaded";
    case FileFields::FILESIZE: return "Filesize";
    case FileFields::PRIORITY: return "Priority";
    default: return QVariant{};
    }
}

void FileTreeModel::traverseRecursively(QString curDir, BaseItem *root, const QList<File> &hFiles, const QModelIndex &parentIndex) {
    if (root != m_rootItem) { // root item has dirName set, but it must not be used
        curDir += root->getPath() + QDir::separator(); // get dir name
    }

    auto fileChildren = root->children();
    auto fileChildrenSize = fileChildren.size();
    for (auto i = 0; i < fileChildrenSize; ++i) {
        auto* fileChild = fileChildren[i];
        if (fileChild->isDir()) {
            QModelIndex childDirIndex = index(i, 0, parentIndex);
            traverseRecursively(curDir, fileChild, hFiles, childDirIndex);
        } else {
            auto fileRelativePath = fileChild->getPath();
            QString fullFilePath = curDir + fileRelativePath;

            // auto hFilesIter = hFiles.find(fullFilePath);
            auto hFilesIter = std::find_if(hFiles.begin(), hFiles.end(), [&fullFilePath](const File& file) {
                return file.filename == fullFilePath;
            });
            if (hFilesIter == hFiles.end()) {
                clearFiles();
                setFiles(hFiles);
                return; // FIXME TODO: Fix this shit
                // throw std::runtime_error("What the fuck happened in file tree model");
            }

            static_cast<FileItem*>(fileChild)->setData(*hFilesIter); // Updage file data
            fileChild->setPath(fileRelativePath); // We need to set it to relative path again, since currently filename is an abs. path

            int row = fileChild->parent()->children().indexOf(fileChild); // Which row it is parent-wise
            QModelIndex topLeft = index(row, 0, parentIndex);
            QModelIndex bottomRight = index(row, FILE_FIELD_COUNT - 1, parentIndex);
            emit dataChanged(topLeft, bottomRight);
        }
    }
}

Qt::ItemFlags FileTreeModel::flags(const QModelIndex &index) const
{
    if (index.column() == static_cast<int>(FileFields::PRIORITY)) {
        return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
    }
    return QAbstractItemModel::flags(index);
}

void FileTreeModel::setFiles(const QList<File> &files) { // Инвариант в том, что новые файлы не добавляются и не удаляются, обновляются только статусы и приоритеты у файлов, если торрент не изменен
    bool bRootVirgin = isRootVirgin();
    if (!bRootVirgin) {
        // FIXME TODO: find a better solution

        traverseRecursively(QString{}, m_rootItem, files, QModelIndex{});
        return;
    }

    // This only happens when a torrent is changed
    auto createOrReturnDir = [](BaseItem* root, const QString& v) -> BaseItem* {
        auto rootItemChildren = root->children();
        auto iter = std::find_if(rootItemChildren.begin(), rootItemChildren.end(), [&v](const auto* child) {
            return child->getValue(static_cast<int>(FileFields::FILENAME)) == v;
        });
        if (iter == rootItemChildren.end()) {
            BaseItem* dir = new DirItem();
            dir->setPath(v); // set dir data

            root->addChild(dir);
            return dir; // This returns a newly allocated object, which we're supposed to take care of, but it is a child/grandchild or whatever of m_rootItem, so it will be deleted
        } else {
            return *iter;
        }
    };

    beginResetModel();
    BaseItem* rootItem = new DirItem();
    rootItem->setPath("Root");

    for (const auto& entry : files) {
        auto filename = entry.filename;
        auto paths = filename.split(QDir::separator());

        if (paths.length() == 1) {
            // Create file and set its data
            BaseItem* file = new FileItem();
            static_cast<FileItem*>(file)->setData(entry);
            // Add this file to the root object
            rootItem->addChild(file);
            continue;
        }

        BaseItem* currentFile = createOrReturnDir(rootItem, paths.first());
        for (auto i = 1; i < paths.length() - 1; ++i) { // -1 because file is different
            currentFile = createOrReturnDir(currentFile, paths[i]);
        }

        // no need to check files are unique
        BaseItem* file = new FileItem();
        static_cast<FileItem*>(file)->setData(entry);
        file->setPath(paths.last()); // Change file name to relative path

        currentFile->addChild(file);
    }

    delete m_rootItem;
    m_rootItem = rootItem;

    endResetModel();
}
