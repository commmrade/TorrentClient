#include "filestablemodel.h"
#include "ui_filestablemodel.h"
#include "utils.h"

FileTableModel::FileTableModel(QObject *parent) : QAbstractTableModel(parent)
{
}

Qt::ItemFlags FileTableModel::flags(const QModelIndex &index) const
{
    if (index.column() == static_cast<int>(FileFields::PRIORITY)) {
        return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
    }
    return QAbstractItemModel::flags(index);
}

QVariant FileTableModel::data(const QModelIndex &index, int role) const
{
    if (role < Qt::UserRole) {
        if (role == Qt::DisplayRole) {
            auto& file =  m_files[index.row()];
            switch (static_cast<FileFields>(index.column())) {
                case FileFields::STATUS: {
                    return {file.isEnabled}; // TODO: center it somehow
                }
                case FileFields::FILENAME: {
                    return {file.filename};
                }
                case FileFields::PROGRESS: {
                    auto ratio = static_cast<double>(file.downloaded) / file.filesize;
                    ratio *= 100.0; // ration was 0.0 to 1.0, scale it
                    ratio = std::round(ratio * 100.0) / 100.0;
                    return {ratio};
                }
                case FileFields::DOWNLOADED: {
                    return {utils::bytesToHigher(file.downloaded)};
                    // return {static_cast<qreal>(file.downloaded)};
                }
                case FileFields::FILESIZE: {
                    return {utils::bytesToHigher(file.filesize)};
                    // return {static_cast<qreal>(file.filesize)};
                }
                case FileFields::PRIORITY: {
                    // TODO: display it nicely
                    auto priorityToString = [](int const priority) -> QString {
                        /* From libtorrent source code
                        constexpr download_priority_t dont_download{0};

                        // The default priority for files and pieces.
                        constexpr download_priority_t default_priority{4};

                        // The lowest priority for files and pieces.
                        constexpr download_priority_t low_priority{1};

                        // The highest priority for files and pieces.
                        constexpr download_priority_t top_priority{7};
                            */
                        switch (priority) {
                            case 0: {
                                return {"Don't download"};
                            }
                            case 4: {
                                return {"Default"};
                            }
                            case 1: {
                                return {"Low"};
                            }
                            case 7: {
                                return {"High"};
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
    }
    return {};
}

bool FileTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role == Qt::EditRole) {
        auto& file = m_files[index.row()];
        switch (static_cast<FileFields>(index.column())) {
        case FileFields::STATUS: {
            file.isEnabled = value.toBool();
            emit statusChanged(file.id, file.isEnabled);
            // emit dataChanged(index, index);
            break;
        }
        case FileFields::PRIORITY: {
            qDebug() << "Priority set data";
            file.priority = value.toInt();
            emit priorityChanged(file.id, file.priority);
            break;
        }
        // ...
        default: {
            throw std::runtime_error("Something is wrong");
            break;
        }
        }
        return true;
    }
    return false;
}

QVariant FileTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole) {
        return {};
    }

    if (orientation == Qt::Horizontal) {
        switch (static_cast<FileFields>(section)) {
            case FileFields::STATUS: {
                return {"Status"};
            }
            case FileFields::FILENAME: {
                return {"Name"};
            }
            case FileFields::PROGRESS: {
                return {"Progress"};
            }
            case FileFields::DOWNLOADED: {
                return {"Downloaded"};
            }
            case FileFields::FILESIZE: {
                return {"Size"};
            }
            case FileFields::PRIORITY: {
                return {"Priority"};
            }
            default: {
                throw std::runtime_error("TF u doing here?");
            }
        }
    } else {
        return {section + 1};
    }
}

QHash<int, QByteArray> FileTableModel::roleNames() const
{
    return {};
}

void FileTableModel::setFiles(const QList<File> &files)
{
    // TODO: Optimize it, since it is suitable for torrents list, but i guess i can do it easier for (almost) static files
    QHash<int, int> newTrackersMap; // key - index in trackers
    QHash<int, int> oldTrackersMap; // key - index in m_trackers

    // Cache trackers
    for (int i = 0; i < files.size(); ++i) {
        newTrackersMap.insert(files[i].id, i);
    }
    for (int i = 0; i < m_files.size(); ++i) {
        oldTrackersMap.insert(m_files[i].id, i);
    }

    for (int i = 0; i < files.size(); ++i) {
        auto const newKey = files[i].id;
        if (oldTrackersMap.contains(newKey)) {
            // Tracker exists, check for updates
            int row = oldTrackersMap[newKey];
            auto& oldFile = m_files[row];

            // Change properties
            // if (oldFile.downloaded != files[i].downloaded) {
            //     oldFile.isEnabled = files[i].isEnabled;
            // }
            oldFile.filename = files[i].filename; // Since files can be renamed
            oldFile.downloaded = files[i].downloaded;
            oldFile.priority = files[i].priority;
        } else {
            int row = m_files.size();
            beginInsertRows(QModelIndex(), row, row);
            m_files.append(files[i]);
            endInsertRows();
        }
    }

    // Handle deletions
    for (int i = m_files.size() - 1; i >= 0; --i) {
        auto const key = m_files[i].id;
        if (!newTrackersMap.contains(key)) {
            beginRemoveRows(QModelIndex(), i, i);
            m_files.removeAt(i);
            endRemoveRows();
        }
    }

    if (!m_files.isEmpty()) {
        emit dataChanged(index(0, 1), index(m_files.size() - 1, columnCount() - 1));
    }
}

void FileTableModel::clearFiles()
{
    beginResetModel();
    m_files.clear();
    endResetModel();
}
