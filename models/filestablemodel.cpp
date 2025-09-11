#include "filestablemodel.h"
#include "ui_filestablemodel.h"
#include "utils.h"

FileTableModel::FileTableModel(QObject *parent) : QAbstractTableModel(parent)
{
}

QVariant FileTableModel::data(const QModelIndex &index, int role) const
{
    if (role < Qt::UserRole) {
        if (role == Qt::DisplayRole) {
            auto& file =  m_files[index.row()];
            switch (static_cast<FileFields>(index.column())) {
                case FileFields::STATUS: {
                    return {file.isEnabled}; // TODO!!!
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
                    // TODO: Handle this
                    return {file.priority};
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
            // TODO: Change priority to 'dont download' in session manager somehow
            emit dataChanged(index, index);
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
    // TODO: Optimize, no point in resetting it every tick
    clearFiles();
    m_files = files;
}

void FileTableModel::clearFiles()
{
    beginResetModel();
    m_files.clear();
    endResetModel();
}
