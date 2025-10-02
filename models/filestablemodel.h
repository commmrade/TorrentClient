#ifndef FILESTABLEMODEL_H
#define FILESTABLEMODEL_H

#include <QWidget>
#include <QAbstractTableModel>
#include "file.h"
#include <libtorrent/download_priority.hpp>
#include "priority.h"
#include "utils.h"
#include <QDir>

class BaseItem {
public:
    BaseItem() = default;
    virtual ~BaseItem() = default;

    virtual bool isDir() const {
        return true;
    }
    virtual void setPath(const QString& newPath) {
    }
    virtual QString getPath() const {
        return {};
    }
    virtual QVariant getValue(int column) const {
        qDebug() << "Default get value";
        return {};
    }
    virtual bool setValue(int column, const QVariant& value) {
        return false;
    }


    void addChild(BaseItem *file) {
        m_children.append(file);
        file->setParent(this);
    }
    QList<BaseItem*> children() const {
        return m_children;
    }
    void removeChild(BaseItem* child) {
        m_children.remove(m_children.indexOf(child));
    }
    int childCount() const {
        return m_children.size();
    }

    void setParent(BaseItem* newParent) {
        if (m_parent && m_parent != newParent) {
            m_parent->removeChild(this);
        }
        m_parent = newParent;
    }
    BaseItem* parent() const {
        return m_parent;
    }


private:
    BaseItem* m_parent{nullptr};
    QList<BaseItem*> m_children;
};

class FileItem : public BaseItem {
    File m_fileData;
public:
    FileItem() = default;
    ~FileItem() override = default;

    bool isDir() const override {
        return false;
    }
    void setPath(const QString& newPath) override {
        m_fileData.filename = newPath;
    }
    QString getPath() const override {
        return m_fileData.filename;
    }

    void setData(const File& data) {
        m_fileData = data;
    }
    File getData() const {
        return m_fileData;
    }

    QVariant getValue(int column) const override {
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

    bool setValue(int column, const QVariant& value) override {
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
};

class DirItem : public BaseItem {
private:
    QString m_path;
public:
    DirItem() = default;
    void setPath(const QString& newPath) override {
        m_path = newPath;
    }
    QString getPath() const override {
        return m_path;
    }

    QVariant getValue(int column) const override {
        switch (static_cast<FileFields>(column)) {
        case FileFields::FILENAME: {
            return m_path;
        }
        default: {
            return QVariant{};
        }
        }
    }
};



class FileTreeModel : public QAbstractItemModel {
    Q_OBJECT

    BaseItem* objByIndex(const QModelIndex& index) const {
        if (!index.isValid()) return m_rootItem;
        return static_cast<FileItem*>(index.internalPointer());
    }

    void resetRoot() {
        delete m_rootItem;

        m_rootItem = nullptr;
        m_rootItem = new DirItem();
        m_rootItem->setPath("Root");
        // m_rootItem->setDirName("Root");
    }

    bool isRootVirgin() const {
        return m_rootItem == nullptr || (m_rootItem && m_rootItem->childCount() == 0);
        // return m_rootItem->childCount() == 0;
    }
public:
    explicit FileTreeModel(QObject* parent = nullptr) : QAbstractItemModel(parent) {
        resetRoot();
    }

    ~FileTreeModel() { delete m_rootItem; }

    QModelIndex index(int row, int column, const QModelIndex &parent) const override {
        if (!hasIndex(row, column, parent)) return QModelIndex();

        auto* obj = objByIndex(parent);
        if (row < 0 || row >= obj->childCount()) return QModelIndex();  // Проверка bounds
        return createIndex(row, column, obj->children().at(row));
    }

    QModelIndex parent(const QModelIndex &child) const override {
        if (!child.isValid()) return QModelIndex();

        auto* childObj = static_cast<FileItem*>(child.internalPointer());
        auto* parentObj = childObj->parent();

        if (parentObj == m_rootItem || parentObj == nullptr) return QModelIndex();

        auto* grandparentObj = parentObj->parent();
        int row = grandparentObj->children().indexOf(parentObj);
        if (row == -1) return QModelIndex();  // Если не найден
        return createIndex(row, 0, parentObj);
    }

    int rowCount(const QModelIndex &parent) const override {
        return objByIndex(parent)->childCount();
    }

    int columnCount(const QModelIndex &parent) const override {
        Q_UNUSED(parent);
        return FILE_FIELD_COUNT;
    }

    QVariant data(const QModelIndex &index, int role) const override {
        if (!index.isValid() || role != Qt::DisplayRole) return QVariant{};
        BaseItem* obj = static_cast<BaseItem*>(index.internalPointer());
        return obj->getValue(index.column());
    }

    bool setData(const QModelIndex &index, const QVariant &value, int role) override {
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



    QVariant headerData(int section, Qt::Orientation orientation, int role) const override {
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

    void clearFiles() {
        qDebug() << "Cleared files";
        beginResetModel();
        resetRoot();
        endResetModel();
    }

    void traverseRecursively(QString curDir, BaseItem* root, const QList<File>& hFiles, const QModelIndex& parentIndex) {
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
                for (const auto& file : hFiles) {
                    qDebug() << "HFiles file:" << file.filename << "Looking for:" << fullFilePath;
                }
                if (hFilesIter == hFiles.end()) {
                    throw std::runtime_error("What the fuck happened in file tree model");
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

    Qt::ItemFlags flags(const QModelIndex &index) const override
    {
        if (index.column() == static_cast<int>(FileFields::PRIORITY)) {
            return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
        }
        return QAbstractItemModel::flags(index);
    }


    void setFiles(const QList<File>& files) { // Инвариант в том, что новые файлы не добавляются и не удаляются, обновляются только статусы и приоритеты у файлов, если торрент не изменен
        bool bRootVirgin = isRootVirgin();
        if (!bRootVirgin) {
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
signals:
    void statusChanged(int index, bool value);
    void priorityChanged(int index, int priority);
private:
    BaseItem* m_rootItem{nullptr};
};



#endif // FILESTABLEMODEL_H
