#include "filetreemodel.h"
#include "utils.h"
#include <libtorrent/download_priority.hpp>
#include "priority.h"

void BaseItem::addChild(BaseItem *file)
{
    m_children.append(file);
    file->setParent(this);
}

void BaseItem::setParent(BaseItem *newParent)
{
    if (m_parent && m_parent != newParent)
    {
        m_parent->removeChild(this);
    }
    m_parent = newParent;
}

QVariant FileItem::getValue(int column) const
{
    switch (static_cast<FileFields>(column))
    {
    case FileFields::STATUS:
    {
        return {m_fileData.isEnabled};
    }
    case FileFields::FILENAME:
    {
        return {m_fileData.filename};
    }
    case FileFields::PROGRESS:
    {
        auto ratio = static_cast<double>(m_fileData.downloaded) / m_fileData.filesize;
        ratio      = std::round((ratio * 100.0) * 100.0) / 100.0;
        return {ratio};
    }
    case FileFields::DOWNLOADED:
    {
        return {utils::bytesToHigher(m_fileData.downloaded)};
    }
    case FileFields::FILESIZE:
    {
        return {utils::bytesToHigher(m_fileData.filesize)};
    }
    case FileFields::PRIORITY:
    {
        auto priorityToString = [](int const priority) -> QString
        {
            switch (priority)
            {
            case lt::dont_download:
            {
                return {Priorities::DONT_DOWNLOAD};
            }
            case lt::default_priority:
            {
                return {Priorities::DEFAULT};
            }
            case lt::low_priority:
            {
                return {Priorities::LOW};
            }
            case lt::top_priority:
            {
                return {Priorities::HIGH};
            }
            default:
            {
                throw std::runtime_error("This type of priority does not exist");
            }
            }
        };
        return {priorityToString(m_fileData.priority)};
    }
    case FileFields::ID: {
        return {m_fileData.id};
    }
    default:
    {
        throw std::runtime_error("Out of bounds for columns");
    }
    }
}

bool FileItem::setValue(int column, const QVariant &value)
{
    switch (static_cast<FileFields>(column))
    {
    case FileFields::STATUS:
    {
        m_fileData.isEnabled = value.toBool();
        return true;
    }
    case FileFields::PRIORITY:
    {
        m_fileData.priority = value.toInt();
        return true;
    }
    // ...
    default:
    {
        return false;
    }
    }
}

QVariant DirItem::getValue(int column) const
{
    switch (static_cast<FileFields>(column))
    {
    case FileFields::FILENAME:
    {
        return m_path;
    }
    default:
    {
        return QVariant{};
    }
    }
}

bool DirItem::setValue(int column, const QVariant &value)
{
    switch (static_cast<FileFields>(column))
    {
    case FileFields::FILENAME:
    {
        m_path = value.toString();
        return true;
    }
    // ...
    default:
    {
        return false;
    }
    }
}

BaseItem *FileTreeModel::objByIndex(const QModelIndex &index) const
{
    if (!index.isValid())
        return m_rootItem;
    return static_cast<FileItem *>(index.internalPointer());
}

void FileTreeModel::resetRoot()
{
    delete m_rootItem;

    m_rootItem = new DirItem();
    m_rootItem->setPath("Root");
}

FileTreeModel::FileTreeModel(QObject *parent) : QAbstractItemModel(parent) { resetRoot(); }

QModelIndex FileTreeModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();
    auto *obj = objByIndex(parent);
    if (row < 0 || row >= obj->childCount())
        return QModelIndex(); // Проверка bounds
    return createIndex(row, column, obj->children().at(row));
}

QModelIndex FileTreeModel::parent(const QModelIndex &child) const
{
    if (!child.isValid())
        return QModelIndex();

    auto *childObj  = static_cast<FileItem *>(child.internalPointer());
    auto *parentObj = childObj->parent();

    if (parentObj == m_rootItem || parentObj == nullptr)
        return QModelIndex();

    auto *grandparentObj = parentObj->parent();
    int   row            = grandparentObj->children().indexOf(parentObj);
    if (row == -1)
        return QModelIndex(); // Если не найден
    return createIndex(row, 0, parentObj);
}

QVariant FileTreeModel::data(const QModelIndex &index, int role) const
{

    if (!index.isValid() || (role != Qt::DisplayRole && role <= Qt::UserRole)) {

        return QVariant{};
    }

    BaseItem* obj = static_cast<BaseItem*>(index.internalPointer());
    if (role == Qt::UserRole + 1) { // TODO: Get rid of magic this
        return {obj->getValue(role)};
    }
    return {obj->getValue(index.column())};
}

bool FileTreeModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid() || role != Qt::EditRole)
        return false;
    if (role == Qt::EditRole)
    {
        BaseItem *obj     = static_cast<BaseItem *>(index.internalPointer());
        FileItem *fileObj = static_cast<FileItem *>(obj);
        auto      data    = fileObj->getData();

        switch (static_cast<FileFields>(index.column()))
        { // TODO: Может сделать коннект с fileObj на statusChanged и priorityChanged и передавать в
          // this::statusChanged, на данный момент это костыль
        case FileFields::STATUS:
        {
            emit statusChanged(data.id, value.toBool());
            break;
        }
        case FileFields::PRIORITY:
        {
            emit priorityChanged(data.id, value.toInt());
            break;
        }
        // ...
        default:
        {
            break;
        }
        }
        return obj->setValue(index.column(), value);
    }
    return false;
}

QVariant FileTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole)
        return QVariant{};

    switch (static_cast<FileFields>(section))
    {
    case FileFields::STATUS:
        return "Status";
    case FileFields::FILENAME:
        return "Filename";
    case FileFields::PROGRESS:
        return "Progress";
    case FileFields::DOWNLOADED:
        return "Downloaded";
    case FileFields::FILESIZE:
        return "Filesize";
    case FileFields::PRIORITY:
        return "Priority";
    default:
        return QVariant{};
    }
}

void FileTreeModel::traverseRecursively(QString curDir, BaseItem *root, const QList<File> &hFiles,
                                        const QModelIndex &parentIndex)
{
    if (root != m_rootItem)
    { // root item has dirName set, but it must not be used
        curDir += root->getPath() + QDir::separator(); // get dir name
    }

    auto fileChildren     = root->children();
    auto fileChildrenSize = fileChildren.size();
    for (auto i = 0; i < fileChildrenSize; ++i)
    {
        auto *fileChild = fileChildren[i];
        if (fileChild->isDir())
        {
            QModelIndex childDirIndex = index(i, 0, parentIndex);
            traverseRecursively(curDir, fileChild, hFiles, childDirIndex);
        }
        else
        {
            auto    fileRelativePath = fileChild->getPath();
            QString fullFilePath     = curDir + fileRelativePath;

            // auto hFilesIter = hFiles.find(fullFilePath);
            auto hFilesIter =
                std::find_if(hFiles.begin(), hFiles.end(), [&fullFilePath](const File &file)
                             { return file.filename == fullFilePath; });
            if (hFilesIter == hFiles.end())
            { // The problem is that it throws when the current torrent is changed, therefore new
              // set of files is passed, which does not contain old files
                // To fix it, i need to prematurily check if newly passed set of files is different
                // from what is in the tree. To do this, i might recursively collect all files in a
                // tree but that is some zoomer level logic
                //
                // clearFiles();
                // setFiles(hFiles);
                // return; // FIXME TODO: Fix this shit
                throw std::runtime_error("What the fuck happened in file tree model");
            }

            static_cast<FileItem *>(fileChild)->setData(*hFilesIter); // Updage file data
            fileChild->setPath(fileRelativePath); // We need to set it to relative path again, since
                                                  // currently filename is an abs. path

            int row =
                fileChild->parent()->children().indexOf(fileChild); // Which row it is parent-wise
            QModelIndex topLeft     = index(row, 0, parentIndex);
            QModelIndex bottomRight = index(row, FILE_FIELD_COUNT - 1, parentIndex);
            emit        dataChanged(topLeft, bottomRight);
        }
    }
}

Qt::ItemFlags FileTreeModel::flags(const QModelIndex &index) const
{
    if (index.column() == static_cast<int>(FileFields::PRIORITY))
    {
        return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
    }
    return QAbstractItemModel::flags(index);
}

QString FileTreeModel::getFirstLeafPath() const
{
    BaseItem *current = m_rootItem;
    QString   path;
    while (current && current->childCount() > 0)
    {
        current = current->children().at(0);
        path += current->getPath() + QDir::separator();
    }
    if (current && !current->isDir())
    {
        return path.left(path.size() - 1); // Убрать последний separator
    }
    return QString();
}

void FileTreeModel::setFiles(const QList<File> &files)
{ // Инвариант в том, что новые файлы не добавляются и не удаляются, обновляются только статусы и
  // приоритеты у файлов, если торрент не изменен
    bool bRootVirgin = isRootVirgin();

    QString path        = getFirstLeafPath();
    auto    shouldReset = (std::find_if(files.begin(), files.end(), [&](const File &file)
                                        { return file.filename == path; }) == files.end());
    if (!bRootVirgin && !shouldReset)
    {
        traverseRecursively(QString{}, m_rootItem, files, QModelIndex{});
        return;
    }

    // This only happens when the current torrent is changed
    auto createOrReturnDir = [](BaseItem *root, const QString &v) -> BaseItem *
    {
        auto rootItemChildren = root->children();
        auto iter =
            std::find_if(rootItemChildren.begin(), rootItemChildren.end(), [&v](const auto *child)
                         { return child->getValue(static_cast<int>(FileFields::FILENAME)) == v; });
        if (iter == rootItemChildren.end())
        {
            BaseItem *dir = new DirItem();
            dir->setPath(v); // set dir data

            root->addChild(dir);
            return dir; // This returns a newly allocated object, which we're supposed to take care
                        // of, but it is a child/grandchild or whatever of m_rootItem, so it will be
                        // deleted
        }
        else
        {
            return *iter;
        }
    };

    beginResetModel();
    BaseItem *rootItem = new DirItem();
    rootItem->setPath("Root");

    for (const auto &entry : files)
    {
        auto filename = entry.filename;
        auto paths    = filename.split(QDir::separator());

        if (paths.length() == 1)
        {
            // Create file and set its data
            BaseItem *file = new FileItem();
            static_cast<FileItem *>(file)->setData(entry);
            // Add this file to the root object
            rootItem->addChild(file);
            continue;
        }

        BaseItem *currentFile = createOrReturnDir(rootItem, paths.first());
        for (auto i = 1; i < paths.length() - 1; ++i)
        { // -1 because file is different
            currentFile = createOrReturnDir(currentFile, paths[i]);
        }

        // no need to check files are unique
        BaseItem *file = new FileItem();
        static_cast<FileItem *>(file)->setData(entry);
        file->setPath(paths.last()); // Change file name to relative path

        currentFile->addChild(file);
    }

    delete m_rootItem;
    m_rootItem = rootItem;

    endResetModel();
}
